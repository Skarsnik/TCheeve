INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += network
DEFINES += QT_DEPRECATED_WARNINGS_SINCE=0x50800

SOURCES += \
    $$PWD/emunwaccessclient.cpp

HEADERS += \
    $$PWD/emunwaccessclient.h

OTHER_FILES += \
    $$PWD/README.md
