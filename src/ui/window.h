#pragma once

#include <QMainWindow>

#include "library/audio_file.h"

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

    FileTableWidget   *fileTableWidget() const;
    FolderTreeWidget  *folderTreeWidget() const;
    HeaderBar         *header() const;
    TransportControls *transport() const;
    WaveformWidget    *waveform() const;

    void    selectDirectory(const QModelIndex &index);
    QString chooseFolderPath();
    void    showScanResult(const QString &rootPath, int fileCount);
    void    showAudioInfo(const library::AudioFile &info);
    void    showError(const QString &message);
    void    setProgress(double fraction);

  private:
    HeaderBar         *m_header;
    WaveformWidget    *m_waveform;
    TransportControls *m_transport;
    FolderTreeWidget  *m_folderTree;
    FileTableWidget   *m_fileTable;
};

} // namespace ui
