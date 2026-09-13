#include "app/library_controller.h"

#include "library/audio_library_scanner.h"
#include "library/folder.h"
#include "ui/file_table_widget.h"
#include "ui/headerbar.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/window.h"

#include <QApplication>
#include <QModelIndex>

namespace app {

LibraryController::LibraryController(ui::Window *window, ui::FolderTreeWidgetModel *folderModel,
                                     ui::FileTableWidgetModel *fileModel, QObject *parent)
    : QObject(parent), m_window(window), m_folderModel(folderModel), m_fileModel(fileModel) {
    m_window->folderTreeWidget()->setModel(m_folderModel);
    m_window->fileTableWidget()->setModel(m_fileModel);

    connect(m_window->header(), &ui::HeaderBar::openFolderRequested, this, [this] {
        const QString path = m_window->chooseFolderPath();
        if (path.isEmpty())
            return;

        QApplication::setOverrideCursor(Qt::WaitCursor);
        openFolder(path);
        QApplication::restoreOverrideCursor();
    });
    connect(m_window->folderTreeWidget(), &ui::FolderTreeWidget::currentDirectoryChanged, this,
            &LibraryController::selectDirectory);
    connect(m_window->fileTableWidget(), &ui::FileTableWidget::currentFileChanged, this, &LibraryController::selectFile);
}

void LibraryController::openFolder(const QString &path) {
    if (path.isEmpty())
        return;

    QString    error;
    const auto folder = library::AudioLibraryScanner::scan(path, &error);
    if (!folder) {
        m_window->showError(QStringLiteral("Could not scan %1: %2").arg(path, error));
        return;
    }

    m_folderModel->setFolder(folder);
    m_window->showScanResult(folder->root->path, folder->fileCount());
    m_window->selectDirectory(m_folderModel->index(0, 0));
}

void LibraryController::selectDirectory(const QModelIndex &index) {
    const QVector<library::AudioFile> *files = m_folderModel->filesFor(index);
    if (!files)
        return;

    m_fileModel->setFiles(*files);
    emit soundsShown();
    m_window->fileTableWidget()->setCurrentRow(0);
}

void LibraryController::selectFile(int row) {
    const library::AudioFile file = m_fileModel->audioFile(row);
    if (!file.filePath.isEmpty())
        emit soundSelected(file);
}

} // namespace app
