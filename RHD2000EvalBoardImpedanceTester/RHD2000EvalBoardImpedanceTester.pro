#-------------------------------------------------
#
# Project created by QtCreator 2015-01-02T16:42:18
#
#-------------------------------------------------

QT       -= gui

TARGET = RHD2000EvalBoardImpedanceTester
TEMPLATE = lib

DEFINES += RHD2000EVALBOARDIMPEDANCETESTER_LIBRARY

SOURCES += rhd2000evalboardimpedancetester.cpp \
    okFrontPanelDLL.cpp \
    platecontrol.cpp \
    randomnumber.cpp \
    rhd2000datablock.cpp \
    rhd2000evalboard.cpp \
    rhd2000registers.cpp \
    signalchannel.cpp \
    signalgroup.cpp \
    signalprocessor.cpp \
    signalsources.cpp

HEADERS += rhd2000evalboardimpedancetester.h\
        rhd2000evalboardimpedancetester_global.h \
    globalconstants.h \
    okFrontPanelDLL.h \
    platecontrol.h \
    qtinclude.h \
    randomnumber.h \
    rhd2000datablock.h \
    rhd2000evalboard.h \
    rhd2000plate.h \
    rhd2000registers.h \
    signalchannel.h \
    signalgroup.h \
    signalprocessor.h \
    signalsources.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
