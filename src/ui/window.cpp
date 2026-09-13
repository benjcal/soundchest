#include "window.h"

#include "file_table_widget.h"
#include "folder_tree_widget.h"
#include "headerbar.h"
#include "transport_controls.h"
#include "waveform_widget.h"

#include <QApplication>
#include <QFileDialog>
#include <QModelIndex>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

namespace ui {

Window::Window(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Sound Chest"));
    resize(1200, 760);
    setMinimumSize(900, 600);

    statusBar()->setObjectName(QStringLiteral("statusBar"));
    statusBar()->showMessage(QStringLiteral("Ready"));

    auto *central = new QWidget(this);
    auto *layout  = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_header    = new HeaderBar(central);
    m_waveform  = new WaveformWidget(central);
    m_transport = new TransportControls(central);

    auto *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setObjectName(QStringLiteral("browserSplitter"));
    splitter->setChildrenCollapsible(false);

    m_folderTree = new FolderTreeWidget(splitter);
    m_fileTable  = new FileTableWidget(splitter);
    splitter->addWidget(m_folderTree);
    splitter->addWidget(m_fileTable);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({260, 940});

    layout->addWidget(m_header);
    layout->addWidget(m_waveform, 3);
    layout->addWidget(m_transport);
    layout->addWidget(splitter, 5);

    setCentralWidget(central);

}

FileTableWidget *Window::fileTableWidget() const { return m_fileTable; }

FolderTreeWidget *Window::folderTreeWidget() const { return m_folderTree; }

HeaderBar *Window::header() const { return m_header; }

TransportControls *Window::transport() const { return m_transport; }

WaveformWidget *Window::waveform() const { return m_waveform; }

void Window::selectDirectory(const QModelIndex &index) {
    m_folderTree->setCurrentIndex(index);
    m_folderTree->expand(index);
}

void Window::showScanResult(const QString &rootPath, int fileCount) {
    statusBar()->showMessage(QStringLiteral("%1 — %2 audio file(s)").arg(rootPath).arg(fileCount));
}

void Window::showAudioInfo(const library::AudioFile &info) {
    statusBar()->showMessage(QStringLiteral("%1 — %2 s, %3 kHz, %4, %5, %6 kbps")
                                 .arg(info.fileName, QString::number(info.durationSec, 'f', 2),
                                      QString::number(info.sampleRate / 1000.0, 'f', 1),
                                      info.channels == 2 ? QStringLiteral("stereo") : QStringLiteral("mono"),
                                      info.format, QString::number(info.bitRateKbps, 'f', 0)));
}

void Window::showError(const QString &message) { statusBar()->showMessage(message); }

void Window::setProgress(double fraction) { m_waveform->setProgress(fraction); }

QString Window::chooseFolderPath() {
    return QFileDialog::getExistingDirectory(this, QStringLiteral("Open Folder"), {}, QFileDialog::ShowDirsOnly);
}

} // namespace ui
