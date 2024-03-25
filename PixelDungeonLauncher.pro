QT       += core gui network gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

#VERSION = 1.0
RC_ICONS = windows.ico
RC_LANG = 0x0800
QMAKE_TARGET_DESCRIPTION = "Desktop Launcher for the Pixel Dungeon"
QMAKE_TARGET_COPYRIGHT = "Copyright(C) 2024"
QMAKE_TARGET_PRODUCT = "Pixel Dungeon Launcher"


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    downloadfile.cpp \
    loading.cpp \
    main.cpp \
    mainwindow.cpp \
    mythread.cpp \
    publicvariables.cpp

HEADERS += \
    downloadfile.h \
    loading.h \
    mainwindow.h \
    mythread.h \
    publicvariables.h

FORMS += \
    loading.ui \
    mainwindow.ui

TRANSLATIONS += PixelDungeonLauncher_zh_CN.ts\
    PixelDungeonLauncher_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations
CONFIG += skip_target_version_ext

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
DISTFILES += \
    PixelDungeonLauncher_en_US.ts \
    PixelDungeonLauncher_zh_CN.ts \
    loding.gif \
    windows.ico
