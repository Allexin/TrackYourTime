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

#include "cdatamanager.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include "os_api.h"
#include "cfilebin.h"

const QString cDataManager::CONF_UPDATE_DELAY_ID = "UPDATE_DELAY";
const QString cDataManager::CONF_IDLE_DELAY_ID = "IDLE_DELAY";
const QString cDataManager::CONF_AUTOSAVE_DELAY_ID = "AUTOSAVE_DELAY";
const QString cDataManager::CONF_STORAGE_FILENAME_ID = "STORAGE_FILENAME";
const QString cDataManager::CONF_LANGUAGE_ID = "LANGUAGE";
const QString cDataManager::CONF_FIRST_LAUNCH_ID = "FIRST_LAUNCH";
const QString cDataManager::CONF_AUTORUN_ID = "AUTORUN_ENABLED";

cDataManager::cDataManager():QObject()
{
    m_CurrentApplicationIndex = -1;

    m_UpdateCounter = 0;
    m_UpdateDelay = DEFAULT_SECONDS_UPDATE_DELAY;

    memset(m_CurrentKeyboardState,0,256*sizeof(bool));
    m_CurrentMousePos = QPoint(0,0);
    m_CurrentApplicationIndex = -1;
    m_CurrentApplicationCategory = 0;

    m_Idle = false;
    m_IdleCounter = 0;
    m_IdleDelay = DEFAULT_SECONDS_IDLE_DELAY;

    m_AutoSaveCounter = 0;
    m_AutoSaveDelay = DEFAULT_SECONDS_AUTOSAVE_DELAY;

    m_CurrentProfile = 0;

    m_StorageFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/db.bin";

    loadPreferences();
    QDir storagePath(QFileInfo(m_StorageFileName).absolutePath());
    if (!storagePath.exists())
        storagePath.mkpath(".");
    loadDB();

    if (m_Profiles.size()==0){
        sProfile defaultProfile;
        defaultProfile.name = tr("Default");
        m_Profiles.push_back(defaultProfile);
    }


    QObject::connect(&m_MainTimer, SIGNAL(timeout()), this, SLOT(process()));
    m_MainTimer.start(1000);
}

cDataManager::~cDataManager()
{    
    saveDB();
}

void cDataManager::addNewProfile(const QString &Name, int CloneProfileIndex)
{
    sProfile profile;
    profile.name = Name;
    m_Profiles.push_back(profile);

    for (int i = 0; i<m_Applications.size(); i++)
        m_Applications[i].categories.push_back(CloneProfileIndex==-1?-1:m_Applications[i].categories[CloneProfileIndex]);
    emit profilesChanged();
}

void cDataManager::mergeProfiles(int profile1, int profile2)
{
    int profileToDelete = profile2;
    int profileToSave = profile1;
    if (profileToSave>profileToDelete){
        profileToDelete = profile1;
        profileToSave = profile2;
    }

    m_Profiles.remove(profileToDelete);
    for (int i = 0; i<m_Applications.size(); i++){
        m_Applications[i].categories.remove(profileToDelete);
        for (int j = 0; j<m_Applications[i].periods.size(); j++){
            if (m_Applications[i].periods[j].profileIndex==profileToDelete){
                m_Applications[i].periods[j].profileIndex = profileToSave;
            }
            else
            if (m_Applications[i].periods[j].profileIndex>profileToDelete){
                m_Applications[i].periods[j].profileIndex--;
            }
        }
    }
    if (profileToDelete==m_CurrentProfile)
        m_CurrentProfile = profileToSave;
    emit profilesChanged();
}

void cDataManager::addNewCategory(const QString &Name, QColor color)
{
    sCategory cat;
    cat.color = color;
    cat.name = Name;
    m_Categories.push_back(cat);
}

void cDataManager::deleteCategory(int index)
{
    for (int i = 0; i<m_Applications.size(); i++){
        for (int j = 0; j<m_Applications[i].categories.size(); j++){
            if (m_Applications[i].categories[j]==index)
                m_Applications[i].categories[j] = -1;
            else
            if (m_Applications[i].categories[j]>index)
                m_Applications[i].categories[j];
        }
    }
    m_Categories.remove(index);
    emit applicationsChanged();
}

void cDataManager::setApplicationCategory(int profile, int appIndex, int category)
{
    if (profile==-1){
        for (int i = 0; i<m_Applications[appIndex].categories.size(); i++)
            m_Applications[appIndex].categories[i] = category;
    }
    else{
        m_Applications[appIndex].categories[profile] = category;
    }

}

void cDataManager::process()
{
    m_ExternalTrackers.update();

    m_UpdateCounter++;
    if (m_UpdateCounter<m_UpdateDelay)
        return;
    m_UpdateCounter = 0;

    bool isUserActive = false;

    //Update keyboard activity
    for (int keyCode = 0; keyCode<256; keyCode++)
        if (m_CurrentKeyboardState[keyCode]!=isKeyPressed(keyCode)){
            isUserActive = true;
            m_CurrentKeyboardState[keyCode] = !m_CurrentKeyboardState[keyCode];
        }

    //Update mouse activity
    QPoint mousePos = getMousePos();
    if (m_CurrentMousePos!=mousePos){
        isUserActive = true;
        m_CurrentMousePos = mousePos;
    }

    //Update application
    bool isAppChanged = false;
    int appIndex = getAppIndex(m_ExternalTrackers.getExternalTrackerState(getCurrentApplication()));
    if (appIndex!=m_CurrentApplicationIndex){
        isUserActive = true;
        isAppChanged = true;
        m_CurrentApplicationIndex = appIndex;
        if (m_CurrentApplicationIndex>-1){
            int appCategory = m_Applications[m_CurrentApplicationIndex].categories[m_CurrentProfile];
            if (m_CurrentApplicationCategory!=appCategory){
                QString hint = m_Profiles[m_CurrentProfile].name+":"+(appCategory==-1?tr("Uncategorized"):m_Categories[appCategory].name);
                emit trayShowHint(hint);
            }
        }
    }

    if (m_CurrentApplicationIndex>-1 && (!m_Idle || isAppChanged)){
        m_Applications[m_CurrentApplicationIndex].incTime(isAppChanged,m_CurrentProfile,m_UpdateDelay);
    }

    if (isUserActive){
        m_IdleCounter = 0;
        if (m_Idle){
            emit trayActive();
            m_Idle = false;
        }
    }
    else{
        m_IdleCounter+=m_UpdateDelay;
        if (m_IdleCounter>m_IdleDelay && !m_Idle){
            emit traySleep();
            m_Idle = true;
            if (m_CurrentApplicationIndex>-1){
                m_Applications[m_CurrentApplicationIndex].periods.last().length-=m_IdleCounter;
            }
            //force autosave
            m_AutoSaveCounter=m_AutoSaveDelay;
        }
    }

    if (!m_Idle)
        m_AutoSaveCounter+=m_UpdateDelay;

    if (m_AutoSaveCounter>=m_AutoSaveDelay){
        m_AutoSaveCounter = 0;
        saveDB();
    }
}

void cDataManager::onPreferencesChanged()
{
    saveDB(); //save to old storage
    loadPreferences(); //read new preferences
    loadDB();//reload current storage or load new if STORAGE_FILENAME changed
    emit profilesChanged();
}


int cDataManager::getAppIndex(sAppFileName FileName)
{
    if (FileName.fileName.isEmpty())
        return -1;

    for (int i = 0; i<m_Applications.size(); i++){
        if (m_Applications[i].name==FileName.fileName){
            if (m_Applications[i].path.isEmpty()){
                m_Applications[i].path = FileName.path;
                emit applicationsChanged();
            }
            return i;
        }
    }

    //app not exists in our list(first launch) - add to list
    sAppInfo info;
    info.name = FileName.fileName;
    info.path = FileName.path;
    info.categories.resize(m_Profiles.size());
    for (int i = 0; i<info.categories.size(); i++)
        info.categories[i] = -1;

    m_Applications.push_back(info);
    emit applicationsChanged();

    return m_Applications.size()-1;
}

const char* FILE_FORMAT_PREFIX = "TYTDB";
const int FILE_FORMAT_PREFIX_SIZE = 5;
const int FILE_FORMAT_VERSION = 1;

void cDataManager::saveDB()
{
    cFileBin file( m_StorageFileName+".new" );
    if ( file.open(QIODevice::WriteOnly) )
    {
        //header
        file.write(FILE_FORMAT_PREFIX,FILE_FORMAT_PREFIX_SIZE);
        file.writeInt(FILE_FORMAT_VERSION);

        //profiles
        file.writeInt(m_Profiles.size());
        for (int i = 0; i<m_Profiles.size(); i++){
            file.writeString(m_Profiles[i].name);
        }
        file.writeInt(m_CurrentProfile);

        //categories
        file.writeInt(m_Categories.size());
        for (int i = 0; i<m_Categories.size(); i++){
            file.writeString(m_Categories[i].name);
            file.writeInt(m_Categories[i].color.rgba());
        }

        //applications
        file.writeInt(m_Applications.size());
        for (int i = 0; i<m_Applications.size(); i++){
            file.writeString(m_Applications[i].name);
            file.writeString(m_Applications[i].path);

            //app category for every profile
            file.writeInt(m_Applications[i].categories.size());
            for (int j = 0; j<m_Applications[i].categories.size(); j++){
                file.writeInt(m_Applications[i].categories[j]);
            }

            //total use time
            file.writeInt(m_Applications[i].periods.size());
            for (int j = 0; j<m_Applications[i].periods.size(); j++){
                file.writeUint(m_Applications[i].periods[j].start.toTime_t());
                file.writeInt(m_Applications[i].periods[j].length);
                file.writeInt(m_Applications[i].periods[j].profileIndex);
            }
        }
        file.close();

        //if at any step of saving app fail proceed - old db will not damaged and can be restored
        QFile::rename(m_StorageFileName, m_StorageFileName+".old");
        QFile::rename(m_StorageFileName+".new", m_StorageFileName);
        QFile::remove(m_StorageFileName+".old");
    }
}

void cDataManager::loadDB()
{
    cFileBin file( m_StorageFileName );
    if ( file.open(QIODevice::ReadOnly) )
    {
        //check header
        char prefix[FILE_FORMAT_PREFIX_SIZE+1]; //add zero for simple convert to string
        prefix[FILE_FORMAT_PREFIX_SIZE] = 0;
        file.read(prefix,FILE_FORMAT_PREFIX_SIZE);
        if (memcmp(prefix,FILE_FORMAT_PREFIX,FILE_FORMAT_PREFIX_SIZE)==0){
            int Version = file.readInt();
            if (Version==FILE_FORMAT_VERSION){

                //profiles
                m_Profiles.resize(file.readInt());
                for (int i = 0; i<m_Profiles.size(); i++){
                    m_Profiles[i].name = file.readString();
                }
                m_CurrentProfile = file.readInt();

                //categories
                m_Categories.resize(file.readInt());
                for (int i = 0; i<m_Categories.size(); i++){
                    m_Categories[i].name = file.readString();
                    m_Categories[i].color = QColor::fromRgba(file.readInt());
                }

                //applications
                m_Applications.resize(file.readInt());
                for (int i = 0; i<m_Applications.size(); i++){
                    m_Applications[i].name = file.readString();
                    m_Applications[i].path = file.readString();

                    //app category for every profile
                    m_Applications[i].categories.resize(file.readInt());
                    for (int j = 0; j<m_Applications[i].categories.size(); j++){
                        m_Applications[i].categories[j] = file.readInt();
                    }

                    //total use time
                    m_Applications[i].periods.resize(file.readInt());
                    for (int j = 0; j<m_Applications[i].periods.size(); j++){
                        m_Applications[i].periods[j].start = QDateTime::fromTime_t(file.readUint());
                        m_Applications[i].periods[j].length = file.readInt();
                        m_Applications[i].periods[j].profileIndex = file.readInt();
                    }
                }
            }
            else
                qCritical() << "Error loading db. Incorrect file format version " << Version << " only " << FILE_FORMAT_VERSION << " supported";
        }
        else
            qCritical() << "Error loading db. Incorrect file format prefix " << prefix;


        file.close();
    }
}

void cDataManager::loadPreferences()
{
    QSettings settings;

    m_UpdateDelay = settings.value(CONF_UPDATE_DELAY_ID,m_UpdateDelay).toInt();
    m_IdleDelay = settings.value(CONF_IDLE_DELAY_ID,m_IdleDelay).toInt();
    m_AutoSaveDelay = settings.value(CONF_AUTOSAVE_DELAY_ID,m_AutoSaveDelay).toInt();
    m_StorageFileName = settings.value(CONF_STORAGE_FILENAME_ID,m_StorageFileName).toString();
}

void cDataManager::savePreferences()
{
    QSettings settings;

    settings.setValue(CONF_UPDATE_DELAY_ID,m_UpdateDelay);
    settings.setValue(CONF_IDLE_DELAY_ID,m_IdleDelay);
    settings.setValue(CONF_AUTOSAVE_DELAY_ID,m_AutoSaveDelay);
    settings.setValue(CONF_STORAGE_FILENAME_ID,m_StorageFileName);
    settings.sync();
}




void sAppInfo::incTime(bool FirstTime, int CurrentProfile, int UpdateDelay)
{
    if (FirstTime){
        sTimePeriod period;
        period.start = QDateTime::currentDateTimeUtc();
        period.length = 0;
        period.profileIndex = CurrentProfile;
        periods.push_back(period);
    }
    periods.last().length+=UpdateDelay;
}
