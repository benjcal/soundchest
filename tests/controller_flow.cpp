#include "app/library_controller.h"
#include "app/playback_controller.h"
#include "app/waveform_controller.h"
#include "audio/player.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/window.h"
#include "waveform/peaks_builder.h"

#include <QApplication>
#include <QMimeData>
#include <QTimer>
#include <QUrl>

#include <cstdio>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <folder>\n", argv[0]);
        return 2;
    }

    ui::Window                window;
    ui::FolderTreeWidgetModel folderModel;
    ui::FileTableWidgetModel  fileModel;
    audio::Player             player;
    waveform::PeaksBuilder    peaksBuilder;

    app::LibraryController  libraryController(&window, &folderModel, &fileModel);
    app::WaveformController waveformController(&window, &peaksBuilder);
    app::PlaybackController playbackController(&window, &player);

    QObject::connect(&libraryController, &app::LibraryController::soundsShown, &waveformController,
                     &app::WaveformController::onSoundsShown);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &waveformController,
                     &app::WaveformController::onSoundSelected);
    QObject::connect(&libraryController, &app::LibraryController::soundSelected, &playbackController,
                     &app::PlaybackController::onSoundSelected);

    window.show();

    QString folder = QString::fromLocal8Bit(argv[1]);
    QTimer::singleShot(0, &libraryController, [&] { libraryController.openFolder(folder); });

    bool activated = false;
    QObject::connect(&libraryController, &app::LibraryController::soundActivated, &app,
                     [&](const library::AudioFile &) { activated = true; });

    int  exitCode = 1;
    QTimer::singleShot(3000, &app, [&] {
        const int rootFiles = fileModel.rowCount();
        const bool gotPeaks = rootFiles > 0 && peaksBuilder.peaks(fileModel.audioFile(0).filePath).valid();

        QModelIndexList indexes;
        for (int row = 0; row < rootFiles; ++row)
            indexes.append(fileModel.index(row, 0));
        QMimeData *mime     = fileModel.mimeData(indexes);
        const int  urlCount = mime ? mime->urls().size() : 0;
        delete mime;

        const QModelIndex root       = folderModel.index(0, 0);
        const int         subdirs    = folderModel.rowCount(root);
        int               nestedRows = 0;
        if (subdirs > 0) {
            window.focusDirectoryInTree(folderModel.index(0, 0, root));
            nestedRows = fileModel.rowCount();
        }

        libraryController.activateFile(0);

        std::printf("rootFiles=%d peaks=%s subdirs=%d nestedFiles=%d activated=%s dragUrls=%d\n", rootFiles,
                    gotPeaks ? "yes" : "no", subdirs, nestedRows, activated ? "yes" : "no", urlCount);
        exitCode = (rootFiles >= 1 && gotPeaks && activated && urlCount == rootFiles) ? 0 : 1;
        app.quit();
    });

    app.exec();
    return exitCode;
}
