#-------------------------------------------------
#
# Project created by QtCreator 2015-01-02T16:50:15
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../RHD2000EvalBoardImpedanceTester/build/release/ -lRHD2000EvalBoardImpedanceTester
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../RHD2000EvalBoardImpedanceTester/build/debug/ -lRHD2000EvalBoardImpedanceTester
else:unix: LIBS += -L$$PWD/../../RHD2000EvalBoardImpedanceTester/build/ -lRHD2000EvalBoardImpedanceTester

INCLUDEPATH += $$PWD/../../RHD2000EvalBoardImpedanceTester/build/debug
DEPENDPATH += $$PWD/../../RHD2000EvalBoardImpedanceTester/build/debug

HEADERS += \
    rhd2000evalboardimpedancetester.h \
    rhd2000evalboardimpedancetester_global.h
