#pragma once

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

  private slots:
    void updateProgress();

  private:
    ui::Window    *m_window;
    audio::Player *m_player;
    QTimer        *m_progressTimer;
};

} // namespace app
