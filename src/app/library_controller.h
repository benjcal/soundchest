#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QString>

#include <memory>

#include "library/audio_file.h"

class QModelIndex;

namespace library {
struct Folder;
} // namespace library

namespace ui {
class FileTableWidgetModel;
class FolderTreeWidgetModel;
class Window;
} // namespace ui

namespace app {

// Result of a background folder scan: exactly one of `folder` and `error` is set.
struct ScanOutcome {
    QString                                path;
    std::shared_ptr<const library::Folder> folder;
    QString                                error;
};

class LibraryController : public QObject {
    Q_OBJECT

  public:
    LibraryController(ui::Window *window, ui::FolderTreeWidgetModel *folderModel, ui::FileTableWidgetModel *fileModel,
                      QObject *parent = nullptr);

    // Starts a background scan. When it finishes, the models are populated and
    // folderOpened() or scanFailed() is emitted; a scan superseded by a newer
    // openFolder() call is discarded.
    void openFolder(const QString &path);

    // Reopens the folder from the previous session when it still exists.
    void openLastFolder();

    // Public so tests can trigger activation without a string-based
    // invokeMethod, which would break silently if the slot were renamed.
    void activateFile(int row);

  signals:
    void folderOpened(const QString &path, int fileCount);
    void scanFailed(const QString &path, const QString &error);
    void soundsShown();
    void soundSelected(const library::AudioFile &file);
    void soundActivated(const library::AudioFile &file);

  private slots:
    void finishScan();
    void showDirectory(const QModelIndex &index);
    void selectFile(int row);

  private:
    void applyFolder(const std::shared_ptr<const library::Folder> &folder);
    void loadDirectory(const QModelIndex &index);

    ui::Window                 *m_window;
    ui::FolderTreeWidgetModel  *m_folderModel;
    ui::FileTableWidgetModel   *m_fileModel;
    QFutureWatcher<ScanOutcome> m_scanWatcher;
};

} // namespace app
