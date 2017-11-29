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
#include "tools.h"

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

const QString CURRENT_VERSION = "0.9.2.5";

QString DurationToString(quint32 durationSeconds)
{
  QString res;
  int seconds = (int) (durationSeconds % 60);
  durationSeconds /= 60;
  int minutes = (int) (durationSeconds % 60);
  durationSeconds /= 60;
  int hours = (int) (durationSeconds % 24);
  int days = (int) (durationSeconds / 24);
  if((hours == 0)&&(days == 0))
      return res.sprintf("00:%02d:%02d", minutes, seconds);
  if (days == 0)
      return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
  return res.sprintf("%dd%02d:%02d:%02d", days, hours, minutes, seconds);
}

QMap<QString,QString> loadPairsFile(const QString& fileName){
    QMap<QString,QString> list;
    QFile textFile(fileName);
    if (textFile.open(QIODevice::ReadOnly)){
        QTextStream textStream(&textFile);
        while (true)
        {
            QString line = textStream.readLine();
            QStringList pair = line.split("==");
            if (pair.size()==2)
                list[pair[0]]=pair[1];

            if (line.isNull())
                break;
        }
    }
    return list;
}

QString readFile(const QString &fileName)
{
    QFile textFile(fileName);
    if (textFile.open(QIODevice::ReadOnly)){
        return textFile.readAll();
    }
    return QString();
}

cSettings::cSettings()
{
    QString OSName = "UNKNOWN";
#ifdef Q_OS_LINUX
    OSName="Linux";
#else
    #ifdef Q_OS_WIN
        OSName="Windows";
    #else
        #ifdef Q_OS_MAC
            OSName="MacOSX";
        #endif
    #endif
#endif
    if (QFileInfo("settings_"+OSName+".conf").exists())
        m_Settings = new QSettings("settings_"+OSName+".conf",QSettings::IniFormat);
    else
    if (QFileInfo("settings.conf").exists())
        m_Settings = new QSettings("settings.conf",QSettings::IniFormat);
    else
        m_Settings = new QSettings();
}

cSettings::~cSettings()
{
    delete m_Settings;
}

QSettings *cSettings::db()
{
    return m_Settings;
}
