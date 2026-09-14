#include "waveform_widget.h"

#include "ui/theme.h"
#include "ui/waveform_renderer.h"

#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

namespace ui {

namespace {

constexpr qreal kUnplayedAlpha   = 0.55;
constexpr qreal kPlayedAlpha     = 0.85;
constexpr qreal kPlayedTintAlpha = 0.10;

} // namespace

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("waveformZone"));
    setMinimumHeight(160);
    setCursor(Qt::PointingHandCursor);
}

void WaveformWidget::setData(const waveform::Peaks &data) {
    m_data          = data;
    m_unplayedCache = QPixmap();
    m_playedCache   = QPixmap();
    update();
}

void WaveformWidget::clear() {
    m_data          = waveform::Peaks();
    m_unplayedCache = QPixmap();
    m_playedCache   = QPixmap();
    m_progress      = 0.0;
    update();
}

void WaveformWidget::setProgress(double fraction) {
    const double clamped = std::clamp(fraction, 0.0, 1.0);
    if (qFuzzyCompare(m_progress, clamped))
        return;
    m_progress = clamped;
    update();
}

void WaveformWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton || !m_data.valid()) {
        QWidget::mousePressEvent(event);
        return;
    }

    emitSeekAt(event->position().x());
    event->accept();
}

void WaveformWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton) || !m_data.valid()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    emitSeekAt(event->position().x());
    event->accept();
}

void WaveformWidget::emitSeekAt(qreal x) {
    if (width() <= 0)
        return;

    emit seekRequested(std::clamp(x / width(), 0.0, 1.0));
}

void WaveformWidget::ensureCache(const QRectF &area) {
    const qreal dpr = devicePixelRatioF();
    const QSize expected(qRound(width() * dpr), qRound(height() * dpr));

    if (!m_unplayedCache.isNull() && m_unplayedCache.size() == expected)
        return;

    renderWaveformCache(m_unplayedCache, kUnplayedAlpha, area);
    renderWaveformCache(m_playedCache, kPlayedAlpha, area);
}

void WaveformWidget::renderWaveformCache(QPixmap &cache, qreal alpha, const QRectF &area) {
    const qreal dpr = devicePixelRatioF();
    const QSize size(qRound(width() * dpr), qRound(height() * dpr));

    cache = QPixmap(size);
    cache.setDevicePixelRatio(dpr);
    cache.fill(Qt::transparent);

    QColor color = theme::waveformColor();
    color.setAlphaF(alpha);

    // Paint the whole envelope with the final color; paintEvent clips the
    // played copy at the playhead instead of drawing a split waveform.
    QPainter painter(&cache);
    ui::paintWaveform(&painter, area, m_data, color, color, 1.0);
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

    ensureCache(area);

    painter.drawPixmap(0, 0, m_unplayedCache);

    if (m_progress > 0.0) {
        const qreal playX = area.left() + area.width() * m_progress;

        painter.save();
        painter.setClipRect(QRectF(area.left(), area.top(), playX - area.left(), area.height()));
        painter.drawPixmap(0, 0, m_playedCache);
        painter.restore();

        // The cached played pixmap colors the waveform itself; this is a
        // separate background wash across the full widget height.
        QColor tint = theme::waveformColor();
        tint.setAlphaF(kPlayedTintAlpha);
        painter.fillRect(QRectF(area.left(), area.top(), playX - area.left(), area.height()), tint);

        QColor head = theme::waveformColor();
        head.setAlphaF(kPlayedAlpha);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.fillRect(QRectF(playX - 1.0, area.top(), 2.0, area.height()), head);
    }
}

} // namespace ui
