# Pinned MIT SARibbon v2.9.0, compiled into the application (no new runtime DLL).
INCLUDEPATH += $$PWD
HEADERS += $$PWD/SARibbon.h
SOURCES += $$PWD/SARibbon.cpp
QT += svg
DEFINES += SARIBBON_USE_3RDPARTY_FRAMELESSHELPER=0
msvc {
    DEFINES += NOMINMAX
    QMAKE_CXXFLAGS += /bigobj
}
