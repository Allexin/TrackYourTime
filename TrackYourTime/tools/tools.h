#ifndef TOOLS_H
#define TOOLS_H

#include <QMap>
#include <QString>

QMap<QString,QString> loadPairsFile(const QString& fileName);
QString readFile(const QString& fileName);

#endif // TOOLS_H
