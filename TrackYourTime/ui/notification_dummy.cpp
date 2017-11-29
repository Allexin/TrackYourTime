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

    showNormal();
    raise();
    activateWindow();
}

void notification_dummy::onButtonApply()
{
    emit onApplyPosAndSize();
    hide();
}

