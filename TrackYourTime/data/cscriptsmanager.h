#ifndef CSCRIPTSMANAGER_H
#define CSCRIPTSMANAGER_H

#include <QObject>
#include <QScriptEngine>
#include "../tools/os_api.h"

class cScriptsManager : public QScriptEngine
{
    Q_OBJECT
public:
    explicit cScriptsManager(QObject *parent = 0);

    QString getAppInfo(const sSysInfo &info, QString script);
    QString evalute(const sSysInfo& info, QString script);

    QString processCustomScript(const sSysInfo &info, QString script, QString prevStepResult);
    QString evaluteCustomScript(const sSysInfo& info, QString script, QString prevStepResult);
signals:

public slots:
};

#endif // CSCRIPTSMANAGER_H
