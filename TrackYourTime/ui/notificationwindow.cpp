#include "notificationwindow.h"
#include "ui_notificationwindow.h"
#include "../tools/tools.h"

NotificationWindow::NotificationWindow(cDataManager *dataManager) :
    QMainWindow(0),
    ui(new Ui::NotificationWindow)
{
    setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    ui->setupUi(this);
    m_DataManager = dataManager;
    onPreferencesChanged();

    connect(&m_Timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
    connect(ui->pushButtonApply,SIGNAL(released()),this,SLOT(onButtonApply()));
    ui->centralwidget->installEventFilter(this);
    ui->groupBoxCategory->installEventFilter(this);
    ui->labelMessage->installEventFilter(this);
    ui->pushButtonApply->installEventFilter(this);
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
    if (m_ConfMoves>0){
        m_EntersCount++;
        if (m_EntersCount>=m_ConfMoves)
            stop();
    }
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
    hide();
    m_Timer.stop();
}

void NotificationWindow::onButtonApply()
{
    if (m_AppIndex>-1){
        if (ui->checkBoxAllProfiles->isChecked()){
            for (int i = 0; i<m_DataManager->profilesCount(); i++)
                m_DataManager->applications(m_AppIndex)->activities[m_ActivityIndex].categories[i] = ui->comboBoxCategory->currentIndex();
        }
        else{
            m_DataManager->applications(m_AppIndex)->activities[m_ActivityIndex].categories[m_DataManager->getCurrentProfileIndex()] = ui->comboBoxCategory->currentIndex();
        }
    }
    stop();
}

void NotificationWindow::onTimeout()
{
    if (m_ClosingInterrupted){
        m_Timer.stop();
        return;
    }
    m_TimerCounter++;
    if (m_TimerCounter>=m_ConfDelay){
        stop();
    }
}

void NotificationWindow::onPreferencesChanged()
{
    cSettings settings;
    m_ConfMessageFormat = settings.db()->value(cDataManager::CONF_NOTIFICATION_MESSAGE_ID).toString();
    m_ConfPosition = settings.db()->value(cDataManager::CONF_NOTIFICATION_POSITION_ID).toPoint();
    m_ConfSize = settings.db()->value(cDataManager::CONF_NOTIFICATION_SIZE_ID).toPoint();
    m_ConfOpacity = settings.db()->value(cDataManager::CONF_NOTIFICATION_OPACITY_ID).toInt();
    m_ConfDelay = settings.db()->value(cDataManager::CONF_NOTIFICATION_HIDE_SECONDS_ID).toInt();
    m_ConfMoves = settings.db()->value(cDataManager::CONF_NOTIFICATION_HIDE_MOVES_ID).toInt();
}

void NotificationWindow::onShow()
{
    if (m_ClosingInterrupted && isVisible())
        return;
    QString appName = tr("UNKNOWN");
    QString appState = tr("UNKNOWN");
    QString appCategory = tr("NONE");
    int profile = m_DataManager->getCurrentProfileIndex();
    int category = -1;
    m_AppIndex = m_DataManager->getCurrentAppliction();
    if (m_AppIndex>-1){
        sAppInfo* info = m_DataManager->applications(m_AppIndex);
        appName = info->activities[0].name;
        m_ActivityIndex = m_DataManager->getCurrentApplictionActivity();
        if (m_ActivityIndex==0)
            appState=tr("default");
        else
            appState=info->activities[m_ActivityIndex].name;
        category = info->activities[m_ActivityIndex].categories[profile];
        if (category==-1)
            appCategory=tr("Uncategorized");
        else
            appCategory=m_DataManager->categories(category)->name;
    }
    QString message = m_ConfMessageFormat;
    message = message.replace("%PROFILE%",m_DataManager->profiles(profile)->name);
    message = message.replace("%APP_NAME%",appName);
    message = message.replace("%APP_STATE%",appState);
    message = message.replace("%APP_CATEGORY%",appCategory);
    ui->labelMessage->setText(message);

    if (category==-1 && m_AppIndex>-1 && m_ConfMoves!=1){
        ui->comboBoxCategory->clear();
        for (int i = 0; i<m_DataManager->categoriesCount(); i++)
            ui->comboBoxCategory->addItem(m_DataManager->categories(i)->name);
        ui->comboBoxCategory->setCurrentIndex(category);

        ui->groupBoxCategory->setVisible(true);
        m_CanCloseInterrupt = true;
    }
    else{
        ui->groupBoxCategory->setVisible(false);
        m_CanCloseInterrupt = false;
    }
    setWindowOpacity(m_ConfOpacity==100?1.0:m_ConfOpacity/100.f);
    setGeometry(m_ConfPosition.x(),m_ConfPosition.y(),m_ConfSize.x(),0);

    m_TimerCounter = 0;
    m_ClosingInterrupted = false;
    m_EntersCount = 0;
    m_Timer.start(1000);

    show();
}
