#pragma once

#include <QWidget>

class WaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};