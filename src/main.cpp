#include "theme.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QPalette>

namespace {

void loadFonts() {
    const QStringList files{
        QStringLiteral(":/fonts/Inter-Regular.ttf"),
        QStringLiteral(":/fonts/Inter-Medium.ttf"),
        QStringLiteral(":/fonts/Inter-SemiBold.ttf"),
        QStringLiteral(":/fonts/Inter-Bold.ttf"),
    };

    for (const QString &file : files)
        QFontDatabase::addApplicationFont(file);

    const QStringList families = QFontDatabase::families(QFontDatabase::Any);
    if (families.contains(QStringLiteral("Inter"))) {
        QFont font(QStringLiteral("Inter"));
        font.setPointSizeF(10.5);
        QApplication::setFont(font);
    }
}

} // namespace

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("soundchest"));
    app.setApplicationDisplayName(QStringLiteral("Sound Chest"));
    app.setOrganizationName(QStringLiteral("soundchest"));

    QApplication::setStyle(QStringLiteral("Fusion"));
    app.setPalette(theme::blenderPalette());

    QFile qss(QStringLiteral(":/styles/app.qss"));
    if (qss.open(QIODevice::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));

    loadFonts();

    MainWindow window;
    window.show();

    return app.exec();
}