#pragma once

#include <QMainWindow>

#include "analysis/sound_stats.h"
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

    FileTableWidget   *fileTable() const;
    FolderTreeWidget  *folderTree() const;
    HeaderBar         *headerBar() const;
    TransportControls *transport() const;
    WaveformWidget    *waveform() const;

    // Window owns only the status bar, dialogs, and tree focus; everything
    // else is reached through the widget accessors above.
    void    focusDirectoryInTree(const QModelIndex &index);
    QString chooseFolderPath();
    QString chooseExportFolderPath(const QString &startDir);
    void    showScanResult(const QString &rootPath, int fileCount);
    void    statusAudioInfo(const library::AudioFile &info, const analysis::SoundStats *stats = nullptr);
    void    setStatusMessage(const QString &message);

  private:
    void showAboutDialog();

    HeaderBar         *m_header;
    WaveformWidget    *m_waveform;
    TransportControls *m_transport;
    FolderTreeWidget  *m_folderTree;
    FileTableWidget   *m_fileTable;
};

} // namespace ui
