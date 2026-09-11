cmake_minimum_required(VERSION 3.16)

project(soundchest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Concurrent)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(soundchest
    src/main.cpp
    src/app.cpp
    src/theme.cpp
    src/icons.cpp
    src/audio/miniaudio_impl.cpp
    src/audio/audiofile.cpp
    src/audio/sfutil.cpp
    src/audio/sf_data_source.cpp
    src/audio/player.cpp
    src/catalog/foldertree.cpp
    src/audio/waveformbuilder.cpp
    src/audio/waveformcache.cpp
    src/ui/mainwindow.cpp
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

target_link_libraries(soundchest PRIVATE Qt6::Widgets Qt6::Svg Qt6::Concurrent sndfile asound pulse dl pthread m)