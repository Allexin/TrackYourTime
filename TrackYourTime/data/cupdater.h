#ifndef CUPDATER_H
#define CUPDATER_H

#include <QObject>

class cUpdater : public QObject
{
    Q_OBJECT
public:
    explicit cUpdater(QObject *parent = 0);

signals:
    void newVersionAvailable();
public slots:
    void checkUpdates();
    void ignoreNewVersion();
};

#endif // CUPDATER_H
