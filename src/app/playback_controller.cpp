#include "app/playback_controller.h"

#include "audio/player.h"
#include "ui/transport_controls.h"
#include "ui/window.h"

#include <QTimer>

namespace app {

PlaybackController::PlaybackController(ui::Window *window, audio::Player *player, QObject *parent)
    : QObject(parent), m_window(window), m_player(player), m_progressTimer(new QTimer(this)) {
    connect(m_window->transport(), &ui::TransportControls::playClicked, m_player, &audio::Player::play);
    connect(m_window->transport(), &ui::TransportControls::stopClicked, m_player, &audio::Player::stop);
    connect(m_window->transport(), &ui::TransportControls::loopToggled, m_player, &audio::Player::setLooping);
    connect(m_window->transport(), &ui::TransportControls::volumeChanged, m_player, &audio::Player::setVolumePercent);

    connect(m_player, &audio::Player::loadFailed, m_window, &ui::Window::showError);

    m_progressTimer->setInterval(50);
    connect(m_progressTimer, &QTimer::timeout, this, &PlaybackController::updateProgress);
    m_progressTimer->start();

    m_window->transport()->setVolume(m_player->volumePercent());
}

void PlaybackController::onSoundSelected(const library::AudioFile &file) {
    if (!m_player->open(file.filePath)) {
        m_window->showError(QStringLiteral("Failed to open %1").arg(file.fileName));
        return;
    }

    m_window->setProgress(0.0);
    m_window->transport()->setControlsEnabled(true);
    m_window->showAudioInfo(file);

    if (m_window->transport()->autoplay())
        m_player->play();
}

void PlaybackController::updateProgress() {
    if (m_player->filePath().isEmpty()) {
        m_window->setProgress(0.0);
        return;
    }

    const double length = m_player->lengthSec();
    m_window->setProgress(length > 0.0 ? m_player->positionSec() / length : 0.0);
}

} // namespace app
