#include "analysis/sound_analyzer.h"
#include "app/analysis_controller.h"
#include "app/export_controller.h"
#include "app/library_controller.h"
#include "app/playback_controller.h"
#include "app/waveform_controller.h"
#include "audio/player.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/style.h"
#include "ui/window.h"
#include "waveform/peaks_builder.h"

#include <QApplication>
#include <QIcon>
#include <QLoggingCategory>
#include <QTimer>

#include <oclero/qlementine/resources/ResourceInitialization.hpp>

#ifndef SOUNDCHEST_VERSION
#define SOUNDCHEST_VERSION "0.1.0"
#endif

int main(int argc, char *argv[]) {
#ifdef Q_OS_LINUX
    // Prefer the desktop's own file picker (xdg-desktop-portal) so qlementine
    // never styles native dialogs. Falls back to the platform dialog when no
    // portal is running.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORMTHEME"))
        qputenv("QT_QPA_PLATFORMTHEME", "xdgdesktopportal");
#endif

    // Audio-only app: skip the FFmpeg video hardware probes, which print driver
    // noise such as VDPAU errors on machines without those drivers.
    if (qEnvironmentVariableIsEmpty("QT_FFMPEG_DECODING_HW_DEVICE_TYPES"))
        qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES", ",");
    if (qEnvironmentVariableIsEmpty("QT_FFMPEG_ENCODING_HW_DEVICE_TYPES"))
        qputenv("QT_FFMPEG_ENCODING_HW_DEVICE_TYPES", ",");

    QApplication app(argc, argv);

    // Quiet by default; QT_LOGGING_RULES still overrides this.
    QLoggingCategory::setFilterRules(QStringLiteral("qt.multimedia.ffmpeg*.info=false"));

    app.setApplicationName(QStringLiteral("soundchest"));
    app.setApplicationDisplayName(QStringLiteral("Sound Chest"));
    app.setOrganizationName(QStringLiteral("soundchest"));
    app.setApplicationVersion(QStringLiteral(SOUNDCHEST_VERSION));
    app.setDesktopFileName(QStringLiteral("soundchest"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/branding/appicon.svg")));

    oclero::qlementine::resources::initializeResources();

    auto *style = new ui::Style;
    QApplication::setStyle(style);
    style->setThemeJsonPath(QStringLiteral(":/themes/gruvbox.json"));

    ui::Window                window;
    ui::FolderTreeWidgetModel folderModel;
    ui::FileTableWidgetModel  fileModel;
    audio::Player             player;
    waveform::PeaksBuilder    peaksBuilder;
    analysis::SoundAnalyzer   soundAnalyzer;

    app::LibraryController  libraryController(&window, &folderModel, &fileModel);
    app::WaveformController waveformController(&window, &peaksBuilder);
    app::PlaybackController playbackController(&window, &player);
    app::AnalysisController analysisController(&window, &soundAnalyzer);
    app::ExportController   exportController(&window, window.fileTable());

    QObject::connect(&libraryController, &app::LibraryController::soundsShown, &waveformController,
                     &app::WaveformController::onSoundsShown);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &waveformController,
                     &app::WaveformController::onSoundSelected);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &playbackController,
                     &app::PlaybackController::onSoundSelected);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &analysisController,
                     &app::AnalysisController::onSoundSelected);
    QObject::connect(&libraryController, &app::LibraryController::soundActivated, &playbackController,
                     &app::PlaybackController::onSoundActivated);

    window.show();
    QTimer::singleShot(0, &libraryController, &app::LibraryController::openLastFolder);
    return app.exec();
}
