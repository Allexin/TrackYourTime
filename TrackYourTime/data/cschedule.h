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
#ifndef CSCHEDULE_H
#define CSCHEDULE_H

#include <QObject>
#include <QRegExp>
#include <QTimer>
#include <QString>
#include "cdatamanager.h"

class cScheduleItem: public QObject{
    Q_OBJECT

public:
    enum eScheduleAction{
        SA_SET_PROFILE = 0,
        SA_CHECK_UPDATE,
        SA_MAKE_BACKUP,
        SA_COUNT
    };

protected:
    eScheduleAction     m_Action;
    QString             m_Param;
    QRegExp             m_RegExp;
public:
    explicit cScheduleItem(eScheduleAction action, const QString& param, QString regexp);
    void process(const QString& currentDateTime, cDataManager* dataManager);

    eScheduleAction action() const{return m_Action;}
    QString param() const{return m_Param;}
    QString condition() const{return m_RegExp.pattern();}
    static QString getActionName(eScheduleAction action);
signals:
    void checkUpdates();
};

class cSchedule : public QObject
{
    Q_OBJECT
protected:
    QVector<cScheduleItem*> m_Items;
    QString             m_PreviousDateTime;
    QTimer              m_Timer;
    cDataManager*       m_DataManager;
    QString getCurrentDateTime();
    void save();
    void load();
public:
    explicit cSchedule(cDataManager* dataManager, QObject *parent = 0);
    virtual ~cSchedule();

    void start();

    int getItemsCount();
    const cScheduleItem* getItem(int index);
    void deleteItem(int index);
    void addItem(cScheduleItem::eScheduleAction action, const QString& param, const QString& regexp);

    QString getSample(){return getCurrentDateTime();}
signals:
    void checkUpdates();
protected slots:
    void timer();
    void onCheckUpdateAction();
};

#endif // CSCHEDULE_H
