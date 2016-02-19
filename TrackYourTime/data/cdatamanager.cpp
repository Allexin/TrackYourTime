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
#include "../tools/tools.h"
#include "../tools/os_api.h"
#include "../tools/cfilebin.h"
#include "cdbversionconverter.h"
#include "capppredefinedinfo.h"

const QString cDataManager::CONF_UPDATE_DELAY_ID = "UPDATE_DELAY";
const QString cDataManager::CONF_IDLE_DELAY_ID = "IDLE_DELAY";
const QString cDataManager::CONF_AUTOSAVE_DELAY_ID = "AUTOSAVE_DELAY";
const QString cDataManager::CONF_STORAGE_FILENAME_ID = "STORAGE_FILENAME";
const QString cDataManager::CONF_LANGUAGE_ID = "LANGUAGE";
const QString cDataManager::CONF_FIRST_LAUNCH_ID = "FIRST_LAUNCH";
const QString cDataManager::CONF_NOTIFICATION_TYPE_ID = "NOTIFICATION_TYPE";
const QString cDataManager::CONF_NOTIFICATION_MESSAGE_ID = "NOTIFICATION_MESSAGE";
const QString cDataManager::CONF_NOTIFICATION_HIDE_SECONDS_ID = "NOTIFICATION_HIDE_SECONDS";
const QString cDataManager::CONF_NOTIFICATION_HIDE_MOVES_ID = "NOTIFICATION_HIDE_MOVES";
const QString cDataManager::CONF_NOTIFICATION_POSITION_ID = "NOTIFICATION_POSITION";
const QString cDataManager::CONF_NOTIFICATION_SIZE_ID = "NOTIFICATION_SIZE";
const QString cDataManager::CONF_NOTIFICATION_OPACITY_ID = "NOTIFICATION_OPACITY";
const QString cDataManager::CONF_AUTORUN_ID = "AUTORUN_ENABLED";
const QString cDataManager::CONF_CLIENT_MODE_ID = "CLIENT_MODE";
const QString cDataManager::CONF_CLIENT_MODE_HOST_ID = "CLIENT_MODE_HOST";

cDataManager::cDataManager():QObject()
{
    m_NotificationType = NT_SYSTEM;

    m_UpdateCounter = 0;
    m_UpdateDelay = DEFAULT_SECONDS_UPDATE_DELAY;

    m_CurrentMousePos = QPoint(0,0);
    m_CurrentApplicationIndex = -1;
    m_CurrentApplicationActivityIndex = -1;
    m_CurrentApplicationActivityCategory = 0;

    m_Idle = false;
    m_IdleCounter = 0;
    m_IdleDelay = DEFAULT_SECONDS_IDLE_DELAY;    

    m_AutoSaveCounter = 0;
    m_AutoSaveDelay = DEFAULT_SECONDS_AUTOSAVE_DELAY;

    m_CurrentProfile = 0;

#if (QT_VERSION < QT_VERSION_CHECK(5, 4, 0))
    m_StorageFileName = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/db.bin";
#else
    m_StorageFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/db.bin";
#endif

    loadPreferences();
    if (!m_StorageFileName.isEmpty()){
        QDir storagePath(QFileInfo(m_StorageFileName).absolutePath());
        if (!storagePath.exists())
            storagePath.mkpath(".");
        loadDB();
    }

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
    for (int i = 0; i<m_Applications.size(); i++)
        delete m_Applications[i];
}

const sProfile *cDataManager::profiles(int index)
{
    if (index<0 || index>=m_Profiles.size())
        return NULL;
    return &m_Profiles[index];
}

void cDataManager::addNewProfile(const QString &Name, int CloneProfileIndex)
{
    sProfile profile;
    profile.name = Name;
    m_Profiles.push_back(profile);

    for (int i = 0; i<m_Applications.size(); i++)
        for (int j = 0; j<m_Applications[i]->activities.size(); j++)
            m_Applications[i]->activities[j].categories.push_back(CloneProfileIndex==-1?-1:m_Applications[i]->activities[j].categories[CloneProfileIndex]);
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
    for (int i = 0; i<m_Applications.size(); i++)
        for (int j = 0; j<m_Applications[i]->activities.size(); j++){
            m_Applications[i]->activities[j].categories.remove(profileToDelete);
            for (int j = 0; j<m_Applications[i]->activities[j].periods.size(); j++){
                if (m_Applications[i]->activities[j].periods[j].profileIndex==profileToDelete){
                    m_Applications[i]->activities[j].periods[j].profileIndex = profileToSave;
                }
                else
                if (m_Applications[i]->activities[j].periods[j].profileIndex>profileToDelete){
                    m_Applications[i]->activities[j].periods[j].profileIndex--;
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
        for (int j = 0; j<m_Applications[i]->activities.size(); j++){
            for (int k = 0; k<m_Applications[i]->activities[j].categories.size(); k++){
                if (m_Applications[i]->activities[j].categories[k]==index)
                    m_Applications[i]->activities[j].categories[k] = -1;
                else
                if (m_Applications[i]->activities[j].categories[k]>index)
                    m_Applications[i]->activities[j].categories[k];
            }
        }
    }
    m_Categories.remove(index);
    emit applicationsChanged();
}

void cDataManager::setApplicationActivityCategory(int profile, int appIndex, int activityIndex, int category)
{
    if (profile==-1){
        for (int i = 0; i<m_Applications[appIndex]->activities[activityIndex].categories.size(); i++)
            m_Applications[appIndex]->activities[activityIndex].categories[i] = category;
    }
    else{
        m_Applications[appIndex]->activities[activityIndex].categories[profile] = category;
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
    if (isKeyboardChanged()){
        isUserActive = true;
    }

    //Update mouse activity
    QPoint mousePos = getMousePos();
    if (m_CurrentMousePos!=mousePos){
        isUserActive = true;
        m_CurrentMousePos = mousePos;
    }

    if (isUserActive)
        m_LastLocalActivity = 0;
    else
        m_LastLocalActivity+=m_UpdateDelay;
    int hostActivity = m_LastLocalActivity+1;
    sOverrideTrackerInfo* info = m_ExternalTrackers.getOverrideTracker();
    if (info)
        hostActivity = info->IdleTime-2000;


    //Update application
    bool isAppChanged = false;
    sSysInfo currentAppInfo = getCurrentApplication();
    int appIndex = getAppIndex(currentAppInfo);
    int activityIndex = appIndex>-1?getActivityIndex(appIndex,currentAppInfo):0;

    if (m_LastLocalActivity*1000>hostActivity){
        if (info){
            sSysInfo remoteInfo;
            remoteInfo.fileName = info->AppFileName;
            remoteInfo.path = "";
            remoteInfo.title = "";
            appIndex = getAppIndex(remoteInfo);
            activityIndex = appIndex>-1?getActivityIndexDirect(appIndex,info->State):0;
            isUserActive = true;
        }
    }

    if (appIndex!=m_CurrentApplicationIndex || activityIndex!=m_CurrentApplicationActivityIndex){
        isUserActive = true;
        isAppChanged = true;
        m_CurrentApplicationIndex = appIndex;
        m_CurrentApplicationActivityIndex = activityIndex;
        if (m_CurrentApplicationIndex>-1){
            int activityCategory = m_Applications[m_CurrentApplicationIndex]->activities[m_CurrentApplicationActivityIndex].categories[m_CurrentProfile];
            if (m_CurrentApplicationActivityCategory!=activityCategory || activityCategory==-1){
                m_CurrentApplicationActivityCategory = activityCategory;

                switch(m_NotificationType){
                    case NT_NONE:{

                    };
                    break;
                    case NT_SYSTEM:{
                        QString hint = m_Profiles[m_CurrentProfile].name+":"+(m_CurrentApplicationActivityCategory==-1?tr("Uncategorized"):m_Categories[m_CurrentApplicationActivityCategory].name);
                        emit trayShowHint(hint);
                    };
                    break;
                    case NT_BUILTIN:{
                        emit showNotification();
                    };
                    break;
                }
            }
        }
    }

    if (m_CurrentApplicationIndex>-1 && (!m_Idle || isAppChanged)){
        m_Applications[m_CurrentApplicationIndex]->activities[m_CurrentApplicationActivityIndex].incTime(isAppChanged,m_CurrentProfile,m_UpdateDelay);
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
                m_Applications[m_CurrentApplicationIndex]->activities[m_CurrentApplicationActivityIndex].periods.last().length-=m_IdleCounter;
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

    if (!m_Idle && m_ClientMode){
        if (m_CurrentApplicationIndex>-1){
            m_ExternalTrackers.sendOverrideTracker(m_Applications[m_CurrentApplicationIndex]->activities[0].name,
                                                   m_Applications[m_CurrentApplicationIndex]->activities[m_CurrentApplicationActivityIndex].name,
                                                   m_IdleCounter,
                                                   m_ClientModeHost);
        }
        else{
            m_ExternalTrackers.sendOverrideTracker("","",m_IdleCounter,m_ClientModeHost);
        }
    }
}

void cDataManager::onPreferencesChanged()
{
    saveDB(); //save to old storage
    loadPreferences(); //read new preferences
    loadDB();//reload current storage or load new if STORAGE_FILENAME changed
    emit profilesChanged();
}


int cDataManager::getAppIndex(const sSysInfo &FileInfo)
{
    if (FileInfo.fileName.isEmpty())
        return -1;

    for (int i = 0; i<m_Applications.size(); i++){
        if (m_Applications[i]->activities[0].name==FileInfo.fileName){
            if (m_Applications[i]->path.isEmpty() && !FileInfo.path.isEmpty()){
                m_Applications[i]->path = FileInfo.path;
                emit applicationsChanged();
            }
            return i;
        }
    }

    //app not exists in our list(first launch) - add to list
    sAppInfo* info = new sAppInfo(FileInfo.fileName,m_Profiles.size());
    info->path = FileInfo.path;

    m_Applications.push_back(info);
    emit applicationsChanged();

    return m_Applications.size()-1;
}

int cDataManager::getActivityIndex(int appIndex,const sSysInfo &FileInfo)
{    
    sAppInfo* appInfo = m_Applications[appIndex];

    if (!m_DebugScript.isEmpty()){
        emit debugScriptResult(m_ScriptsManager.evalute(FileInfo,m_DebugScript),FileInfo);
    }

    if (appInfo->trackerType==sAppInfo::eTrackerType::TT_EXECUTABLE_DETECTOR)
        return 0;

    QString activity;

    switch(appInfo->trackerType){
        case sAppInfo::eTrackerType::TT_EXECUTABLE_DETECTOR:{
            return 0;
        };
            break;
        case sAppInfo::eTrackerType::TT_EXTERNAL_DETECTOR:{
            if (!m_ExternalTrackers.getExternalTrackerState(appInfo->activities[0].name,activity))
                activity="";
        };
            break;
        case sAppInfo::eTrackerType::TT_PREDEFINED_SCRIPT:{
            activity = m_ScriptsManager.getAppInfo(FileInfo,appInfo->predefinedInfo->script());
        };
            break;
        case sAppInfo::eTrackerType::TT_CUSTOM_SCRIPT:{
            activity = m_ScriptsManager.getAppInfo(FileInfo,appInfo->customScript);
        };
        break;
    }

    return getActivityIndexDirect(appIndex,activity);
}

int cDataManager::getActivityIndexDirect(int appIndex, QString activityName)
{
    if (activityName.isEmpty())
        return 0;

    for (int i = 0; i<m_Applications[appIndex]->activities.size(); i++){
        if (m_Applications[appIndex]->activities[i].name==activityName){
            return i;
        }
    }

    sActivityInfo ainfo;
    ainfo.visible = true;
    ainfo.name = activityName;
    ainfo.categories.resize(m_Profiles.size());
    for (int i = 0; i<ainfo.categories.size(); i++)
        ainfo.categories[i] = -1;
    m_Applications[appIndex]->activities.push_back(ainfo);
    emit applicationsChanged();
    return m_Applications[appIndex]->activities.size()-1;
}

const int FILE_FORMAT_VERSION = 2;

void cDataManager::saveDB()
{
    if (m_StorageFileName.isEmpty())
        return;
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
            file.writeInt(m_Applications[i]->visible?1:0);
            file.writeString(m_Applications[i]->path);
            file.writeInt(m_Applications[i]->trackerType);
            file.writeString(m_Applications[i]->customScript);

            file.writeInt(m_Applications[i]->activities.size());
            for (int activity = 0; activity<m_Applications[i]->activities.size(); activity++){
                sActivityInfo* info = &m_Applications[i]->activities[activity];
                file.writeInt(info->visible?1:0);
                file.writeString(info->name);

                //app category for every profile
                file.writeInt(info->categories.size());
                for (int j = 0; j<info->categories.size(); j++){
                    file.writeInt(info->categories[j]);
                }

                //total use time
                file.writeInt(info->periods.size());
                for (int j = 0; j<info->periods.size(); j++){
                    file.writeUint(info->periods[j].start.toTime_t());
                    file.writeInt(info->periods[j].length);
                    file.writeInt(info->periods[j].profileIndex);
                }
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
    if (m_StorageFileName.isEmpty())
        return;
    qDebug() << "cDataManager: start DB loading\n";
    for (int i = 0; i<m_Applications.size(); i++)
        delete m_Applications[i];
    m_Applications.resize(0);

    convertToVersion2(m_StorageFileName,m_StorageFileName);
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
                    m_Applications[i] = new sAppInfo();
                    m_Applications[i]->visible = file.readInt()==1;
                    m_Applications[i]->path = file.readString();
                    m_Applications[i]->trackerType = (sAppInfo::eTrackerType)file.readInt();
                    m_Applications[i]->customScript = file.readString();

                    m_Applications[i]->activities.resize(file.readInt());
                    for (int activity = 0; activity<m_Applications[i]->activities.size(); activity++){
                        sActivityInfo* info = &m_Applications[i]->activities[activity];
                        info->visible = file.readInt()==1;
                        info->name = file.readString();

                        //app category for every profile
                        info->categories.resize(file.readInt());
                        for (int j = 0; j<info->categories.size(); j++){
                            info->categories[j] = file.readInt();
                        }

                        //total use time
                        info->periods.resize(file.readInt());
                        for (int j = 0; j<info->periods.size(); j++){
                            info->periods[j].start = QDateTime::fromTime_t(file.readUint());
                            info->periods[j].length = file.readInt();
                            info->periods[j].profileIndex = file.readInt();
                        }
                    }
                    m_Applications[i]->predefinedInfo = new cAppPredefinedInfo(m_Applications[i]->activities[0].name);
                }
            }
            else
                qCritical() << "Error loading db. Incorrect file format version " << Version << " only " << FILE_FORMAT_VERSION << " supported";
        }
        else
            qCritical() << "Error loading db. Incorrect file format prefix " << prefix;


        file.close();
    }
    qDebug() << "cDataManager: end DB loading\n";
}

void cDataManager::loadPreferences()
{
    cSettings settings;

    m_UpdateDelay = settings.db()->value(CONF_UPDATE_DELAY_ID,m_UpdateDelay).toInt();
    m_IdleDelay = settings.db()->value(CONF_IDLE_DELAY_ID,m_IdleDelay).toInt();
    m_AutoSaveDelay = settings.db()->value(CONF_AUTOSAVE_DELAY_ID,m_AutoSaveDelay).toInt();
    m_StorageFileName = settings.db()->value(CONF_STORAGE_FILENAME_ID,m_StorageFileName).toString();
    m_NotificationType = (eNotificationType)settings.db()->value(CONF_NOTIFICATION_TYPE_ID,m_NotificationType).toInt();
    m_ClientMode = settings.db()->value(CONF_CLIENT_MODE_ID,m_ClientMode).toBool();
    m_ClientModeHost = settings.db()->value(CONF_CLIENT_MODE_HOST_ID,m_ClientModeHost).toString();
}



void sActivityInfo::incTime(bool FirstTime, int CurrentProfile, int UpdateDelay)
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

sAppInfo::sAppInfo(QString name, int profilesCount)
{
    predefinedInfo = new cAppPredefinedInfo(name);
    visible = true;
    sActivityInfo ainfo;
    ainfo.visible = true;
    ainfo.name = name;
    ainfo.categories.resize(profilesCount);
    for (int i = 0; i<ainfo.categories.size(); i++)
        ainfo.categories[i] = -1;
    activities.push_back(ainfo);

    trackerType = predefinedInfo->trackerType();
    customScript = predefinedInfo->script();
}

sAppInfo::sAppInfo()
{
    predefinedInfo = NULL;
}

sAppInfo::~sAppInfo()
{
    delete predefinedInfo;
}
