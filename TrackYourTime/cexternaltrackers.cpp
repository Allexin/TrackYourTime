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

unsigned char simpleCheckSum(QByteArray& data, int size){
    unsigned char checkSum = 0;
    for (int i = 0; i<size; i++)
        checkSum += data[i];
    return checkSum;
}

QString readUtf8(QDataStream& stream){
    int size;
    stream.readRawData((char*)&size,sizeof(int));
    if (size==0)
        return "";
    char* utf8 = new char[size];
    stream.readRawData(utf8,size);
    QString value = QString::fromUtf8(utf8,size);
    delete [] utf8;
    return value;
}

const char* EXTERNAL_TRACKER_FORMAT_PREFIX = "TYTET";
const int EXTERNAL_TRACKER_FORMAT_PREFIX_SIZE = 5;
const int EXTERNAL_TRACKER_FORMAT_VERSION = 1;

cExternalTrackers::cExternalTrackers(QObject *parent) : QObject(parent)
{
    m_Server.bind(QHostAddress::LocalHost, EXTERNAL_TRACKERS_UDP_PORT);
    connect(&m_Server, SIGNAL(readyRead()), this, SLOT(readyRead()));
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

const sAppFileName cExternalTrackers::getExternalTrackerState(const sAppFileName &appName)
{
    for (int i = 0; i<m_Pairs.size(); i++){
        if (m_Pairs[i].HostAppFileName==appName.fileName){
            sAppFileName TrackerName;
            TrackerName.fileName = m_Pairs[i].ClientState;
            TrackerName.path = appName.path+"/"+appName.fileName;
            return TrackerName;
        }
    }

    return appName;
}

void cExternalTrackers::readyRead()
{
    QByteArray buffer;
    buffer.resize(m_Server.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_Server.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    unsigned char checkSum = buffer[buffer.size()-1];
    if (checkSum==simpleCheckSum(buffer,buffer.size()-1)){
        QDataStream stream(buffer);
        char prefix[EXTERNAL_TRACKER_FORMAT_PREFIX_SIZE+1]; //add zero for simple convert to string
        prefix[EXTERNAL_TRACKER_FORMAT_PREFIX_SIZE] = 0;
        stream.readRawData(prefix,EXTERNAL_TRACKER_FORMAT_PREFIX_SIZE);

        if (memcmp(prefix,EXTERNAL_TRACKER_FORMAT_PREFIX,EXTERNAL_TRACKER_FORMAT_PREFIX_SIZE)==0){
            int Version;
            stream.readRawData((char*)&Version,sizeof(int));
            if (Version==EXTERNAL_TRACKER_FORMAT_VERSION){
                QString CurrentState = readUtf8(stream);

                int AppCount;
                stream.readRawData((char*)&AppCount,sizeof(int));
                for (int i = 0; i<AppCount; i++){
                    QString AppName = readUtf8(stream);
                    addPair(AppName,CurrentState);
                }
            }
            else
                qWarning() << "Error reading external tracker. Incorrect format version " << Version << " only " << EXTERNAL_TRACKER_FORMAT_VERSION << " supported";
        }
        else
            qWarning() << "Error reading external tracker. Incorrect format prefix " << prefix;

    }
    else
        qWarning() << "incorrect external tracker checksum";
}

