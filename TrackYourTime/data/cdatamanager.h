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

struct sActivityInfo{
    QString name;
    QVector<sTimePeriod> periods;
    QVector<int> categories;
    bool visible;
    void incTime(bool FirstTime, int CurrentProfile, int UpdateDelay);
};

class cAppPredefinedInfo;

class sAppInfo{
public:
    enum eTrackerType{
        TT_EXECUTABLE_DETECTOR = 0,
        TT_EXTERNAL_DETECTOR,
        TT_PREDEFINED_SCRIPT,
        TT_CUSTOM_SCRIPT
    };

    QString path;

    bool visible;
    eTrackerType trackerType;
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
    static const int    DEFAULT_SECONDS_UPDATE_DELAY = 1;
    static const int    DEFAULT_SECONDS_IDLE_DELAY = 300;
    static const int    DEFAULT_SECONDS_AUTOSAVE_DELAY = 1500;

    static const QString CONF_UPDATE_DELAY_ID;
    static const QString CONF_IDLE_DELAY_ID;
    static const QString CONF_AUTOSAVE_DELAY_ID;
    static const QString CONF_STORAGE_FILENAME_ID;
    static const QString CONF_LANGUAGE_ID;
    static const QString CONF_FIRST_LAUNCH_ID;
    static const QString CONF_SHOW_BALOONS_ID;
    static const QString CONF_AUTORUN_ID;
protected:
    cExternalTrackers   m_ExternalTrackers;
    cScriptsManager     m_ScriptsManager;
    QTimer              m_MainTimer;

    QVector<sCategory>  m_Categories;
    QVector<sAppInfo*>  m_Applications;
    QVector<sProfile>   m_Profiles;

    int                 m_CurrentProfile;
    QString             m_StorageFileName;

    QString             m_DebugScript;

    bool                m_ShowBaloons;

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
    void saveDB();
    void loadDB();
    void loadPreferences();
    void savePreferences();
public:
    cDataManager();
    virtual ~cDataManager();

    int profilesCount(){return m_Profiles.size();}
    const sProfile* profiles(int index){return &m_Profiles[index];}
    int getCurrentProfileIndex(){return m_CurrentProfile;}
    void setCurrentProfileIndex(int ProfileIndex){ m_CurrentProfile = ProfileIndex; emit profilesChanged();}
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

    QString getStorageFileName(){return m_StorageFileName;}
    void setDebugScript(const QString& script){m_DebugScript = script;}
public slots:
    void process();
    void onPreferencesChanged();
signals:
    void trayShowHint(const QString& text);
    void trayActive();
    void traySleep();

    void profilesChanged();
    void applicationsChanged();
    void debugScriptResult(QString result, const sSysInfo& data);
};

#endif // CDATAMANAGER_H
