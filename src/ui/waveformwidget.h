#pragma once

#include <QWidget>

#include "waveform/waveform.h"

namespace ui {

class WaveformWidget : public QWidget {
    Q_OBJECT

  public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void setData(const waveform::WaveformData &data);
    void clear();
    void setProgress(double fraction);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    waveform::WaveformData m_data;
    double                 m_progress = 0.0;
};

} // namespace ui
