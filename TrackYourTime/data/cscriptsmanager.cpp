/*
 * TrackYourTime - cross-platform time tracker
 * Copyright (C) 2015-2017  Alexander Basov <basovav@gmail.com>
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
#include "cscriptsmanager.h"
#include <QDebug>


/* Qt Script realisation
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
*/


cScriptsManager::cScriptsManager(QObject *parent) : QJSEngine(parent)
{

}

QString cScriptsManager::getAppInfo(const sSysInfo &info,QString script)
{
    QJSValue result = evalute(info, script);
    if (result.isError()){
        qCritical() << "script execution for app " << info.fileName << "failed with exception: " << result.toString();
        return "";
    }
    if (result.toString()=="undefined"){
        qCritical() << "script execution for app " << info.fileName << "failed. no result returned";
        return "";
    }
    return result.toString().trimmed();
}

QJSValue cScriptsManager::evalute(const sSysInfo &info, QString script)
{
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


    QJSValue fun = evaluate(script);
    if (fun.isError())
        return fun;
    QJSValueList args;
    args << info.fileName << info.title << OS;
    QJSValue result = globalObject().property("parseTitle").call(args);
    return result;
}

QString cScriptsManager::processCustomScript(const sSysInfo &info,QString script, QString prevStepResult)
{
    QJSValue result = evaluteCustomScript(info, script,prevStepResult);
    if (result.isError()){
        qCritical() << "script execution for app " << info.fileName << "failed with exception: " << result.toString();
        return "";
    }
    if (result.toString()=="undefined"){
        qCritical() << "script execution for app " << info.fileName << "failed. no result returned";
        return "";
    }
    return result.toString().trimmed();
}

QJSValue cScriptsManager::evaluteCustomScript(const sSysInfo &info, QString script, QString prevStepResult)
{
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


    QJSValue fun = evaluate(script);
    if (fun.isError())
        return fun;
    QJSValueList args;
    args << info.fileName << info.title << prevStepResult << OS;
    QJSValue result = globalObject().property("parseData").call(args);

    return result;
}
