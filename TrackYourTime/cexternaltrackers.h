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

#ifndef CEXTERNALTRACKERS_H
#define CEXTERNALTRACKERS_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include "os_api.h"

struct sExternalTrackerPair{
    QString HostAppFileName;
    QString ClientState;
    int LifeTime;
};

class cExternalTrackers : public QObject
{
    Q_OBJECT
public:
    static const int    EXTERNAL_TRACKERS_UDP_PORT = 25855;
    static const int    EXTERNAL_TRACKERS_PAIR_LIFE_TIME_SECOND = 5;
protected:
    QUdpSocket          m_Server;
    QVector<sExternalTrackerPair> m_Pairs;
    void addPair(const QString& AppName, const QString& CurrentState);
public:
    explicit cExternalTrackers(QObject *parent = 0);

    void update();

    const sAppFileName getExternalTrackerState(const sAppFileName& appName);

signals:

public slots:
    void readyRead();
};

#endif // CEXTERNALTRACKERS_H
