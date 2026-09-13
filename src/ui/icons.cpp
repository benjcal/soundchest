#include "icons.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>

#include <oclero/qlementine/style/QlementineStyle.hpp>

namespace {

oclero::qlementine::QlementineStyle *qlementineStyle() {
    return qobject_cast<oclero::qlementine::QlementineStyle *>(qApp->style());
}

QPixmap renderSvg(const QString &name, const QSize &size) {
    QSvgRenderer renderer(QStringLiteral(":/icons/%1.svg").arg(name));
    QPixmap      pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return pixmap;
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

namespace icons {

QIcon mono(const QString &name) { return QIcon(QStringLiteral(":/icons/%1.svg").arg(name)); }

void setAutoRecolor(QWidget *widget) {
    if (qlementineStyle())
        oclero::qlementine::QlementineStyle::setAutoIconColor(widget,
                                                              oclero::qlementine::AutoIconColor::ForegroundColor);
}

QPixmap colorized(const QString &name, const QSize &size, const QColor &color) {
    return recolor(renderSvg(name, size), color);
}

QColor accent() {
    if (auto *style = qlementineStyle())
        return style->theme().primaryColor;
    return qApp->palette().color(QPalette::Highlight);
}

} // namespace icons
