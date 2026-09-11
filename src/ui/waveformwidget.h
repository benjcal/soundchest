#pragma once

#include <QWidget>

#include "audio/waveform.h"

class WaveformWidget : public QWidget {
    Q_OBJECT

  public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void setData(const audio::WaveformData &data);
    void clear();
    void setProgress(double fraction);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    audio::WaveformData m_data;
    double              m_progress = 0.0;
};