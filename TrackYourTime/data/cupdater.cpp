#include "cupdater.h"
#include "../tools/tools.h"
#include "cdatamanager.h"

static const QString RELEASE_HOST = "develop.sol-online.org";
static const int RELEASE_PORT = 3000;
static const QString RELEASE_URL = "/tyt_version.txt";

cUpdater::cUpdater(QObject *parent) : QObject(parent)
{
    connect(&m_Socket, SIGNAL(disconnected()), this, SLOT(processDisconnected()));
    connect(&m_Socket, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
    connect(&m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
}

void cUpdater::checkUpdates()
{
    m_Data = "";
    QString GET;
    GET  = "GET "+RELEASE_URL+" HTTP/1.1\r\n";
    GET += "Host: "+RELEASE_HOST+"\r\n";
    GET += "User-Agent: Mozilla/4.0 (compatible; MSIE 5.0; Windows 98)\r\n";
    GET += "Accept: text/html\r\n";
    GET += "Connection: close\r\n";
    GET += "\r\n";

    m_Socket.connectToHost(RELEASE_HOST,RELEASE_PORT);
    if (!m_Socket.waitForConnected()){
        qCritical() << "cUpdater: connection to host failed" ;
        return;
    }

    m_Socket.write(GET.toUtf8());
}

void cUpdater::ignoreNewVersion()
{
    cSettings settings;
    settings.db()->setValue(cDataManager::CONF_LAST_AVAILABLE_VERSION_ID,m_AvailableVersion);
    settings.db()->sync();
}

void cUpdater::processError(QAbstractSocket::SocketError error)
{
    qCritical() << "cUpdater: error " << m_Socket.errorString();
}

void cUpdater::processDisconnected()
{
    if (m_Data.isEmpty()){
        qCritical() << "cUpdater: check failed. no response";
        return;
    }

    int pos = m_Data.indexOf("HTTP/1.1");
    if (pos==-1)
        pos = m_Data.indexOf("HTTP/1.0");
    if (pos==-1){
        qCritical() << "cUpdater: check failed. no response";
        return;
    }

    pos = m_Data.indexOf(" ",pos);
    int pos2 = m_Data.indexOf("\r",pos);
    QString Status = m_Data.mid(pos+1,pos2-(pos+1));
    if (Status!="200 OK"){
        qCritical() << "cUpdater: check failed. response:" << Status;
        return;
    }

    m_AvailableVersion = m_Data.split("\n").last();
    cSettings settings;
    QString lastAvailableVersion = settings.db()->value(cDataManager::CONF_LAST_AVAILABLE_VERSION_ID,CURRENT_VERSION).toString();
    if (m_AvailableVersion!=lastAvailableVersion)
        emit newVersionAvailable(m_AvailableVersion);
}


void cUpdater::processReadyRead()
{
    QString read = m_Socket.readAll().data();
    m_Data += read;

    m_Socket.disconnectFromHost();
}
