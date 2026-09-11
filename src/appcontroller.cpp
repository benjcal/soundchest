#include "appcontroller.h"

#include "audio/player.h"
#include "catalog/foldertree.h"
#include "ui/filetablemodel.h"
#include "ui/foldertreemodel.h"
#include "ui/mainview.h"
#include "waveform/waveformbuilder.h"
#include "waveform/waveformcache.h"

#include <QModelIndex>
#include <QTimer>

#include <algorithm>
#include <memory>

AppController::AppController(ui::FolderTreeModel *folderModel, ui::FileTableModel *fileModel, ui::MainView *view,
                             QObject *parent)
    : QObject(parent), m_folderTreeModel(folderModel), m_fileTableModel(fileModel), m_view(view),
      m_player(new audio::Player(this)), m_waveformBuilder(new waveform::WaveformBuilder(this)),
      m_waveformCache(new waveform::WaveformCache(this)), m_progressTimer(new QTimer(this)) {
    m_fileTableModel->setWaveformCache(m_waveformCache);

    m_progressTimer->setInterval(50);
    connect(m_progressTimer, &QTimer::timeout, this, &AppController::updateProgress);
    m_progressTimer->start();

    connect(m_waveformBuilder, &waveform::WaveformBuilder::waveformReady, this, &AppController::onWaveformReady);
}

void AppController::start() {
    m_view->setFolderModel(m_folderTreeModel);
    m_view->setFileModel(m_fileTableModel);
    m_view->setVolume(m_player->volumePercent());

    connect(m_view, &ui::MainView::folderChosen, this, &AppController::openFolder);
    connect(m_view, &ui::MainView::directorySelected, this, &AppController::selectDirectory);
    connect(m_view, &ui::MainView::fileSelected, this, &AppController::loadFile);
    connect(m_view, &ui::MainView::visibleRowsChanged, this, &AppController::requestVisibleWaveforms);
    connect(m_view, &ui::MainView::playClicked, this, &AppController::playCurrent);
    connect(m_view, &ui::MainView::stopClicked, this, &AppController::stopCurrent);
    connect(m_view, &ui::MainView::loopToggled, this, &AppController::setLooping);
    connect(m_view, &ui::MainView::volumeChanged, this, &AppController::setVolumePercent);
    connect(m_view, &ui::MainView::autoplayChanged, this, &AppController::setAutoplay);
}

int AppController::volumePercent() const { return m_player->volumePercent(); }

void AppController::openFolder(const QString &path) {
    auto tree = std::make_shared<catalog::FolderTree>();
    if (!catalog::scanFolder(path, tree.get())) {
        m_view->showError(QStringLiteral("Could not scan %1").arg(path));
        return;
    }

    const QString root      = tree->root->path;
    const int     fileCount = tree->fileCount();
    m_folderTreeModel->setTree(std::move(tree));
    m_view->showScanResult(root, fileCount);
    m_view->selectDirectory(m_folderTreeModel->index(0, 0));
}

void AppController::selectDirectory(const QModelIndex &index) {
    const QVector<audio::AudioInfo> *files = m_folderTreeModel->filesFor(index);
    if (!files)
        return;

    m_waveformBuilder->cancel();
    m_view->clearWaveform();

    m_fileTableModel->setFiles(*files);
}

void AppController::loadFile(int row) {
    if (row < 0)
        return;

    const audio::AudioInfo info = m_fileTableModel->audioInfo(row);
    if (info.filePath.isEmpty())
        return;

    if (!m_player->open(info.filePath)) {
        m_view->showError(QStringLiteral("Failed to load %1: %2").arg(info.fileName, m_player->errorString()));
        return;
    }

    m_view->showFileInfo(info);
    m_view->setControlsEnabled(true);
    m_view->clearWaveform();

    const int columns = std::max(64, m_view->waveformWidth() / 2);
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
    m_waveformBuilder->request(filePath, columnCount);
}

void AppController::requestVisibleWaveforms(int firstRow, int lastRow) {
    const int rowCount = m_fileTableModel->rowCount();
    firstRow           = std::max(0, firstRow);
    lastRow            = std::min(rowCount - 1, lastRow);

    for (int row = firstRow; row <= lastRow; ++row) {
        const QString filePath = m_fileTableModel->audioInfo(row).filePath;
        if (!filePath.isEmpty())
            m_waveformCache->request(filePath);
    }
}

void AppController::onWaveformReady(const QString &filePath, waveform::WaveformData data) {
    if (filePath != m_player->filePath() || !data.valid())
        return;
    m_view->setWaveform(data);
}

void AppController::updateProgress() {
    if (m_player->filePath().isEmpty())
        return;
    const double length = m_player->lengthSec();
    if (length > 0.0)
        m_view->setProgress(m_player->positionSec() / length);
}
