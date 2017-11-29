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

#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <QVector>



enum eActiveWindowState{
    ACTIVE_WINDOW_OK,
    CANT_OPEN_X_DYSPLAY,
    CANT_GET_NET_ACTIVE_WINDOW,
    CANT_GET_WINDOW_CLASS
};

eActiveWindowState checkActiveWindow(){
    Display *display	= XOpenDisplay(NULL);
    if (!display)
        return CANT_OPEN_X_DYSPLAY;

    Window root 	= DefaultRootWindow(display);
    Atom active 	= XInternAtom(display, "_NET_ACTIVE_WINDOW", False);

    XTextProperty text;

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
	XCloseDisplay(display);
        if (text.value==NULL)
            return CANT_GET_WINDOW_CLASS;
        return ACTIVE_WINDOW_OK;
    }
    else{
	XCloseDisplay(display);
        return CANT_GET_NET_ACTIVE_WINDOW;
	}
}

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    eActiveWindowState av_state = checkActiveWindow();

    switch(av_state){
    case ACTIVE_WINDOW_OK:{
        qStdOut() << "X Window System - OK" << '\n';
    }
        break;
    case CANT_OPEN_X_DYSPLAY:{
        qStdOut() << "X Window System - FAILED" << '\n';
        qStdOut() << "XOpenDisplay - FAILED" << '\n';
        qStdOut() << "How to fix: run X Window System(e.g startx)" << '\n';
    }
        break;
    case CANT_GET_NET_ACTIVE_WINDOW:{time_t idle_time;
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
        qStdOut() << "X Window System - FAILED" << '\n';
        qStdOut() << "_NET_ACTIVE_WINDOW property not available" << '\n';
        qStdOut() << "How to fix: no way to fix it" << '\n';
    }
        break;
    case CANT_GET_WINDOW_CLASS:{
        qStdOut() << "X Window System - FAILED" << '\n';
        qStdOut() << "WM_CLASS property not available" << '\n';
        qStdOut() << "How to fix: no way to fix it" << '\n';
    }
        break;
    }
    if (av_state!=ACTIVE_WINDOW_OK)
        qStdOut() << "X Window System failure is critical. Before you fix it - time tracker can't work" << '\n';

    return 0;
}
