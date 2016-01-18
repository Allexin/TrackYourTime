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

#include "os_api.h"
#include <QCursor>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QThread>

QPoint getMousePos()
{
    return QCursor::pos();
}

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
#include <psapi.h>

QString getWindowApplication(HWND Wnd)
{
    QString appFileName;
    DWORD pid;
    GetWindowThreadProcessId(Wnd, &pid);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION , FALSE, pid);
    if (hProcess != 0){
        try {
            char path[MAX_PATH];
            if (GetProcessImageFileNameA(hProcess,path, MAX_PATH-1) != 0)
                appFileName=path;
            else
                qCritical() << "GetProcessImageFileName Error " << GetLastError();
        }
        catch(...) {

        }
        CloseHandle(hProcess);
    }
    return appFileName;
}

sAppFileName getCurrentApplication()
{
    QFileInfo fileInfo(getWindowApplication(GetForegroundWindow()));
    sAppFileName fileName;
    fileName.fileName = fileInfo.fileName();
    fileName.path = fileInfo.absolutePath();

    return fileName;
}

static bool KeyboardState[256];

bool isKeyboardChanged()
{
    bool stateChanged = false;
    for (int i = 0; i<256; i++)
        if (KeyboardState[i]!=(GetAsyncKeyState(i) & 0x8000)){
            stateChanged = true;
            KeyboardState[i] = !KeyboardState[i];
        }

    return stateChanged;
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

char * GetActiveWindowClass()
{
    Display *display	= XOpenDisplay(NULL);
    if (!display)
        return NULL;

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

    if(XGetWindowProperty(display, root, active, 0, 0x7fffffff, False, XA_WINDOW,
    &type_ret, &format_ret, &items_ret, &after_ret, &prop_data) == Success)
    {
        Window *active_window = (Window *)prop_data;
        Atom wm_state = XInternAtom(display, "WM_CLASS", False);
        XGetTextProperty(display, *active_window, &text, wm_state);
        Xutf8TextPropertyToTextList(display, &text, &name, &param);        
    };
    XCloseDisplay(display);
    if (name==NULL)
        return NULL;
    return *name;
}

sAppFileName getCurrentApplication()
{
    //TODO - now i can't get current window executable and use window class. but it's sucks.
    sAppFileName fileName;
    fileName.fileName = "";
    fileName.path = "";
    char* className = GetActiveWindowClass();

    if (className!=NULL)
        fileName.fileName = GetActiveWindowClass();

    return fileName;
}

// udev version. require access to /dev/input/eventX
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>

bool firstKeyboardUpdate = true;
const int KEYBOARD_STATE_SIZE = KEY_MAX/8 + 1;
struct sKeyboardState{
    QString id;
    unsigned char state[KEYBOARD_STATE_SIZE];
};
QVector<sKeyboardState> keyboards;


bool isKeyboardChanged()
{
    if (firstKeyboardUpdate){
        firstKeyboardUpdate = false;
        QStringList keyboardsList = QDir("/dev/input/by-id/").entryList(QStringList() << "*keyboard*");
        keyboards.resize(keyboardsList.size());
        for (int i = 0; i<keyboards.size(); i++){
            sKeyboardState *kb_s = &keyboards[i];
            kb_s->id = "/dev/input/by-id/"+keyboardsList[i];

        }
    }

    bool noAccesToKeyboard = true;

    for (int i = 0; i<keyboards.size(); i++){
        int fd = open(keyboards[i].id.toUtf8().constData(), 0);
        if (fd>-1){
            noAccesToKeyboard = false;
            unsigned char key_b[KEY_MAX/8 + 1];
            memset(key_b, 0, KEYBOARD_STATE_SIZE);
            ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);
            close(fd);

            if (memcmp(keyboards[i].state,key_b,KEYBOARD_STATE_SIZE)!=0){
                memcpy(keyboards[i].state,key_b,KEYBOARD_STATE_SIZE);
                return true;
            }
        }
    }

    //if we have't access to keyboard, we can't fall asleep
    if (noAccesToKeyboard)
        return true;

    return false;

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
    writeStringListToFile(desktopLink,getAutoRunLink(),'\n');
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

QString uniCFStrToQStr(const CFStringRef cfString)
{
    QChar qchar;
    QString qString("");
    int lenCF=(int)CFStringGetLength(cfString);  // возвращает длину строки
    for (int i=0;i<lenCF;i++)
    {
        qchar=((ushort)CFStringGetCharacterAtIndex(cfString,(CFIndex)i));  // получает символ из строки
        qString=qString+qchar;
    }
    return qString;
}


sAppFileName getCurrentApplication()
{
    QString appOwner;

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
        CFNumberRef window_layer = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dictionary, kCGWindowLayer));
        int layer;
        CFNumberGetValue(window_layer, kCFNumberIntType, &layer);
        if (layer==0){
            appOwner = uniCFStrToQStr(owner);
            break;
        }
    }

    sAppFileName fileName;
    fileName.fileName = appOwner;
    fileName.path = "";

    return fileName;
}

//i know, this is wrong way. but it's simple, and i do not need slot/signals functionaloty in this part of code.
//also i do not want add synchronization to read/write bool isKeyboardChanged() because any collisions can't break anything
class cMacOSXKeyboardThread : public QThread
{
private:
    static bool m_KeyboardChanged;

    // The following callback method is invoked on every keypress.
    static CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
        if (type != kCGEventKeyDown && type != kCGEventFlagsChanged && type != kCGEventKeyUp) {
            return event;
        }

        m_KeyboardChanged = true;

        return event;
    }

    virtual void run() override
    {
        m_KeyboardChanged = false;

        CGEventMask eventMask = (CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged));
        CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, CGEventCallback, NULL );

        if(!eventTap) {
            qCritical() << "ERROR: Unable to create event tap.\n";
            return;
        }

        CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
        CGEventTapEnable(eventTap, true);

        CFRunLoopRun();
    }
public:
    bool isKeyboardChanged(){
        bool keyboardChanged = m_KeyboardChanged;
        m_KeyboardChanged = false;
        return keyboardChanged;
    }
};
bool cMacOSXKeyboardThread::m_KeyboardChanged;

static cMacOSXKeyboardThread loopThread;

bool isKeyboardChanged()
{
    if (!loopThread.isRunning())
        loopThread.start();

    return loopThread.isFinished() || loopThread.isKeyboardChanged(); //if thread finished - something goes wrong, but application can work even without sleep functionality
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
