#pragma once

#include <QMainWindow>

#include "analysis/sound_stats.h"
#include "library/audio_file.h"

class QCloseEvent;
class QModelIndex;
class QSplitter;

namespace oclero::qlementine {
class LoadingSpinner;
} // namespace oclero::qlementine

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
    QString lastOpenDir() const;
    QString chooseExportFolderPath(const QString &startDir);
    void    showScanResult(const QString &rootPath, int fileCount);
    void    setScanning(bool scanning, const QString &path = {});
    void    statusAudioInfo(const library::AudioFile &info, const analysis::SoundStats *stats = nullptr);
    void    setStatusMessage(const QString &message);

  protected:
    void closeEvent(QCloseEvent *event) override;

  private:
    void restoreSettings();
    void saveSettings();
    void showAboutDialog();

    HeaderBar         *m_header;
    WaveformWidget    *m_waveform;
    TransportControls *m_transport;
    QSplitter         *m_splitter;
    FolderTreeWidget  *m_folderTree;
    FileTableWidget   *m_fileTable;

    oclero::qlementine::LoadingSpinner *m_spinner;
};

} // namespace ui
