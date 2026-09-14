#include "app/playback_controller.h"

#include "audio/player.h"
#include "ui/file_table_widget.h"
#include "ui/transport_controls.h"
#include "ui/waveform_widget.h"
#include "ui/window.h"

#include <QFileInfo>
#include <QTimer>

#include <algorithm>

namespace app {

namespace {

// The FFmpeg backend reports the position every 50 ms; this ticker fills the
// gaps so the playhead moves at roughly 60 fps.
constexpr int kProgressTickMs = 16;

} // namespace

PlaybackController::PlaybackController(ui::Window *window, audio::Player *player, QObject *parent)
    : QObject(parent), m_window(window), m_player(player), m_tickTimer(new QTimer(this)) {
    connect(m_window->transport(), &ui::TransportControls::playClicked, m_player, &audio::Player::play);
    connect(m_window->transport(), &ui::TransportControls::stopClicked, m_player, &audio::Player::stop);
    connect(m_window->transport(), &ui::TransportControls::loopToggled, m_player, &audio::Player::setLooping);
    connect(m_window->transport(), &ui::TransportControls::volumeChanged, m_player, &audio::Player::setVolumePercent);

    connect(m_player, &audio::Player::errorOccurred, this, [this](const QString &message) {
        const QString fileName = QFileInfo(m_player->filePath()).fileName();
        m_window->setStatusMessage(QStringLiteral("Could not play %1: %2").arg(fileName, message));
    });

    connect(m_player, &audio::Player::positionChanged, this, &PlaybackController::onPositionChanged);
    connect(m_player, &audio::Player::playingChanged, this, &PlaybackController::onPlayingChanged);

    connect(m_window->fileTable(), &ui::FileTableWidget::playPauseRequested, this, &PlaybackController::togglePlayback);
    connect(m_window->waveform(), &ui::WaveformWidget::seekRequested, this, &PlaybackController::seekToFraction);

    m_tickTimer->setInterval(kProgressTickMs);
    m_tickTimer->setTimerType(Qt::PreciseTimer);
    connect(m_tickTimer, &QTimer::timeout, this, &PlaybackController::updateProgress);

    m_clock.start();

    m_window->transport()->setVolume(m_player->volumePercent());
}

void PlaybackController::onSoundSelected(const library::AudioFile &file) {
    if (openSound(file) && m_window->transport()->autoplay())
        m_player->play();
}

void PlaybackController::onSoundActivated(const library::AudioFile &file) {
    if (openSound(file))
        m_player->play();
}

bool PlaybackController::openSound(const library::AudioFile &file) {
    if (!m_player->open(file.filePath)) {
        m_window->setStatusMessage(QStringLiteral("Failed to open %1").arg(file.fileName));
        return false;
    }

    m_basePositionSec = 0.0;
    m_baseClockMs     = m_clock.elapsed();
    m_window->waveform()->setProgress(0.0);
    m_window->transport()->setControlsEnabled(true);
    m_window->statusAudioInfo(file);
    return true;
}

void PlaybackController::onPositionChanged(double seconds) {
    m_basePositionSec = seconds;
    m_baseClockMs     = m_clock.elapsed();
    updateProgress();
}

void PlaybackController::onPlayingChanged(bool playing) {
    if (playing) {
        m_baseClockMs = m_clock.elapsed();
        m_tickTimer->start();
        return;
    }

    m_tickTimer->stop();
    updateProgress();
}

void PlaybackController::seekToFraction(double fraction) {
    const double length = m_player->lengthSec();
    if (!m_player->hasMedia() || length <= 0.0)
        return;

    const double seconds = std::clamp(fraction, 0.0, 1.0) * length;
    m_player->setPositionSec(seconds);

    // Move the interpolated playhead immediately instead of waiting for the
    // player's next position update.
    m_basePositionSec = seconds;
    m_baseClockMs     = m_clock.elapsed();
    updateProgress();
}

void PlaybackController::togglePlayback() {
    if (!m_player->hasMedia())
        return;

    if (m_player->isPlaying())
        m_player->stop();
    else
        m_player->play();
}

void PlaybackController::updateProgress() {
    const double length = m_player->lengthSec();
    if (!m_player->hasMedia() || length <= 0.0) {
        m_window->waveform()->setProgress(0.0);
        return;
    }

    double position = m_basePositionSec;
    if (m_tickTimer->isActive())
        position += static_cast<double>(m_clock.elapsed() - m_baseClockMs) / 1000.0;

    m_window->waveform()->setProgress(std::clamp(position, 0.0, length) / length);
}

} // namespace app
