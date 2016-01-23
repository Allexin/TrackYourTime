#-------------------------------------------------
#
# Project created by QtCreator 2016-01-07T17:46:23
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

mac:LIBS += -framework CoreGraphics
mac:LIBS += -framework AppKit

TARGET = TrackYourTime
TEMPLATE = app
CONFIG+=address_sanitizer

TRANSLATIONS = lang_en.ts  lang_ru.ts

unix:!mac:QMAKE_CXXFLAGS += -std=c++0x

win32:QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01

win32:RC_ICONS = main.ico
mac:ICON = main.icns

win32:LIBS += -luser32
unix:!mac:LIBS += -lX11


SOURCES += \
    main.cpp \
    settingswindow.cpp \
    os_api.cpp \
    cdatamanager.cpp \
    cfilebin.cpp \
    ctrayicon.cpp \
    statisticwindow.cpp \
    applicationswindow.cpp \
    profileswindow.cpp \
    cexternaltrackers.cpp \
    aboutwindow.cpp

HEADERS  += \
    settingswindow.h \
    os_api.h \
    cdatamanager.h \
    cfilebin.h \
    ctrayicon.h \
    statisticwindow.h \
    applicationswindow.h \
    profileswindow.h \
    cexternaltrackers.h \
    aboutwindow.h

FORMS    += \
    settingswindow.ui \
    statisticwindow.ui \
    applicationswindow.ui \
    profileswindow.ui \
    aboutwindow.ui
