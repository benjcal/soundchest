#pragma once

#include <QMainWindow>

#include "library/audio_file.h"
#include "waveform/peaks.h"

class QAbstractItemModel;
class QModelIndex;

namespace ui {

class FileTableWidget;
class FolderTreeWidget;
class HeaderBar;
class TransportControls;
class WaveformWidget;

class Window : public QMainWindow {
    Q_OBJECT

  public:
    explicit Window(QWidget *parent = nullptr);

    void setFolderModel(QAbstractItemModel *model);
    void setFileModel(QAbstractItemModel *model);
    void selectDirectory(const QModelIndex &index);

    void showScanResult(const QString &rootPath, int fileCount);
    void showAudioInfo(const library::AudioFile &info);
    void showError(const QString &message);
    void setControlsEnabled(bool enabled);
    void setVolume(int value);
    void clearWaveform();
    void setPeaks(const waveform::Peaks &peaks);
    void setProgress(double fraction);
    int  waveformWidth() const;

  signals:
    void folderChosen(const QString &path);
    void directorySelected(const QModelIndex &index);
    void fileSelected(int row);
    void visibleRowsChanged(int firstRow, int lastRow);
    void playClicked();
    void stopClicked();
    void loopToggled(bool enabled);
    void volumeChanged(int value);
    void autoplayChanged(bool enabled);
    void statusMessage(const QString &message);

  private:
    void chooseFolder();

    HeaderBar        *m_header;
    WaveformWidget   *m_waveform;
    TransportControls *m_transport;
    FolderTreeWidget *m_folderTree;
    FileTableWidget  *m_fileTable;
};

} // namespace ui
