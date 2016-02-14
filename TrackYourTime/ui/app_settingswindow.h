#ifndef APP_SETTINGSWINDOW_H
#define APP_SETTINGSWINDOW_H

#include <QMainWindow>
#include "../data/cdatamanager.h"

namespace Ui {
class App_SettingsWindow;
}

class App_SettingsWindow : public QMainWindow
{
    Q_OBJECT

protected:
    cDataManager*       m_DataManager;
    int                 m_AppIndex;
public:
    explicit App_SettingsWindow(cDataManager* DataManager);
    ~App_SettingsWindow();

private:
    Ui::App_SettingsWindow *ui;
private slots:
    void onApply();
    void onSetDebug();
    void onScriptResult(QString value, const sSysInfo& data);
public slots:
    void showApp(int appIndex);
};

#endif // APP_SETTINGSWINDOW_H
