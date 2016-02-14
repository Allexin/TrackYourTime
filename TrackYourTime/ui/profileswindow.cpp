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

#include "profileswindow.h"
#include "ui_profileswindow.h"
#include <QMessageBox>

void ProfilesWindow::rebuild()
{
    ui->listWidgetProfiles->clear();
    for (int i = 0; i<m_DataManager->profilesCount(); i++){
        QListWidgetItem* item = new QListWidgetItem(m_DataManager->profiles(i)->name);
        item->setData(Qt::UserRole,i);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(Qt::Unchecked);
        ui->listWidgetProfiles->addItem(item);

    }
}

ProfilesWindow::ProfilesWindow(cDataManager *DataManager) : QMainWindow(0),
    ui(new Ui::ProfilesWindow)
{
    ui->setupUi(this);

    m_DataManager = DataManager;

    m_Menu.addAction(tr("New profile"))->setData("NEW_PROFILE");
    m_Menu.addAction(tr("Clone profile"))->setData("CLONE_PROFILE");
    m_Menu.addAction(tr("Merge profiles"))->setData("MERGE_PROFILES");
    connect(&m_Menu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));


    ui->listWidgetProfiles->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidgetProfiles,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(onContextMenu(QPoint)));

    connect(ui->listWidgetProfiles, SIGNAL (itemChanged(QListWidgetItem *)),this, SLOT (onProfileChanged(QListWidgetItem *)));
}

ProfilesWindow::~ProfilesWindow()
{
    delete ui;
}

void ProfilesWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent( event );
    rebuild();
}

void ProfilesWindow::onProfileChanged(QListWidgetItem *item)
{
    m_DataManager->setProfileName(item->data(Qt::UserRole).toInt(),item->text());
}

void ProfilesWindow::onContextMenu(const QPoint &pos)
{
    QPoint pt(pos);
    m_Menu.exec( ui->listWidgetProfiles->mapToGlobal(pos) );
}

void ProfilesWindow::onMenuSelection(QAction *menuAction)
{
    QString id = menuAction->data().toString();

    QVector<int> checkedItems;
    for (int i = 0; i<ui->listWidgetProfiles->count(); i++){
        QListWidgetItem* item = ui->listWidgetProfiles->item(i);
        if (item->checkState()==Qt::Checked)
            checkedItems.push_back(item->data(Qt::UserRole).toInt());
    }

    if (id=="NEW_PROFILE"){
        m_DataManager->addNewProfile(tr("New Profile"));
        rebuild();
        return;
    }

    if (id=="CLONE_PROFILE"){
        if (checkedItems.size()!=1){
            QMessageBox::critical(this,tr("Incorrect arguments"),tr("Please select one and only one profile for cloning"),QMessageBox::Ok);
            return;
        }
        int index = checkedItems[0];
        m_DataManager->addNewProfile(m_DataManager->profiles(index)->name+" "+tr("copy"),index);
        rebuild();
        return;
    }

    if (id=="MERGE_PROFILES"){
        if (checkedItems.size()<2){
            QMessageBox::critical(this,tr("Incorrect arguments"),tr("Please select at least two profiles for merging"),QMessageBox::Ok);
            return;
        }
        if (QMessageBox::warning(this,tr("Data will be lost"),tr("Warning! After the merging uniqe difference between profiles will be lost. This operation can't be undone. Proceed?"),QMessageBox::Yes,QMessageBox::Cancel)==QMessageBox::Yes){
            while (checkedItems.size()>1){
                m_DataManager->mergeProfiles(checkedItems[checkedItems.size()-2],checkedItems[checkedItems.size()-1]);
                checkedItems.pop_back();
            }
            rebuild();
        }
        return;
    }
}
