#pragma once

#include <QColor>
#include <QRectF>

class QPainter;

namespace waveform {
struct Peaks;
}

namespace ui {

// Draws the min/max peak envelope, one device pixel column at a time. Source
// columns are folded into display pixels, so quiet and loud passages keep the
// same shape at any width. Columns left of progressFraction use playedColor,
// the rest unplayedColor.
void paintWaveform(QPainter *painter, const QRectF &area, const waveform::Peaks &peaks, const QColor &playedColor,
                   const QColor &unplayedColor, qreal progressFraction);

} // namespace ui
