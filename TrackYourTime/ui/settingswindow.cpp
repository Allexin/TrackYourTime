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
#include "ui_settingswindow.h"
#include <QSettings>
#include <QFileDialog>
#include "../tools/cfilebin.h"

void SettingsWindow::loadPreferences()
{
    QSettings settings;

    int UpdateDelay = settings.value(cDataManager::CONF_UPDATE_DELAY_ID,cDataManager::DEFAULT_SECONDS_UPDATE_DELAY).toInt();
    int IdleDelay = settings.value(cDataManager::CONF_IDLE_DELAY_ID,cDataManager::DEFAULT_SECONDS_IDLE_DELAY).toInt();
    int AutoSaveDelay = settings.value(cDataManager::CONF_AUTOSAVE_DELAY_ID,cDataManager::DEFAULT_SECONDS_AUTOSAVE_DELAY).toInt();
    bool ShowBaloons = settings.value(cDataManager::CONF_SHOW_BALOONS_ID,true).toBool();
    bool Autorun = settings.value(cDataManager::CONF_AUTORUN_ID,true).toBool();
    QString StorageFileName = settings.value(cDataManager::CONF_STORAGE_FILENAME_ID,m_DataManager->getStorageFileName()).toString();
    QString Language = QLocale::system().name();
    Language.truncate(Language.lastIndexOf('_'));
    Language = settings.value(cDataManager::CONF_LANGUAGE_ID,Language).toString();


    ui->comboBoxLanguage->setCurrentIndex(-1);
    for (int i = 0; i<ui->comboBoxLanguage->count(); i++)
        if (ui->comboBoxLanguage->itemData(i).toString()==Language){
            ui->comboBoxLanguage->setCurrentIndex(i);
            break;
        }
    ui->spinBoxUpdateDelay->setValue(UpdateDelay);
    ui->spinBoxIdleDelay->setValue(IdleDelay);
    ui->spinBoxAutosaveDelay->setValue(AutoSaveDelay);
    ui->lineEditStorageFileName->setText(StorageFileName);
    ui->checkBoxShowBaloon->setChecked(ShowBaloons);
    ui->checkBoxAutorun->setChecked(Autorun);

}

void SettingsWindow::applyPreferences()
{
    QSettings settings;

    settings.setValue(cDataManager::CONF_UPDATE_DELAY_ID,ui->spinBoxUpdateDelay->value());
    settings.setValue(cDataManager::CONF_IDLE_DELAY_ID,ui->spinBoxIdleDelay->value());
    settings.setValue(cDataManager::CONF_AUTOSAVE_DELAY_ID,ui->spinBoxAutosaveDelay->value());
    settings.setValue(cDataManager::CONF_STORAGE_FILENAME_ID,ui->lineEditStorageFileName->text());

    if (ui->comboBoxLanguage->currentIndex()>-1)
        settings.setValue(cDataManager::CONF_LANGUAGE_ID,ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString());
    settings.setValue(cDataManager::CONF_SHOW_BALOONS_ID,ui->checkBoxShowBaloon->isChecked());
    if (ui->checkBoxAutorun->isChecked()){
        setAutorun();
        settings.setValue(cDataManager::CONF_AUTORUN_ID,true);
    }
    else{
        removeAutorun();
        settings.setValue(cDataManager::CONF_AUTORUN_ID,false);
    }

    settings.sync();

    emit preferencesChange();
}

SettingsWindow::SettingsWindow(cDataManager *DataManager) : QMainWindow(0),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint );

    connect(ui->pushButtonApply, SIGNAL (released()),this, SLOT (handleButtonApply()));
    connect(ui->pushButtonCancel, SIGNAL (released()),this, SLOT (handleButtonCancel()));
    connect(ui->pushButtonBrowseStorageFileName, SIGNAL (released()),this, SLOT (handleButtonBrowse()));

    QString languagesPath = QDir::currentPath()+"/data/languages";
    QStringList languagesList = QDir(languagesPath).entryList(QStringList() << "*.qm");
    for (int i = 0; i<languagesList.size(); i++){
        QString lang = languagesList[i].mid(5,2);
        QString langName = "";

        cFileBin file(languagesPath+"/lang_"+lang+"_name.utf8");
        if (file.open(QIODevice::ReadOnly))
        {
            langName = file.readUtf8Line();
        }

        ui->comboBoxLanguage->addItem("["+lang+"]"+langName,lang);
    }

    m_DataManager = DataManager;
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent( event );
    loadPreferences();
}

void SettingsWindow::handleButtonApply()
{
    applyPreferences();
    close();
}

void SettingsWindow::handleButtonCancel()
{
    close();
}

void SettingsWindow::handleButtonBrowse()
{

    QString NewStorageFileName = QFileDialog::getOpenFileName(this,
                                 tr("Select DB location"),
                                 ui->lineEditStorageFileName->text(),
                                 tr("Default DB (db.bin);;All files (*.*)")
                                 );
    if (!NewStorageFileName.isEmpty())
        ui->lineEditStorageFileName->setText(NewStorageFileName);
}
