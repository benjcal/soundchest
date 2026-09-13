#include "waveform_widget.h"

#include "ui/waveform_painter.h"

#include <QPainter>

#include <algorithm>

namespace ui {

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("waveformZone"));
    setMinimumHeight(160);
}

void WaveformWidget::setData(const waveform::Peaks &data) {
    m_data = data;
    update();
}

void WaveformWidget::clear() {
    m_data     = waveform::Peaks();
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

    const QRectF area = rect();

    if (!m_data.valid()) {
        painter.setPen(palette().color(QPalette::PlaceholderText));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Select a file to view its waveform"));
        return;
    }

    const QColor played   = palette().color(QPalette::WindowText);
    QColor       unplayed = palette().color(QPalette::Highlight);
    unplayed.setAlpha(150);

    const int   columns     = m_data.mins.size();
    const qreal columnWidth = waveform::barWidth(area, columns);
    const qreal playX       = area.width() * m_progress;

    painter.setPen(Qt::NoPen);
    for (int i = 0; i < columns; ++i) {
        const QRectF bar = waveform::barRect(area, i, columnWidth, m_data.mins.at(i), m_data.maxs.at(i));
        painter.fillRect(bar, bar.left() < playX ? played : unplayed);
    }

    waveform::paintMidline(&painter, area, palette().color(QPalette::Mid));

    if (m_progress > 0.0) {
        painter.setPen(QPen(palette().color(QPalette::WindowText), 1));
        painter.drawLine(QPointF(playX, 0), QPointF(playX, height()));
    }
}

} // namespace ui
