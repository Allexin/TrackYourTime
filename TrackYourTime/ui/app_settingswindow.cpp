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
    appInfo->customScript = ui->plainTextEditScript->toPlainText();
    hide();
}

void App_SettingsWindow::onSetDebug()
{
    m_DataManager->setDebugScript(ui->plainTextEditScript->toPlainText());
}

void App_SettingsWindow::onScriptResult(QString value, const sSysInfo &data)
{
    ui->labelDebugApp->setText(data.fileName);
    ui->labelDebugTitle->setText(data.title);
    ui->labelDebugResult->setText(value);
}

void App_SettingsWindow::showApp(int appIndex)
{
    m_AppIndex = appIndex;
    const sAppInfo* appInfo = m_DataManager->applications(m_AppIndex);
    ui->labelApplication->setText(appInfo->activities[0].name);
    ui->comboBoxTrackingType->setCurrentIndex(appInfo->trackerType);
    ui->plainTextEditScript->setPlainText(appInfo->customScript);
    ui->labelAdditionalInfo->setText(appInfo->predefinedInfo->info());
    show();
}
