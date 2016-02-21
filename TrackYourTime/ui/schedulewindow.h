#ifndef SCHEDULEWINDOW_H
#define SCHEDULEWINDOW_H

#include <QMainWindow>
#include <QShowEvent>
#include "../data/cdatamanager.h"
#include "../data/cschedule.h"


namespace Ui {
class ScheduleWindow;
}

class ScheduleWindow : public QMainWindow
{
    Q_OBJECT
protected:
    cDataManager*       m_DataManager;
    cSchedule*          m_Schedule;
public:
    explicit ScheduleWindow(cDataManager* dataManager, cSchedule* schedule);
    ~ScheduleWindow();

public slots:
    void rebuild();
    void updateTimeSample();
    void add();
    void remove();
protected:
    virtual void showEvent(QShowEvent * event) override;
private:
    Ui::ScheduleWindow *ui;
};

#endif // SCHEDULEWINDOW_H
