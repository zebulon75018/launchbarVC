QT += core gui widgets

CONFIG += c++11

TARGET = LaunchBar
TEMPLATE = app

SOURCES += \
    main.cpp \
    launchbar.cpp

HEADERS += \
    launchbar.h


# Fichier de ressources
RESOURCES += \
    resources.qrc


# Installation
target.path = /usr/local/bin
INSTALLS += target
