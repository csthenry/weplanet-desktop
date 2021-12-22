QT       += core gui
QT       += sql
QT       += charts
QT       += network
QT       += axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    checkupdate.cpp \
    comboboxdelegate.cpp \
    excelexport.cpp \
    formlogin.cpp \
    main.cpp \
    mainwindow.cpp \
    querymodel.cpp \
    readOnlyDelegate.cpp \
    service.cpp

HEADERS += \
    checkupdate.h \
    comboboxdelegate.h \
    excelexport.h \
    formlogin.h \
    mainwindow.h \
    querymodel.h \
    readOnlyDelegate.h \
    service.h

FORMS += \
    formlogin.ui \
    mainwindow.ui

RC_ICONS = mainWindow_favicon_x32.ico

#info
VERSION = 0.0.4

QMAKE_TARGET_DESCRIPTION = Magic Light Assistant

QMAKE_TARGET_COMPANY = bytecho.net

QMAKE_TARGET_COPYRIGHT = Copyright (c) 2017-2021 bytecho.net. Written by Henry.

RC_LANG = 0x0004

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
