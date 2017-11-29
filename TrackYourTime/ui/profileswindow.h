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

#ifndef PROFILESWINDOW_H
#define PROFILESWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMenu>
#include <QAction>
#include "../data/cdatamanager.h"

namespace Ui {
class ProfilesWindow;
}

class ProfilesWindow : public QMainWindow
{
    Q_OBJECT
protected:
    cDataManager*       m_DataManager;

    QMenu               m_Menu;
    void rebuild();
public:
    explicit ProfilesWindow(cDataManager* DataManager);
    ~ProfilesWindow();
protected:
    virtual void showEvent(QShowEvent * event) override;
private:
    Ui::ProfilesWindow *ui;
public slots:
    void onProfileChanged(QListWidgetItem * item);
    void onContextMenu(const QPoint& pos);
    void onMenuSelection(QAction* menuAction);

};

#endif // PROFILESWINDOW_H
