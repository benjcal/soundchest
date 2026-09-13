#include "waveform_widget.h"

#include "ui/theme.h"
#include "ui/waveform_renderer.h"

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
    painter.fillRect(rect(), theme::workspaceBackground());

    painter.setPen(theme::border());
    painter.drawLine(rect().topLeft(), rect().topRight());

    const QRectF area = rect().adjusted(0, 1, 0, 0);

    if (!m_data.valid()) {
        painter.setPen(palette().color(QPalette::PlaceholderText));
        painter.drawText(area, Qt::AlignCenter, QStringLiteral("Select a file to view its waveform"));
        return;
    }

    const QColor wave = theme::waveformColor();

    QColor played = wave;
    played.setAlphaF(0.85f);
    QColor unplayed = wave;
    unplayed.setAlphaF(0.55f);

    ui::paintWaveform(&painter, area, m_data, played, unplayed, m_progress);

    if (m_progress > 0.0) {
        const qreal playX = area.left() + area.width() * m_progress;

        QColor tint = wave;
        tint.setAlphaF(0.1f);
        painter.fillRect(QRectF(area.left(), area.top(), playX - area.left(), area.height()), tint);

        QColor head = wave;
        head.setAlphaF(0.85f);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.fillRect(QRectF(playX - 1.0, area.top(), 2.0, area.height()), head);
    }
}

} // namespace ui
