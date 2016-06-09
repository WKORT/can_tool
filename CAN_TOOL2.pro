#-------------------------------------------------
#
# Project created by QtCreator 2016-06-09T09:27:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CAN_TOOL2
TEMPLATE = app


SOURCES += main.cpp\
        cantool.cpp \
    canisotp.cpp \
    can_sequencer.cpp

HEADERS  += cantool.h \
    canisotp.h \
    std_type.h \
    can_sequencer.h

FORMS    += cantool.ui
