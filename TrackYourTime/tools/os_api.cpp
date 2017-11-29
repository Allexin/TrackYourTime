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

#include "os_api.h"
#include <QCursor>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QThread>

QStringList readFileToStringList(const QString& FileName){
    QStringList stringList;
    QFile textFile(FileName);
    if (!textFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return stringList;

    QTextStream textStream(&textFile);
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            stringList.append(line);
    }
    return stringList;
}

void writeStringListToFile(QStringList& lines, const QString& FileName, const QString& lineEnging){
    QFile textFile(FileName);
    if (!textFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream textStream(&textFile);

    for (int i = 0; i<lines.size(); i++)
        textStream << lines[i] << lineEnging;
    textFile.close();
}



#ifdef Q_OS_WIN
#include <windows.h>

QString getUserName()
{
    return qgetenv("USERNAME");
}

typedef BOOL (__stdcall *GetProcessImageFileNamePtr)(HANDLE, WCHAR* ,DWORD);

class cGetProcessImageFileName{
protected:
    GetProcessImageFileNamePtr m_GetProcessImageFileName; //psapi
    GetProcessImageFileNamePtr m_K32GetProcessImageFileName; //kernel32
    HMODULE             m_Kernel32Lib;
    HMODULE            m_psapiLib;
public:
    cGetProcessImageFileName():m_GetProcessImageFileName(0),m_K32GetProcessImageFileName(0){
        m_Kernel32Lib = LoadLibraryA("kernel32.dll");
        if (m_Kernel32Lib)
            m_K32GetProcessImageFileName = (GetProcessImageFileNamePtr)GetProcAddress(m_Kernel32Lib,"K32GetProcessImageFileNameW");
        m_psapiLib = LoadLibraryA("psapi.dll");
        if (m_psapiLib)
            m_GetProcessImageFileName = (GetProcessImageFileNamePtr)GetProcAddress(m_psapiLib,"GetProcessImageFileNameW");
    }
    ~cGetProcessImageFileName(){
        FreeLibrary(m_Kernel32Lib);
        FreeLibrary(m_psapiLib);
    }

    BOOL GetProcessFileName(HANDLE process, WCHAR* path,DWORD length){
        if (m_K32GetProcessImageFileName!=0)
            return m_K32GetProcessImageFileName(process,path,length);
        return m_GetProcessImageFileName(process,path,length);
    }
};

QString GetAppNameFromPID(DWORD pid)
{
    static cGetProcessImageFileName processFileName;
    QString appFileName = "";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION , FALSE, pid);
    if (hProcess != 0){
        try {
            WCHAR path[MAX_PATH];
            if (processFileName.GetProcessFileName(hProcess,path, MAX_PATH-1) != 0)
                appFileName=QString::fromWCharArray(path);
            else
                qCritical() << "GetProcessImageFileName Error " << GetLastError();
        }
        catch(...) {

        }
        CloseHandle(hProcess);
    }
    return appFileName;
}

struct sAppPIDs{
    DWORD ownerPID;
    DWORD childPID;
};

BOOL CALLBACK EnumChildWindowsProc(HWND wnd, LPARAM lp)
{
    sAppPIDs* pids = (sAppPIDs*)lp;
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    if (pid!=pids->ownerPID)
        pids->childPID = pid;
    return TRUE;
}

QString getWindowApplicationModernWin(HWND wnd, DWORD pid)
{
    sAppPIDs pids;
    pids.childPID = pid;
    pids.ownerPID = pid;
    EnumChildWindows(wnd,EnumChildWindowsProc,(LPARAM)&pids);
    return GetAppNameFromPID(pids.childPID);
}

QString getWindowApplication(HWND wnd)
{
    QString appFileName;
    DWORD pid;
    GetWindowThreadProcessId(wnd, &pid);
    appFileName = GetAppNameFromPID(pid);
    QFileInfo fInfo(appFileName);
    QString fName = fInfo.baseName().toUpper();
    if (fName=="WWAHOST" || fName=="APPLICATIONFRAMEHOST"){
        return getWindowApplicationModernWin(wnd,pid);
    }

    return appFileName;
}

sSysInfo getCurrentApplication()
{
    HWND wnd = GetForegroundWindow();
    QFileInfo fileInfo(getWindowApplication(wnd));
    sSysInfo appInfo;
    appInfo.fileName = fileInfo.fileName().simplified();
    appInfo.path = fileInfo.absolutePath().simplified();
    WCHAR title[256];
    int l = GetWindowTextW(wnd,title,256);
    if (l>0)
        appInfo.title = QString::fromWCharArray(title).simplified();

    return appInfo;
}

QString getAutoRunLink()
{
    QString startupFolder = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup";
    QString appPath = QCoreApplication::applicationFilePath();
    QFileInfo appInfo(appPath);
    return startupFolder+"/"+appInfo.baseName()+".lnk";
}

void setAutorun()
{
    QFile file;
    file.setFileName( QCoreApplication::applicationFilePath() );
    file.link(getAutoRunLink());
}

void removeAutorun()
{
    QFile::remove(getAutoRunLink());
}

#endif



#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

QString getUserName()
{
    return qgetenv("USER");
}

bool GetActiveWindowClassAndTitle(QString& windowClass,QString& windowTitle)
{
    Display *display	= XOpenDisplay(NULL);
    if (!display)
        return false;

    Window root 	= DefaultRootWindow(display);
    Atom active 	= XInternAtom(display, "_NET_ACTIVE_WINDOW", False);

    XTextProperty text;
    char **name = NULL;
    int param;

    Atom type_ret;
    int format_ret;
    unsigned long items_ret;
    unsigned long after_ret;
    unsigned char *prop_data = 0;

    bool success = false;

    if(XGetWindowProperty(display, root, active, 0, 0x7fffffff, False, XA_WINDOW,
    &type_ret, &format_ret, &items_ret, &after_ret, &prop_data) == Success)
    {
        Window *active_window = (Window *)prop_data;
        Atom wm_state = XInternAtom(display, "WM_CLASS", False);
        XGetTextProperty(display, *active_window, &text, wm_state);
        Xutf8TextPropertyToTextList(display, &text, &name, &param);        
        if (name!=NULL){
            success = true;
            windowClass = *name;
            XFree(name);

            XTextProperty title;
            char **Tname = NULL;
            XGetTextProperty(display, *active_window, &title, XA_WM_NAME);
            Xutf8TextPropertyToTextList(display, &title, &Tname, &param);
            if (Tname!=NULL){
                windowTitle=*Tname;
                XFree(Tname);
            }
        }

    };
    XCloseDisplay(display);

    return success;
}

sSysInfo getCurrentApplication()
{
    //TODO - now i can't get current window executable and use window class. but it's sucks.
    sSysInfo fileInfo;
    fileInfo.fileName = "";
    fileInfo.path = "";
    QString windowClass;
    QString windowTitle;
    if (GetActiveWindowClassAndTitle(windowClass,windowTitle)){
        fileInfo.fileName = windowClass.simplified();
        fileInfo.title = windowTitle.simplified();
    }

    return fileInfo;
}

QString getAutoRunLink()
{
    QString startupFolder = QDir::homePath()+"/.config/autostart";
    QDir startupDir(startupFolder);
    if (!startupDir.exists())
        startupDir.mkpath(".");
    QString appPath = QCoreApplication::applicationFilePath();
    QFileInfo appInfo(appPath);
    return startupFolder+"/"+appInfo.baseName()+".desktop";
}

void setAutorun()
{
    QString appPath = QCoreApplication::applicationFilePath();
    QFileInfo appInfo(appPath);

    QStringList desktopLink;
    desktopLink.push_back("[Desktop Entry]");
    desktopLink.push_back("Exec=cd "+QCoreApplication::applicationDirPath()+" &&  "+QCoreApplication::applicationFilePath());
    desktopLink.push_back("Icon="+QCoreApplication::applicationDirPath()+"/data/icons/main.ico");
    desktopLink.push_back("Type=Application");
    desktopLink.push_back("Terminal=false");
    desktopLink.push_back("Name="+appInfo.baseName());
    desktopLink.push_back("GenericName=Cross-platform time tracker");
    desktopLink.push_back("Categories=Office");
    desktopLink.push_back("InitialPreference=9");
    writeStringListToFile(desktopLink,getAutoRunLink(),"\n");
}

void removeAutorun()
{
    QFile::remove(getAutoRunLink());
}

#endif


#ifdef Q_OS_MAC

//#include </System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Headers/CGWindow.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>

QString getUserName()
{
    return qgetenv("USER");
}

QString uniCFStrToQStr(const CFStringRef cfString)
{
    QChar qchar;
    QString qString("");
    int lenCF=cfString != nullptr ? (int)CFStringGetLength(cfString): 0;  // возвращает длину строки
    for (int i=0;i<lenCF;i++)
    {
        qchar=((ushort)CFStringGetCharacterAtIndex(cfString,(CFIndex)i));  // получает символ из строки
        qString=qString+qchar;
    }
    return qString;
}


sSysInfo getCurrentApplication()
{
    QString appOwner;
    QString appTitle;

    //get visible windows from front to back. first window with layer 0 - current window
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    CFIndex cfiLen = CFArrayGetCount(windowList);
    CFDictionaryRef dictionary;
    for (CFIndex cfiI = 0; cfiI < cfiLen; cfiI++){
        /*
         * item sample
        kCGWindowAlpha = 1;
        kCGWindowBounds =         {
            Height = 22;
            Width = 212;
            X = 1662;
            Y = 0;
        };
        kCGWindowIsOnscreen = 1;
        kCGWindowLayer = 25;
        kCGWindowMemoryUsage = 30104;
        kCGWindowName = "";
        kCGWindowNumber = 14;
        kCGWindowOwnerName = SystemUIServer;
        kCGWindowOwnerPID = 99;
        kCGWindowSharingState = 1;
        kCGWindowStoreType = 2;
         */
        dictionary = (CFDictionaryRef) CFArrayGetValueAtIndex(windowList, cfiI);
        CFStringRef owner = reinterpret_cast<CFStringRef>(CFDictionaryGetValue(dictionary,kCGWindowOwnerName));
        CFStringRef title = reinterpret_cast<CFStringRef>(CFDictionaryGetValue(dictionary,kCGWindowName));
        CFNumberRef window_layer = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dictionary, kCGWindowLayer));
        int layer;
        CFNumberGetValue(window_layer, kCFNumberIntType, &layer);
        if (layer==0){
            appOwner = uniCFStrToQStr(owner);
            appTitle = uniCFStrToQStr(title);
            break;
        }
    }

    sSysInfo fileInfo;
    fileInfo.fileName = appOwner.simplified();
    fileInfo.path = "";
    fileInfo.title = appTitle.simplified();

    return fileInfo;
}

QString getAutoRunLink()
{
    QString appPath = QCoreApplication::applicationFilePath();
    QFileInfo appInfo(appPath);
    return QDir::homePath()+"/Library/LaunchAgents/org.sol-online."+appInfo.baseName()+".plist";
}

void setAutorun()
{
    QString appPath = QCoreApplication::applicationFilePath();
    QFileInfo appInfo(appPath);

    QStringList agentLink;
    agentLink.push_back("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    agentLink.push_back("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");
    agentLink.push_back("<plist version=\"1.0\">");
    agentLink.push_back("<dict>");
    agentLink.push_back("   <key>Label</key>");
    agentLink.push_back("   <string>org.sol-online.TrackYourTime</string>");
    agentLink.push_back("   <key>LimitLoadToSessionType</key>");
    agentLink.push_back("   <string>Aqua</string>");
    agentLink.push_back("   <key>ProgramArguments</key>");
    agentLink.push_back("   <array>");
    agentLink.push_back("       <string>"+appPath+"</string>");
    agentLink.push_back("       <string>-RunAsAgent</string>");
    agentLink.push_back("       <string>YES</string>");
    agentLink.push_back("   </array>");
    agentLink.push_back("   <key>WorkingDirectory</key>");
    agentLink.push_back("   <string>"+appInfo.absolutePath()+"/</string>");
    agentLink.push_back("   <key>RunAtLoad</key>");
    agentLink.push_back("   <true/>");
    agentLink.push_back("   <key>KeepAlive</key>");
    agentLink.push_back("   <false/>");
    agentLink.push_back("   <key>Disabled</key>");
    agentLink.push_back("   <false/>");
    agentLink.push_back("</dict>");
    agentLink.push_back("</plist>");

    writeStringListToFile(agentLink,getAutoRunLink(),"\r");
}

void removeAutorun()
{
    QFile::remove(getAutoRunLink());
}

#endif







#ifdef Q_OS_WIN
#include <windows.h>

int getIdleTime()
{
    LASTINPUTINFO li;
    li.cbSize = sizeof(LASTINPUTINFO);
    GetLastInputInfo(&li);
    DWORD te = GetTickCount();
    int elapsed = (te - li.dwTime)/1000;
    return elapsed;
}

#endif


#ifdef Q_OS_LINUX
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

int getIdleTime() {
        time_t idle_time;
        static XScreenSaverInfo *mit_info;
        Display *display;
        int screen;
        mit_info = XScreenSaverAllocInfo();
        if((display=XOpenDisplay(NULL)) == NULL) { return(-1); }
        screen = DefaultScreen(display);
        XScreenSaverQueryInfo(display, RootWindow(display,screen), mit_info);
        idle_time = (mit_info->idle) / 1000;
        XFree(mit_info);
        XCloseDisplay(display);
        return idle_time;
}
#endif

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>

int getIdleTime() {
    CFTimeInterval timeSinceLastEvent = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateHIDSystemState, kCGAnyInputEventType);
    return timeSinceLastEvent;
}
#endif
