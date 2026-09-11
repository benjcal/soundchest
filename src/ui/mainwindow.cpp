#include "mainwindow.h"

#include "app.h"
#include "filetable.h"
#include "filetablemodel.h"
#include "folderbrowser.h"
#include "foldertreemodel.h"
#include "headerbar.h"
#include "transportbar.h"
#include "waveformwidget.h"

#include <QApplication>
#include <QFileDialog>
#include <QModelIndex>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

#include <algorithm>

MainWindow::MainWindow(App* app, QWidget* parent)
    : QMainWindow(parent)
    , m_app(app)
{
    setWindowTitle(QStringLiteral("Sound Chest"));
    resize(1200, 760);
    setMinimumSize(900, 600);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_header = new HeaderBar(central);
    m_waveform = new WaveformWidget(central);
    m_transport = new TransportBar(central);

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setObjectName(QStringLiteral("browserSplitter"));
    splitter->setChildrenCollapsible(false);

    m_folderBrowser = new FolderBrowser(splitter);
    m_folderBrowser->setModel(m_app->folderTreeModel());
    m_fileTable = new FileTable(splitter);
    m_fileTable->setModel(m_app->fileTableModel());

    splitter->addWidget(m_folderBrowser);
    splitter->addWidget(m_fileTable);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({ 260, 940 });

    layout->addWidget(m_header);
    layout->addWidget(m_waveform, 3);
    layout->addWidget(m_transport);
    layout->addWidget(splitter, 5);

    setCentralWidget(central);

    statusBar()->setObjectName(QStringLiteral("statusBar"));
    statusBar()->showMessage(QStringLiteral("Ready"));

    connect(m_header, &HeaderBar::openFolderRequested, this, &MainWindow::onOpenFolderRequested);
    connect(m_folderBrowser, &FolderBrowser::currentDirectoryChanged, this,
            &MainWindow::currentDirectoryChanged);
    connect(m_fileTable, &FileTable::currentFileChanged, this, &MainWindow::currentFileChanged);
    connect(m_transport, &TransportBar::playClicked, this, &MainWindow::playClicked);
    connect(m_transport, &TransportBar::stopClicked, this, &MainWindow::stopClicked);
    connect(m_transport, &TransportBar::loopToggled, this, &MainWindow::loopToggled);
    connect(m_transport, &TransportBar::volumeChanged, this, &MainWindow::volumeChanged);
    connect(m_transport, &TransportBar::autoplayChanged, this, &MainWindow::autoplayChanged);

    m_transport->setVolume(m_app->volumePercent());
}

void MainWindow::onOpenFolderRequested()
{
    const QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Open Folder"));
    if (dir.isEmpty())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    emit openFolderRequested(dir);
    QApplication::restoreOverrideCursor();
}

void MainWindow::onFolderScanned(const QString& rootPath, int fileCount)
{
    const QModelIndex root = m_app->folderTreeModel()->index(0, 0);
    m_folderBrowser->setCurrentIndex(root);
    m_folderBrowser->expand(root);

    statusBar()->showMessage(QStringLiteral("%1 — %2 audio file(s)").arg(rootPath).arg(fileCount));
}

void MainWindow::onScanFailed(const QString& path)
{
    statusBar()->showMessage(QStringLiteral("Could not scan %1").arg(path));
}

void MainWindow::onDirectoryChanged()
{
    m_waveform->clear();
}

void MainWindow::onFileLoaded(const audio::AudioInfo& info)
{
    statusBar()->showMessage(QStringLiteral("%1 — %2 s, %3 kHz, %4, %5, %6 kbps")
                                 .arg(info.fileName,
                                      QString::number(info.durationSec, 'f', 2),
                                      QString::number(info.sampleRate / 1000.0, 'f', 1),
                                      info.channels == 2 ? QStringLiteral("stereo")
                                                         : QStringLiteral("mono"),
                                      info.format,
                                      QString::number(info.bitRateKbps, 'f', 0)));

    m_transport->setControlsEnabled(true);
    m_waveform->clear();

    const int columns = std::max(64, width() / 2);
    m_app->requestWaveform(info.filePath, columns);
}

void MainWindow::onFileLoadFailed(const QString& fileName, const QString& error)
{
    statusBar()->showMessage(QStringLiteral("Failed to load %1: %2").arg(fileName, error));
}

void MainWindow::onWaveformReady(const QString& filePath, audio::WaveformData data)
{
    Q_UNUSED(filePath);
    m_waveform->setData(data);
}

void MainWindow::onProgressChanged(double fraction)
{
    m_waveform->setProgress(fraction);
}