#pragma once

#include <QWidget>

#include "audio/audioinfo.h"
#include "waveform/waveform.h"

class QAbstractItemModel;
class QModelIndex;

namespace ui {

class FileTable;
class FolderBrowser;
class HeaderBar;
class TransportBar;
class WaveformWidget;

class MainView : public QWidget {
    Q_OBJECT

  public:
    explicit MainView(QWidget *parent = nullptr);

    void setFolderModel(QAbstractItemModel *model);
    void setFileModel(QAbstractItemModel *model);
    void selectDirectory(const QModelIndex &index);

    void showScanResult(const QString &rootPath, int fileCount);
    void showFileInfo(const audio::AudioInfo &info);
    void showError(const QString &message);
    void setControlsEnabled(bool enabled);
    void setVolume(int value);
    void clearWaveform();
    void setWaveform(const waveform::WaveformData &data);
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

    HeaderBar      *m_header;
    WaveformWidget *m_waveform;
    TransportBar   *m_transport;
    FolderBrowser  *m_folderBrowser;
    FileTable      *m_fileTable;
};

} // namespace ui
