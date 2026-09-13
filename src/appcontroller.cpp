#include "appcontroller.h"

#include "audio/player.h"
#include "library/folder.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/window.h"
#include "waveform/builder.h"
#include "waveform/peaks_cache.h"

#include <QModelIndex>
#include <QTimer>

#include <algorithm>
#include <memory>

AppController::AppController(ui::FolderTreeWidgetModel *folderModel, ui::FileTableWidgetModel *fileModel, ui::Window *view,
                             QObject *parent)
    : QObject(parent), m_folderTreeModel(folderModel), m_fileTableModel(fileModel), m_window(view),
      m_player(new audio::Player(this)), m_peaksBuilder(new waveform::PeaksBuilder(this)),
      m_peaksCache(new waveform::PeaksCache(this)), m_progressTimer(new QTimer(this)) {
    m_fileTableModel->setPeaksCache(m_peaksCache);

    m_progressTimer->setInterval(50);
    connect(m_progressTimer, &QTimer::timeout, this, &AppController::updateProgress);
    m_progressTimer->start();

    connect(m_peaksBuilder, &waveform::PeaksBuilder::peaksReady, this, &AppController::onWaveformReady);
}

void AppController::start() {
    m_window->setFolderModel(m_folderTreeModel);
    m_window->setFileModel(m_fileTableModel);
    m_window->setVolume(m_player->volumePercent());

    connect(m_window, &ui::Window::folderChosen, this, &AppController::openFolder);
    connect(m_window, &ui::Window::directorySelected, this, &AppController::selectDirectory);
    connect(m_window, &ui::Window::fileSelected, this, &AppController::loadFile);
    connect(m_window, &ui::Window::visibleRowsChanged, this, &AppController::requestVisibleWaveforms);
    connect(m_window, &ui::Window::playClicked, this, &AppController::playCurrent);
    connect(m_window, &ui::Window::stopClicked, this, &AppController::stopCurrent);
    connect(m_window, &ui::Window::loopToggled, this, &AppController::setLooping);
    connect(m_window, &ui::Window::volumeChanged, this, &AppController::setVolumePercent);
    connect(m_window, &ui::Window::autoplayChanged, this, &AppController::setAutoplay);
}

int AppController::volumePercent() const { return m_player->volumePercent(); }

void AppController::openFolder(const QString &path) {
    auto tree = std::make_shared<library::Folder>();
    if (!library::scan(path, tree.get())) {
        m_window->showError(QStringLiteral("Could not scan %1").arg(path));
        return;
    }

    const QString root      = tree->root->path;
    const int     fileCount = tree->fileCount();
    m_folderTreeModel->setFolder(std::move(tree));
    m_window->showScanResult(root, fileCount);
    m_window->selectDirectory(m_folderTreeModel->index(0, 0));
}

void AppController::selectDirectory(const QModelIndex &index) {
    const QVector<library::AudioFile> *files = m_folderTreeModel->filesFor(index);
    if (!files)
        return;

    m_peaksBuilder->cancel();
    m_window->clearWaveform();

    m_fileTableModel->setFiles(*files);
}

void AppController::loadFile(int row) {
    if (row < 0)
        return;

    const library::AudioFile info = m_fileTableModel->audioInfo(row);
    if (info.filePath.isEmpty())
        return;

    if (!m_player->open(info.filePath)) {
        m_window->showError(QStringLiteral("Failed to load %1: %2").arg(info.fileName, m_player->errorString()));
        return;
    }

    m_window->showAudioInfo(info);
    m_window->setControlsEnabled(true);
    m_window->clearWaveform();

    const int columns = std::max(64, m_window->waveformWidth() / 2);
    requestWaveform(info.filePath, columns);

    if (m_autoplay)
        m_player->play();
}

void AppController::playCurrent() { m_player->play(); }

void AppController::stopCurrent() { m_player->stop(); }

void AppController::setLooping(bool enabled) { m_player->setLooping(enabled); }

void AppController::setVolumePercent(int percent) { m_player->setVolumePercent(percent); }

void AppController::setAutoplay(bool enabled) {
    m_autoplay = enabled;
    if (enabled && !m_player->filePath().isEmpty() && !m_player->isPlaying())
        m_player->play();
}

void AppController::requestWaveform(const QString &filePath, int columnCount) {
    m_peaksBuilder->request(filePath, columnCount);
}

void AppController::requestVisibleWaveforms(int firstRow, int lastRow) {
    const int rowCount = m_fileTableModel->rowCount();
    firstRow           = std::max(0, firstRow);
    lastRow            = std::min(rowCount - 1, lastRow);

    for (int row = firstRow; row <= lastRow; ++row) {
        const QString filePath = m_fileTableModel->audioInfo(row).filePath;
        if (!filePath.isEmpty())
            m_peaksCache->request(filePath);
    }
}

void AppController::onWaveformReady(const QString &filePath, waveform::Peaks data) {
    if (filePath != m_player->filePath() || !data.valid())
        return;
    m_window->setPeaks(data);
}

void AppController::updateProgress() {
    if (m_player->filePath().isEmpty())
        return;
    const double length = m_player->lengthSec();
    if (length > 0.0)
        m_window->setProgress(m_player->positionSec() / length);
}
