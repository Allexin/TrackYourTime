#ifndef TOOLS_H
#define TOOLS_H

#include <QMap>
#include <QString>
#include <QSettings>

QMap<QString,QString> loadPairsFile(const QString& fileName);
QString readFile(const QString& fileName);

class cSettings{
protected:
    QSettings*          m_Settings;
public:
    cSettings();
    ~cSettings();

    QSettings* db();
};

#endif // TOOLS_H
