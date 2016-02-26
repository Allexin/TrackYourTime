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

#include "ui/settingswindow.h"
#include "ui/statisticwindow.h"
#include "ui/applicationswindow.h"
#include "ui/profileswindow.h"
#include "ui/aboutwindow.h"
#include "ui/app_settingswindow.h"
#include "ui/schedulewindow.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QTimer>
#include "tools/tools.h"
#include <QTranslator>
#include <QSystemTrayIcon>
#include "data/cdatamanager.h"
#include "data/cschedule.h"
#include "data/cupdater.h"
#include "ui/ctrayicon.h"
#include "ui/notificationwindow.h"
#include "ui/updateavailablewindow.h"

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


    qDebug() << "application start\n";

    cSettings settings;
    QString Language = QLocale::system().name();
    Language.truncate(Language.lastIndexOf('_'));
    Language = settings.db()->value(cDataManager::CONF_LANGUAGE_ID,Language).toString();
    if (settings.db()->value(cDataManager::CONF_FIRST_LAUNCH_ID,true).toBool()){
        settings.db()->setValue(cDataManager::CONF_FIRST_LAUNCH_ID,false);
        settings.db()->setValue(cDataManager::CONF_LANGUAGE_ID,Language);
        settings.db()->setValue(cDataManager::CONF_AUTORUN_ID,true);
        setAutorun();
        settings.db()->sync();
    }

    qDebug() << "laod translation\n";
    QTranslator translator;
    translator.load("lang_" + Language,QDir::currentPath()+"/data/languages");
    QApplication::installTranslator(&translator);

    qDebug() << "init datamanager\n";
    cDataManager datamanager;
    qDebug() << "init schedule\n";
    cSchedule schedule(&datamanager);
    qDebug() << "init updater\n";
    cUpdater updater;
    QObject::connect(&schedule,SIGNAL(checkUpdates()),&updater,SLOT(checkUpdates()));

    qDebug() << "init tray icon\n";
    cTrayIcon trIcon(&datamanager);
    QObject::connect(&datamanager, SIGNAL(trayActive()), &trIcon, SLOT(setActive()));
    QObject::connect(&datamanager, SIGNAL(traySleep()), &trIcon, SLOT(setInactive()));
    QObject::connect(&datamanager, SIGNAL(trayShowHint(QString)), &trIcon, SLOT(showHint(QString)));
    QObject::connect(&datamanager, SIGNAL(profilesChanged()), &trIcon, SLOT(onProfilesChange()));

    qDebug() << "init applications window\n";
    ApplicationsWindow applicationsWindow(&datamanager);
    QObject::connect(&trIcon, SIGNAL(showApplications()), &applicationsWindow, SLOT(showNormal()));
    QObject::connect(&datamanager, SIGNAL(profilesChanged()), &applicationsWindow, SLOT(onProfilesChange()));
    QObject::connect(&datamanager, SIGNAL(applicationsChanged()), &applicationsWindow, SLOT(onApplicationsChange()));

    qDebug() << "init profiles window\n";
    ProfilesWindow profilesWindow(&datamanager);
    QObject::connect(&applicationsWindow, SIGNAL(showProfiles()), &profilesWindow, SLOT(showNormal()));

    qDebug() << "init app settings window\n";
    App_SettingsWindow app_settingsWindow(&datamanager);
    QObject::connect(&applicationsWindow, SIGNAL(showAppSettings(int)), &app_settingsWindow, SLOT(showApp(int)));
    QObject::connect(&datamanager, SIGNAL(debugScriptResult(QString,sSysInfo)), &app_settingsWindow, SLOT(onScriptResult(QString,sSysInfo)));

    qDebug() << "init settings window\n";
    SettingsWindow settingsWindow(&datamanager);
    QObject::connect(&trIcon, SIGNAL(showSettings()), &settingsWindow, SLOT(showNormal()));
    QObject::connect(&settingsWindow, SIGNAL(preferencesChange()), &datamanager, SLOT(onPreferencesChanged()));

    qDebug() << "init schedule window\n";
    ScheduleWindow scheduleWindow(&datamanager,&schedule);
    QObject::connect(&datamanager, SIGNAL(profilesChanged()), &scheduleWindow, SLOT(rebuild()));
    QObject::connect(&trIcon, SIGNAL(showSchedule()), &scheduleWindow, SLOT(showNormal()));

    qDebug() << "init statistic window\n";
    StatisticWindow statisticWindow(&datamanager);
    QObject::connect(&trIcon, SIGNAL(showStatistic()), &statisticWindow, SLOT(showNormal()));

    qDebug() << "init about window\n";
    AboutWindow aboutWindow;
    QObject::connect(&trIcon, SIGNAL(showAbout()), &aboutWindow, SLOT(showNormal()));

    qDebug() << "init update window\n";
    UpdateAvailableWindow updateAvailableWindow;
    QObject::connect(&updater, SIGNAL(newVersionAvailable(QString)), &updateAvailableWindow, SLOT(showUpdate(QString)));
    QObject::connect(&updateAvailableWindow, SIGNAL(ignoreUpdate()), &updater, SLOT(ignoreNewVersion()));

    qDebug() << "init notification window\n";
    NotificationWindow notificationWindow(&datamanager);
    QObject::connect(&settingsWindow, SIGNAL(preferencesChange()), &notificationWindow, SLOT(onPreferencesChanged()));
    QObject::connect(&datamanager, SIGNAL(showNotification()), &notificationWindow, SLOT(onShow()));

    qDebug() << "start schedule\n";
    schedule.start();

    qDebug() << "start app loop\n";
    int result = a.exec();
    qDebug() << "application close\n";
    return result;
}
