#include "waveform_renderer.h"

#include "waveform/peaks.h"

#include <QPainter>
#include <QPainterPath>
#include <QVector>

#include <algorithm>
#include <cmath>

namespace ui {

namespace {

QVector<QRectF> columnBands(const QRectF &area, const waveform::Peaks &peaks, int columns, qreal columnWidth) {
    const int   sourceColumns = peaks.mins.size();
    const qreal midY          = area.center().y();
    const qreal amplitude     = area.height() * 0.46;

    QVector<QRectF> bands(columns);
    for (int column = 0; column < columns; ++column) {
        const int from = static_cast<int>(static_cast<qint64>(column) * sourceColumns / columns);
        const int to   = std::max(from + 1, static_cast<int>(static_cast<qint64>(column + 1) * sourceColumns / columns));

        float peak = 0.0f;
        for (int i = from; i < to; ++i)
            peak = std::max(peak, std::max(std::abs(peaks.mins.at(i)), std::abs(peaks.maxs.at(i))));

        const qreal x      = area.left() + column * columnWidth;
        const qreal height = std::max(0.0, peak * amplitude);
        bands[column]      = QRectF(x, midY - height, columnWidth, height * 2.0);
    }

    return bands;
}

QPainterPath bandPath(const QVector<QRectF> &bands, qreal gap) {
    QPainterPath path;
    const int    count = bands.size();

    path.moveTo(bands.first().left(), bands.first().top());
    for (int column = 0; column < count; ++column) {
        const QRectF &band = bands.at(column);
        path.lineTo(band.left(), band.top());
        path.lineTo(band.right() - gap, band.top());
    }
    for (int column = count - 1; column >= 0; --column) {
        const QRectF &band = bands.at(column);
        path.lineTo(band.right() - gap, band.bottom());
        path.lineTo(band.left(), band.bottom());
    }
    path.closeSubpath();

    return path;
}

} // namespace

void paintWaveform(QPainter *painter, const QRectF &area, const waveform::Peaks &peaks, const QColor &playedColor,
                   const QColor &unplayedColor, qreal progressFraction) {
    const int sourceColumns = peaks.mins.size();
    if (sourceColumns == 0 || peaks.maxs.size() != sourceColumns || area.width() <= 0.0 || area.height() <= 0.0)
        return;

    const qreal ratio       = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const int   columns     = std::max(1, qRound(area.width() * ratio));
    const qreal columnWidth = area.width() / columns;
    const qreal gap         = columnWidth >= 2.0 ? 1.0 : 0.0;

    const QVector<QRectF> bands = columnBands(area, peaks, columns, columnWidth);
    const QPainterPath    path  = bandPath(bands, gap);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    painter->fillPath(path, unplayedColor);

    if (progressFraction > 0.0) {
        const qreal playX = area.left() + area.width() * progressFraction;
        painter->save();
        painter->setClipRect(QRectF(area.left(), area.top(), playX - area.left(), area.height()));
        painter->fillPath(path, playedColor);
        painter->restore();
    }

    painter->restore();
}

} // namespace ui
