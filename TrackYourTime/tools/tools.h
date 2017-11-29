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
#ifndef TOOLS_H
#define TOOLS_H

#include <QMap>
#include <QString>
#include <QSettings>

extern const QString CURRENT_VERSION;

class cStatisticResolver{
public:
    virtual int getTodayTotalTime() = 0;
    virtual int getTodayApplicationTime(int application) = 0;
    virtual int getTodayActivityTime(int application, int activity) = 0;
    virtual int getTodayCategoryTime(int category) = 0;
    virtual bool isTodayStatisticAvailable() = 0;
};

QString DurationToString(quint32 durationSeconds);
QMap<QString,QString> loadPairsFile(const QString& fileName);
QString readFile(const QString& fileName);

class cSettings{
protected:
    QSettings*          m_Settings;
public:
    cSettings();
    ~cSettings();

    QSettings* db();
};

#endif // TOOLS_H
