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
#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include <QMainWindow>
#include "../data/cdatamanager.h"
#include "../tools/tools.h"

enum eMouseBehavior{
    NO_ACTIONS = 0,
    HIDE_ON_CLICK,
    HIDE_ON_MOVE,
    ESCAPE
};

enum eCategorySelectionBehavior{
    ALWAYS_HIDE = 0,
    ALWAYS_VISIBLE,
    IF_CATEGORY_NOT_SET
};

enum eVisibilityBehavior{
    ON_CATEGORY = 0,
    ON_ACTIVITY,
    ON_ANY,
    ON_MENU
};

namespace Ui {
class NotificationWindow;
}

class NotificationWindow : public QMainWindow
{
    Q_OBJECT
protected:
    cDataManager*       m_DataManager;
    QString             m_ConfMessageFormat;
    QPoint              m_ConfPosition;    
    QPoint              m_ConfSize;
    QPoint              m_EscapePos;
    int                 m_ConfOpacity;
    int                 m_ConfDelay;

    eMouseBehavior      m_MouseBehavior;
    eCategorySelectionBehavior m_CategorySelectionBehavior;
    eVisibilityBehavior m_VisibilityBehavior;

    cStatisticResolver* m_Statistic;

    QTimer              m_Timer;
    int                 m_TimerCounter;
    bool                m_ClosingInterrupted;
    bool                m_CanCloseInterrupt;

    int                 m_AppIndex;
    int                 m_ActivityIndex;
    int                 m_Category;
public:
    explicit NotificationWindow(cDataManager* dataManager, cStatisticResolver* statistic);
    ~NotificationWindow();

private:
    Ui::NotificationWindow *ui;
protected:
    virtual void enterEvent(QEvent * event) override;
    virtual bool eventFilter(QObject *object, QEvent *event) override;
    virtual void focusInEvent(QFocusEvent * event) override;
    void stop();
protected slots:
    void onButtonSetCurrent();
    void onButtonSetAll();
    void onTimeout();
    void onEscapeTimer();
public slots:    
    void onPreferencesChanged();
    void onShow();
};

#endif // NOTIFICATIONWINDOW_H
