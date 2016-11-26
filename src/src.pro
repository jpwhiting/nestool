#-------------------------------------------------
#
# Project created by QtCreator 2016-10-19T12:07:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nestool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        nametable.cpp \
        swatch.cpp \
        tile.cpp \
        tileset.cpp \
    settingsdialog.cpp \
    edittiledialog.cpp \
    palette.cpp

HEADERS  += mainwindow.h \
        nametable.h \
        palette.h \
        swatch.h \
        tile.h \
        tileset.h \
    settingsdialog.h \
    edittiledialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    edittiledialog.ui \
    tileset.ui

RESOURCES += \
    nestool.qrc
