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
