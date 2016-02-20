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

#include "cexternaltrackers.h"
#include <QDebug>

const QString EXTERNAL_TRACKER_PREFIX = "TYTET";
const QString OVERRIDE_TRACKER_PREFIX = "TYTOT";
const QString EXTERNAL_TRACKER_FORMAT_VERSION = "1";

cExternalTrackers::cExternalTrackers(QObject *parent) : QObject(parent),m_HTTPServer(EXTERNAL_TRACKERS_HTTP_PORT)
{
    m_Server.bind(QHostAddress::Any, EXTERNAL_TRACKERS_UDP_PORT);
    connect(&m_Server, SIGNAL(readyRead()), this, SLOT(readyRead()));

    connect(&m_HTTPServer,SIGNAL(dataReady(QString)), this, SLOT(onDataReady(QString)));
}

void cExternalTrackers::addPair(const QString& AppName, const QString& CurrentState)
{
    for (int i = 0; i<m_Pairs.size(); i++){
        if (m_Pairs[i].HostAppFileName==AppName){
            m_Pairs[i].ClientState = CurrentState;
            m_Pairs[i].LifeTime = EXTERNAL_TRACKERS_PAIR_LIFE_TIME_SECOND;
            return;
        }
    }

    sExternalTrackerPair pair;
    pair.HostAppFileName = AppName;
    pair.ClientState = CurrentState;
    pair.LifeTime = EXTERNAL_TRACKERS_PAIR_LIFE_TIME_SECOND;
    m_Pairs.push_back(pair);
}

void cExternalTrackers::addOverride(const QString &AppName, const QString &CurrentState, int idleTime)
{
    for (int i = 0; i<m_Override.size(); i++){
        if (m_Override[i].AppFileName==AppName){
            m_Override[i].State = CurrentState;
            m_Override[i].LifeTime = OVERRIDE_TRACKERS_PAIR_LIFE_TIME_SECOND;
            m_Override[i].IdleTime = idleTime;
            return;
        }
    }

    sOverrideTrackerInfo pair;
    pair.AppFileName = AppName;
    pair.State = CurrentState;
    pair.LifeTime = OVERRIDE_TRACKERS_PAIR_LIFE_TIME_SECOND;
    pair.IdleTime = idleTime;
    m_Override.push_back(pair);
}

void cExternalTrackers::update()
{
    int i = 0;
    while (i<m_Pairs.size()){
        m_Pairs[i].LifeTime--;
        if (m_Pairs[i].LifeTime<=0){
            m_Pairs[i] = m_Pairs.last();
            m_Pairs.pop_back();
        }
        else
            i++;
    }

    i = 0;
    while (i<m_Override.size()){
        m_Override[i].LifeTime--;
        if (m_Override[i].LifeTime<=0){
            m_Override[i] = m_Override.last();
            m_Override.pop_back();
        }
        else
            i++;
    }
}

bool cExternalTrackers::getExternalTrackerState(const QString &appName, QString& outValue)
{
    for (int i = 0; i<m_Pairs.size(); i++){
        if (m_Pairs[i].HostAppFileName==appName){
            outValue = m_Pairs[i].ClientState;
            return true;
        }
    }

    return false;
}

sOverrideTrackerInfo *cExternalTrackers::getOverrideTracker()
{
    if (m_Override.size()==0)
        return NULL;
    int min = m_Override[0].IdleTime;
    sOverrideTrackerInfo* tracker = &m_Override[0];
    for (int i = 1; i<m_Override.size(); i++)
        if (m_Override[i].IdleTime<min){
            min = m_Override[i].IdleTime;
            tracker = &m_Override[i];
        }
    return tracker;
}

void cExternalTrackers::sendOverrideTracker(const QString &AppName, const QString &CurrentState, int idleTime, const QString &host)
{
    QString data;
    data += "PREFIX="+OVERRIDE_TRACKER_PREFIX;
    data += "&VERSION=1";
    data += "&APP_FILENAME="+AppName;
    data += "&STATE="+CurrentState;
    data += "&USER_INACTIVE_TIME="+QString::number(idleTime);
    data += "&USER_NAME="+getUserName();
    m_Client.writeDatagram(data.toUtf8(),QHostAddress(host),EXTERNAL_TRACKERS_UDP_PORT);
}

void cExternalTrackers::readyRead()
{
    QByteArray buffer;
    buffer.resize(m_Server.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_Server.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    QString Data(buffer);
    onDataReady(Data);
}

void cExternalTrackers::onDataReady(QString data)
{
    QStringList list = data.split('&');
    QMap<QString,QString> pairs;
    for (int i = 0; i<list.size(); i++){
        QStringList pair = list[i].split('=');
        if (pair.size()==2){
            pairs[pair[0]]=pair[1];
        }
    }

    if (pairs["VERSION"].compare(EXTERNAL_TRACKER_FORMAT_VERSION)!=0){
        qWarning() << "unknown exterinal tracker with VERSION=" << pairs["VERSION"];
        return;
    }

    QString state = pairs["STATE"].trimmed();
    if (state.isEmpty()){
        qWarning() << "external tracker state is empty";
        return;
    }

    if (pairs["PREFIX"].compare(EXTERNAL_TRACKER_PREFIX)==0){
        int i = 1;
        while (true){
            QString key = "APP_"+QString().setNum(i);
            if (pairs.contains(key)){
                addPair(pairs[key],state);
            }
            else
                break;
            i++;
        }
    }
    else
    if (pairs["PREFIX"].compare(OVERRIDE_TRACKER_PREFIX)==0){
        if (!pairs.contains("APP_FILENAME")){
            qWarning() << "override tracker APP_FILENAME not defined";
            return;
        }
        if (!pairs.contains("USER_INACTIVE_TIME")){
            qWarning() << "override tracker USER_INACTIVE_TIME not defined";
            return;
        }

        addOverride(pairs["APP_FILENAME"],pairs["STATE"],pairs["USER_INACTIVE_TIME"].toInt());
    }
    else{
        qWarning() << "unknown exterinal tracker with PREFIX=" << pairs["PREFIX"];
    }


}


cHTTPTrackerServer::cHTTPTrackerServer(int port)
{
    if (!listen(QHostAddress::Any,port)){
        qCritical() << "http server start error: " << errorString();
    }
}

void cHTTPTrackerServer::incomingConnection(qintptr handle)
{
    QTcpSocket* socket = new QTcpSocket();
    socket->setSocketDescriptor(handle);

    connect(socket,SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket,SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

void cHTTPTrackerServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    QString data = socket->readAll();
    data = data.split("\r")[0];
    int dataPos = data.indexOf("?");
    if (dataPos>-1){
        data = data.mid(dataPos+1);
        dataPos = data.indexOf(" ");
        if (dataPos>-1){
            data = data.mid(0,dataPos);
            emit dataReady(data);
        }
    }

    QString response = "HTTP/1.1 200 OK";
    socket->write(response.toUtf8());
    socket->disconnectFromHost();
}

void cHTTPTrackerServer::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    socket->close();
    socket->deleteLater();
}
