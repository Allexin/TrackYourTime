#ifndef CUPDATER_H
#define CUPDATER_H

#include <QObject>
#include <QTcpSocket>

class cUpdater : public QObject
{
    Q_OBJECT
protected:
    QTcpSocket          m_Socket;
    QString             m_Data;
    QString             m_AvailableVersion;
public:
    explicit cUpdater(QObject *parent = 0);

signals:
    void newVersionAvailable(QString version);
public slots:
    void checkUpdates();
    void ignoreNewVersion();
private slots:
    void processError(QAbstractSocket::SocketError error);
    void processDisconnected();
    void processReadyRead();
};

#endif // CUPDATER_H
