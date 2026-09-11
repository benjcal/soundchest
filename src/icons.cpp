#include "icons.h"

#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

namespace icons {

QIcon themed(const QString& name, const QColor& color)
{
    QFile file(QStringLiteral(":/icons/%1.svg").arg(name));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QString svg = QString::fromUtf8(file.readAll());
    const QString fill = QStringLiteral("fill=\"%1\"").arg(color.name());
    svg.replace(QStringLiteral("<svg "), QStringLiteral("<svg %1 ").arg(fill));

    QIcon icon;
    const QList<int> sizes{ 16, 20, 24, 32, 48, 64 };
    for (const int size : sizes) {
        QSvgRenderer renderer(svg.toUtf8());
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        painter.end();
        icon.addPixmap(pixmap);
    }
    return icon;
}

} // namespace icons