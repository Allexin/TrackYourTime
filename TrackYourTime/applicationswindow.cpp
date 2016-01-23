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

#include "applicationswindow.h"
#include "ui_applicationswindow.h"
#include <QColorDialog>

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

QTreeWidgetItem* createTreeItemCategory(int index, QColor color, const QString& text){
    QPixmap pixmap(16,16);
    pixmap.fill(color);
    QIcon icon(pixmap);

    QTreeWidgetItem* item = new QTreeWidgetItem(cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY);
    item->setText(0,text);
    item->setIcon(0,icon);
    item->setData(0,Qt::UserRole,index);
    return item;
}

void ApplicationsWindow::rebuildApplicationsList()
{
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

    //place applications in category
    int currentProfile = m_DataManager->getCurrentProfileIndex();
    for (int i = 0; i<m_DataManager->applicationsCount(); i++){
        const sAppInfo* app = m_DataManager->applications(i);

        QTreeWidgetItem* item = new QTreeWidgetItem(cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION);
        item->setText(0,app->name);
        item->setToolTip(0,app->path+"/"+app->name);
        item->setData(0,Qt::UserRole,i);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        if (app->categories[currentProfile]==-1)
            uncategorized->addChild(item);
        else
            categories[app->categories[currentProfile]]->addChild(item);
    }

    //expand all categories
    uncategorized->setExpanded(true);
    for (int i = 0; i<categories.size(); i++)
        categories[i]->setExpanded(true);
}

ApplicationsWindow::ApplicationsWindow(cDataManager *DataManager) : QMainWindow(0),
    ui(new Ui::ApplicationsWindow)
{
    ui->setupUi(this);
    m_DataManager = DataManager;
    m_LoadingData = false;


    connect(ui->comboBoxProfiles, SIGNAL(currentIndexChanged(int)), this, SLOT(onProfileSelection(int)));

    connect(ui->pushButtonEditProfiles, SIGNAL(released()), this, SLOT(onEditProfiles()));


    m_CategoriesMenu.addAction(tr("New category"))->setData("NEW_CATEGORY_MENU");
    m_CategoriesMenu.addAction(tr("Delete category"))->setData("DELETE_CATEGORY_MENU");
    m_CategoriesMenu.addAction(tr("Set category color"))->setData("SET_CATEGORY_COLOR_MENU");
    connect(&m_CategoriesMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));

    ui->treeWidgetApplications->setAcceptDrops(true);
    ui->treeWidgetApplications->setDragEnabled(true);
    ui->treeWidgetApplications->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->treeWidgetApplications,SIGNAL(itemMoved(QTreeWidgetItem*,QTreeWidgetItem*)),this,SLOT(onApplicationMoved(QTreeWidgetItem*,QTreeWidgetItem*)));

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
    rebuildProfilesList();
    rebuildApplicationsList();
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
    if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY){
        int categoryIndex = item->data(0,Qt::UserRole).toInt();
        if (categoryIndex>-1)
            m_DataManager->setCategoryName(categoryIndex,item->text(0));
    }
}

void ApplicationsWindow::onContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeWidgetApplications->itemAt( pos );
    bool canEditItem = false;
    if (item){
        if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_CATEGORY)
            if (item->data(0,Qt::UserRole).toInt()>-1)
                canEditItem = true;
        item->setSelected(true);
    }

    QList<QAction*> actions = m_CategoriesMenu.actions();
    for (int i = 0; i<actions.size(); i++){
        QString id = actions[i]->data().toString();
        if (id=="DELETE_CATEGORY_MENU" || id=="SET_CATEGORY_COLOR_MENU"){
            actions[i]->setVisible(canEditItem);
        }
    }\

    QPoint pt(pos);
    m_CategoriesMenu.exec( ui->treeWidgetApplications->mapToGlobal(pos) );
}

void ApplicationsWindow::onMenuSelection(QAction *menuAction)
{
    QString id = menuAction->data().toString();
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

void ApplicationsWindow::onProfileSelection(int newProfileIndex)
{
    if (m_LoadingData)
        return;
    if (newProfileIndex>-1)
        m_DataManager->setCurrentProfileIndex(newProfileIndex);
}

void ApplicationsWindow::onApplicationMoved(QTreeWidgetItem* item,QTreeWidgetItem* newParent)
{            
    if (item->type()==cApplicationsTreeWidget::TREE_ITEM_TYPE_APPLICATION){
        m_DataManager->setApplicationCategory(QApplication::keyboardModifiers()==Qt::ControlModifier?-1:m_DataManager->getCurrentProfileIndex(), item->data(0,Qt::UserRole).toInt(), newParent->data(0,Qt::UserRole).toInt());
    }
}
