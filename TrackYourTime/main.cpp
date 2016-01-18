/*
 * TrackYourTime - cross-platform time tracker
 * Copyright (C) 2015-2016  Alexander Basov <basovav@gmail.com>
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

#include "settingswindow.h"
#include "statisticwindow.h"
#include "applicationswindow.h"
#include "profileswindow.h"
#include "aboutwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include <QTranslator>
#include <QSystemTrayIcon>
#include "cdatamanager.h"
#include "ctrayicon.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("SRFGames");
    QCoreApplication::setOrganizationDomain("sol-online.org"),
    QCoreApplication::setApplicationName("TrackYourTime");

#ifdef Q_OS_MAC
    QDir dir(argv[0]);
    dir.cdUp();
    QString currentDir = dir.absolutePath();
    dir.cdUp();
    dir.cd("PlugIns");
    QCoreApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

#ifdef Q_OS_MAC
    QDir::setCurrent(currentDir);
#endif



    QSettings settings;
    QString Language = QLocale::system().name();
    Language.truncate(Language.lastIndexOf('_'));
    Language = settings.value(cDataManager::CONF_LANGUAGE_ID,Language).toString();
    if (settings.value(cDataManager::CONF_FIRST_LAUNCH_ID,true).toBool()){
        settings.setValue(cDataManager::CONF_FIRST_LAUNCH_ID,false);
        settings.setValue(cDataManager::CONF_LANGUAGE_ID,Language);
        settings.setValue(cDataManager::CONF_AUTORUN_ID,true);
        setAutorun();
        settings.sync();
    }

    QTranslator translator;
    translator.load("lang_" + Language,QDir::currentPath()+"/data/languages");
    QApplication::installTranslator(&translator);

    cDataManager datamanager;

    cTrayIcon trIcon(&datamanager);
    QObject::connect(&datamanager, SIGNAL(trayActive()), &trIcon, SLOT(setActive()));
    QObject::connect(&datamanager, SIGNAL(traySleep()), &trIcon, SLOT(setInactive()));
    QObject::connect(&datamanager, SIGNAL(trayShowHint(QString)), &trIcon, SLOT(showHint(QString)));
    QObject::connect(&datamanager, SIGNAL(profilesChanged()), &trIcon, SLOT(onProfilesChange()));

    ApplicationsWindow applicationsWindow(&datamanager);
    QObject::connect(&trIcon, SIGNAL(showApplications()), &applicationsWindow, SLOT(show()));
    QObject::connect(&datamanager, SIGNAL(profilesChanged()), &applicationsWindow, SLOT(onProfilesChange()));
    QObject::connect(&datamanager, SIGNAL(applicationsChanged()), &applicationsWindow, SLOT(onApplicationsChange()));

    ProfilesWindow profilesWindow(&datamanager);
    QObject::connect(&applicationsWindow, SIGNAL(showProfiles()), &profilesWindow, SLOT(show()));

    SettingsWindow settingsWindow(&datamanager);
    QObject::connect(&trIcon, SIGNAL(showSettings()), &settingsWindow, SLOT(show()));
    QObject::connect(&settingsWindow, SIGNAL(preferencesChange()), &datamanager, SLOT(onPreferencesChanged()));

    StatisticWindow statisticWindow(&datamanager);
    QObject::connect(&trIcon, SIGNAL(showStatistic()), &statisticWindow, SLOT(show()));

    AboutWindow aboutWindow;
    QObject::connect(&trIcon, SIGNAL(showAbout()), &aboutWindow, SLOT(show()));

    int result = a.exec();

    return result;
}
