cmake_minimum_required(VERSION 3.21)

project(soundchest VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Concurrent Multimedia)
find_package(qlementine REQUIRED)

# libsndfile and libebur128 provide pkg-config files but no CMake config on most
# systems, so prefer pkg-config and fall back to plain find_path/find_library
# when it is unavailable.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(EBUR128 QUIET IMPORTED_TARGET libebur128)
    pkg_check_modules(SNDFILE QUIET IMPORTED_TARGET sndfile)
endif()

if(EBUR128_FOUND)
    set(SOUNDCHEST_EBUR128 PkgConfig::EBUR128)
else()
    find_path(EBUR128_INCLUDE_DIR NAMES ebur128.h REQUIRED)
    find_library(EBUR128_LIBRARY NAMES ebur128 REQUIRED)
    add_library(soundchest_ebur128 INTERFACE)
    target_include_directories(soundchest_ebur128 INTERFACE ${EBUR128_INCLUDE_DIR})
    target_link_libraries(soundchest_ebur128 INTERFACE ${EBUR128_LIBRARY})
    set(SOUNDCHEST_EBUR128 soundchest_ebur128)
endif()

if(SNDFILE_FOUND)
    set(SOUNDCHEST_SNDFILE PkgConfig::SNDFILE)
else()
    find_path(SNDFILE_INCLUDE_DIR NAMES sndfile.h REQUIRED)
    find_library(SNDFILE_LIBRARY NAMES sndfile REQUIRED)
    add_library(soundchest_sndfile INTERFACE)
    target_include_directories(soundchest_sndfile INTERFACE ${SNDFILE_INCLUDE_DIR})
    target_link_libraries(soundchest_sndfile INTERFACE ${SNDFILE_LIBRARY})
    set(SOUNDCHEST_SNDFILE soundchest_sndfile)
endif()

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

set(DOMAIN_SOURCES
    src/analysis/sound_analyzer.cpp
    src/audio/audio_file_reader.cpp
    src/library/audio_library_scanner.cpp
    src/library/file_exporter.cpp
    src/library/folder.cpp
    src/waveform/peaks_builder.cpp
)

set(APP_SOURCES
    src/app/analysis_controller.cpp
    src/app/export_controller.cpp
    src/app/library_controller.cpp
    src/app/playback_controller.cpp
    src/app/waveform_controller.cpp
    src/audio/player.cpp
    src/ui/icons.cpp
    src/ui/style.cpp
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

qt_add_executable(soundchest
    src/main.cpp
    ${DOMAIN_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(soundchest PRIVATE src)
target_compile_definitions(soundchest PRIVATE SOUNDCHEST_VERSION="${PROJECT_VERSION}")

target_link_libraries(soundchest PRIVATE
    Qt6::Widgets Qt6::Svg Qt6::Concurrent Qt6::Multimedia
    qlementine::qlementine
    ${SOUNDCHEST_SNDFILE} ${SOUNDCHEST_EBUR128}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(soundchest PRIVATE dl pthread m)
endif()

if(APPLE)
    target_sources(soundchest PRIVATE assets/generated/soundchest.icns)
    set_source_files_properties(assets/generated/soundchest.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(WIN32)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/generated/soundchest.ico" SOUNDCHEST_ICON_PATH)
    configure_file(packaging/windows/soundchest.rc.in "${CMAKE_CURRENT_BINARY_DIR}/soundchest.rc" @ONLY)
    target_sources(soundchest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/soundchest.rc")
endif()

set_target_properties(soundchest PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Sound Chest"
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.benjcal.soundchest"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_ICON_FILE "soundchest.icns"
)

qt_add_executable(soundchest_test
    tests/soundchest_test.cpp
    ${DOMAIN_SOURCES}
)
target_include_directories(soundchest_test PRIVATE src)
target_link_libraries(soundchest_test PRIVATE Qt6::Core ${SOUNDCHEST_SNDFILE} ${SOUNDCHEST_EBUR128})

qt_add_executable(controller_flow
    tests/controller_flow.cpp
    ${DOMAIN_SOURCES}
    ${APP_SOURCES}
)
target_include_directories(controller_flow PRIVATE src)
target_link_libraries(controller_flow PRIVATE
    Qt6::Widgets Qt6::Svg Qt6::Concurrent Qt6::Multimedia
    qlementine::qlementine
    ${SOUNDCHEST_SNDFILE} ${SOUNDCHEST_EBUR128}
)
if(UNIX AND NOT APPLE)
    target_link_libraries(controller_flow PRIVATE dl pthread m)
endif()

enable_testing()
add_test(NAME soundchest_test COMMAND soundchest_test)

install(TARGETS soundchest
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES packaging/linux/soundchest.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES assets/branding/soundchest-appicon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME soundchest.svg)
    foreach(size 16 24 32 48 64 128 256 512 1024)
        install(FILES assets/generated/soundchest-${size}.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps
            RENAME soundchest.png)
    endforeach()
    install(FILES LICENSE THIRD_PARTY_NOTICES.md DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/soundchest)
endif()
