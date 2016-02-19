#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include <QMainWindow>
#include "../data/cdatamanager.h"

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
    int                 m_ConfOpacity;
    int                 m_ConfDelay;
    int                 m_ConfMoves;

    QTimer              m_Timer;
    int                 m_TimerCounter;
    bool                m_ClosingInterrupted;
    int                 m_EntersCount;
    bool                m_CanCloseInterrupt;

    int                 m_AppIndex;
    int                 m_ActivityIndex;
public:
    explicit NotificationWindow(cDataManager* dataManager);
    ~NotificationWindow();

private:
    Ui::NotificationWindow *ui;
protected:
    virtual void enterEvent(QEvent * event) override;
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void focusInEvent(QFocusEvent * event) override;
    void stop();
protected slots:
    void onButtonApply();
    void onTimeout();
public slots:
    void onPreferencesChanged();
    void onShow();
};

#endif // NOTIFICATIONWINDOW_H
