cmake_minimum_required(VERSION 3.16)

project(soundchest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(soundchest
    src/main.cpp
    src/theme.cpp
    src/icons.cpp
    src/ui/mainwindow.cpp
    src/ui/headerbar.cpp
    src/ui/transportbar.cpp
    src/ui/waveformwidget.cpp
    src/ui/folderbrowser.cpp
    src/ui/filetable.cpp
    resources.qrc
)

target_include_directories(soundchest PRIVATE src)

target_link_libraries(soundchest PRIVATE Qt6::Widgets Qt6::Svg)