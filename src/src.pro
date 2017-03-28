#-------------------------------------------------
#
# Project created by QtCreator 2016-10-19T12:07:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nestool
TEMPLATE = app


SOURCES += \
    colorchooserdialog.cpp \
    edittiledialog.cpp \
    importdialog.cpp \
    main.cpp\
    mainwindow.cpp \
    nametable.cpp \
    palette.cpp \
    project.cpp \
    settingsdialog.cpp \
    swatch.cpp \
    tile.cpp \
    tileset.cpp

HEADERS  += \
    colorchooserdialog.h \
    edittiledialog.h \
    importdialog.h \
    mainwindow.h \
    nametable.h \
    palette.h \
    project.h \
    settingsdialog.h \
    swatch.h \
    tile.h \
    tileset.h

FORMS    += \
    colorchooserdialog.ui \
    edittiledialog.ui \
    importdialog.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tileset.ui

RESOURCES += \
    nestool.qrc
