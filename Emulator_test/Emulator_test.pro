#-------------------------------------------------
#
# Project created by QtCreator 2018-05-07T22:10:04
#
#-------------------------------------------------

QT       += core gui printsupport xml sql serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Emulator_test
TEMPLATE = app

VERSION = 0.9.2    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qcustomplot.cpp \
    ../../svlib/sv_settings.cpp \
    ../../svlib/sv_log.cpp \
    ../../svlib/sv_sqlite.cpp \
    geo.cpp \
    nmea.cpp \
    sv_ais.cpp \
    sv_area.cpp \
    sv_gps.cpp \
    sv_navtexeditor.cpp \
    sv_networkeditor.cpp \
    sv_serialeditor.cpp \
    sv_vesseleditor.cpp \
    sv_vessel.cpp \
    sv_mapobjects.cpp \
    sv_echo.cpp \
    sv_lag.cpp \
    sv_navtex.cpp \
    ../../svlib/sv_tcpserverclient.cpp \
    ../../svlib/sv_secondmeter.cpp \
    sv_navstateditor.cpp

HEADERS += \
        mainwindow.h \
    qcustomplot.h \
    ../../svlib/sv_exception.h \
    ../../svlib/sv_settings.h \
    ../../svlib/sv_log.h \
    ../../svlib/sv_sqlite.h \
    geo.h \
    nmea.h \
    sql_defs.h \
    sv_ais.h \
    sv_area.h \
    sv_gps.h \
    sv_navtexeditor.h \
    sv_networkeditor.h \
    sv_serialeditor.h \
    sv_vesseleditor.h \
    sv_idevice.h \
    sv_vessel.h \
    sv_mapobjects.h \
    sv_echo.h \
    sv_lag.h \
    sv_navtex.h \
    ../../svlib/sv_tcpserverclient.h \
    ../../svlib/sv_secondmeter.h \
    sv_navstateditor.h

FORMS += \
        mainwindow.ui \
    sv_networkeditor.ui \
    sv_serialeditor.ui \
    sv_vesseleditor.ui \
    sv_navtexeditor.ui \
    sv_navstateditor.ui

RESOURCES += \
    res.qrc
