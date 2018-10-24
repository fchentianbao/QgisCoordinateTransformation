#-------------------------------------------------
#
# Project created by QtCreator 2014-08-22T16:08:49
#
#-------------------------------------------------

QT       +=core gui sql widgets
TEMPLATE = lib
TARGET = $$qtLibraryTarget(dmCoordConvert)


DEFINES += newsWindows WIN_XP

CONFIG  += staticlib

include(../common.pri)
include(../proj4/proj4.pri)

SOURCES += \
    coordinatetransform.cpp \
    projectionselector.cpp

HEADERS +=\
    coordinatetransform.h \
    projectionselector.h

RESOURCES += \
    image.qrc

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    projectionselector.ui
