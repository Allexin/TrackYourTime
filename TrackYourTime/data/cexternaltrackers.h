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
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include "../tools/os_api.h"

struct sExternalTrackerPair{
    QString HostAppFileName;
    QString ClientState;
    int LifeTime;    
};

struct sOverrideTrackerInfo{
    QString AppFileName;
    QString State;
    int IdleTime;
    int LifeTime;
};

class cHTTPTrackerServer: public QTcpServer
{
    Q_OBJECT
public:
    cHTTPTrackerServer(int port);

    virtual void incomingConnection(qintptr handle) override;
signals:
    void dataReady(QString data);
protected slots:
    void onReadyRead();
    void onDisconnected();
};

class cExternalTrackers : public QObject
{
    Q_OBJECT
public:
    static const int    EXTERNAL_TRACKERS_UDP_PORT = 25855;
    static const int    EXTERNAL_TRACKERS_HTTP_PORT = 25856;
    static const int    EXTERNAL_TRACKERS_PAIR_LIFE_TIME_SECOND = 5;
    static const int    OVERRIDE_TRACKERS_PAIR_LIFE_TIME_SECOND = 4;
protected:
    QUdpSocket          m_Client;

    QUdpSocket          m_Server;    
    cHTTPTrackerServer  m_HTTPServer;
    QVector<sOverrideTrackerInfo> m_Override;
    QVector<sExternalTrackerPair> m_Pairs;
    void addPair(const QString& AppName, const QString& CurrentState);
    void addOverride(const QString& AppName, const QString& CurrentState, int idleTime);
public:
    explicit cExternalTrackers(QObject *parent = 0);

    void update();

    bool getExternalTrackerState(const QString &appName, QString& outValue);
    sOverrideTrackerInfo* getOverrideTracker();

    void sendOverrideTracker(const QString& AppName, const QString& CurrentState, int idleTime, const QString& host);
signals:

public slots:
    void readyRead();
    void onDataReady(QString data);
};

#endif // CEXTERNALTRACKERS_H
