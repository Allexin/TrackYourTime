/*
 * TrackYourTime - cross-platform time tracker
 * Copyright (C) 2015-2017  Alexander Basov <basovav@gmail.com>
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
#include "schedulewindow.h"
#include "ui_schedulewindow.h"

ScheduleWindow::ScheduleWindow(cDataManager *dataManager, cSchedule *schedule) :
    QMainWindow(0),
    ui(new Ui::ScheduleWindow)
{
    m_DataManager = dataManager;
    m_Schedule = schedule;
    ui->setupUi(this);

    connect(ui->pushButtonAdd,SIGNAL(released()),this,SLOT(add()));
    connect(ui->pushButtonDelete,SIGNAL(released()),this,SLOT(remove()));
}

ScheduleWindow::~ScheduleWindow()
{
    delete ui;
}

void ScheduleWindow::rebuild()
{
    ui->comboBoxAction->clear();
    for (int i = 0; i<cScheduleItem::SA_COUNT; i++)
        ui->comboBoxAction->addItem(cScheduleItem::getActionName((cScheduleItem::eScheduleAction)i),i);
    ui->comboBoxAction->setCurrentIndex(0);

    ui->comboBoxProfiles->clear();
    for (int i = 0; i<m_DataManager->profilesCount(); i++)
        ui->comboBoxProfiles->addItem(m_DataManager->profiles(i)->name,i);
    ui->comboBoxProfiles->setCurrentIndex(0);
    ui->lineEditCondition->setText(QDateTime::currentDateTime().toString(".*HH:mm"));

    ui->treeWidgetSchedule->clear();
    for (int i = 0; i<m_Schedule->getItemsCount(); i++){
        const cScheduleItem* action = m_Schedule->getItem(i);

        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0,cScheduleItem::getActionName(action->action()));
        item->setData(0,Qt::UserRole,i);
        if (action->action()==cScheduleItem::SA_SET_PROFILE){
            const sProfile* profile = m_DataManager->profiles(action->param().toInt());
            if (profile)
                item->setText(1,profile->name);
        }

        item->setText(2,action->condition());
        ui->treeWidgetSchedule->addTopLevelItem(item);
    }
}

void ScheduleWindow::updateTimeSample()
{
    if (!isVisible())
        return;

    ui->labelCurrentDateTime->setText(m_Schedule->getSample());
    QTimer::singleShot(25000,this,SLOT(updateTimeSample()));
}

void ScheduleWindow::add()
{
    m_Schedule->addItem((cScheduleItem::eScheduleAction)ui->comboBoxAction->currentIndex(),QString().setNum(ui->comboBoxProfiles->currentIndex()),ui->lineEditCondition->text());
    rebuild();
}

void ScheduleWindow::remove()
{
    if (ui->treeWidgetSchedule->selectedItems().size()==0)
        return;
    m_Schedule->deleteItem(ui->treeWidgetSchedule->selectedItems()[0]->data(0,Qt::UserRole).toInt());
    rebuild();
}

void ScheduleWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent( event );
    rebuild();
    updateTimeSample();

    raise();
    activateWindow();
}
