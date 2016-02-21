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

#include "ctrayicon.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>



void cTrayIcon::rebuildMenu()
{
    m_ProfilesMenu.clear();
    for (int i = 0; i<m_DataManager->profilesCount(); i++){
        QAction* profile = m_ProfilesMenu.addAction(m_DataManager->profiles(i)->name);
        if (m_DataManager->getCurrentProfileIndex()==i){
            profile->setCheckable(true);
            profile->setChecked(true);            
        }
        profile->setData(i);
    }
}

cTrayIcon::cTrayIcon(cDataManager *DataManager):QSystemTrayIcon()
{    
    m_DataManager = DataManager;
    rebuildMenu();

    connect(this,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(onTray(QSystemTrayIcon::ActivationReason)));

    connect(&m_ProfilesMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));
    m_ProfilesMenu.setTitle(tr("Profiles"));

    connect(&m_Menu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));
    setContextMenu(&m_Menu);
    m_Menu.addAction(tr("Applications..."))->setData("APPLICATIONS");
    m_Menu.addAction(tr("Statistic..."))->setData("STATISTIC");
    m_Menu.addSeparator();
    m_Menu.addAction(tr("Settings..."))->setData("SETTINGS");
    m_Menu.addAction(tr("Schedule..."))->setData("SCHEDULE");
    m_Menu.addSeparator();
#ifdef Q_OS_WIN
    m_Menu.addMenu(&m_ProfilesMenu);
#else
    m_Menu.addAction(m_ProfilesMenu.title()+" ►")->setData("PROFILES");
#endif
    m_Menu.addSeparator();
    m_Menu.addAction(tr("About..."))->setData("ABOUT");
    m_Menu.addAction(tr("Help..."))->setData("HELP");
    m_Menu.addSeparator();
    m_Menu.addAction(tr("Exit"))->setData("EXIT");


    setActive();
    show();
}

void cTrayIcon::onTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason==QSystemTrayIcon::Trigger)
        emit showApplications();
}

void cTrayIcon::setActive()
{    
#ifdef Q_OS_MAC
    setIcon(QIcon("data/icons/main.png"));
#else
    QIcon icon("data/icons/main.ico");
    if (icon.isNull())
        icon = QIcon("data/icons/main.png");
    setIcon(icon);
#endif
}

void cTrayIcon::setInactive()
{    
#ifdef Q_OS_MAC
    setIcon(QIcon("data/icons/main_gray.png"));
#else
    QIcon icon("data/icons/main_gray.ico");
    if (icon.isNull())
        icon = QIcon("data/icons/main_gray.png");
    setIcon(icon);
#endif
}

void cTrayIcon::showHint(QString text)
{
    showMessage("",text,QSystemTrayIcon::Information,5000);
}

void cTrayIcon::onProfilesChange()
{
    //never rebuild menu directly from method, because method may be inside menu event and we can delete menu item while we is insede this item handler which can cause problems.
    QTimer::singleShot(1, this, SLOT(rebuildMenu()));
}

void cTrayIcon::onMenuSelection(QAction *menuAction)
{
    QString id = menuAction->data().toString();
    if (id=="EXIT"){
        QApplication::quit();
        return;
    }

    if (id=="APPLICATIONS"){
        emit showApplications();
        return;
    }

    if (id=="SETTINGS"){
        emit showSettings();
        return;
    }

    if (id=="SCHEDULE"){
        emit showSchedule();
        return;
    }

    if (id=="STATISTIC"){
        emit showStatistic();
        return;
    }

    if (id=="ABOUT"){
        emit showAbout();
        return;
    }

    if (id=="HELP"){
        QDesktopServices::openUrl(QUrl(tr("https://github.com/Allexin/TrackYourTime/wiki/User-Manual")));
        return;
    }

    if (id=="PROFILES"){
        m_ProfilesMenu.popup(QCursor::pos());
        return;
    }

    bool isProfile;
    int profileID = menuAction->data().toInt(&isProfile);
    if (isProfile)
        m_DataManager->setCurrentProfileIndex(profileID);
}
