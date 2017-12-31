#-------------------------------------------------
#
# Project created by QtCreator 2017-10-25T21:19:58
#
#-------------------------------------------------

QT       += core gui network webkitwidgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sMail
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
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
    smtp.cpp \
    pop.cpp \
    mime.cpp \
    maileditwidget.cpp \
    logindialog.cpp \
    attachmentwidget.cpp \
    smaildb.cpp \
    user.cpp \
    mailmodel.cpp

HEADERS += \
        mainwindow.h \
    smtp.h \
    pop.h \
    mime.h \
    maileditwidget.h \
    logindialog.h \
    attachmentwidget.h \
    smaildb.h \
    user.h \
    mailmodel.h

FORMS += \
        mainwindow.ui \
    maileditwidget.ui \
    logindialog.ui

RESOURCES += \
    res.qrc
