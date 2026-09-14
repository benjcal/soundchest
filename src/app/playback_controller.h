#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include "library/audio_file.h"

namespace audio {
class Player;
} // namespace audio

namespace ui {
class Window;
} // namespace ui

namespace app {

class PlaybackController : public QObject {
    Q_OBJECT

  public:
    PlaybackController(ui::Window *window, audio::Player *player, QObject *parent = nullptr);

  public slots:
    void onSoundSelected(const library::AudioFile &file);
    void onSoundActivated(const library::AudioFile &file);

  private slots:
    void updateProgress();
    void onPositionChanged(double seconds);
    void onPlayingChanged(bool playing);
    void seekToFraction(double fraction);
    void togglePlayback();

  private:
    bool openSound(const library::AudioFile &file);

    // Playhead interpolation: the player reports position every ~50 ms, so the
    // 16 ms ticker adds elapsed time on top of the last known position.
    // Invariant: while m_tickTimer runs, position = m_basePositionSec +
    // (now - m_baseClockMs). Every position update and seek resets the base.
    ui::Window    *m_window;
    audio::Player *m_player;
    QTimer        *m_tickTimer;
    QElapsedTimer  m_clock;
    double         m_basePositionSec = 0.0;
    qint64         m_baseClockMs     = 0;
};

} // namespace app
