#include "icons.h"

#include "theme.h"

#include <QApplication>
#include <QPainter>
#include <QSvgRenderer>

#include <oclero/qlementine/style/QlementineStyle.hpp>

namespace {

QPixmap renderSvgPath(const QString &path, const QSize &size) {
    QSvgRenderer renderer(path);
    QPixmap      pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return pixmap;
}

QPixmap renderSvg(const QString &name, const QSize &size) {
    return renderSvgPath(QStringLiteral(":/icons/%1.svg").arg(name), size);
}

QPixmap recolor(const QPixmap &source, const QColor &color) {
    QPixmap result(source.size());
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.drawPixmap(0, 0, source);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(result.rect(), color);
    return result;
}

} // namespace

namespace ui::icons {

QIcon mono(const QString &name) { return QIcon(QStringLiteral(":/icons/%1.svg").arg(name)); }

void setAutoRecolor(QWidget *widget) {
    if (theme::qlementineStyle())
        oclero::qlementine::QlementineStyle::setAutoIconColor(widget,
                                                              oclero::qlementine::AutoIconColor::ForegroundColor);
}

QPixmap colorized(const QString &name, const QSize &size, const QColor &color) {
    return recolor(renderSvg(name, size), color);
}

QPixmap pixmap(const QString &resourcePath, const QSize &size) {
    QSvgRenderer renderer(resourcePath);

    // viewBoxF() keeps the aspect ratio exact; defaultSize() rounds to ints.
    const QRectF viewBox     = renderer.viewBoxF();
    const QSizeF naturalSize = viewBox.isEmpty() ? QSizeF(renderer.defaultSize()) : viewBox.size();

    QSize target = size;
    if (target.width() <= 0 && target.height() > 0 && !naturalSize.isEmpty())
        target.setWidth(qRound(target.height() * naturalSize.width() / naturalSize.height()));
    if (target.height() <= 0 && target.width() > 0 && !naturalSize.isEmpty())
        target.setHeight(qRound(target.width() * naturalSize.height() / naturalSize.width()));
    if (target.isEmpty())
        return {};

    const qreal dpr = qApp->devicePixelRatio();
    QPixmap     result(target * dpr);
    result.setDevicePixelRatio(dpr);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    renderer.render(&painter, QRectF(QPointF(0.0, 0.0), QSizeF(target)));
    return result;
}

} // namespace ui::icons
