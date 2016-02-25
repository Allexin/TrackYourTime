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
#include "../tools/tools.h"
#include <QFileDialog>
#include "../tools/cfilebin.h"
#include <QFileInfo>

void SettingsWindow::loadPreferences()
{
    cSettings settings;

    int IdleDelay = settings.db()->value(cDataManager::CONF_IDLE_DELAY_ID,cDataManager::DEFAULT_SECONDS_IDLE_DELAY).toInt();
    int AutoSaveDelay = settings.db()->value(cDataManager::CONF_AUTOSAVE_DELAY_ID,cDataManager::DEFAULT_SECONDS_AUTOSAVE_DELAY).toInt();
    cDataManager::eNotificationType NotificationType = (cDataManager::eNotificationType)settings.db()->value(cDataManager::CONF_NOTIFICATION_TYPE_ID,1).toInt();
    bool Autorun = settings.db()->value(cDataManager::CONF_AUTORUN_ID,true).toBool();
    QString StorageFileName = settings.db()->value(cDataManager::CONF_STORAGE_FILENAME_ID,m_DataManager->getStorageFileName()).toString();
    QString Language = QLocale::system().name();
    Language.truncate(Language.lastIndexOf('_'));
    Language = settings.db()->value(cDataManager::CONF_LANGUAGE_ID,Language).toString();
    bool ClientMode = settings.db()->value(cDataManager::CONF_CLIENT_MODE_ID,false).toBool();
    QString ClientModeHost = settings.db()->value(cDataManager::CONF_CLIENT_MODE_HOST_ID,"").toString();
    QString NotificationMessage = settings.db()->value(cDataManager::CONF_NOTIFICATION_MESSAGE_ID,getDefaultMessage()).toString();
    m_NotifPos = settings.db()->value(cDataManager::CONF_NOTIFICATION_POSITION_ID,QPoint(10,10)).toPoint();
    m_NotifSize = settings.db()->value(cDataManager::CONF_NOTIFICATION_SIZE_ID,QPoint(250,100)).toPoint();
    int NotificationDelay = settings.db()->value(cDataManager::CONF_NOTIFICATION_HIDE_SECONDS_ID,4).toInt();
    int NotificationMoves = settings.db()->value(cDataManager::CONF_NOTIFICATION_HIDE_MOVES_ID,3).toInt();
    int NotificationOpacity = settings.db()->value(cDataManager::CONF_NOTIFICATION_OPACITY_ID,100).toInt();

    QFileInfo info(StorageFileName);
    QString BackupFileName = settings.db()->value(cDataManager::CONF_BACKUP_FILENAME_ID,info.absolutePath()+"/backup/").toString();
    int BackupDelay = settings.db()->value(cDataManager::CONF_BACKUP_DELAY_ID,cDataManager::BD_ONE_WEEK).toInt();

    ui->checkBoxClientMode->setChecked(ClientMode);
    ui->lineEditClientModeHost->setText(ClientModeHost);

    ui->lineEditBackupFolder->setText(BackupFileName);
    ui->comboBoxBackupDelay->setCurrentIndex(BackupDelay);

    ui->lineEditNotif_Message->setText(NotificationMessage);
    ui->spinBoxNotif_Delay->setValue(NotificationDelay);
    if (NotificationMoves==0)
        ui->radioButtonNotif_NerverHideOnMouse->setChecked(true);
    else
    if (NotificationMoves==1)
        ui->radioButtonNotif_HideOnMouse1->setChecked(true);
    else
        ui->radioButtonNotif_HideOnMouse3->setChecked(true);
    ui->spinBoxNotif_Opacity->setValue(NotificationOpacity);

    ui->comboBoxLanguage->setCurrentIndex(-1);
    for (int i = 0; i<ui->comboBoxLanguage->count(); i++)
        if (ui->comboBoxLanguage->itemData(i).toString()==Language){
            ui->comboBoxLanguage->setCurrentIndex(i);
            break;
        }
    ui->spinBoxIdleDelay->setValue(IdleDelay);
    ui->spinBoxAutosaveDelay->setValue(AutoSaveDelay);
    ui->lineEditStorageFileName->setText(StorageFileName);
    ui->checkBoxAutorun->setChecked(Autorun);

    switch(NotificationType){
        case cDataManager::NT_NONE:{
            ui->radioButtonNotif_Off->setChecked(true);
        }
        break;
        case cDataManager::NT_SYSTEM:{
            ui->radioButtonNotif_System->setChecked(true);
        }
        break;
        case cDataManager::NT_BUILTIN:{
            ui->radioButtonNotif_Custom->setChecked(true);
        }
        break;
    }

}

void SettingsWindow::applyPreferences()
{
    cSettings settings;

    settings.db()->setValue(cDataManager::CONF_IDLE_DELAY_ID,ui->spinBoxIdleDelay->value());
    settings.db()->setValue(cDataManager::CONF_AUTOSAVE_DELAY_ID,ui->spinBoxAutosaveDelay->value());
    settings.db()->setValue(cDataManager::CONF_STORAGE_FILENAME_ID,ui->lineEditStorageFileName->text().trimmed());
    settings.db()->setValue(cDataManager::CONF_CLIENT_MODE_ID,ui->checkBoxClientMode->isChecked());
    settings.db()->setValue(cDataManager::CONF_CLIENT_MODE_HOST_ID,ui->lineEditClientModeHost->text());
    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_MESSAGE_ID,ui->lineEditNotif_Message->text());
    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_POSITION_ID,m_NotifPos);
    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_SIZE_ID,m_NotifSize);
    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_HIDE_SECONDS_ID,ui->spinBoxNotif_Delay->value());

    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_OPACITY_ID,ui->spinBoxNotif_Opacity->value());

    int movesToHide = 3;
    if (ui->radioButtonNotif_NerverHideOnMouse->isChecked())
        movesToHide = 0;
    else
    if (ui->radioButtonNotif_HideOnMouse1->isChecked())
        movesToHide = 1;
    else
    if (ui->radioButtonNotif_HideOnMouse3->isChecked())
        movesToHide = 3;
    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_HIDE_MOVES_ID,movesToHide);

    if (ui->comboBoxLanguage->currentIndex()>-1)
        settings.db()->setValue(cDataManager::CONF_LANGUAGE_ID,ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString());

    settings.db()->setValue(cDataManager::CONF_BACKUP_FILENAME_ID,ui->lineEditBackupFolder->text());
    settings.db()->setValue(cDataManager::CONF_BACKUP_DELAY_ID,ui->comboBoxBackupDelay->currentIndex());

    cDataManager::eNotificationType notificationType = cDataManager::NT_SYSTEM;
    if (ui->radioButtonNotif_Off->isChecked())
        notificationType = cDataManager::NT_NONE;
    if (ui->radioButtonNotif_System->isChecked())
        notificationType = cDataManager::NT_SYSTEM;
    if (ui->radioButtonNotif_Custom->isChecked())
        notificationType = cDataManager::NT_BUILTIN;
    settings.db()->setValue(cDataManager::CONF_NOTIFICATION_TYPE_ID,notificationType);
    if (ui->checkBoxAutorun->isChecked()){
        setAutorun();
        settings.db()->setValue(cDataManager::CONF_AUTORUN_ID,true);
    }
    else{
        removeAutorun();
        settings.db()->setValue(cDataManager::CONF_AUTORUN_ID,false);
    }

    settings.db()->sync();

    emit preferencesChange();
}

QString SettingsWindow::getDefaultMessage()
{
    return tr("<center>Current profile: %PROFILE%<br>Application: %APP_NAME%<br>State: %APP_STATE%<br>Category: %APP_CATEGORY%</center>");
}

SettingsWindow::SettingsWindow(cDataManager *DataManager) : QMainWindow(0),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint );

    connect(ui->pushButtonApply, SIGNAL (released()),this, SLOT (handleButtonApply()));
    connect(ui->pushButtonCancel, SIGNAL (released()),this, SLOT (handleButtonCancel()));
    connect(ui->pushButtonBrowseStorageFileName, SIGNAL (released()),this, SLOT (handleButtonBrowse()));
    connect(ui->pushButtonBrowseBackupFolder, SIGNAL (released()),this, SLOT (handleButtonBrowseBackup()));
    connect(ui->pushButtonSetNotificationWindow, SIGNAL (released()),this, SLOT (handleButtonSetNotificationWindow()));
    connect(ui->pushButtonResetNotificationWindow, SIGNAL (released()),this, SLOT (handleButtonResetNotificationWindow()));
    connect(ui->pushButtonSetDefaultMessage,SIGNAL(released()),this, SLOT(handleButtonSetDefaultMessage()));

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
    m_NotificationSetupWindow = new notification_dummy(this);
    connect(m_NotificationSetupWindow,SIGNAL(onApplyPosAndSize()),this,SLOT(onNotificationSetPosAndSize()));
}

SettingsWindow::~SettingsWindow()
{
    delete m_NotificationSetupWindow;
    delete ui;
}

void SettingsWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent( event );
    loadPreferences();

    raise();
    activateWindow();
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

void SettingsWindow::handleButtonBrowseBackup()
{
    QString NewBackupFolder = QFileDialog::getExistingDirectory(this, tr("Select backup location"),
                                                                  ui->lineEditBackupFolder->text(),
                                                                  QFileDialog::ShowDirsOnly
                                                                  | QFileDialog::DontResolveSymlinks);
    if (!NewBackupFolder.isEmpty())
        ui->lineEditBackupFolder->setText(NewBackupFolder);
}

void SettingsWindow::handleButtonSetNotificationWindow()
{
    m_NotificationSetupWindow->showWithMessage(ui->lineEditNotif_Message->text(), ui->radioButtonNotif_HideOnMouse1->isChecked());
    m_NotificationSetupWindow->setGeometry(m_NotifPos.x(),m_NotifPos.y(),m_NotifSize.x(),m_NotifSize.y());
    m_NotificationSetupWindow->setWindowOpacity(ui->spinBoxNotif_Opacity->value()/100.f);
}

void SettingsWindow::handleButtonResetNotificationWindow()
{
    m_NotifPos = QPoint(0,0);
    m_NotifSize = QPoint(250,100);
    m_NotificationSetupWindow->setGeometry(m_NotifPos.x(),m_NotifPos.y(),m_NotifSize.x(),m_NotifSize.y());
}

void SettingsWindow::handleButtonSetDefaultMessage()
{
    ui->lineEditNotif_Message->setText(getDefaultMessage());
}

void SettingsWindow::onNotificationSetPosAndSize()
{
    m_NotifPos = QPoint(m_NotificationSetupWindow->geometry().left(),m_NotificationSetupWindow->geometry().top());
    m_NotifSize = QPoint(m_NotificationSetupWindow->geometry().width(),m_NotificationSetupWindow->geometry().height());
}
