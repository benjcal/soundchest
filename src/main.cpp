#include "app/library_controller.h"
#include "app/playback_controller.h"
#include "app/waveform_controller.h"
#include "audio/player.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/window.h"
#include "waveform/peaks_builder.h"

#include <QApplication>
#include <QLoggingCategory>

#include <oclero/qlementine/resources/ResourceInitialization.hpp>
#include <oclero/qlementine/style/QlementineStyle.hpp>

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

    oclero::qlementine::resources::initializeResources();

    auto *style = new oclero::qlementine::QlementineStyle;
    QApplication::setStyle(style);
    style->setThemeJsonPath(QStringLiteral(":/themes/gruvbox.json"));

    ui::Window               window;
    ui::FolderTreeWidgetModel folderModel;
    ui::FileTableWidgetModel  fileModel;
    audio::Player             player;
    waveform::PeaksBuilder    peaksBuilder;

    app::LibraryController libraryController(&window, &folderModel, &fileModel);
    app::WaveformController waveformController(&window, &peaksBuilder);
    app::PlaybackController playbackController(&window, &player);

    QObject::connect(&libraryController, &app::LibraryController::soundsShown, &waveformController,
                     &app::WaveformController::onSoundsShown);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &waveformController,
                     &app::WaveformController::onSoundSelected);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &playbackController,
                     &app::PlaybackController::onSoundSelected);

    window.show();
    return app.exec();
}
