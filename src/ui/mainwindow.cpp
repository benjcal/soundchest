#include "mainwindow.h"

#include "mainview.h"

#include <QStatusBar>

namespace ui {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_view(new MainView(this)) {
    setWindowTitle(QStringLiteral("Sound Chest"));
    resize(1200, 760);
    setMinimumSize(900, 600);

    setCentralWidget(m_view);

    statusBar()->setObjectName(QStringLiteral("statusBar"));
    statusBar()->showMessage(QStringLiteral("Ready"));

    connect(m_view, &MainView::statusMessage, this,
            [this](const QString &message) { statusBar()->showMessage(message); });
}

MainView *MainWindow::mainView() const { return m_view; }

} // namespace ui
