#include "mainwindow.h"

#include "filetable.h"
#include "folderbrowser.h"
#include "headerbar.h"
#include "transportbar.h"
#include "waveformwidget.h"

#include <QFileDialog>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Sound Chest"));
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
    m_fileTable = new FileTable(splitter);

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
    statusBar()->showMessage(tr("Ready"));

    connect(m_header, &HeaderBar::openFolderRequested, this, [this] {
        const QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"));
        if (!dir.isEmpty())
            statusBar()->showMessage(tr("Selected: %1").arg(dir));
    });
}