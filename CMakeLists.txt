cmake_minimum_required(VERSION 3.21)

project(soundchest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Concurrent Multimedia)
find_package(qlementine REQUIRED)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(soundchest
    src/main.cpp
    src/app/library_controller.cpp
    src/app/playback_controller.cpp
    src/app/waveform_controller.cpp
    src/audio/player.cpp
    src/audio/audio_file_reader.cpp
    src/library/folder.cpp
    src/library/audio_library_scanner.cpp
    src/waveform/peaks_builder.cpp
    src/ui/icons.cpp
    src/ui/theme.cpp
    src/ui/window.cpp
    src/ui/headerbar.cpp
    src/ui/transport_controls.cpp
    src/ui/waveform_widget.cpp
    src/ui/waveform_renderer.cpp
    src/ui/folder_tree_widget.cpp
    src/ui/file_table_widget.cpp
    src/ui/folder_tree_widget_model.cpp
    src/ui/file_table_widget_model.cpp
    resources.qrc
)

target_include_directories(soundchest PRIVATE src)

target_link_libraries(soundchest PRIVATE Qt6::Widgets Qt6::Svg Qt6::Concurrent Qt6::Multimedia qlementine::qlementine sndfile dl pthread m)

qt_add_executable(soundchest_test
    tests/soundchest_test.cpp
    src/audio/audio_file_reader.cpp
    src/library/audio_library_scanner.cpp
    src/library/folder.cpp
    src/waveform/peaks_builder.cpp
)
target_include_directories(soundchest_test PRIVATE src)
target_link_libraries(soundchest_test PRIVATE Qt6::Core sndfile)

qt_add_executable(controller_flow
    tests/controller_flow.cpp
    src/app/library_controller.cpp
    src/app/playback_controller.cpp
    src/app/waveform_controller.cpp
    src/audio/audio_file_reader.cpp
    src/audio/player.cpp
    src/library/audio_library_scanner.cpp
    src/library/folder.cpp
    src/waveform/peaks_builder.cpp
    src/ui/window.cpp
    src/ui/headerbar.cpp
    src/ui/transport_controls.cpp
    src/ui/waveform_widget.cpp
    src/ui/waveform_renderer.cpp
    src/ui/folder_tree_widget.cpp
    src/ui/file_table_widget.cpp
    src/ui/folder_tree_widget_model.cpp
    src/ui/file_table_widget_model.cpp
    src/ui/icons.cpp
    src/ui/theme.cpp
    resources.qrc
)
target_include_directories(controller_flow PRIVATE src)
target_link_libraries(controller_flow PRIVATE Qt6::Widgets Qt6::Svg Qt6::Concurrent Qt6::Multimedia qlementine::qlementine sndfile dl pthread m)
