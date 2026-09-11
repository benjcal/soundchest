#pragma once

#include <QWidget>

class QCheckBox;
class QSlider;
class QToolButton;

class TransportBar : public QWidget
{
    Q_OBJECT

public:
    explicit TransportBar(QWidget* parent = nullptr);

    void setControlsEnabled(bool enabled);
    int volume() const;
    void setVolume(int value);

signals:
    void playClicked();
    void stopClicked();
    void loopToggled(bool enabled);
    void autoplayChanged(bool enabled);
    void volumeChanged(int value);

private:
    QToolButton* m_playButton;
    QToolButton* m_stopButton;
    QToolButton* m_loopButton;
    QCheckBox* m_autoplayCheck;
    QSlider* m_volumeSlider;
};