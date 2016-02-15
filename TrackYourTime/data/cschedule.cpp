#include "cschedule.h"
#include <QDateTime>
#include "../tools/tools.h"

const QString cScheduleItem::scheduleActionNames[cScheduleItem::SA_COUNT] = {tr("Set profile")};

QString cSchedule::getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("ddd yyyy.mm.dd HH:mm");
}

void cSchedule::save()
{
    cSettings settings;
    settings.db()->setValue("schedule/count",m_Items.size());
    for (int i = 0; i<m_Items.size(); i++){
        QString settingsKey = "schedule"+QString().setNum(i)+"/";
        settings.db()->setValue(settingsKey+"action",m_Items[i]->action());
        settings.db()->setValue(settingsKey+"param",m_Items[i]->param());
        settings.db()->setValue(settingsKey+"regexp",m_Items[i]->condition());
    }
    settings.db()->sync();
}

void cSchedule::load()
{
    for (int i = 0; i<m_Items.size(); i++)
        delete m_Items[i];

    cSettings settings;
    m_Items.resize(settings.db()->value("schedule/count",0).toInt());
    for (int i = 0; i<m_Items.size(); i++){
        QString settingsKey = "schedule"+QString().setNum(i)+"/";
        cScheduleItem::eScheduleAction action = (cScheduleItem::eScheduleAction)settings.db()->value(settingsKey+"action").toInt();
        QString param = settings.db()->value(settingsKey+"param").toString();
        QString regexp = settings.db()->value(settingsKey+"regexp").toString();
        m_Items[i] = new cScheduleItem(action,param,regexp);
    }
}

cSchedule::cSchedule(cDataManager *dataManager, QObject *parent) : QObject(parent)
{
    m_DataManager = dataManager;
    m_PreviousDateTime = "";

    load();
    connect(&m_Timer,SIGNAL(timeout()),this,SLOT(timer()));
}

cSchedule::~cSchedule()
{
    for (int i = 0; i<m_Items.size(); i++)
        delete m_Items[i];
}

void cSchedule::start()
{
    m_Timer.start(55000);
}

int cSchedule::getItemsCount()
{
    return m_Items.size();
}

const cScheduleItem *cSchedule::getItem(int index)
{
    if (index<0 || index>=m_Items.size())
        return NULL;
    return m_Items[index];
}

void cSchedule::deleteItem(int index)
{
    if (index<0 || index>=m_Items.size())
        return;
    delete m_Items[index];
    m_Items.remove(index);
    save();
}

void cSchedule::addItem(cScheduleItem::eScheduleAction action, const QString &param, const QString &regexp)
{
    m_Items.push_back(new cScheduleItem(action,param,regexp));
    save();
}

void cSchedule::timer()
{
    if (m_Items.size()==0)
        return;

    QString currentDateTime = getCurrentDateTime();
    if (currentDateTime==m_PreviousDateTime)
        return;
    m_PreviousDateTime = currentDateTime;

    for (int i = 0; i<m_Items.size(); i++)
        m_Items[i]->process(currentDateTime, m_DataManager);
}




cScheduleItem::cScheduleItem(eScheduleAction action, const QString &param, QString regexp):
    QObject(0),
    m_Action(action),
    m_Param(param),
    m_RegExp(regexp)
{

}

void cScheduleItem::process(const QString &currentDateTime, cDataManager* dataManager)
{
    if (m_RegExp.exactMatch(currentDateTime)){
        switch(m_Action){
            case SA_SET_PROFILE:{
                dataManager->setCurrentProfileIndexSafe(m_Param.toInt());
            }
            break;
        }
    }
}
