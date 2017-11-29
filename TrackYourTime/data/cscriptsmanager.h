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
