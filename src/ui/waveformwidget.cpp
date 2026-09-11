#include "waveformwidget.h"

#include <QPainter>

WaveformWidget::WaveformWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("waveformZone"));
    setMinimumHeight(160);
}

void WaveformWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().color(QPalette::Window));
    painter.setPen(palette().color(QPalette::PlaceholderText));
    painter.drawText(rect(), Qt::AlignCenter, tr("Waveform view — arrives in Stage 4"));
}