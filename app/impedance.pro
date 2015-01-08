#-------------------------------------------------
#
# Project created by QtCreator 2014-11-03T15:32:58
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = impedance.bin

TEMPLATE = app
CONFIG   += console
CONFIG   -= app_bundle

#LIBS=-ldl

SOURCES += main.cpp \
    okFrontPanelDLL.cpp \
    rhd2000datablock.cpp \
    rhd2000evalboard.cpp \
    rhd2000registers.cpp \
    rhd2000impedance.cpp \
    signalprocessor.cpp \
    signalchannel.cpp \
    randomnumber.cpp \
    signalgroup.cpp \
    signalsources.cpp \
    platecontrol.cpp

OTHER_FILES += \
    okFrontPanel.dll\
#    okFrontPanel.so \
    main.bit

HEADERS += \
    okFrontPanelDLL.h \
    rhd2000datablock.h \
    rhd2000evalboard.h \
    rhd2000registers.h \
    rhd2000impedance.h \
    signalprocessor.h \
    globalconstants.h \
    signalchannel.h \
    randomnumber.h \
    signalgroup.h \
    signalsources.h \
    qtinclude.h \
    platecontrol.h
