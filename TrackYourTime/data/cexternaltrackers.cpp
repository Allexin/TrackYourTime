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

#include <QMessageLogger>
#include <QDebug>
#include "cexternaltrackers.h"

const QString EXTERNAL_TRACKER_PREFIX = "TYTET";
const QString EXTERNAL_TRACKER_FORMAT_VERSION = "1";

cExternalTrackers::cExternalTrackers(QObject *parent) : QObject(parent),m_HTTPServer(EXTERNAL_TRACKERS_HTTP_PORT)
{
    m_Server.bind(QHostAddress::LocalHost, EXTERNAL_TRACKERS_UDP_PORT);
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

void cExternalTrackers::readyRead()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    qInfo() << "external tracker info";
#endif
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
    QStringList list = data.split(';');
    QMap<QString,QString> pairs;
    for (int i = 0; i<list.size(); i++){
        QStringList pair = list[i].split('=');
        if (pair.size()==2){
            pairs[pair[0]]=pair[1];
        }
    }

    if (pairs["PREFIX"].compare(EXTERNAL_TRACKER_PREFIX)!=0){
        qWarning() << "unknown exterinal tracker with PREFIX=" << pairs["PREFIX"];
        return;
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

    bool correctNumber;
    int appCount=pairs["APP_COUNT"].toInt(&correctNumber);
    if (!correctNumber){
        qWarning() << "unknown exterinal tracker with APP_COUNT=" << pairs["VERSION"];
        return;
    }

    for (int i = 0; i<appCount; i++){
         QString app = pairs["APP_"+QString().setNum(i+1)];
        if (app.isEmpty()){
            qWarning() << "app " << i+1 << " not present";
            return;
        }
        addPair(app,state);
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
    int dataPos = data.indexOf("[");
    if (dataPos>-1){
        data = data.mid(dataPos+1);
        dataPos = data.indexOf("]");
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
