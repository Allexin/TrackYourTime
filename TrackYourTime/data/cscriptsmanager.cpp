#include "cscriptsmanager.h"
#include <QDebug>

cScriptsManager::cScriptsManager(QObject *parent) : QScriptEngine(parent)
{

}

QString cScriptsManager::getAppInfo(const sSysInfo &info,QString script)
{
    QString result = evalute(info, script);
    if (hasUncaughtException()){
        qCritical() << "script execution for app " << info.fileName << "failed with exception: " << result;
        return "";
    }
    if (result=="undefined"){
        qCritical() << "script execution for app " << info.fileName << "failed. no result returned";
        return "";
    }
    return result;
}

QString cScriptsManager::evalute(const sSysInfo &info, QString script)
{
    QScriptValue global = globalObject();
    global.setProperty("application",info.fileName);
    global.setProperty("title",info.title);

    QString OS = "UNKNOWN";
#ifdef Q_OS_LINUX
    OS = "LINUX";
#else
    #ifdef Q_OS_WIN32
        OS = "WINDOWS";
    #else
        #ifdef Q_OS_MAC
            OS = "MAC_OS_X";
        #endif
    #endif
#endif
    global.setProperty("OS",OS);

    QScriptValue result = evaluate(script+"\nparseTitle(application,title,OS)");
    return result.toString().trimmed();
}

QString cScriptsManager::processCustomScript(const sSysInfo &info,QString script, QString prevStepResult)
{
    QString result = evaluteCustomScript(info, script,prevStepResult);
    if (hasUncaughtException()){
        qCritical() << "script execution for app " << info.fileName << "failed with exception: " << result;
        return "";
    }
    if (result=="undefined"){
        qCritical() << "script execution for app " << info.fileName << "failed. no result returned";
        return "";
    }
    return result;
}

QString cScriptsManager::evaluteCustomScript(const sSysInfo &info, QString script, QString prevStepResult)
{
    QScriptValue global = globalObject();
    global.setProperty("application",info.fileName);
    global.setProperty("title",info.title);
    global.setProperty("prevStepResult",prevStepResult);

    QString OS = "UNKNOWN";
#ifdef Q_OS_LINUX
    OS = "LINUX";
#else
    #ifdef Q_OS_WIN32
        OS = "WINDOWS";
    #else
        #ifdef Q_OS_MAC
            OS = "MAC_OS_X";
        #endif
    #endif
#endif
    global.setProperty("OS",OS);

    QScriptValue result = evaluate(script+"\nparseData(application,title,prevStepResult,OS)");
    return result.toString().trimmed();
}

