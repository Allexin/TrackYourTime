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
    m_Applications = m_Menu.addAction(tr("Applications..."));
    m_Statistic = m_Menu.addAction(tr("Statistic..."));
    m_Menu.addSeparator();
    m_Settings = m_Menu.addAction(tr("Settings..."));
    m_Menu.addSeparator();
    m_Profiles.resize(m_DataManager->profilesCount());
    for (int i = 0; i<m_DataManager->profilesCount(); i++){
        m_Profiles[i] = m_Menu.addAction(m_DataManager->profiles(i)->name);
        if (m_DataManager->getCurrentProfileIndex()==i){
            m_Profiles[i]->setCheckable(true);
            m_Profiles[i]->setChecked(true);
        }
    }
    m_Menu.addSeparator();
    m_About = m_Menu.addAction(tr("About..."));
    m_Menu.addSeparator();
    m_Exit = m_Menu.addAction(tr("Exit"));
}

cTrayIcon::cTrayIcon(cDataManager *DataManager):QSystemTrayIcon()
{
    m_DataManager = DataManager;
    rebuildMenu();
    setContextMenu(&m_Menu);
    connect(&m_Menu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));

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
    if (menuAction==m_Exit){
        QApplication::quit();
        return;
    }

    if (menuAction==m_Applications){
        emit showApplications();
        return;
    }

    if (menuAction==m_Settings){
        emit showSettings();
        return;
    }

    if (menuAction==m_Statistic){
        emit showStatistic();
        return;
    }

    if (menuAction==m_About){
        emit showAbout();
        return;
    }

    for (int i = 0; i<m_Profiles.size(); i++)
        if (menuAction==m_Profiles[i]){
            m_DataManager->setCurrentProfileIndex(i);
            return;
        }
}
