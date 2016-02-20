#include "notification_dummy.h"
#include "ui_notification_dummy.h"

notification_dummy::notification_dummy(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::notification_dummy)
{
    ui->setupUi(this);
    connect(ui->pushButtonApply,SIGNAL(released()),this,SLOT(onButtonApply()));
    connect(ui->pushButtonClose,SIGNAL(released()),this,SLOT(hide()));
}

notification_dummy::~notification_dummy()
{
    delete ui;
}

void notification_dummy::showWithMessage(const QString &format, bool compactMode)
{
    ui->labelMessage->setText(format);
    ui->comboBoxCategories->setVisible(!compactMode);
    ui->labelCategory->setVisible(!compactMode);

    show();
}

void notification_dummy::onButtonApply()
{
    emit onApplyPosAndSize();
    hide();
}

