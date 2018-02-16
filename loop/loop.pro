#-------------------------------------------------
#
# Project created by QtCreator 2017-10-04T15:58:54
#
#-------------------------------------------------

QT       += core gui charts widgets

CONFIG -= app_bundle

TARGET = ClosedLoop_BMI_GUI
TEMPLATE = app


SOURCES += main.cpp\
        gui.cpp \
    chart.cpp \
    protocole.cpp \
    spkandbehav.cpp

HEADERS  += gui.h \
    alp.h \
    cbhwlib.h \
    cbsdk.h \
    chart.h \
    defines.h \
    protocole.h \
    spkandbehav.h
	
DISTFILES += \
    loop.ico

RC_ICONS = loop.ico

unix|win32: LIBS += -L$$PWD/libNSP/ -lcbsdkx64

INCLUDEPATH += $$PWD/libNSP
DEPENDPATH += $$PWD/libNSP

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/libNSP/cbsdkx64.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/libNSP/libcbsdkx64.a

QMAKE_CXXFLAGS_RELEASE *= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O3

unix|win32: LIBS += -L$$PWD/libALP/ -lalp4395

INCLUDEPATH += $$PWD/libALP
DEPENDPATH += $$PWD/libALP
