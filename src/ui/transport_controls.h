#pragma once

#include <QWidget>

class QLabel;
class QSlider;
class QToolButton;

namespace oclero::qlementine {
class Switch;
} // namespace oclero::qlementine

namespace ui {

class TransportControls : public QWidget {
    Q_OBJECT

  public:
    explicit TransportControls(QWidget *parent = nullptr);

    void setControlsEnabled(bool enabled);
    void setVolume(int value);
    bool autoplay() const;

  signals:
    void playClicked();
    void stopClicked();
    void loopToggled(bool enabled);
    void autoplayChanged(bool enabled);
    void volumeChanged(int value);

  private:
    QToolButton                *m_playButton;
    QToolButton                *m_stopButton;
    QToolButton                *m_loopButton;
    oclero::qlementine::Switch *m_autoplaySwitch;
    QSlider                    *m_volumeSlider;
};

} // namespace ui
