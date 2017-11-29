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

#include "applicationswindow.h"
#include "ui_applicationswindow.h"
#include <QColorDialog>
#include <QScrollBar>

void ApplicationsWindow::rebuildProfilesList()
{
    m_LoadingData = true;
    ui->comboBoxProfiles->clear();
    for (int i = 0; i<m_DataManager->profilesCount(); i++){
        const sProfile* profile = m_DataManager->profiles(i);
        ui->comboBoxProfiles->addItem(profile->name);
    }
    ui->comboBoxProfiles->setCurrentIndex(m_DataManager->getCurrentProfileIndex());
    m_LoadingData = false;
}

QIcon createColorIcon(QColor color){
    QPixmap pixmap(16,16);
    pixmap.fill(color);
    return QIcon(pixmap);
}

QTreeWidgetItem* createTreeItemCategory(int index, QColor color, const QString& text){
    QTreeWidgetItem* item = new QTreeWidgetItem(cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY);
    item->setText(0,text);
    item->setIcon(0,createColorIcon(color));
    item->setData(0,Qt::UserRole,index);
    return item;
}

void ApplicationsWindow::rebuildApplicationsList()
{
    rebuildContextMenu();
    m_CategoriesExpandedState.resize(ui->treeWidgetApplications->topLevelItemCount());
    for (int i = 0; i<m_CategoriesExpandedState.size(); i++){
        QTreeWidgetItem* item = ui->treeWidgetApplications->topLevelItem(i);
        m_CategoriesExpandedState[i] = item?item->isExpanded():false;
    }
    m_ScrollPos = ui->treeWidgetApplications->verticalScrollBar()->value();

    //create categories
    QVector<QTreeWidgetItem*> categories;
    categories.resize(m_DataManager->categoriesCount());
    ui->treeWidgetApplications->clear();
    for (int i = 0; i<m_DataManager->categoriesCount(); i++){
        const sCategory* category = m_DataManager->categories(i);
        categories[i] = createTreeItemCategory(i,category->color,category->name);
        categories[i]->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
        ui->treeWidgetApplications->addTopLevelItem(categories[i]);
    }    
    QTreeWidgetItem* uncategorized = createTreeItemCategory(-1,QColor(Qt::gray),tr("Uncategorized"));
    uncategorized->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    ui->treeWidgetApplications->addTopLevelItem(uncategorized);

    bool showHidden = ui->checkBoxShowHidden->isChecked();

    QBrush visibleItem(Qt::black);
    QBrush invisibleItem(Qt::gray);

    //place applications in category
    int currentProfile = m_DataManager->getCurrentProfileIndex();
    for (int i = 0; i<m_DataManager->applicationsCount(); i++){
        const sAppInfo* app = m_DataManager->applications(i);

        if (showHidden || app->visible){
            QVector<QTreeWidgetItem*> app_in_categories;
            app_in_categories.resize(categories.size());
            for (int j = 0; j<app_in_categories.size(); j++)
                app_in_categories[j] = NULL;
            QTreeWidgetItem* app_uncategorized = NULL;

            for (int j = 0; j<app->activities.size(); j++){
                const sActivityInfo* ainfo = &app->activities[j];
                if (showHidden || ainfo->categories[currentProfile].visible){
                    QTreeWidgetItem* parent = NULL;
                    if (ainfo->categories[currentProfile].category==-1){
                        parent = app_uncategorized;
                        if (parent==NULL){
                            QTreeWidgetItem* item = new QTreeWidgetItem(cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION);
                            item->setText(0,app->activities[0].name);
                            item->setToolTip(0,app->path+"/"+app->activities[0].name);
                            item->setData(0,Qt::UserRole,i);
                            if (app->trackerType==sAppInfo::eTrackerType::TT_EXTERNAL_DETECTOR)
                                item->setIcon(0,m_ExternalDetector);
                            else
                            if (app->trackerType==sAppInfo::eTrackerType::TT_PREDEFINED_SCRIPT)
                                item->setIcon(0,m_ScriptDetector);
                            else
                                item->setIcon(0,createColorIcon(QColor(Qt::gray)));
                            item->setFlags(Qt::ItemIsEnabled);
                            item->setForeground(0,invisibleItem);
                            uncategorized->addChild(item);
                            app_uncategorized = item;
                            parent = item;
                        }
                    }
                    else{
                        parent = app_in_categories[ainfo->categories[currentProfile].category];
                        if (parent==NULL){
                            const sCategory* category = m_DataManager->categories(ainfo->categories[currentProfile].category);
                            QTreeWidgetItem* item = new QTreeWidgetItem(cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION);
                            item->setText(0,app->activities[0].name);
                            item->setToolTip(0,app->path+"/"+app->activities[0].name);
                            item->setData(0,Qt::UserRole,i);
                            if (app->trackerType==sAppInfo::eTrackerType::TT_EXTERNAL_DETECTOR)
                                item->setIcon(0,m_ExternalDetector);
                            else
                            if (app->trackerType==sAppInfo::eTrackerType::TT_PREDEFINED_SCRIPT)
                                item->setIcon(0,m_ScriptDetector);
                            else
                                item->setIcon(0,createColorIcon(category->color));
                            item->setFlags(Qt::ItemIsEnabled);
                            item->setForeground(0,invisibleItem);
                            categories[ainfo->categories[currentProfile].category]->addChild(item);
                            app_in_categories[ainfo->categories[currentProfile].category] = item;
                            parent = item;
                        }
                    }

                    QTreeWidgetItem* item = new QTreeWidgetItem(cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION_ACTIVITY);
                    if (j==0)
                        item->setText(0,ainfo->name+tr("(default)"));
                    else
                        item->setText(0,ainfo->name);
                    item->setData(0,Qt::UserRole,i);
                    item->setData(0,Qt::UserRole+1,j);
                    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
                    if (ainfo->categories[currentProfile].visible){
                        item->setForeground(0,visibleItem);
                        parent->setForeground(0,visibleItem);
                    }
                    else{
                        item->setForeground(0,invisibleItem);
                    }

                    parent->addChild(item);
                }
            }
        }
    }

    for (int i = 0; i<m_DataManager->categoriesCount(); i++)
        if (i<m_CategoriesExpandedState.size())
            categories[i]->setExpanded(m_CategoriesExpandedState[i]);
    if (categories.size()+1<m_CategoriesExpandedState.size())
        uncategorized->setExpanded(categories.size()+1);
    ui->treeWidgetApplications->verticalScrollBar()->setValue(m_ScrollPos);
}

void ApplicationsWindow::rebuildContextMenu()
{
    m_MoveToMenu.clear();
    for (int i = 0; i<m_DataManager->categoriesCount(); i++)
        m_MoveToMenu.addAction(m_DataManager->categories(i)->name)->setData(i);
}

ApplicationsWindow::ApplicationsWindow(cDataManager *DataManager) : QMainWindow(0),
    ui(new Ui::ApplicationsWindow),
    m_ScriptDetector("data/icons/script.png"),
    m_ExternalDetector("data/icons/extern_tracker.png")
{
    ui->setupUi(this);
    m_DataManager = DataManager;
    m_LoadingData = false;


    connect(ui->comboBoxProfiles, SIGNAL(currentIndexChanged(int)), this, SLOT(onProfileSelection(int)));

    connect(ui->pushButtonEditProfiles, SIGNAL(released()), this, SLOT(onEditProfiles()));

    connect(ui->checkBoxShowHidden,SIGNAL(stateChanged(int)),this,SLOT(onApplicationsChange()));

    m_MoveToMenu.setTitle(tr("Move to category"));
    connect(&m_MoveToMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMoveToMenuSelection(QAction*)));

    m_CategoriesMenu.addAction(tr("New category"))->setData("NEW_CATEGORY_MENU");
    m_CategoriesMenu.addAction(tr("Delete category"))->setData("DELETE_CATEGORY_MENU");
    m_CategoriesMenu.addAction(tr("Set category color"))->setData("SET_CATEGORY_COLOR_MENU");
    m_CategoriesMenu.addAction(tr("Hide activity"))->setData("HIDE_ACTIVITY");
    m_CategoriesMenu.addAction(tr("Show activity"))->setData("SHOW_ACTIVITY");
    m_CategoriesMenu.addAction(tr("Settings..."))->setData("APP_SETTINGS");
    m_CategoriesMenu.addMenu(&m_MoveToMenu)->setData("MOVE_TO_CATEGORY");
    connect(&m_CategoriesMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));

    ui->treeWidgetApplications->setAcceptDrops(true);
    ui->treeWidgetApplications->setDragEnabled(true);
    ui->treeWidgetApplications->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->treeWidgetApplications,SIGNAL(itemMoved(QTreeWidgetItem*,QTreeWidgetItem*)),this,SLOT(onApplicationMoved(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->treeWidgetApplications,SIGNAL(needRebuild()),this,SLOT(onDelayedRebuild()));

    connect(ui->treeWidgetApplications, SIGNAL (itemChanged(QTreeWidgetItem *, int)),this, SLOT (onCategoryChanged(QTreeWidgetItem *, int)));

    ui->treeWidgetApplications->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidgetApplications,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(onContextMenu(QPoint)));
}

ApplicationsWindow::~ApplicationsWindow()
{
    delete ui;
}

void ApplicationsWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent( event );
    m_ScrollPos = 0;
    m_CategoriesExpandedState.clear();
    rebuildProfilesList();
    rebuildApplicationsList();

    raise();
    activateWindow();
}

void ApplicationsWindow::onProfilesChange()
{
    rebuildProfilesList();
    rebuildApplicationsList();
}

void ApplicationsWindow::onApplicationsChange()
{
    rebuildApplicationsList();
}

void ApplicationsWindow::onCategoryChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY){
        int categoryIndex = item->data(0,Qt::UserRole).toInt();
        if (categoryIndex>-1)
            m_DataManager->setCategoryName(categoryIndex,item->text(0));
    }
}

void ApplicationsWindow::onContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeWidgetApplications->itemAt( pos );
    m_ContextMenuItem = item;
    bool canEditItem = false;
    bool canHideItem = false;
    bool canShowItem = false;
    bool canMoveToCategory = false;
    bool haveSettings = false;
    if (item){
        haveSettings = item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION;
        if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY)
            if (item->data(0,Qt::UserRole).toInt()>-1)
                canEditItem = true;
        if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION_ACTIVITY){
            canMoveToCategory = true;
            const sAppInfo* app = m_DataManager->applications(item->data(0,Qt::UserRole).toInt());
            if (app->activities[item->data(0,Qt::UserRole+1).toInt()].categories[m_DataManager->getCurrentProfileIndex()].visible)
                canHideItem = true;
            else
                canShowItem = true;
        }
        item->setSelected(true);
    }

    QList<QAction*> actions = m_CategoriesMenu.actions();
    for (int i = 0; i<actions.size(); i++){
        QString id = actions[i]->data().toString();
        if (id=="DELETE_CATEGORY_MENU" || id=="SET_CATEGORY_COLOR_MENU"){
            actions[i]->setVisible(canEditItem);
        }
        if (id=="HIDE_ACTIVITY"){
            actions[i]->setVisible(canHideItem);
        }
        if (id=="SHOW_ACTIVITY"){
            actions[i]->setVisible(canShowItem);
        }
        if (id=="APP_SETTINGS"){
            actions[i]->setVisible(haveSettings);
        }
        if (id=="MOVE_TO_CATEGORY"){
            actions[i]->setVisible(canMoveToCategory);
        }
    }

    m_CategoriesMenu.exec( ui->treeWidgetApplications->mapToGlobal(pos) );
}

void ApplicationsWindow::onMenuSelection(QAction *menuAction)
{
    QString id = menuAction->data().toString();
    if (id=="SHOW_ACTIVITY"){
        QList<QTreeWidgetItem*> items =  ui->treeWidgetApplications->selectedItems();
        for (int i = 0; i<items.size(); i++){
            QTreeWidgetItem* item = items[i];
            if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION_ACTIVITY){
                sAppInfo* app = m_DataManager->applications(item->data(0,Qt::UserRole).toInt());
                int activityIndex = item->data(0,Qt::UserRole+1).toInt();
                if (activityIndex>-1)
                    app->activities[activityIndex].categories[m_DataManager->getCurrentProfileIndex()].visible = true;
            }
        }
    }
    if (id=="HIDE_ACTIVITY"){
        QList<QTreeWidgetItem*> items =  ui->treeWidgetApplications->selectedItems();
        QVector<QTreeWidgetItem*> itemsToDelete;
        for (int i = 0; i<items.size(); i++){
            QTreeWidgetItem* item = items[i];
            if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION_ACTIVITY){
                sAppInfo* app = m_DataManager->applications(item->data(0,Qt::UserRole).toInt());
                int activityIndex = item->data(0,Qt::UserRole+1).toInt();
                if (activityIndex>-1){
                    app->activities[activityIndex].categories[m_DataManager->getCurrentProfileIndex()].visible = false;
                    if (!ui->checkBoxShowHidden->isChecked())
                        itemsToDelete.push_back(item);
                }
            }
        }
        for (int i = 0; i<itemsToDelete.size(); i++)
            delete itemsToDelete[i];
    }
    if (id=="APP_SETTINGS"){
        if (m_ContextMenuItem)
            if (m_ContextMenuItem->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION){
                int index = m_ContextMenuItem->data(0,Qt::UserRole).toInt();
                if (index>-1)
                    emit showAppSettings(index);
            }
        return;
    }
    if (id=="NEW_CATEGORY_MENU"){
        m_DataManager->addNewCategory(tr("New Category"),QColor::fromHsv(rand() % 255,rand() % 255,255));
        rebuildApplicationsList();
        return;
    }
    if (id=="DELETE_CATEGORY_MENU"){
        QList<QTreeWidgetItem*> items =  ui->treeWidgetApplications->selectedItems();
        if (items.size()==1){
            QTreeWidgetItem* item = items.first();
            if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY){
                int index = item->data(0,Qt::UserRole).toInt();
                if (index>-1){
                    m_DataManager->deleteCategory(index);
                }
            }
        }
        return;
    }
    if (id=="SET_CATEGORY_COLOR_MENU"){
        QList<QTreeWidgetItem*> items =  ui->treeWidgetApplications->selectedItems();
        if (items.size()==1){
            QTreeWidgetItem* item = items.first();
            if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY){
                int index = item->data(0,Qt::UserRole).toInt();
                if (index>-1){
                    QColor newColor = QColorDialog::getColor(m_DataManager->categories(index)->color);
                    if (newColor.isValid()){
                        m_DataManager->setCategoryColor(index,newColor);
                        QPixmap pixmap(16,16);
                        pixmap.fill(newColor);
                        QIcon icon(pixmap);
                        item->setIcon(0,icon);
                    }
                }
            }
        }
        return;
    }
}

void ApplicationsWindow::onMoveToMenuSelection(QAction *menuAction)
{
    QList<QTreeWidgetItem*> items =  ui->treeWidgetApplications->selectedItems();
    for (int i = 0; i<items.size(); i++){
        QTreeWidgetItem* item = items[i];
        if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION_ACTIVITY){
            sAppInfo* app = m_DataManager->applications(item->data(0,Qt::UserRole).toInt());
            int activityIndex = item->data(0,Qt::UserRole+1).toInt();
            app->activities[activityIndex].categories[m_DataManager->getCurrentProfileIndex()].category = menuAction->data().toInt();
        }
    }
    onDelayedRebuild();
}

void ApplicationsWindow::onProfileSelection(int newProfileIndex)
{
    if (m_LoadingData)
        return;
    if (newProfileIndex>-1)
        m_DataManager->setCurrentProfileIndex(newProfileIndex);
}

void ApplicationsWindow::onApplicationMoved(QTreeWidgetItem* item,QTreeWidgetItem* newParent)
{            
    if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION_ACTIVITY){
        m_DataManager->setApplicationActivityCategory(QApplication::keyboardModifiers()==Qt::ControlModifier?-1:m_DataManager->getCurrentProfileIndex(), item->data(0,Qt::UserRole).toInt(), item->data(0,Qt::UserRole+1).toInt(), newParent->data(0,Qt::UserRole).toInt());
    }
}

void ApplicationsWindow::onDelayedRebuild()
{
    QTimer::singleShot(10,this,SLOT(onApplicationsChange()));
}
