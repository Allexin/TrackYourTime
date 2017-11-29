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

