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
#include "cdbversionconverter.h"
#include "../tools/cfilebin.h"
#include <QDebug>
#include "cdatamanager.h"

const char* FILE_FORMAT_PREFIX = "TYTDB";

int getDBVersion(const QString& FileName){
    int Version = -1;
    cFileBin file(FileName);
    if (file.open(QIODevice::ReadOnly)){
        char prefix[FILE_FORMAT_PREFIX_SIZE+1]; //add zero for simple convert to string
        prefix[FILE_FORMAT_PREFIX_SIZE] = 0;
        file.read(prefix,FILE_FORMAT_PREFIX_SIZE);
        if (memcmp(prefix,FILE_FORMAT_PREFIX,FILE_FORMAT_PREFIX_SIZE)==0){
            Version = file.readInt();
        }
        file.close();
    }
    return Version;
}

bool convertToVersion2(const QString& SrcFileName,const QString& DstFileName, bool makeBackup)
{
    int CurrentVersion = getDBVersion(SrcFileName);
    if (CurrentVersion==-1){
        qCritical() << "can't access file " << SrcFileName << " or incorrect/broken file";
        return false;
    }
    if (CurrentVersion==0){
        qCritical() << "incorrect db version 0";
        return false;
    }
    if (CurrentVersion==2){
        if (SrcFileName==DstFileName)
            return true;
        return QFile::copy(SrcFileName,DstFileName);
    }
    if (CurrentVersion>2){
        qCritical() << "can't convert db to version 2. too high version " << CurrentVersion;
        return false;
    }

    //CURRENT VERSION == 1
    QString tmpFileName = DstFileName+"_";
    cFileBin out(tmpFileName);
    cFileBin file(SrcFileName);
    if (file.open(QIODevice::ReadOnly)){
        bool success = false;
        if (out.open(QIODevice::WriteOnly)){
            char prefix[FILE_FORMAT_PREFIX_SIZE+1]; //add zero for simple convert to string
            prefix[FILE_FORMAT_PREFIX_SIZE] = 0;
            file.read(prefix,FILE_FORMAT_PREFIX_SIZE);
            file.readInt();//version - its 1. no variants
            //header
            out.write(FILE_FORMAT_PREFIX,FILE_FORMAT_PREFIX_SIZE);
            out.writeInt(2);

            //profiles
            int profilesCount = file.readInt();
            out.writeInt(profilesCount);
            for (int i = 0; i<profilesCount; i++)
                out.writeString(file.readString());//profile name
            out.writeInt(file.readInt());//current profile

            //categories
            int categoriesCount = file.readInt();
            out.writeInt(categoriesCount);
            for (int i = 0; i<categoriesCount; i++){
                out.writeString(file.readString());//category name
                out.writeInt(file.readInt());//category color
            }

            //applications
            int appCount = file.readInt();
            out.writeInt(appCount);
            for (int i = 0; i<appCount; i++){
                out.writeInt(1);//app is visible
                QString name = file.readString();
                out.writeString(file.readString());//path
                out.writeInt(sAppInfo::eTrackerType::TT_EXECUTABLE_DETECTOR);
                out.writeString("");

                //activities
                out.writeInt(1); //activities count - only default
                out.writeInt(1);//activity is visible
                out.writeString(name); //name

                categoriesCount = file.readInt();
                out.writeInt(categoriesCount);
                for (int j = 0; j<categoriesCount; j++)
                    out.writeInt(file.readInt());

                int periodsCount = file.readInt();
                out.writeInt(periodsCount);
                for (int j = 0; j<periodsCount; j++){
                    out.writeUint(file.readUint());
                    out.writeInt(file.readInt());
                    out.writeInt(file.readInt());
                }
            }

            out.close();
            success = true;
        }
        else
            success = false;
        file.close();
        if (success){
            if (makeBackup)
                QFile::rename(DstFileName,DstFileName+".version.1");
            QFile::remove(DstFileName);
            QFile::rename(tmpFileName,DstFileName);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
            qInfo() << "db converted from version 1 to version 2";
#endif
        }
        return success;
    }
    else
        return false;
}

bool convertToVersion3(const QString &SrcFileName, const QString &DstFileName, bool makeBackup)
{
    int CurrentVersion = getDBVersion(SrcFileName);
    if (CurrentVersion==3){
        if (SrcFileName==DstFileName)
            return true;
        return QFile::copy(SrcFileName,DstFileName);
    }
    if (CurrentVersion>3){
        qCritical() << "can't convert db to version 3. too high version " << CurrentVersion;
        return false;
    }
    if (CurrentVersion<2){
        if (!convertToVersion2(SrcFileName, DstFileName, makeBackup))
            return false;
        makeBackup = false;
    }


    //CURRENT VERSION == 2
    QString tmpFileName = DstFileName+"_";
    cFileBin out(tmpFileName);
    cFileBin file(SrcFileName);
    if (file.open(QIODevice::ReadOnly)){
        bool success = false;
        if (out.open(QIODevice::WriteOnly)){
            char prefix[FILE_FORMAT_PREFIX_SIZE+1]; //add zero for simple convert to string
            prefix[FILE_FORMAT_PREFIX_SIZE] = 0;
            file.read(prefix,FILE_FORMAT_PREFIX_SIZE);
            file.readInt();//version - its 2. no variants
            //header
            out.write(FILE_FORMAT_PREFIX,FILE_FORMAT_PREFIX_SIZE);
            out.writeInt(3);

            //profiles
            int profilesCount = file.readInt();
            out.writeInt(profilesCount);
            for (int i = 0; i<profilesCount; i++)
                out.writeString(file.readString());//profile name
            out.writeInt(file.readInt());//current profile

            //categories
            int categoriesCount = file.readInt();
            out.writeInt(categoriesCount);
            for (int i = 0; i<categoriesCount; i++){
                out.writeString(file.readString());//category name
                out.writeInt(file.readInt());//category color
            }

            //applications
            int appCount = file.readInt();
            out.writeInt(appCount);
            for (int i = 0; i<appCount; i++){
                out.writeInt(file.readInt());
                out.writeString(file.readString());
                out.writeInt(file.readInt());
                out.writeString(file.readString());

                //activities
                int actCount = file.readInt();
                out.writeInt(actCount);
                for (int j = 0; j<actCount; j++){
                    file.readInt();//skip visible
                    out.writeString(file.readString());

                    //only read activity
                    QVector<sActivityProfileState> categories;
                    categoriesCount = file.readInt();
                    categories.resize(categoriesCount);
                    for (int k = 0; k<categoriesCount; k++){
                        categories[k].category = file.readInt();
                        categories[k].visible = false;
                    }

                    QVector<sTimePeriod> periods;
                    int periodsCount = file.readInt();
                    periods.resize(periodsCount);
                    for (int k  = 0; k<periodsCount; k++){
                        periods[k].start = QDateTime::fromTime_t(file.readUint());
                        periods[k].length = file.readInt();
                        periods[k].profileIndex = file.readInt();
                        categories[periods[k].profileIndex].visible = true;
                    }

                    //only write activity
                    out.writeInt(categoriesCount);
                    for (int k = 0; k<categoriesCount; k++){
                        out.writeInt(categories[k].category);
                        out.writeInt(categories[k].visible?1:0);
                    }

                    out.writeInt(periodsCount);
                    for (int k  = 0; k<periodsCount; k++){
                        out.writeUint(periods[k].start.toTime_t());
                        out.writeInt(periods[k].length);
                        out.writeInt(periods[k].profileIndex);
                    }
                }
            }

            out.close();
            success = true;
        }
        else
            success = false;
        file.close();
        if (success){
            if (makeBackup)
                QFile::rename(DstFileName,DstFileName+".version.2");
            QFile::remove(DstFileName);
            QFile::rename(tmpFileName,DstFileName);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
            qInfo() << "db converted from version 2 to version 3";
#endif
        }
        return success;
    }
    else
        return false;
}

bool convertToVersion4(const QString &SrcFileName, const QString &DstFileName, bool makeBackup)
{
    int CurrentVersion = getDBVersion(SrcFileName);
    if (CurrentVersion==4){
        if (SrcFileName==DstFileName)
            return true;
        return QFile::copy(SrcFileName,DstFileName);
    }
    if (CurrentVersion>4){
        qCritical() << "can't convert db to version 4. too high version " << CurrentVersion;
        return false;
    }
    if (CurrentVersion<3){
        if (!convertToVersion3(SrcFileName, DstFileName, makeBackup))
            return false;
        makeBackup = false;
    }


    //CURRENT VERSION == 3
    QString tmpFileName = DstFileName+"_";
    cFileBin out(tmpFileName);
    cFileBin file(SrcFileName);
    if (file.open(QIODevice::ReadOnly)){
        bool success = false;
        if (out.open(QIODevice::WriteOnly)){
            char prefix[FILE_FORMAT_PREFIX_SIZE+1]; //add zero for simple convert to string
            prefix[FILE_FORMAT_PREFIX_SIZE] = 0;
            file.read(prefix,FILE_FORMAT_PREFIX_SIZE);
            file.readInt();//version - its 3. no variants
            //header
            out.write(FILE_FORMAT_PREFIX,FILE_FORMAT_PREFIX_SIZE);
            out.writeInt(4);

            //profiles
            int profilesCount = file.readInt();
            out.writeInt(profilesCount);
            for (int i = 0; i<profilesCount; i++)
                out.writeString(file.readString());//profile name
            out.writeInt(file.readInt());//current profile

            //categories
            int categoriesCount = file.readInt();
            out.writeInt(categoriesCount);
            for (int i = 0; i<categoriesCount; i++){
                out.writeString(file.readString());//category name
                out.writeInt(file.readInt());//category color
            }

            //applications
            int appCount = file.readInt();
            out.writeInt(appCount);
            for (int i = 0; i<appCount; i++){
                out.writeInt(file.readInt());
                out.writeString(file.readString());
                int trackerType = file.readInt();
                //VERSION 4 CONVERSION
                int useCustomScript = 0;
                if (trackerType==3){
                    trackerType = 0;
                    useCustomScript = 1;
                };

                out.writeInt(trackerType); //old but converted field
                out.writeInt(useCustomScript); //new field
                out.writeString(file.readString());

                //activities
                int actCount = file.readInt();
                out.writeInt(actCount);
                for (int j = 0; j<actCount; j++){
                    out.writeString(file.readString());

                    //only read activity
                    QVector<sActivityProfileState> categories;
                    categoriesCount = file.readInt();
                    categories.resize(categoriesCount);
                    for (int k = 0; k<categoriesCount; k++){
                        categories[k].category = file.readInt();
                        categories[k].visible = file.readInt();
                    }

                    QVector<sTimePeriod> periods;
                    int periodsCount = file.readInt();
                    periods.resize(periodsCount);
                    for (int k  = 0; k<periodsCount; k++){
                        periods[k].start = QDateTime::fromTime_t(file.readUint());
                        periods[k].length = file.readInt();
                        periods[k].profileIndex = file.readInt();
                    }

                    //only write activity
                    out.writeInt(categoriesCount);
                    for (int k = 0; k<categoriesCount; k++){
                        out.writeInt(categories[k].category);
                        out.writeInt(categories[k].visible?1:0);
                    }

                    out.writeInt(periodsCount);
                    for (int k  = 0; k<periodsCount; k++){
                        out.writeUint(periods[k].start.toTime_t());
                        out.writeInt(periods[k].length);
                        out.writeInt(periods[k].profileIndex);
                    }
                }
            }

            out.close();
            success = true;
        }
        else
            success = false;
        file.close();
        if (success){
            if (makeBackup)
                QFile::rename(DstFileName,DstFileName+".version.3");
            QFile::remove(DstFileName);
            QFile::rename(tmpFileName,DstFileName);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
            qInfo() << "db converted from version 3 to version 4";
#endif
        }
        return success;
    }
    else
        return false;
}
