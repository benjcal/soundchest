#pragma once

#include <QObject>
#include <QString>

#include "audio/audiofile.h"
#include "audio/waveform.h"
#include "catalog/foldertree.h"

class FileTableModel;
class FolderTreeModel;
class MainWindow;
class QModelIndex;
class QTimer;

namespace audio {
class Player;
class WaveformBuilder;
class WaveformCache;
} // namespace audio

class App : public QObject
{
    Q_OBJECT

public:
    explicit App(QObject* parent = nullptr);

    void start();

    FolderTreeModel* folderTreeModel() const;
    FileTableModel* fileTableModel() const;
    int volumePercent() const;

public slots:
    void openFolder(const QString& path);
    void selectDirectory(const QModelIndex& index);
    void loadFile(int row);
    void playCurrent();
    void stopCurrent();
    void setLooping(bool enabled);
    void setVolumePercent(int percent);
    void setAutoplay(bool enabled);
    void requestWaveform(const QString& filePath, int columnCount);

signals:
    void folderScanned(const QString& rootPath, int fileCount);
    void scanFailed(const QString& path);
    void directoryChanged();
    void fileLoaded(const audio::AudioInfo& info);
    void fileLoadFailed(const QString& fileName, const QString& error);
    void waveformReady(const QString& filePath, audio::WaveformData data);
    void progressChanged(double fraction);

private:
    void onWaveformReady(const QString& filePath, audio::WaveformData data);
    void updateProgress();

    catalog::FolderTree m_tree;
    audio::Player* m_player;
    audio::WaveformBuilder* m_waveformBuilder;
    audio::WaveformCache* m_waveformCache;
    QTimer* m_progressTimer;
    bool m_autoplay = false;
    FolderTreeModel* m_folderTreeModel;
    FileTableModel* m_fileTableModel;
    MainWindow* m_window = nullptr;
};