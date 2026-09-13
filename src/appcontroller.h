#pragma once

#include <QObject>
#include <QString>

#include "library/audio_file.h"
#include "waveform/peaks.h"

namespace audio {
class Player;
} // namespace audio

namespace waveform {
class PeaksBuilder;
class PeaksCache;
} // namespace waveform

namespace ui {
class FileTableWidgetModel;
class FolderTreeWidgetModel;
class Window;
} // namespace ui

class QModelIndex;
class QTimer;

class AppController : public QObject {
    Q_OBJECT

  public:
    AppController(ui::FolderTreeWidgetModel *folderModel, ui::FileTableWidgetModel *fileModel, ui::Window *view,
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
    void onWaveformReady(const QString &filePath, waveform::Peaks data);
    void updateProgress();

    ui::FolderTreeWidgetModel       *m_folderTreeModel;
    ui::FileTableWidgetModel        *m_fileTableModel;
    ui::Window              *m_window;
    audio::Player             *m_player;
    waveform::PeaksBuilder *m_peaksBuilder;
    waveform::PeaksCache    *m_peaksCache;
    QTimer                    *m_progressTimer;
    bool                       m_autoplay = false;
};
