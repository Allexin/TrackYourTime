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
#include "notificationwindow.h"
#include "ui_notificationwindow.h"
#include "../tools/tools.h"
#include <QPalette>

NotificationWindow::NotificationWindow(cDataManager *dataManager, cStatisticResolver* statistic) :
    QMainWindow(0,Qt::Dialog),
    m_Statistic(statistic),
    ui(new Ui::NotificationWindow)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    ui->setupUi(this);
    m_DataManager = dataManager;
    onPreferencesChanged();

    connect(&m_Timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
    connect(ui->pushButtonSetFoCurrentProfile,SIGNAL(released()),this,SLOT(onButtonSetCurrent()));
    connect(ui->pushButtonSetForAllProfiles,SIGNAL(released()),this,SLOT(onButtonSetAll()));
    ui->centralwidget->installEventFilter(this);
    ui->groupBoxCategory->installEventFilter(this);
    ui->labelMessage->installEventFilter(this);
    ui->pushButtonSetFoCurrentProfile->installEventFilter(this);
    ui->pushButtonSetForAllProfiles->installEventFilter(this);
    ui->comboBoxCategory->installEventFilter(this);
}

NotificationWindow::~NotificationWindow()
{
    delete ui;
}

void NotificationWindow::enterEvent(QEvent *event)
{
    QMainWindow::enterEvent(event);
    if (m_ClosingInterrupted)
        return;
    if (m_MouseBehavior==eMouseBehavior::NO_ACTIONS || m_MouseBehavior==eMouseBehavior::HIDE_ON_CLICK)
        return;
    if (m_MouseBehavior==eMouseBehavior::HIDE_ON_MOVE){
        stop();
        return;
    }
    if (m_MouseBehavior==eMouseBehavior::ESCAPE){
        onEscapeTimer();
        return;
    }

    //WTF??
}

bool NotificationWindow::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)
    if (event->type()==QEvent::MouseButtonPress && m_CanCloseInterrupt){
        m_ClosingInterrupted = true;
        setWindowOpacity(1.0);
    }
    return false;
}

void NotificationWindow::focusInEvent(QFocusEvent *event)
{
    m_ClosingInterrupted = true;
    QMainWindow::focusInEvent(event);
}

void NotificationWindow::stop()
{
    m_ClosingInterrupted = false;
    if (m_ConfDelay==0)
        return;
    hide();
    m_Timer.stop();
}

void NotificationWindow::onButtonSetCurrent()
{
    if (m_AppIndex>-1){
        m_DataManager->applications(m_AppIndex)->activities[m_ActivityIndex].categories[m_DataManager->getCurrentProfileIndex()].category = ui->comboBoxCategory->currentIndex();
    }
    stop();
}

void NotificationWindow::onButtonSetAll()
{
    if (m_AppIndex>-1){
        for (int i = 0; i<m_DataManager->profilesCount(); i++)
            m_DataManager->applications(m_AppIndex)->activities[m_ActivityIndex].categories[i].category = ui->comboBoxCategory->currentIndex();
    }
    stop();
}

void NotificationWindow::onTimeout()
{
    if (m_ConfDelay==0)
        return;
    if (m_ClosingInterrupted){
        m_Timer.stop();
        return;
    }
    m_TimerCounter++;
    if (m_TimerCounter>=m_ConfDelay){
        stop();
    }
}

void NotificationWindow::onEscapeTimer()
{
    if (m_MouseBehavior!=eMouseBehavior::ESCAPE){
        return;
    }
    QPoint cursorPos = QCursor::pos();
    QRect r(m_EscapePos,QSize(width(),height()));
    if (!r.contains(cursorPos)){
        return;
    }

    int center = m_ConfPosition.y() + height()/2;
    if (abs(cursorPos.y()<center)){
        m_EscapePos.setY(cursorPos.y()+1);
    }
    else{
        m_EscapePos.setY(cursorPos.y()-1-height());
    }
    setGeometry(m_EscapePos.x(),m_EscapePos.y(),m_ConfSize.x(),0);
}

void NotificationWindow::onPreferencesChanged()
{
    cSettings settings;
    m_ConfMessageFormat = settings.db()->value(cDataManager::CONF_NOTIFICATION_MESSAGE_ID).toString();
    m_ConfPosition = settings.db()->value(cDataManager::CONF_NOTIFICATION_POSITION_ID).toPoint();
    m_ConfSize = settings.db()->value(cDataManager::CONF_NOTIFICATION_SIZE_ID).toPoint();
    m_ConfOpacity = settings.db()->value(cDataManager::CONF_NOTIFICATION_OPACITY_ID).toInt();
    m_ConfDelay = settings.db()->value(cDataManager::CONF_NOTIFICATION_HIDE_SECONDS_ID).toInt();


    m_MouseBehavior  = (eMouseBehavior)settings.db()->value(cDataManager::CONF_NOTIFICATION_MOUSE_BEHAVIOR_ID,1).toInt();
    m_CategorySelectionBehavior = (eCategorySelectionBehavior)settings.db()->value(cDataManager::CONF_NOTIFICATION_CAT_SELECT_BEHAVIOR_ID,2).toInt();
    m_VisibilityBehavior = (eVisibilityBehavior)settings.db()->value(cDataManager::CONF_NOTIFICATION_VISIBILITY_BEHAVIOR_ID,0).toInt();

    if (settings.db()->value(cDataManager::CONF_NOTIFICATION_HIDE_BORDERS_ID,false).toBool()){
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
    }
    else{
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint | Qt::Tool);
    }
}

void NotificationWindow::onShow()
{
    if (!isVisible()){
        m_ClosingInterrupted = false;
    }

    if (m_ClosingInterrupted && isVisible())
        return;

    bool needResetTimer = !m_Timer.isActive();
    bool canShow = isVisible();
    switch(m_VisibilityBehavior){
        case eVisibilityBehavior::ON_CATEGORY:{
            int appIndex = m_DataManager->getCurrentAppliction();
            if (appIndex >-1){
                int activityIndex = m_DataManager->getCurrentApplictionActivity();
                if (activityIndex>-1){
                    int profile = m_DataManager->getCurrentProfileIndex();
                    sAppInfo* info = m_DataManager->applications(appIndex);
                    int category = info->activities[activityIndex].categories[profile].category;
                    if (category!=m_Category || category==-1 && (m_AppIndex!=appIndex || m_ActivityIndex!=activityIndex)){
                        canShow = true;
                        needResetTimer = needResetTimer || category!=m_Category ;
                    }
                }
            }
        };break;
        case eVisibilityBehavior::ON_ACTIVITY:{
            int appIndex = m_DataManager->getCurrentAppliction();
            if (m_AppIndex != appIndex){
                canShow = true;
                needResetTimer = true;
            }
            else{
                if (appIndex>-1){
                    int activityIndex = m_DataManager->getCurrentApplictionActivity();
                    if (activityIndex!=m_ActivityIndex){
                        canShow = true;
                        needResetTimer = true;
                    }
                }
            }
        };break;
        case eVisibilityBehavior::ON_ANY:{
            canShow = true;
        };break;
        case eVisibilityBehavior::ON_MENU:{
        };break;
    }

    if (!canShow)
        return;
    QString appName = tr("UNKNOWN");
    QString appState = tr("UNKNOWN");
    QString appCategory = tr("NONE");
    int profile = m_DataManager->getCurrentProfileIndex();
    m_Category = -1;
    m_AppIndex = m_DataManager->getCurrentAppliction();
    if (m_AppIndex>-1){
        sAppInfo* info = m_DataManager->applications(m_AppIndex);
        appName = info->activities[0].name;
        m_ActivityIndex = m_DataManager->getCurrentApplictionActivity();
        if (m_ActivityIndex==0)
            appState=tr("default");
        else
            appState=info->activities[m_ActivityIndex].name;
        m_Category = info->activities[m_ActivityIndex].categories[profile].category;
        if (m_Category==-1)
            appCategory=tr("Uncategorized");
        else
            appCategory=m_DataManager->categories(m_Category)->name;
    }

    QString todayTotalTime;
    QString todayApplicationTime;
    QString todayActivityTime;
    QString todayCategoryTime;
    if (m_Statistic){
        if (m_Statistic->isTodayStatisticAvailable()){
            todayTotalTime = DurationToString(m_Statistic->getTodayTotalTime());
            todayApplicationTime = DurationToString(m_Statistic->getTodayApplicationTime(m_AppIndex));
            todayActivityTime = DurationToString(m_Statistic->getTodayActivityTime(m_AppIndex,m_ActivityIndex));
            todayCategoryTime = DurationToString(m_Statistic->getTodayCategoryTime(m_Category));
        }
    }

    QString message = m_ConfMessageFormat;
    message = message.replace("%PROFILE%",m_DataManager->profiles(profile)->name);
    message = message.replace("%APP_NAME%",appName);
    message = message.replace("%APP_STATE%",appState);
    message = message.replace("%APP_CATEGORY%",appCategory);
    message = message.replace("%TODAY_TIME%",todayTotalTime);
    message = message.replace("%TODAY_APP_TIME%",todayApplicationTime);
    message = message.replace("%TODAY_STATE_TIME%",todayActivityTime);
    message = message.replace("%TODAY_CATEGORY_TIME%",todayCategoryTime);

    QColor catColor = m_Category==-1?Qt::gray:m_DataManager->categories(m_Category)->color;
    QColor catColorText = catColor.lightness()<127?Qt::white:Qt::black;
    message = "<font color="+catColorText.name()+">"+message+"</font>";

    ui->labelMessage->setText(message);

    bool showCategorySelection = false;
    switch(m_CategorySelectionBehavior){
        case eCategorySelectionBehavior::ALWAYS_HIDE:{
            showCategorySelection = false;
        };break;
        case eCategorySelectionBehavior::ALWAYS_VISIBLE:{
            showCategorySelection = m_AppIndex>-1;
        };break;
        case eCategorySelectionBehavior::IF_CATEGORY_NOT_SET:{
            showCategorySelection = m_Category==-1 && m_AppIndex>-1;
        };break;
    }

    if (showCategorySelection){
        ui->comboBoxCategory->clear();
        for (int i = 0; i<m_DataManager->categoriesCount(); i++)
            ui->comboBoxCategory->addItem(m_DataManager->categories(i)->name);
        ui->comboBoxCategory->setCurrentIndex(m_Category);

        ui->groupBoxCategory->setVisible(true);
        m_CanCloseInterrupt = true;
    }
    else{
        ui->groupBoxCategory->setVisible(false);
        m_CanCloseInterrupt = false;
    }
    setWindowOpacity(m_ConfOpacity==100?1.0:m_ConfOpacity/100.f);    

    QPalette Pal(palette());

    if (m_Category>-1)
        Pal.setColor(QPalette::Background, m_DataManager->categories(m_Category)->color);
    else
        Pal.setColor(QPalette::Background, Qt::gray);
    setAutoFillBackground(true);
    setPalette(Pal);

    if (needResetTimer){
        m_TimerCounter = 0;
        m_ClosingInterrupted = false;
        m_Timer.start(1000);
    }

    if (!isVisible()){
        m_EscapePos = m_ConfPosition;
        show();
        setGeometry(m_EscapePos.x(),m_EscapePos.y(),m_ConfSize.x(),0);
    }
}

