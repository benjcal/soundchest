#pragma once

#include <QColor>
#include <QRectF>

class QPainter;

namespace waveform {

struct Peaks;

qreal  barWidth(const QRectF &area, int columns);
QRectF barRect(const QRectF &area, int column, qreal columnWidth, float min, float max);
void   paintMidline(QPainter *painter, const QRectF &area, const QColor &color);

} // namespace waveform
