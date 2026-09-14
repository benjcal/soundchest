#include "app/library_controller.h"

#include "library/audio_library_scanner.h"
#include "library/folder.h"
#include "ui/file_table_widget.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/headerbar.h"
#include "ui/window.h"

#include <QFileInfo>
#include <QModelIndex>
#include <QSignalBlocker>
#include <QtConcurrent/QtConcurrentRun>

namespace app {

LibraryController::LibraryController(ui::Window *window, ui::FolderTreeWidgetModel *folderModel,
                                     ui::FileTableWidgetModel *fileModel, QObject *parent)
    : QObject(parent), m_window(window), m_folderModel(folderModel), m_fileModel(fileModel), m_scanWatcher(this) {
    m_window->folderTree()->setModel(m_folderModel);
    m_window->fileTable()->setModel(m_fileModel);

    connect(&m_scanWatcher, &QFutureWatcher<ScanOutcome>::finished, this, &LibraryController::finishScan);

    connect(m_window->headerBar(), &ui::HeaderBar::openFolderRequested, this, [this] {
        const QString path = m_window->chooseFolderPath();
        if (!path.isEmpty())
            openFolder(path);
    });
    connect(m_window->folderTree(), &ui::FolderTreeWidget::currentDirectoryChanged, this,
            &LibraryController::showDirectory);
    connect(m_window->fileTable(), &ui::FileTableWidget::currentFileChanged, this, &LibraryController::selectFile);
    connect(m_window->fileTable(), &ui::FileTableWidget::fileActivated, this, &LibraryController::activateFile);
}

void LibraryController::openFolder(const QString &path) {
    if (path.isEmpty())
        return;

    m_window->setScanning(true, path);

    // setFuture() detaches from any previous scan, so its finished() never
    // reaches finishScan() and a stale result cannot overwrite a newer folder.
    m_scanWatcher.setFuture(QtConcurrent::run([path] {
        ScanOutcome outcome;
        outcome.path   = path;
        outcome.folder = library::AudioLibraryScanner::scan(path, &outcome.error);
        return outcome;
    }));
}

void LibraryController::openLastFolder() {
    const QString path = m_window->lastOpenDir();
    if (path.isEmpty() || !QFileInfo(path).isDir())
        return;

    openFolder(path);
}

void LibraryController::finishScan() {
    const ScanOutcome outcome = m_scanWatcher.result();
    m_window->setScanning(false);

    if (!outcome.folder) {
        m_window->setStatusMessage(QStringLiteral("Could not scan %1: %2").arg(outcome.path, outcome.error));
        emit scanFailed(outcome.path, outcome.error);
        return;
    }

    applyFolder(outcome.folder);
}

void LibraryController::applyFolder(const std::shared_ptr<const library::Folder> &folder) {
    m_folderModel->setFolder(folder);
    m_window->showScanResult(folder->root->path, folder->fileCount());

    const QModelIndex root = m_folderModel->index(0, 0);

    // Focus the root in the tree for the user, but load the files here rather
    // than relying on the tree's selection signal to bounce back into
    // showDirectory(). The signal is blocked only around this programmatic
    // selection; user clicks still flow through showDirectory().
    {
        const QSignalBlocker blocker(m_window->folderTree());
        m_window->focusDirectoryInTree(root);
    }
    loadDirectory(root);

    emit folderOpened(folder->root->path, folder->fileCount());
}

void LibraryController::showDirectory(const QModelIndex &index) { loadDirectory(index); }

void LibraryController::loadDirectory(const QModelIndex &index) {
    const QVector<library::AudioFile> *files = m_folderModel->filesFor(index);
    if (!files)
        return;

    m_fileModel->setFiles(*files);
    emit soundsShown();

    // Resetting the file model clears the view's current index, so setting
    // row 0 emits currentFileChanged even when row 0 was already selected.
    m_window->fileTable()->setCurrentRow(0);
}

void LibraryController::selectFile(int row) {
    const library::AudioFile file = m_fileModel->audioFile(row);
    if (!file.filePath.isEmpty())
        emit soundSelected(file);
}

void LibraryController::activateFile(int row) {
    const library::AudioFile file = m_fileModel->audioFile(row);
    if (!file.filePath.isEmpty())
        emit soundActivated(file);
}

} // namespace app
