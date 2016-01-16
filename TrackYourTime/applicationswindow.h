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

#ifndef APPLICATIONSWINDOW_H
#define APPLICATIONSWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDropEvent>
#include "cdatamanager.h"

class cApplicationsTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    static const int TREE_ITEM_TYPE_CATEGORY    = QTreeWidgetItem::UserType + 1;
    static const int TREE_ITEM_TYPE_APPLICATION = QTreeWidgetItem::UserType + 2;
public:
    explicit cApplicationsTreeWidget(QWidget *parent = 0):QTreeWidget(parent){}

    virtual void dropEvent(QDropEvent * event) override{
        QTreeWidgetItem* newParent = itemAt(event->pos());
        if (newParent)
            if (newParent->type()==TREE_ITEM_TYPE_CATEGORY){
                if (selectedItems().size()>0)
                    emit itemMoved(selectedItems().first(),newParent);
                QTreeWidget::dropEvent(event);
            }
    }
signals:
    void itemMoved(QTreeWidgetItem* item, QTreeWidgetItem* newParent);
};

namespace Ui {
class ApplicationsWindow;
}

class ApplicationsWindow : public QMainWindow
{
    Q_OBJECT
protected:
    QMenu               m_CategoriesMenu;
    QAction*            m_AddCategory;
    QAction*            m_DeleteCategory;
    QAction*            m_SetCategoryColor;

    cDataManager*       m_DataManager;
    bool                m_LoadingData;
    void rebuildProfilesList();
    void rebuildApplicationsList();
public:
    explicit ApplicationsWindow(cDataManager* DataManager);
    ~ApplicationsWindow();

private:
    Ui::ApplicationsWindow *ui;
protected:
    virtual void showEvent(QShowEvent * event) override;
signals:
    void showProfiles();
public slots:
    void onProfilesChange();
    void onApplicationsChange();
    void onCategoryChanged(QTreeWidgetItem *item, int column);
    void onContextMenu(const QPoint& pos);
    void onMenuSelection(QAction* menuAction);
    void onProfileSelection(int newProfileIndex);
    void onApplicationMoved(QTreeWidgetItem* item, QTreeWidgetItem* newParent);
    void onEditProfiles(){emit showProfiles();}
};

#endif // APPLICATIONSWINDOW_H
