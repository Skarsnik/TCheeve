QT       += widgets core gui network websockets texttospeech qml quick multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(EmuNWAccess-qt.pri)
include(SkarsnikQtCommon/Logs/sqlogging.pri)
include(SkarsnikQtCommon/SQApplication/sqapplication.pri)

INCLUDEPATH += rcheevos

#QMAKE_CXXFLAGS += /fsanitize=address

SOURCES += \
    achievementchecker.cpp \
    achievementlistitem.cpp \
    achievementmodel.cpp \
    badgeimageprovider.cpp \
    logindialog.cpp \
    main.cpp \
    memoryviewer.cpp \
    memoryviewwidget.cpp \
    nwaccess.cpp \
    ramanager.cpp \
    rastuff.cpp \
    rcheevos/condition.c \
    rcheevos/condset.c \
    rcheevos/format.c \
    rcheevos/rc_validate.c \
    rcheevos/memref.c \
    rcheevos/alloc.c \
    rcheevos/value.c \
    rcheevos/operand.c \
    rcheevos/consoleinfo.c \
    rcheevos/rc_util.c \
    rcheevos/trigger.c \
    thankscheeveengine.cpp \
    usb2snes.cpp

HEADERS += \
    achievementchecker.h \
    achievementlistitem.h \
    achievementmodel.h \
    badgeimageprovider.h \
    logindialog.h \
    memoryviewer.h \
    memoryviewwidget.h \
    nwaccess.h \
    ramanager.h \
    rastruct.h \
    rastuff.h \
    rcheevos/rc_error.h \
    rcheevos/rc_internal.h \
    rcheevos/rc_runtime_types.h \
    rcheevos/rc_util.h \
    rcheevos/rc_validate.h \
    rcheevos/rc_export.h \
    rcheevos/rc_compat.h \
    rcheevos/rc_consoles.h \
    rcheevos/rc_version.h \
    sharedstruct.h \
    thankscheeveengine.h \
    usb2snes.h

DEFINES += RC_DISABLE_LUA
DEFINES += SQPROJECT_DEVEL

RC_FILE = ThanksCheeve.rc

FORMS += \
    achievementlistitem.ui \
    logindialog.ui \
    memoryviewer.ui \
    rastuff.ui

DISTFILES += \
    sqproject.json


RESOURCES += \
    ThanksCheeve.qrc

TARGET = TCheeve
