QT       += core gui
QT += core
QT += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dashboardwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    payloadhistory.cpp


HEADERS += \
    client_callback.h \
    dashboardwidget.h \
    mainwindow.h \
    payloadhistory.h \
    subscriber.h

FORMS += \
    mainwindow.ui

unix|win32: LIBS += -lpaho-mqtt3c
unix|win32: LIBS += -lpaho-mqtt3a
unix|win32: LIBS += -lpaho-mqttpp3



