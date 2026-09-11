cmake_minimum_required(VERSION 3.21)

project(soundchest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Concurrent)
find_package(qlementine REQUIRED)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(soundchest
    src/main.cpp
    src/appcontroller.cpp
    src/icons.cpp
    src/audio/miniaudio_impl.cpp
    src/audio/audioinfo.cpp
    src/audio/sndfiledatasource.cpp
    src/audio/player.cpp
    src/catalog/foldertree.cpp
    src/waveform/waveformbuilder.cpp
    src/waveform/waveformcache.cpp
    src/waveform/waveformrenderer.cpp
    src/ui/mainwindow.cpp
    src/ui/mainview.cpp
    src/ui/headerbar.cpp
    src/ui/transportbar.cpp
    src/ui/waveformwidget.cpp
    src/ui/waveformdelegate.cpp
    src/ui/folderbrowser.cpp
    src/ui/filetable.cpp
    src/ui/foldertreemodel.cpp
    src/ui/filetablemodel.cpp
    resources.qrc
)

target_include_directories(soundchest PRIVATE src third_party/miniaudio)

target_link_libraries(soundchest PRIVATE Qt6::Widgets Qt6::Svg Qt6::Concurrent qlementine::qlementine sndfile asound pulse dl pthread m)
