#pragma once

#include <QPixmap>
#include <QRectF>
#include <QWidget>

#include "waveform/peaks.h"

namespace ui {

class WaveformWidget : public QWidget {
    Q_OBJECT

  public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void setData(const waveform::Peaks &data);
    void clear();
    void setProgress(double fraction);

  signals:
    void seekRequested(double fraction);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    // Rendering the waveform path is expensive, so cache it as two pixmaps
    // (played and unplayed) and only blit them as progress changes.
    void ensureCache(const QRectF &area);
    void renderWaveformCache(QPixmap &cache, qreal alpha, const QRectF &area);
    void emitSeekAt(qreal x);

    waveform::Peaks m_data;
    QPixmap         m_unplayedCache;
    QPixmap         m_playedCache;
    double          m_progress = 0.0;
};

} // namespace ui
