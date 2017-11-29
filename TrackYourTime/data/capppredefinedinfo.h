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
#ifndef CAPPPREDEFINEDINFO_H
#define CAPPPREDEFINEDINFO_H

#include <QObject>
#include <QString>
#include "cdatamanager.h"

class cAppPredefinedInfo : public QObject
{
    Q_OBJECT
protected:
    sAppInfo::eTrackerType        m_TrackerType;
    QString             m_Script;
    QString             m_Info;
public:
    explicit cAppPredefinedInfo(const QString& appName);

    sAppInfo::eTrackerType trackerType(){return m_TrackerType;}
    QString script(){return m_Script;}
    QString info(){return m_Info;}
signals:

public slots:
};

#endif // CAPPPREDEFINEDINFO_H
