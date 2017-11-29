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

#ifndef STATISTICWINDOW_H
#define STATISTICWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QColor>
#include <QPaintEvent>
#include <QTreeWidgetItem>
#include "../data/cdatamanager.h"
#include "../tools/tools.h"

namespace Ui {
class StatisticWindow;
}

struct sStatisticItem{
    QString Name;
    QColor Color;
    int TotalTime;
    float NormalValue;
    QTreeWidgetItem* item;
    QVector<sStatisticItem> childs;
};

class cStatisticDiagramWidget: public QWidget
{
    Q_OBJECT
protected:    
    sStatisticItem*          m_Uncategorized;
    QVector<sStatisticItem>* m_Categories;
    int                      m_TotalTime;
public:
    cStatisticDiagramWidget(QWidget* parent = 0):QWidget(parent),m_Uncategorized(0),m_Categories(0),m_TotalTime(0){}
    void setCategories(QVector<sStatisticItem>* Categories, sStatisticItem* Uncategorized){m_Categories = Categories; m_Uncategorized = Uncategorized;}
    void setTotalTime(int TotalTime){m_TotalTime = TotalTime;}
private:
    virtual void paintEvent(QPaintEvent *event) override;
};

class StatisticWindow : public QMainWindow, public cStatisticResolver
{
    Q_OBJECT
protected:
    bool                    m_FastUpdateAvailable;
    int                     m_TotalTime;
    cDataManager*           m_DataManager;

    sStatisticItem          m_Uncategorized;
    QVector<sStatisticItem> m_Categories;
    QVector<sStatisticItem> m_Applications;
    void rebuild(QDate from, QDate to);
    void calcNormalizedValues();
    void saveToCSV(const QVector<sStatisticItem*> &items,  const QString& FileName);
public:
    virtual int getTodayTotalTime() override;
    virtual int getTodayApplicationTime(int application) override;
    virtual int getTodayActivityTime(int application, int activity) override;
    virtual int getTodayCategoryTime(int category) override;
    virtual bool isTodayStatisticAvailable() override;
public:
    explicit StatisticWindow(cDataManager* DataManager);
    ~StatisticWindow();
protected:
    virtual void showEvent(QShowEvent * event) override;
private:
    Ui::StatisticWindow *ui;    
public slots:
    void onSetPeriodPress();
    void onUpdatePress();
    void onExportCategoriesCSVPress();
    void onExportApplicationsCSVPress();

    void showAndUpdate();

    void fastUpdate(int application, int activity, int category, int secondsCount, bool fullUpdate);
};

#endif // STATISTICWINDOW_H
