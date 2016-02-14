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
