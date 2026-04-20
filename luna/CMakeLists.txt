cmake_minimum_required(VERSION 3.16)
project(LunarCalendar VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

message(STATUS "Building with Qt ${Qt${QT_VERSION_MAJOR}_VERSION}")

set(SOURCES
    main.cpp
    MainWindow.cpp
    LunarCalculator.cpp
    MoonWidget.cpp
)

set(HEADERS
    MainWindow.h
    LunarCalculator.h
    MoonWidget.h
)

add_executable(LunarCalendar ${SOURCES} ${HEADERS})

target_link_libraries(LunarCalendar PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)


if(APPLE)
    set_target_properties(LunarCalendar PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Lunar Calendar"
    )
endif()

if(WIN32)
    set_target_properties(LunarCalendar PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()


install(TARGETS LunarCalendar
    RUNTIME DESTINATION bin
    BUNDLE  DESTINATION .
)
