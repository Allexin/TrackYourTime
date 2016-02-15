#ifndef CSCHEDULE_H
#define CSCHEDULE_H

#include <QObject>
#include <QRegExp>
#include <QTimer>
#include <QString>
#include "cdatamanager.h"

class cScheduleItem: public QObject{
    Q_OBJECT
public:
    enum eScheduleAction{
        SA_SET_PROFILE=0,
        SA_COUNT
    };
    static const QString scheduleActionNames[SA_COUNT];
protected:
    eScheduleAction     m_Action;
    QString             m_Param;
    QRegExp             m_RegExp;
public:
    explicit cScheduleItem(eScheduleAction action, const QString& param, QString regexp);
    void process(const QString& currentDateTime, cDataManager* dataManager);

    eScheduleAction action() const{return m_Action;}
    QString param() const{return m_Param;}
    QString condition() const{return m_RegExp.pattern();}
};

class cSchedule : public QObject
{
    Q_OBJECT
protected:
    QVector<cScheduleItem*> m_Items;
    QString             m_PreviousDateTime;
    QTimer              m_Timer;
    cDataManager*       m_DataManager;
    QString getCurrentDateTime();
    void save();
    void load();
public:
    explicit cSchedule(cDataManager* dataManager, QObject *parent = 0);
    virtual ~cSchedule();

    void start();

    int getItemsCount();
    const cScheduleItem* getItem(int index);
    void deleteItem(int index);
    void addItem(cScheduleItem::eScheduleAction action, const QString& param, const QString& regexp);

    QString getSample(){return getCurrentDateTime();}
signals:

protected slots:
    void timer();
};

#endif // CSCHEDULE_H
