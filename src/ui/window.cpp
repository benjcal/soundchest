#include "window.h"

#include "file_table_widget.h"
#include "folder_tree_widget.h"
#include "headerbar.h"
#include "transport_controls.h"
#include "waveform_widget.h"

#include <QApplication>
#include <QFileDialog>
#include <QListView>
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

    connect(m_header, &HeaderBar::openFolderRequested, this, &Window::chooseFolder);
    connect(m_folderTree, &FolderTreeWidget::currentDirectoryChanged, this, &Window::directorySelected);
    connect(m_fileTable, &FileTableWidget::currentFileChanged, this, &Window::fileSelected);
    connect(m_fileTable, &FileTableWidget::visibleRowsChanged, this, &Window::visibleRowsChanged);
    connect(m_transport, &TransportControls::playClicked, this, &Window::playClicked);
    connect(m_transport, &TransportControls::stopClicked, this, &Window::stopClicked);
    connect(m_transport, &TransportControls::loopToggled, this, &Window::loopToggled);
    connect(m_transport, &TransportControls::volumeChanged, this, &Window::volumeChanged);
    connect(m_transport, &TransportControls::autoplayChanged, this, &Window::autoplayChanged);
    connect(this, &Window::statusMessage, this, [this](const QString &message) { statusBar()->showMessage(message); });
}

void Window::setFolderModel(QAbstractItemModel *model) { m_folderTree->setModel(model); }

void Window::setFileModel(QAbstractItemModel *model) { m_fileTable->setModel(model); }

void Window::selectDirectory(const QModelIndex &index) {
    m_folderTree->setCurrentIndex(index);
    m_folderTree->expand(index);
}

void Window::showScanResult(const QString &rootPath, int fileCount) {
    emit statusMessage(QStringLiteral("%1 — %2 audio file(s)").arg(rootPath).arg(fileCount));
}

void Window::showAudioInfo(const library::AudioFile &info) {
    emit statusMessage(QStringLiteral("%1 — %2 s, %3 kHz, %4, %5, %6 kbps")
                           .arg(info.fileName, QString::number(info.durationSec, 'f', 2),
                                QString::number(info.sampleRate / 1000.0, 'f', 1),
                                info.channels == 2 ? QStringLiteral("stereo") : QStringLiteral("mono"), info.format,
                                QString::number(info.bitRateKbps, 'f', 0)));
}

void Window::showError(const QString &message) { emit statusMessage(message); }

void Window::setControlsEnabled(bool enabled) { m_transport->setControlsEnabled(enabled); }

void Window::setVolume(int value) { m_transport->setVolume(value); }

void Window::clearWaveform() { m_waveform->clear(); }

void Window::setPeaks(const waveform::Peaks &peaks) { m_waveform->setData(peaks); }

void Window::setProgress(double fraction) { m_waveform->setProgress(fraction); }

int Window::waveformWidth() const { return m_waveform->width(); }

void Window::chooseFolder() {
    QFileDialog dialog(this, QStringLiteral("Open Folder"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    // Qlementine mis-sizes item views whose iconSize was never set (the dialog
    // sidebar), causing its icons to overlap the labels. Give it an explicit size.
    for (QListView *view : dialog.findChildren<QListView *>()) {
        if (view->objectName() == QLatin1String("sidebar"))
            view->setIconSize(QSize(16, 16));
    }

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QStringList selected = dialog.selectedFiles();
    if (selected.isEmpty() || selected.first().isEmpty())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    emit folderChosen(selected.first());
    QApplication::restoreOverrideCursor();
}

} // namespace ui
