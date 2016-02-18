#ifndef NOTIFICATION_DUMMY_H
#define NOTIFICATION_DUMMY_H

#include <QMainWindow>

namespace Ui {
class notification_dummy;
}

class notification_dummy : public QMainWindow
{
    Q_OBJECT

public:
    explicit notification_dummy(QWidget *parent = 0);
    ~notification_dummy();

    void showWithMessage(const QString& format);
private:
    Ui::notification_dummy *ui;
signals:
    void onApplyPosAndSize();
public slots:
    void onButtonApply();
};

#endif // NOTIFICATION_DUMMY_H
