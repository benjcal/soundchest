#include "mainview.h"

#include "filetable.h"
#include "folderbrowser.h"
#include "headerbar.h"
#include "transportbar.h"
#include "waveformwidget.h"

#include <QApplication>
#include <QFileDialog>
#include <QListView>
#include <QModelIndex>
#include <QSplitter>
#include <QVBoxLayout>

namespace ui {

MainView::MainView(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_header    = new HeaderBar(this);
    m_waveform  = new WaveformWidget(this);
    m_transport = new TransportBar(this);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(QStringLiteral("browserSplitter"));
    splitter->setChildrenCollapsible(false);

    m_folderBrowser = new FolderBrowser(splitter);
    m_fileTable     = new FileTable(splitter);
    splitter->addWidget(m_folderBrowser);
    splitter->addWidget(m_fileTable);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({260, 940});

    layout->addWidget(m_header);
    layout->addWidget(m_waveform, 3);
    layout->addWidget(m_transport);
    layout->addWidget(splitter, 5);

    connect(m_header, &HeaderBar::openFolderRequested, this, &MainView::chooseFolder);
    connect(m_folderBrowser, &FolderBrowser::currentDirectoryChanged, this, &MainView::directorySelected);
    connect(m_fileTable, &FileTable::currentFileChanged, this, &MainView::fileSelected);
    connect(m_fileTable, &FileTable::visibleRowsChanged, this, &MainView::visibleRowsChanged);
    connect(m_transport, &TransportBar::playClicked, this, &MainView::playClicked);
    connect(m_transport, &TransportBar::stopClicked, this, &MainView::stopClicked);
    connect(m_transport, &TransportBar::loopToggled, this, &MainView::loopToggled);
    connect(m_transport, &TransportBar::volumeChanged, this, &MainView::volumeChanged);
    connect(m_transport, &TransportBar::autoplayChanged, this, &MainView::autoplayChanged);
}

void MainView::setFolderModel(QAbstractItemModel *model) { m_folderBrowser->setModel(model); }

void MainView::setFileModel(QAbstractItemModel *model) { m_fileTable->setModel(model); }

void MainView::selectDirectory(const QModelIndex &index) {
    m_folderBrowser->setCurrentIndex(index);
    m_folderBrowser->expand(index);
}

void MainView::showScanResult(const QString &rootPath, int fileCount) {
    emit statusMessage(QStringLiteral("%1 — %2 audio file(s)").arg(rootPath).arg(fileCount));
}

void MainView::showFileInfo(const audio::AudioInfo &info) {
    emit statusMessage(QStringLiteral("%1 — %2 s, %3 kHz, %4, %5, %6 kbps")
                           .arg(info.fileName, QString::number(info.durationSec, 'f', 2),
                                QString::number(info.sampleRate / 1000.0, 'f', 1),
                                info.channels == 2 ? QStringLiteral("stereo") : QStringLiteral("mono"), info.format,
                                QString::number(info.bitRateKbps, 'f', 0)));
}

void MainView::showError(const QString &message) { emit statusMessage(message); }

void MainView::setControlsEnabled(bool enabled) { m_transport->setControlsEnabled(enabled); }

void MainView::setVolume(int value) { m_transport->setVolume(value); }

void MainView::clearWaveform() { m_waveform->clear(); }

void MainView::setWaveform(const waveform::WaveformData &data) { m_waveform->setData(data); }

void MainView::setProgress(double fraction) { m_waveform->setProgress(fraction); }

int MainView::waveformWidth() const { return m_waveform->width(); }

void MainView::chooseFolder() {
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
