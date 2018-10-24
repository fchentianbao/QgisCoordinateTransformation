#-------------------------------------------------
#
# Project created by QtCreator 2018-10-23T14:48:06
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TextConverter
TEMPLATE = app

include(../common.pri)
include(../coordconvert/coordconvert.pri)
include(../proj4/proj4.pri)



SOURCES += main.cpp\
        dialog.cpp \
    textfilecoordconvertdialog.cpp \
    textfileconverter.cpp

HEADERS  += dialog.h \
    textfileconverter.h \
    textfilecoordconvertdialog.h

FORMS    += dialog.ui \
    textfilecoordconvertdialog.ui
