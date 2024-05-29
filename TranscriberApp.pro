QT += core gui

CONFIG += c++11

TARGET = TranscriberApp
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp

HEADERS += mainwindow.h \
           json.hpp

FORMS += mainwindow.ui

LIBS += -lcurl

INCLUDEPATH += path/to/your/curl/include
DEPENDPATH += path/to/your/curl/include

LIBS += -Lpath/to/your/curl/lib -lcurl
