#include "waveformwidget.h"

#include <QPainter>

#include <algorithm>

constexpr int minColumnWidth = 2;

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("waveformZone"));
    setMinimumHeight(160);
}

void WaveformWidget::setData(const audio::WaveformData &data) {
    m_data = data;
    update();
}

void WaveformWidget::clear() {
    m_data     = audio::WaveformData();
    m_progress = 0.0;
    update();
}

void WaveformWidget::setProgress(double fraction) {
    const double clamped = std::clamp(fraction, 0.0, 1.0);
    if (qFuzzyCompare(m_progress, clamped))
        return;
    m_progress = clamped;
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), palette().color(QPalette::Window));

    if (!m_data.valid()) {
        painter.setPen(palette().color(QPalette::PlaceholderText));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Select a file to view its waveform"));
        return;
    }

    const QColor played   = palette().color(QPalette::WindowText);
    QColor       unplayed = palette().color(QPalette::Highlight);
    unplayed.setAlpha(150);

    const int   columns  = m_data.mins.size();
    const int   colWidth = std::max(minColumnWidth, width() / columns);
    const qreal midY     = height() / 2.0;
    const qreal amp      = height() * 0.42;
    const qreal playX    = width() * m_progress;

    for (int i = 0; i < columns; ++i) {
        const qreal x         = i * colWidth;
        const qreal minY      = midY - m_data.maxs.at(i) * amp;
        const qreal maxY      = midY - m_data.mins.at(i) * amp;
        const qreal barBottom = std::max(minY, maxY);
        const qreal barHeight = std::max(1.0, std::abs(maxY - minY));

        painter.fillRect(QRectF(x, barBottom, colWidth, barHeight), x < playX ? played : unplayed);
    }

    painter.setPen(QPen(palette().color(QPalette::Mid), 1));
    painter.drawLine(QPointF(0, midY), QPointF(width(), midY));

    if (m_progress > 0.0) {
        painter.setPen(QPen(palette().color(QPalette::WindowText), 1));
        painter.drawLine(QPointF(playX, 0), QPointF(playX, height()));
    }
}