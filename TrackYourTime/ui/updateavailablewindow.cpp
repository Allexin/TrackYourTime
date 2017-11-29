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
#include "updateavailablewindow.h"
#include "ui_updateavailablewindow.h"
#include "../tools/tools.h"
#include <QDesktopServices>
#include <QUrl>

UpdateAvailableWindow::UpdateAvailableWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UpdateAvailableWindow)
{
    ui->setupUi(this);

    connect(ui->pushButtonChangelog,SIGNAL(released()),this,SLOT(processButtonChangelog()));
    connect(ui->pushButtonDownload,SIGNAL(released()),this,SLOT(processButtonDownload()));
    connect(ui->pushButtonIgnoreVersion,SIGNAL(released()),this,SLOT(processButtonIgnoreUpdate()));
    connect(ui->pushButtonRemindLater,SIGNAL(released()),this,SLOT(processButtonRemindLater()));
}

UpdateAvailableWindow::~UpdateAvailableWindow()
{
    delete ui;
}

void UpdateAvailableWindow::processButtonDownload()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Allexin/TrackYourTime/releases/latest"));
}

void UpdateAvailableWindow::processButtonChangelog()
{
    QDesktopServices::openUrl(QUrl("https://raw.githubusercontent.com/Allexin/TrackYourTime/master/change.log"));
}

void UpdateAvailableWindow::processButtonRemindLater()
{
    hide();
}

void UpdateAvailableWindow::processButtonIgnoreUpdate()
{
    emit ignoreUpdate();
    hide();
}

void UpdateAvailableWindow::showUpdate(QString newVersion)
{
    ui->labelCurrentVersion->setText(CURRENT_VERSION);
    ui->labelNewVersion->setText(newVersion);

    showNormal();
    raise();
    activateWindow();
}
