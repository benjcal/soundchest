#include "waveform_painter.h"

#include <QPainter>

#include <algorithm>
#include <cmath>

namespace waveform {

qreal barWidth(const QRectF &area, int columns) { return columns > 0 ? area.width() / columns : area.width(); }

QRectF barRect(const QRectF &area, int column, qreal columnWidth, float min, float max) {
    const qreal midY      = area.center().y();
    const qreal amplitude = area.height() * 0.42;

    const qreal x         = area.left() + column * columnWidth;
    const qreal top       = midY - max * amplitude;
    const qreal bottom    = midY - min * amplitude;
    const qreal barTop    = std::min(top, bottom);
    const qreal barHeight = std::max(1.0, std::abs(bottom - top));

    return QRectF(x, barTop, std::max(1.0, columnWidth), barHeight);
}

void paintMidline(QPainter *painter, const QRectF &area, const QColor &color) {
    painter->setPen(QPen(color, 1));
    painter->drawLine(QPointF(area.left(), area.center().y()), QPointF(area.right(), area.center().y()));
}

} // namespace waveform
