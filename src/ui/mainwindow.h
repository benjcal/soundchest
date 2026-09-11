#pragma once

#include <QMainWindow>

#include "audio/audiofile.h"
#include "audio/waveform.h"

class App;
class FileTable;
class FolderBrowser;
class HeaderBar;
class QModelIndex;
class TransportBar;
class WaveformWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(App* app, QWidget* parent = nullptr);

signals:
    void openFolderRequested(const QString& path);
    void currentDirectoryChanged(const QModelIndex& index);
    void currentFileChanged(int row);
    void playClicked();
    void stopClicked();
    void loopToggled(bool enabled);
    void volumeChanged(int value);
    void autoplayChanged(bool enabled);

public slots:
    void onFolderScanned(const QString& rootPath, int fileCount);
    void onScanFailed(const QString& path);
    void onDirectoryChanged();
    void onFileLoaded(const audio::AudioInfo& info);
    void onFileLoadFailed(const QString& fileName, const QString& error);
    void onWaveformReady(const QString& filePath, audio::WaveformData data);
    void onProgressChanged(double fraction);

private:
    void onOpenFolderRequested();

    App* m_app;
    HeaderBar* m_header;
    WaveformWidget* m_waveform;
    TransportBar* m_transport;
    FolderBrowser* m_folderBrowser;
    FileTable* m_fileTable;
};