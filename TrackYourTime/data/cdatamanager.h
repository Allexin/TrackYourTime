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

#ifndef CDATAMANAGER_H
#define CDATAMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QColor>
#include <QTimer>
#include "cexternaltrackers.h"
#include "cscriptsmanager.h"

struct sProfile{
    QString name;
};

struct sTimePeriod{
    QDateTime start;
    int length;
    int profileIndex;
};

struct sActivityProfileState{
    int category;
    bool visible;
};

struct sActivityInfo{
    QString name;
    QString nameUpcase;
    QVector<sTimePeriod> periods;
    QVector<sActivityProfileState> categories;
    void incTime(bool FirstTime, int CurrentProfile, int UpdateDelay);
};

class cAppPredefinedInfo;

class sAppInfo{
public:
    enum eTrackerType{
        TT_EXECUTABLE_DETECTOR = 0,
        TT_EXTERNAL_DETECTOR,
        TT_PREDEFINED_SCRIPT
    };

    QString path;

    bool visible;
    eTrackerType trackerType;
    bool useCustomScript;
    QString customScript;
    cAppPredefinedInfo* predefinedInfo;

    QVector<sActivityInfo> activities;
public:
    sAppInfo(QString name, int profilesCount);
    sAppInfo();
    ~sAppInfo();
};

struct sCategory{
    QString name;
    QColor color;
};

class cDataManager : public QObject {
    Q_OBJECT
public:
    enum eBackupDelay{
        BD_ONE_DAY = 0,
        BD_ONE_WEEK,
        BD_ONE_MONTH,
        BD_ONE_YEAR,
        BD_FOREVER
    };

    static const int    DEFAULT_SECONDS_UPDATE_DELAY = 1;
    static const int    DEFAULT_SECONDS_IDLE_DELAY = 300;
    static const int    DEFAULT_SECONDS_AUTOSAVE_DELAY = 1500;

    static const QString CONF_UPDATE_DELAY_ID;
    static const QString CONF_IDLE_DELAY_ID;
    static const QString CONF_AUTOSAVE_DELAY_ID;
    static const QString CONF_STORAGE_FILENAME_ID;
    static const QString CONF_LANGUAGE_ID;
    static const QString CONF_FIRST_LAUNCH_ID;
    static const QString CONF_NOTIFICATION_SHOW_SYSTEM_ID;
    static const QString CONF_NOTIFICATION_MESSAGE_ID;
    static const QString CONF_NOTIFICATION_HIDE_SECONDS_ID;
    static const QString CONF_NOTIFICATION_POSITION_ID;
    static const QString CONF_NOTIFICATION_SIZE_ID;
    static const QString CONF_NOTIFICATION_OPACITY_ID;    
    static const QString CONF_NOTIFICATION_MOUSE_BEHAVIOR_ID;
    static const QString CONF_NOTIFICATION_CAT_SELECT_BEHAVIOR_ID;
    static const QString CONF_NOTIFICATION_VISIBILITY_BEHAVIOR_ID;
    static const QString CONF_NOTIFICATION_HIDE_BORDERS_ID;
    static const QString CONF_AUTORUN_ID;
    static const QString CONF_CLIENT_MODE_ID;
    static const QString CONF_CLIENT_MODE_HOST_ID;
    static const QString CONF_LAST_AVAILABLE_VERSION_ID;
    static const QString CONF_BACKUP_FILENAME_ID;
    static const QString CONF_BACKUP_DELAY_ID;
protected:
    cExternalTrackers   m_ExternalTrackers;
    cScriptsManager     m_ScriptsManager;
    QTimer              m_MainTimer;

    QVector<sCategory>  m_Categories;
    QVector<sAppInfo*>  m_Applications;
    QVector<sProfile>   m_Profiles;

    int                 m_LastLocalActivity;
    int                 m_CurrentProfile;
    QString             m_StorageFileName;
    QString             m_BackupFolder;
    eBackupDelay        m_BackupDelay;

    QString             m_DebugScript;

    bool                m_ShowSystemNotifications;

    bool                m_ClientMode;
    QString             m_ClientModeHost;

    int                 m_UpdateCounter;
    int                 m_UpdateDelay;

    QPoint              m_CurrentMousePos;
    int                 m_CurrentApplicationIndex;
    int                 m_CurrentApplicationActivityIndex;
    int                 m_CurrentApplicationActivityCategory;

    bool                m_Idle;
    int                 m_IdleCounter;
    int                 m_IdleDelay;

    int                 m_AutoSaveCounter;
    int                 m_AutoSaveDelay;
    int getAppIndex(const sSysInfo& FileInfo);
    int getActivityIndex(int appIndex,const sSysInfo &FileInfo);
    int getActivityIndexDirect(int appIndex, QString activityName);
    void saveDB();
    void loadDB();
    void saveJSON();
    void loadJSON();

    void loadPreferences();
public:
    cDataManager();
    virtual ~cDataManager();

    int profilesCount(){return m_Profiles.size();}
    const sProfile* profiles(int index);
    int getCurrentProfileIndex(){return m_CurrentProfile;}
    void setCurrentProfileIndex(int ProfileIndex){ m_CurrentProfile = ProfileIndex; emit profilesChanged();}
    void setCurrentProfileIndexSafe(int ProfileIndex){
        if (ProfileIndex<0 || ProfileIndex>=m_Profiles.size())
            return;
        setCurrentProfileIndex(ProfileIndex);
    }
    void setProfileName(int index, const QString& Name){m_Profiles[index].name = Name; emit profilesChanged();}
    void addNewProfile(const QString &Name, int CloneProfileIndex = -1);
    void mergeProfiles(int profile1, int profile2);


    int categoriesCount(){return m_Categories.size();}
    const sCategory* categories(int index){return &m_Categories[index];}
    void setCategoryName(int index, const QString& Name){m_Categories[index].name = Name;}
    void setCategoryColor(int index, const QColor& color){m_Categories[index].color = color;}
    void addNewCategory(const QString& Name, QColor color);
    void deleteCategory(int index);

    int applicationsCount(){return m_Applications.size();}
    sAppInfo* applications(int index){return m_Applications[index];}
    void setApplicationActivityCategory(int profile, int appIndex, int activityIndex, int category);

    int getCurrentAppliction(){return m_CurrentApplicationIndex;}
    int getCurrentApplictionActivity(){return m_CurrentApplicationActivityIndex;}

    QString getStorageFileName(){return m_StorageFileName;}
    void setDebugScript(const QString& script){m_DebugScript = script;}

    void makeBackup();
public slots:
    void process();
    void onPreferencesChanged();
signals:
    void trayShowHint(const QString& text);
    void trayActive();
    void traySleep();

    void profilesChanged();
    void applicationsChanged();
    void debugScriptResult(QString result, const sSysInfo& data, QString trackingResult);
    void showNotification();

    void statisticFastUpdate(int application, int activity, int category, int secondsCount, bool fullUpdate);
};

#endif // CDATAMANAGER_H
