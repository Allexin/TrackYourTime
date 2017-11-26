/*
 * TrackYourTime - cross-platform time tracker
 * Copyright (C) 2015-2016  Alexander Basov <basovav@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "statisticwindow.h"
#include "ui_statisticwindow.h"
#include <QFileDialog>
#include <QPainter>
#include <QDate>

QString DurationToString(quint32 durationSeconds)
{
  QString res;
  int seconds = (int) (durationSeconds % 60);
  durationSeconds /= 60;
  int minutes = (int) (durationSeconds % 60);
  durationSeconds /= 60;
  int hours = (int) (durationSeconds % 24);
  int days = (int) (durationSeconds / 24);
  if((hours == 0)&&(days == 0))
      return res.sprintf("00:%02d:%02d", minutes, seconds);
  if (days == 0)
      return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
  return res.sprintf("%dd%02d:%02d:%02d", days, hours, minutes, seconds);
}

QString fixSize(const QString& value, int minSize)
{
    QString result = value;
    while (result.size()<minSize)
        result="0"+result;
    return result;
}

bool lessThan( const sStatisticItem & e1, const sStatisticItem & e2 )
{
    return e1.TotalTime>e2.TotalTime;
}

void StatisticWindow::rebuild(QDate from, QDate to)
{
    QDateTime statStart(from);
    QDateTime statEnd(to.addDays(1));

    //prepare containers
    m_Uncategorized.TotalTime = 0;
    m_Uncategorized.NormalValue = 0;
    m_Uncategorized.Name = tr("Uncategorized");
    m_Uncategorized.Color = Qt::gray;
    m_Categories.resize(m_DataManager->categoriesCount());
    for (int i = 0; i<m_Categories.size(); i++){
        m_Categories[i].TotalTime = 0;
        m_Categories[i].NormalValue = 0;
        m_Categories[i].Name = m_DataManager->categories(i)->name;
        m_Categories[i].Color = m_DataManager->categories(i)->color;
    }

    m_Applications.resize(m_DataManager->applicationsCount());
    for (int i = 0; i<m_Applications.size(); i++){
        sAppInfo* app = m_DataManager->applications(i);
        m_Applications[i].TotalTime = 0;
        m_Applications[i].NormalValue = 0;
        m_Applications[i].Name = app->activities[0].name;
        m_Applications[i].Color = Qt::white;
        m_Applications[i].childs.resize(app->activities.size());
        for (int j = 0; j<m_Applications[i].childs.size(); j++){
            m_Applications[i].childs[j].TotalTime = 0;
            m_Applications[i].childs[j].NormalValue = 0;
            m_Applications[i].childs[j].Name = app->activities[j].name;
            m_Applications[i].childs[j].Color = Qt::white;
        }
    }

    //calculation
    m_TotalTime = 0;
    for (int i = 0; i<m_Applications.size(); i++){
        const sAppInfo* app = m_DataManager->applications(i);
        for (int activity = 0; activity<app->activities.size(); activity++){
            const sActivityInfo* ainfo = &app->activities[activity];
            for (int j = ainfo->periods.size()-1; j>=0; --j){
                QDateTime start = ainfo->periods[j].start;
                QDateTime end = ainfo->periods[j].start.addSecs(ainfo->periods[j].length);
                if (end>statStart && start<statEnd){
                    if (start<statStart)
                        start = statStart;
                    if (end>statEnd)
                        end = statEnd;
                    int duration = start.secsTo(end);
                    m_TotalTime+=duration;
                    m_Applications[i].TotalTime+=duration;
                    m_Applications[i].childs[activity].TotalTime+=duration;
                    int cat = ainfo->categories[ainfo->periods[j].profileIndex].category;
                    if (cat==-1)
                        m_Uncategorized.TotalTime+=duration;
                    else
                        m_Categories[cat].TotalTime+=duration;
                }
                if (end<statStart)
                    break;
            }
        }
    }

    calcNormalizedValues();

    qSort( m_Categories.begin(), m_Categories.end(), lessThan );

    ui->widgetDiagram->setTotalTime(m_TotalTime);
    ui->widgetDiagram->update();


    //fill applications widget
    ui->treeWidgetApplications->setSortingEnabled(false);
    ui->treeWidgetApplications->clear();
    for (int i = 0; i<m_Applications.size(); i++){
        if (m_Applications[i].TotalTime>0){
            m_Applications[i].item = new QTreeWidgetItem();
            m_Applications[i].item->setText(0,m_Applications[i].Name);
            m_Applications[i].item->setData(0,Qt::UserRole,i);
            m_Applications[i].item->setText(1,DurationToString(m_Applications[i].TotalTime));
            m_Applications[i].item->setText(2,fixSize(QString::number(m_Applications[i].NormalValue*100,'f',2),5)+"%");
            ui->treeWidgetApplications->addTopLevelItem(m_Applications[i].item);
            for (int j = 0; j<m_Applications[i].childs.size(); j++){
                if (m_Applications[i].childs[j].TotalTime>0){
                    m_Applications[i].childs[j].item = new QTreeWidgetItem();
                    QString childName = m_Applications[i].childs[j].Name;
                    if (j==0)
                        childName+=tr("(default)");
                    m_Applications[i].childs[j].item->setText(0,childName);
                    m_Applications[i].childs[j].item->setData(0,Qt::UserRole,i);
                    m_Applications[i].childs[j].item->setData(0,Qt::UserRole+1,j);
                    m_Applications[i].childs[j].item->setText(1,DurationToString(m_Applications[i].childs[j].TotalTime));
                    m_Applications[i].childs[j].item->setText(2,fixSize(QString::number(m_Applications[i].childs[j].NormalValue*100,'f',2),5)+"%");
                    m_Applications[i].item->addChild(m_Applications[i].childs[j].item);
                }
            }

        }
    }
    ui->treeWidgetApplications->setSortingEnabled(true);
    m_FastUpdateAvailable = true;
}

void StatisticWindow::calcNormalizedValues()
{
    //calculate normalized values
    if (m_TotalTime>0){
        m_Uncategorized.NormalValue = (float)m_Uncategorized.TotalTime/m_TotalTime;
        for (int i = 0; i<m_Categories.size(); i++)
            m_Categories[i].NormalValue = (float)m_Categories[i].TotalTime/m_TotalTime;

        for (int i = 0; i<m_Applications.size(); i++){
            m_Applications[i].NormalValue = (float)m_Applications[i].TotalTime/m_TotalTime;
            for (int j = 0; j<m_Applications[i].childs.size(); j++)
                m_Applications[i].childs[j].NormalValue = (float)m_Applications[i].childs[j].TotalTime/m_TotalTime;
        }
    }
}

void StatisticWindow::saveToCSV(const QVector<sStatisticItem*> &items,  const QString& FileName)
{
    QFile outputFile(FileName);
    outputFile.open(QIODevice::WriteOnly);

    if(!outputFile.isOpen()){
        qDebug() << "Error: unable to open" << FileName << "for output";
        return;
    }

    QTextStream outStream(&outputFile);

    for (int i = 0; i<items.size(); i++)
        outStream << items[i]->Name << "," << items[i]->TotalTime  << "," << DurationToString(items[i]->TotalTime) << "," << QString::number(items[i]->NormalValue*100,'f',2) << "\r\n";

    outputFile.close();
}

void StatisticWindow::onExportApplicationsCSVPress()
{
    QString FileName = QFileDialog::getSaveFileName(0,tr("Select applications file"),"applications.csv","Comma Separated Values(*.csv)");
    if (!FileName.isEmpty()){
        QVector<sStatisticItem*> items;
        items.resize(m_Applications.size());
        for (int i = 0; i<m_Applications.size(); i++)
            items[i] = &m_Applications[i];

        saveToCSV(items,FileName);
    }
}

void StatisticWindow::showAndUpdate()
{
    showNormal();
    if (ui->dateEditTo->date()==QDateTime::currentDateTime().date())
        onUpdatePress();
}

void StatisticWindow::fastUpdate(int application, int activity, int secondsCount, bool fullUpdate)
{
    if (!isVisible())
        return;
    if (fullUpdate){
        onUpdatePress();
        return;
    }
    if (!m_FastUpdateAvailable)
        return;
    if (ui->dateEditTo->date()!=QDateTime::currentDateTime().date())
        return;

    if (m_Categories.size()!=m_DataManager->categoriesCount() || m_Applications.size()!=m_DataManager->applicationsCount()){
        onUpdatePress();
        return;
    }

    if (application>=m_Applications.size())
        return;
    if (activity>=m_Applications[application].childs.size())
        return;

    m_TotalTime+=secondsCount;
    m_Applications[application].TotalTime+=secondsCount;
    m_Applications[application].childs[activity].TotalTime+=secondsCount;
    calcNormalizedValues();

    m_Applications[application].item->setText(1,DurationToString(m_Applications[application].TotalTime));
    m_Applications[application].childs[activity].item->setText(1,DurationToString(m_Applications[application].childs[activity].TotalTime));

    bool needGlobalInvalidate = fixSize(QString::number(m_Applications[application].NormalValue*100,'f',2),5)+"%"!=m_Applications[application].item->text(2) ||
            fixSize(QString::number(m_Applications[application].childs[activity].NormalValue*100,'f',2),5)+"%"!=m_Applications[application].childs[activity].item->text(2);
    if (needGlobalInvalidate){
        for (int i = 0; i<m_Applications.size(); i++){
            if (m_Applications[i].TotalTime>0){
                m_Applications[i].item->setText(2,fixSize(QString::number(m_Applications[i].NormalValue*100,'f',2),5)+"%");
                for (int j = 0; j<m_Applications[i].childs.size(); j++){
                    if (m_Applications[i].childs[j].TotalTime>0){

                        m_Applications[i].childs[j].item->setText(2,fixSize(QString::number(m_Applications[i].childs[j].NormalValue*100,'f',2),5)+"%");
                    }
                }

            }
        }
    }

    ui->widgetDiagram->setTotalTime(m_TotalTime);
    ui->widgetDiagram->update();
}

void StatisticWindow::onExportCategoriesCSVPress()
{
    QString FileName = QFileDialog::getSaveFileName(0,tr("Select categories file"),"categories.csv","Comma Separated Values(*.csv)");
    if (!FileName.isEmpty()){
        QVector<sStatisticItem*> items;
        items.resize(m_Categories.size()+1);
        for (int i = 0; i<m_Categories.size(); i++)
            items[i] = &m_Categories[i];
        items.last() = &m_Uncategorized;

        saveToCSV(items,FileName);
    }
}

StatisticWindow::StatisticWindow(cDataManager *DataManager) :
    QMainWindow(0),    
    m_FastUpdateAvailable(false),
    ui(new Ui::StatisticWindow)
{
    ui->setupUi(this);

    ui->dateEditFrom->setDate(QDate::currentDate());
    ui->dateEditTo->setDate(QDate::currentDate());

    ui->treeWidgetApplications->setColumnCount(3);
    ui->treeWidgetApplications->sortByColumn(0);
    ui->treeWidgetApplications->sortByColumn(1);
    ui->treeWidgetApplications->sortByColumn(2);

    m_DataManager = DataManager;

    ui->widgetDiagram->setCategories(&m_Categories,&m_Uncategorized);

    connect(ui->pushButtonSetPeriod, SIGNAL(released()), this, SLOT(onSetPeriodPress()));
    connect(ui->comboBoxPeriod,SIGNAL(currentIndexChanged(int)),this, SLOT(onSetPeriodPress()));
    connect(ui->pushButtonUpdate, SIGNAL(released()), this, SLOT(onUpdatePress()));
    connect(ui->pushButtonExportApplicationsCSV, SIGNAL(released()), this, SLOT(onExportApplicationsCSVPress()));
    connect(ui->pushButtonExportCategoriesCSV, SIGNAL(released()), this, SLOT(onExportCategoriesCSVPress()));

    ui->pushButtonExportApplicationsCSV->setEnabled(false);
    ui->pushButtonExportCategoriesCSV->setEnabled(false);
}

StatisticWindow::~StatisticWindow()
{
    delete ui;
}

void StatisticWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    raise();
    activateWindow();
}

enum ePeriod{
    P_TODAY = 0,
    P_WEEK,
    P_MONTH,
    P_YEAR
};

void StatisticWindow::onSetPeriodPress()
{
    switch(ui->comboBoxPeriod->currentIndex()){
        case P_TODAY:{
        ui->dateEditFrom->setDate(QDate::currentDate());
        }
        break;
        case P_WEEK:{
            QDate date = QDate::currentDate();
            ui->dateEditFrom->setDate(date.addDays(-date.dayOfWeek()+1));
        }
        break;
        case P_MONTH:{
            QDate date = QDate::currentDate();
            ui->dateEditFrom->setDate(date.addDays(-date.day()+1));
        }
        break;
        case P_YEAR:{
            QDate date = QDate::currentDate();
            ui->dateEditFrom->setDate(date.addDays(-date.dayOfYear()+1));
        }
        break;
    }
    ui->dateEditTo->setDate(QDate::currentDate());

    onUpdatePress();
}

void StatisticWindow::onUpdatePress()
{
    rebuild(ui->dateEditFrom->date(),ui->dateEditTo->date());
    ui->pushButtonExportApplicationsCSV->setEnabled(true);
    ui->pushButtonExportCategoriesCSV->setEnabled(true);
}







void cStatisticDiagramWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter( this );
    painter.setPen( QPen( Qt::black, 1 ) );

    if (m_TotalTime==0 || m_Categories==NULL || m_Uncategorized==NULL){
        painter.drawText(0,0,this->width(),this->height(),Qt::AlignCenter | Qt::TextSingleLine,tr("NO DATA"));
    }
    else{
        float secondsInPixel = (float)m_TotalTime/height();
        int miscTotal = 0;
        int shift = 0;
        if (m_Uncategorized->TotalTime>0 && m_Uncategorized->TotalTime<secondsInPixel)
            miscTotal=m_Uncategorized->TotalTime;
        for (int i = 0; i<m_Categories->size(); i++){
            const sStatisticItem& item = m_Categories->at(i);
            if (item.TotalTime>0){
                if ((float)item.TotalTime/secondsInPixel<2){
                  miscTotal+=item.TotalTime;
                }
                else{
                    int catHeight = (float)item.TotalTime/secondsInPixel;
                    painter.setBrush(QBrush(item.Color));
                    QRectF r(0,shift,width()-1,catHeight);
                    painter.drawRect(r);
                    if (catHeight>20){
                        painter.drawText(r,Qt::AlignCenter | Qt::TextSingleLine,item.Name+"["+DurationToString(item.TotalTime)+"]["+QString::number(item.NormalValue*100,'f',2)+"%]");
                    }
                    shift+=catHeight;
                }
            }
        }

        if (m_Uncategorized->TotalTime>0 && m_Uncategorized->TotalTime>=secondsInPixel){
            int catHeight = (float)m_Uncategorized->TotalTime/secondsInPixel;
            painter.setBrush(QBrush(m_Uncategorized->Color));
            QRectF r(0,shift,width()-1,catHeight);
            painter.drawRect(r);
            if (catHeight>20){
                painter.drawText(r,Qt::AlignCenter | Qt::TextSingleLine,m_Uncategorized->Name+"["+DurationToString(m_Uncategorized->TotalTime)+"]["+QString::number(m_Uncategorized->NormalValue*100,'f',2)+"%]");
            }
            shift+=catHeight;
        }

        int catHeight = height()-1-miscTotal;
        if (catHeight>1){
            painter.setBrush(QBrush(Qt::white));
            QRectF r(0,shift,width()-1,catHeight);
            painter.drawRect(r);
            if (catHeight>20){
                painter.drawText(r,Qt::AlignCenter | Qt::TextSingleLine,tr("Other categories")+"["+DurationToString(miscTotal)+"]");
            }
        }

    }

}
