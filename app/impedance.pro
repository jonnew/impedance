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

LIBS=-ldl

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
    platecontrol.cpp \
    autoimpedance.cpp \
    impedancelog.cpp

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
    platecontrol.h \
    autoimpedance.h \
    impedancelog.h

# Installation step to transfer required files to build directory
install_it.path = $$OUT_PWD
install_it.files += \
    ./resources/main.bit\
    ./resources/linux-64/libokFrontPanel.so \
    ./resources/mac-os-x/libokFrontPanel.dylib \
    ./resources/windows/okFrontPanel.dll

INSTALLS += install_it

