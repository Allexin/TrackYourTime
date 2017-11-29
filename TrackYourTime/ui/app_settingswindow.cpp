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
#include "app_settingswindow.h"
#include "ui_app_settingswindow.h"
#include <QDebug>
#include "../data/capppredefinedinfo.h"

App_SettingsWindow::App_SettingsWindow(cDataManager *DataManager) :
    QMainWindow(NULL),
    ui(new Ui::App_SettingsWindow)
{
    ui->setupUi(this);    
    m_DataManager = DataManager;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    ui->plainTextEditScript->setPlaceholderText("Place title parser code here. Look predefined scripts for example.");
#endif
    connect(ui->pushButtonApply,SIGNAL(released()),this,SLOT(onApply()));
    connect(ui->pushButtonStartDebug,SIGNAL(released()),this,SLOT(onSetDebug()));
}

App_SettingsWindow::~App_SettingsWindow()
{
    delete ui;
}

void App_SettingsWindow::onApply()
{
    sAppInfo* appInfo = m_DataManager->applications(m_AppIndex);
    appInfo->trackerType = (sAppInfo::eTrackerType)ui->comboBoxTrackingType->currentIndex();
    appInfo->useCustomScript = ui->checkBoxCustomScript->isChecked();
    appInfo->customScript = ui->plainTextEditScript->toPlainText();
    m_DataManager->setDebugScript("");
    hide();
}

void App_SettingsWindow::onSetDebug()
{
    m_DataManager->setDebugScript(ui->plainTextEditScript->toPlainText());
}

void App_SettingsWindow::onScriptResult(QString value, const sSysInfo &data, QString trackingResult)
{
    ui->labelDebugApp->setText(data.fileName);
    ui->labelDebugTitle->setText(data.title);
    ui->labelDebugTrackingResult->setText(trackingResult);
    ui->labelDebugResult->setText(value);
}

void App_SettingsWindow::showApp(int appIndex)
{
    m_AppIndex = appIndex;
    const sAppInfo* appInfo = m_DataManager->applications(m_AppIndex);
    ui->labelApplication->setText(appInfo->activities[0].name);
    ui->comboBoxTrackingType->setCurrentIndex(appInfo->trackerType);
    ui->checkBoxCustomScript->setChecked(appInfo->useCustomScript);
    QString script = appInfo->customScript;
    if (script.isEmpty())
        script = "function parseData(appName, appTitle, trackingResult, currentOS){\n  return trackingResult;\n}";
    ui->plainTextEditScript->setPlainText(script);
    ui->labelAdditionalInfo->setText(appInfo->predefinedInfo->info());
    showNormal();
    raise();
    activateWindow();
}
