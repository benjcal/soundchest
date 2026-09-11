#pragma once

#include <QObject>
#include <QString>

#include "audio/audioinfo.h"
#include "waveform/waveform.h"

namespace audio {
class Player;
} // namespace audio

namespace waveform {
class WaveformBuilder;
class WaveformCache;
} // namespace waveform

namespace ui {
class FileTableModel;
class FolderTreeModel;
class MainView;
} // namespace ui

class QModelIndex;
class QTimer;

class AppController : public QObject {
    Q_OBJECT

  public:
    AppController(ui::FolderTreeModel *folderModel, ui::FileTableModel *fileModel, ui::MainView *view,
                  QObject *parent = nullptr);

    void start();

    int volumePercent() const;

  public slots:
    void openFolder(const QString &path);
    void selectDirectory(const QModelIndex &index);
    void loadFile(int row);
    void playCurrent();
    void stopCurrent();
    void setLooping(bool enabled);
    void setVolumePercent(int percent);
    void setAutoplay(bool enabled);
    void requestWaveform(const QString &filePath, int columnCount);
    void requestVisibleWaveforms(int firstRow, int lastRow);

  private:
    void onWaveformReady(const QString &filePath, waveform::WaveformData data);
    void updateProgress();

    ui::FolderTreeModel       *m_folderTreeModel;
    ui::FileTableModel        *m_fileTableModel;
    ui::MainView              *m_view;
    audio::Player             *m_player;
    waveform::WaveformBuilder *m_waveformBuilder;
    waveform::WaveformCache   *m_waveformCache;
    QTimer                    *m_progressTimer;
    bool                       m_autoplay = false;
};
