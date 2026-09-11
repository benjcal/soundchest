#include "app.h"

#include "audio/player.h"
#include "audio/waveformbuilder.h"
#include "audio/waveformcache.h"
#include "catalog/foldertree.h"
#include "ui/filetablemodel.h"
#include "ui/foldertreemodel.h"
#include "ui/mainwindow.h"

#include <QModelIndex>
#include <QTimer>

App::App(QObject* parent)
    : QObject(parent)
    , m_player(new audio::Player(this))
    , m_waveformBuilder(new audio::WaveformBuilder(this))
    , m_waveformCache(new audio::WaveformCache(this))
    , m_progressTimer(new QTimer(this))
    , m_folderTreeModel(new FolderTreeModel(this))
    , m_fileTableModel(new FileTableModel(this))
{
    m_progressTimer->setInterval(50);
    connect(m_progressTimer, &QTimer::timeout, this, &App::updateProgress);
    m_progressTimer->start();

    connect(m_waveformBuilder, &audio::WaveformBuilder::waveformReady, this,
            &App::onWaveformReady);

    m_fileTableModel->setWaveformCache(m_waveformCache);
}

void App::start()
{
    m_window = new MainWindow(this);
    m_window->show();

    connect(m_window, &MainWindow::openFolderRequested, this, &App::openFolder);
    connect(m_window, &MainWindow::currentDirectoryChanged, this, &App::selectDirectory);
    connect(m_window, &MainWindow::currentFileChanged, this, &App::loadFile);
    connect(m_window, &MainWindow::playClicked, this, &App::playCurrent);
    connect(m_window, &MainWindow::stopClicked, this, &App::stopCurrent);
    connect(m_window, &MainWindow::loopToggled, this, &App::setLooping);
    connect(m_window, &MainWindow::volumeChanged, this, &App::setVolumePercent);
    connect(m_window, &MainWindow::autoplayChanged, this, &App::setAutoplay);

    connect(this, &App::folderScanned, m_window, &MainWindow::onFolderScanned);
    connect(this, &App::scanFailed, m_window, &MainWindow::onScanFailed);
    connect(this, &App::directoryChanged, m_window, &MainWindow::onDirectoryChanged);
    connect(this, &App::fileLoaded, m_window, &MainWindow::onFileLoaded);
    connect(this, &App::fileLoadFailed, m_window, &MainWindow::onFileLoadFailed);
    connect(this, &App::waveformReady, m_window, &MainWindow::onWaveformReady);
    connect(this, &App::progressChanged, m_window, &MainWindow::onProgressChanged);
}

FolderTreeModel* App::folderTreeModel() const
{
    return m_folderTreeModel;
}

FileTableModel* App::fileTableModel() const
{
    return m_fileTableModel;
}

int App::volumePercent() const
{
    return m_player->volumePercent();
}

void App::openFolder(const QString& path)
{
    catalog::FolderTree tree;
    if (!catalog::scanFolder(path, &tree)) {
        emit scanFailed(path);
        return;
    }

    m_tree = std::move(tree);
    m_folderTreeModel->setTree(&m_tree);
    emit folderScanned(m_tree.root->path, m_tree.fileCount());
}

void App::selectDirectory(const QModelIndex& index)
{
    const QVector<audio::AudioInfo>* files = m_folderTreeModel->filesFor(index);
    if (!files)
        return;

    m_waveformBuilder->cancel();
    emit directoryChanged();

    m_fileTableModel->setFiles(*files);
}

void App::loadFile(int row)
{
    const audio::AudioInfo info = m_fileTableModel->audioInfo(row);
    if (info.filePath.isEmpty())
        return;

    if (!m_player->open(info.filePath)) {
        emit fileLoadFailed(info.fileName, m_player->errorString());
        return;
    }

    emit fileLoaded(info);

    if (m_autoplay)
        m_player->play();
}

void App::playCurrent()
{
    m_player->play();
}

void App::stopCurrent()
{
    m_player->stop();
}

void App::setLooping(bool enabled)
{
    m_player->setLooping(enabled);
}

void App::setVolumePercent(int percent)
{
    m_player->setVolumePercent(percent);
}

void App::setAutoplay(bool enabled)
{
    m_autoplay = enabled;
    if (enabled && !m_player->filePath().isEmpty() && !m_player->isPlaying())
        m_player->play();
}

void App::requestWaveform(const QString& filePath, int columnCount)
{
    m_waveformBuilder->request(filePath, columnCount);
}

void App::onWaveformReady(const QString& filePath, audio::WaveformData data)
{
    if (filePath != m_player->filePath() || !data.valid())
        return;
    emit waveformReady(filePath, std::move(data));
}

void App::updateProgress()
{
    if (m_player->filePath().isEmpty())
        return;
    const double length = m_player->lengthSec();
    if (length > 0.0)
        emit progressChanged(m_player->positionSec() / length);
}