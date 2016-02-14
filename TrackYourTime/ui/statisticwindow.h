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

#ifndef STATISTICWINDOW_H
#define STATISTICWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QColor>
#include <QPaintEvent>
#include "../data/cdatamanager.h"

namespace Ui {
class StatisticWindow;
}

struct sStatisticItem{
    QString Name;
    QColor Color;
    int TotalTime;
    float NormalValue;
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

class StatisticWindow : public QMainWindow
{
    Q_OBJECT
protected:
    cDataManager*       m_DataManager;

    sStatisticItem          m_Uncategorized;
    QVector<sStatisticItem> m_Categories;
    QVector<sStatisticItem> m_Applications;
    void rebuild(QDate from, QDate to);
    void saveToCSV(const QVector<sStatisticItem*> &items,  const QString& FileName);
public:
    explicit StatisticWindow(cDataManager* DataManager);
    ~StatisticWindow();

private:
    Ui::StatisticWindow *ui;    
public slots:
    void onSetTodayPress();
    void onUpdatePress();
    void onExportCategoriesCSVPress();
    void onExportApplicationsCSVPress();
};

#endif // STATISTICWINDOW_H
