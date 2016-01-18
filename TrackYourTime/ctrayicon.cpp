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



void cTrayIcon::rebuildMenu()
{    
    m_Menu.clear();
    m_Menu.addAction(tr("Applications..."))->setData("APPLICATIONS");
    m_Menu.addAction(tr("Statistic..."))->setData("STATISTIC");
    m_Menu.addSeparator();
    m_Menu.addAction(tr("Settings..."))->setData("SETTINGS");
    m_Menu.addSeparator();
    for (int i = 0; i<m_DataManager->profilesCount(); i++){
        QAction* profile = m_Menu.addAction(m_DataManager->profiles(i)->name);
        if (m_DataManager->getCurrentProfileIndex()==i){
            profile->setCheckable(true);
            profile->setChecked(true);
            profile->setData(i);
        }
    }
    m_Menu.addSeparator();
    m_Menu.addAction(tr("About..."))->setData("ABOUT");
    m_Menu.addSeparator();
    m_Menu.addAction(tr("Exit"))->setData("EXIT");
}

cTrayIcon::cTrayIcon(cDataManager *DataManager):QSystemTrayIcon()
{    
    m_DataManager = DataManager;
    rebuildMenu();

    connect(&m_Menu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));

    //In some WM in Linux rebuilding context menu not working properly. We do not use context tray menu and show menu manually.
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayTriggered(QSystemTrayIcon::ActivationReason)));


    setActive();
    show();
}

void cTrayIcon::setActive()
{    
     setIcon(QIcon("data/icons/main.ico"));
}

void cTrayIcon::setInactive()
{
    setIcon(QIcon("data/icons/main_gray.ico"));
}

void cTrayIcon::showHint(QString text)
{
    showMessage("",text,QSystemTrayIcon::Information,5000);
}

void cTrayIcon::onProfilesChange()
{
    rebuildMenu();
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

    if (id=="STATISTIC"){
        emit showStatistic();
        return;
    }

    if (id=="ABOUT"){
        emit showAbout();
        return;
    }

    bool isProfile;
    int profileID = menuAction->data().toInt(&isProfile);
    if (isProfile)
        m_DataManager->setCurrentProfileIndex(profileID);
}

void cTrayIcon::onTrayTriggered(QSystemTrayIcon::ActivationReason reason)
{
    if (reason==QSystemTrayIcon::Context){
        m_Menu.popup(QCursor::pos());
    }
}
