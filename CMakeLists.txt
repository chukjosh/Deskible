cmake_minimum_required(VERSION 3.16)

project(Deskible VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

add_executable(Deskible
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp
    src/biblelocalreader.h src/biblelocalreader.cpp
    src/settingsdialog.h src/settingsdialog.cpp
    src/systemtraymanager.h src/systemtraymanager.cpp
    src/startupmanager.h src/startupmanager.cpp
    resources/resources.qrc
)

target_link_libraries(Deskible PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)

if(WIN32)
    set_target_properties(Deskible PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
