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

QPoint getMousePos()
{
    return QCursor::pos();
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

bool isKeyPressed(unsigned char KeyCode)
{
    return (GetAsyncKeyState(KeyCode) & 0x8000)!=0;
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

void writeStringListToFile(QStringList& lines, const QString& FileName){
    QFile textFile(FileName);
    if (!textFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream textStream(&textFile);

    for (int i = 0; i<lines.size(); i++)
        textStream << lines[i] << '\n';
    textFile.close();
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
    writeStringListToFile(desktopLink,getAutoRunLink());
}

void removeAutorun()
{
    QFile::remove(getAutoRunLink());
}

#endif
