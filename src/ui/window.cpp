#include "window.h"

#include "file_table_widget.h"
#include "folder_tree_widget.h"
#include "format.h"
#include "headerbar.h"
#include "icons.h"
#include "transport_controls.h"
#include "waveform_widget.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QKeySequence>
#include <QModelIndex>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

#include <oclero/qlementine/widgets/AboutDialog.hpp>

namespace ui {

namespace {

constexpr auto kLastOpenDirKey = "open/lastDir";
constexpr auto kRepoUrl        = "https://github.com/benjcal/soundchest";
constexpr auto kNoticesUrl     = "https://github.com/benjcal/soundchest/blob/main/THIRD_PARTY_NOTICES.md";

} // namespace

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

    connect(m_header, &HeaderBar::aboutRequested, this, &Window::showAboutDialog);

    auto *aboutShortcut = new QShortcut(QKeySequence::HelpContents, this);
    connect(aboutShortcut, &QShortcut::activated, this, &Window::showAboutDialog);
}

FileTableWidget *Window::fileTable() const { return m_fileTable; }

FolderTreeWidget *Window::folderTree() const { return m_folderTree; }

HeaderBar *Window::headerBar() const { return m_header; }

TransportControls *Window::transport() const { return m_transport; }

WaveformWidget *Window::waveform() const { return m_waveform; }

void Window::focusDirectoryInTree(const QModelIndex &index) {
    m_folderTree->setCurrentIndex(index);
    m_folderTree->expand(index);
}

void Window::showScanResult(const QString &rootPath, int fileCount) {
    statusBar()->showMessage(QStringLiteral("%1 — %2 audio file(s)").arg(rootPath).arg(fileCount));
}

void Window::statusAudioInfo(const library::AudioFile &info, const analysis::SoundStats *stats) {
    QString message =
        QStringLiteral("%1 — %2, %3 kHz, %4, %5, %6 kbps")
            .arg(info.fileName, formatDuration(info.durationSec), QString::number(info.sampleRate / 1000.0, 'f', 1),
                 formatChannels(info.channels), info.format, QString::number(info.bitRateKbps, 'f', 0));

    if (stats) {
        message += QStringLiteral(" — peak %1 dBFS, true peak %2 dBTP, RMS %3 dBFS, %4 LUFS")
                       .arg(stats->samplePeakDbfs, 0, 'f', 1)
                       .arg(stats->truePeakDbtp, 0, 'f', 1)
                       .arg(stats->rmsDbfs, 0, 'f', 1)
                       .arg(stats->lufs, 0, 'f', 1);
    }

    statusBar()->showMessage(message);
}

void Window::setStatusMessage(const QString &message) { statusBar()->showMessage(message); }

QString Window::chooseFolderPath() {
    const QSettings settings;
    const QString   startDir = settings.value(kLastOpenDirKey, QDir::homePath()).toString();

    const QString path =
        QFileDialog::getExistingDirectory(this, QStringLiteral("Open Folder"), startDir, QFileDialog::ShowDirsOnly);

    if (!path.isEmpty()) {
        QSettings writableSettings;
        writableSettings.setValue(kLastOpenDirKey, path);
    }
    return path;
}

QString Window::chooseExportFolderPath(const QString &startDir) {
    return QFileDialog::getExistingDirectory(this, QStringLiteral("Export To Folder"), startDir,
                                             QFileDialog::ShowDirsOnly);
}

void Window::showAboutDialog() {
    oclero::qlementine::AboutDialog dialog(this);
    dialog.setIcon(QIcon(QStringLiteral(":/branding/appicon.svg")));
    dialog.setApplicationVersion(QApplication::applicationVersion());
    dialog.setDescription(QStringLiteral("Browse and audition a folder full of sound effects."));
    dialog.setWebsiteUrl(QString::fromLatin1(kRepoUrl));
    dialog.setLicense(QStringLiteral("MIT License"));
    dialog.setCopyright(QStringLiteral("© 2026 Benjamin Calderon"));

    const QColor iconColor = palette().color(QPalette::WindowText);
    dialog.addSocialMediaLink(QStringLiteral("GitHub"), QString::fromLatin1(kRepoUrl),
                              QIcon(icons::colorized(QStringLiteral("github-logo"), QSize(20, 20), iconColor)));
    dialog.addSocialMediaLink(QStringLiteral("Third-party notices"), QString::fromLatin1(kNoticesUrl),
                              QIcon(icons::colorized(QStringLiteral("file-text"), QSize(20, 20), iconColor)));

    dialog.exec();
}

} // namespace ui
