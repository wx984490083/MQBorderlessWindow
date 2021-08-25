

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

win32:{
SOURCES += \
    $$PWD/appwindow_win.cpp
}
mac:{
SOURCES += \
    $$PWD/appwindow_mac.mm
}


HEADERS += \
    $$PWD/appwindow.h

INCLUDEPATH += $$PWD


