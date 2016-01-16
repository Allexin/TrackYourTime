/*
 * TrackYourTime - cross-platform time tracker
 * Copyright (C) 2015-2016  Alexander Basov <basovav@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CFILEBIN_H
#define CFILEBIN_H

#include <QFile>
#include <QString>

class cFileBin : public QFile
{
    Q_OBJECT
public:
    cFileBin(const QString& FileName):QFile(FileName){}

    int readInt();
    uint readUint();
    QString readString();
    QString readUtf8Line();

    void writeInt(int value);
    void writeUint(uint value);
    void writeString(const QString& value);
};

#endif // CFILEBIN_H
