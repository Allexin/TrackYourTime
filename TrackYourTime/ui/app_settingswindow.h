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
#ifndef APP_SETTINGSWINDOW_H
#define APP_SETTINGSWINDOW_H

#include <QMainWindow>
#include "../data/cdatamanager.h"

namespace Ui {
class App_SettingsWindow;
}

class App_SettingsWindow : public QMainWindow
{
    Q_OBJECT

protected:
    cDataManager*       m_DataManager;
    int                 m_AppIndex;
public:
    explicit App_SettingsWindow(cDataManager* DataManager);
    ~App_SettingsWindow();

private:
    Ui::App_SettingsWindow *ui;
private slots:
    void onApply();
    void onSetDebug();
    void onScriptResult(QString value, const sSysInfo& data, QString trackingResult);
public slots:
    void showApp(int appIndex);
};

#endif // APP_SETTINGSWINDOW_H
