#ifndef UPDATEAVAILABLEWINDOW_H
#define UPDATEAVAILABLEWINDOW_H

#include <QMainWindow>

namespace Ui {
class UpdateAvailableWindow;
}

class UpdateAvailableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateAvailableWindow(QWidget *parent = 0);
    ~UpdateAvailableWindow();

private:
    Ui::UpdateAvailableWindow *ui;
signals:
    void ignoreUpdate();
protected slots:
    void processButtonDownload();
    void processButtonChangelog();
    void processButtonRemindLater();
    void processButtonIgnoreUpdate();
public slots:
    void showUpdate(QString newVersion);
};

#endif // UPDATEAVAILABLEWINDOW_H
