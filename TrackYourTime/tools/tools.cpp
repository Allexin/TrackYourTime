#include "tools.h"

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

const QString CURRENT_VERSION = "0.9.2.1";

QMap<QString,QString> loadPairsFile(const QString& fileName){
    QMap<QString,QString> list;
    QFile textFile(fileName);
    if (textFile.open(QIODevice::ReadOnly)){
        QTextStream textStream(&textFile);
        while (true)
        {
            QString line = textStream.readLine();
            QStringList pair = line.split("==");
            if (pair.size()==2)
                list[pair[0]]=pair[1];

            if (line.isNull())
                break;
        }
    }
    return list;
}

QString readFile(const QString &fileName)
{
    QFile textFile(fileName);
    if (textFile.open(QIODevice::ReadOnly)){
        return textFile.readAll();
    }
    return QString();
}

cSettings::cSettings()
{
    QString OSName = "UNKNOWN";
#ifdef Q_OS_LINUX
    OSName="Linux";
#else
    #ifdef Q_OS_WIN
        OSName="Windows";
    #else
        #ifdef Q_OS_MAC
            OSName="MacOSX";
        #endif
    #endif
#endif
    if (QFileInfo("settings_"+OSName+".conf").exists())
        m_Settings = new QSettings("settings_"+OSName+".conf",QSettings::IniFormat);
    else
    if (QFileInfo("settings.conf").exists())
        m_Settings = new QSettings("settings.conf",QSettings::IniFormat);
    else
        m_Settings = new QSettings();
}

cSettings::~cSettings()
{
    delete m_Settings;
}

QSettings *cSettings::db()
{
    return m_Settings;
}
