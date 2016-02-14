#include "capppredefinedinfo.h"

#include <QFileInfo>
#include "../tools/tools.h"

cAppPredefinedInfo::cAppPredefinedInfo(const QString &appName) : QObject(0)
{
    QString fileName = "data/app_predef/"+appName+".predef";
    QFileInfo predef(fileName);
    if (predef.exists()){
        QMap<QString,QString> values = loadPairsFile(fileName);
        QString tt = values["TYPE"];
        m_TrackerType = sAppInfo::eTrackerType::TT_EXECUTABLE_DETECTOR;
        if (tt=="EXTERNAL_TRACKER")
            m_TrackerType = sAppInfo::eTrackerType::TT_EXTERNAL_DETECTOR;
        else
        if (tt=="PREDEFINED_SCRIPT")
            m_TrackerType = sAppInfo::eTrackerType::TT_PREDEFINED_SCRIPT;
        m_Info = values["INFO"];
    }
    else{
        m_TrackerType = sAppInfo::eTrackerType::TT_EXECUTABLE_DETECTOR;
        m_Info = "";
    }

    QString scriptFileName = "data/app_predef/scripts/"+appName+".script";
    QFileInfo script(scriptFileName);
    if (script.exists()){
        m_Script = readFile(scriptFileName);
    }
    else{
        m_Script = "";
    }
}

