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


//LINUX
/*
xprop -root


dpy = XOpenDisplay(NULL);
window = XDefaultRootWindow(dpy);
XCloseDisplay(dpy);*/
