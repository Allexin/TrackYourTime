/*
 * TrackYourTime - cross-platform time tracker
 * Copyright (C) 2015-2017  Alexander Basov <basovav@gmail.com>
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

#include "cfilebin.h"

int cFileBin::readInt()
{
    int value;
    read((char*)&value,sizeof(int));
    return value;
}

uint cFileBin::readUint()
{
    uint value;
    read((char*)&value,sizeof(uint));
    return value;
}

QString cFileBin::readString()
{
    QString value;
    int size = readInt();
    if (size>0){
        char* utf8 = new char[size];
        read(utf8,size);
        value = QString::fromUtf8(utf8,size);
        delete [] utf8;
    }
    return value;
}

QString cFileBin::readUtf8Line()
{
    QByteArray buffer;
    char c = '\0';
    do{
        if (c!='\0')
            buffer.push_back(c);
        if (read(&c,1)<=0)
            break;

    }while (c!='\0' && c!='\n' && c!='\r');
    return QString::fromUtf8(buffer);
}

void cFileBin::writeInt(int value)
{
    write((char*)&value,sizeof(int));
}

void cFileBin::writeUint(uint value)
{
    write((char*)&value,sizeof(uint));
}

void cFileBin::writeString(const QString &value)
{
    QByteArray data = value.toUtf8();
    writeInt(data.size());
    if (data.size()>0)
        write(data.constData(),data.size());
}
