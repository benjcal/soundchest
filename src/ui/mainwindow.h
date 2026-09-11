#pragma once

#include <QMainWindow>

class FileTable;
class FolderBrowser;
class HeaderBar;
class TransportBar;
class WaveformWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    HeaderBar* m_header;
    WaveformWidget* m_waveform;
    TransportBar* m_transport;
    FolderBrowser* m_folderBrowser;
    FileTable* m_fileTable;
};