QT       += core widgets
CONFIG   += c++17
TARGET    = LunarCalendar
TEMPLATE  = app

# Suppress deprecated-API warnings (set to the Qt version you are using)
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    LunarCalculator.cpp \
    MoonWidget.cpp

HEADERS += \
    MainWindow.h \
    LunarCalculator.h \
    MoonWidget.h

# macOS bundle
macx {
    QMAKE_INFO_PLIST = Info.plist
}

# Windows: no console window in release builds
win32:CONFIG(release, debug|release): QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS
