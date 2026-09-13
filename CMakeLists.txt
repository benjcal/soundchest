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
    src/audio/miniaudio_impl.cpp
    src/audio/sndfiledatasource.cpp
    src/audio/player.cpp
    src/library/audio_file.cpp
    src/library/folder.cpp
    src/waveform/builder.cpp
    src/waveform/peaks_cache.cpp
    src/ui/icons.cpp
    src/ui/window.cpp
    src/ui/headerbar.cpp
    src/ui/transport_controls.cpp
    src/ui/waveform_widget.cpp
    src/ui/waveform_painter.cpp
    src/ui/folder_tree_widget.cpp
    src/ui/file_table_widget.cpp
    src/ui/folder_tree_widget_model.cpp
    src/ui/file_table_widget_model.cpp
    src/ui/file_table_widget_waveform_delegate.cpp
    resources.qrc
)

target_include_directories(soundchest PRIVATE src third_party/miniaudio)

target_link_libraries(soundchest PRIVATE Qt6::Widgets Qt6::Svg Qt6::Concurrent qlementine::qlementine sndfile asound pulse dl pthread m)
