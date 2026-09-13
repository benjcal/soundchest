#include "player.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QUrl>

#include <algorithm>

#ifdef Q_OS_LINUX
#include <dlfcn.h>
#include <link.h>

#include <cstring>
#endif

namespace audio {

namespace {

#ifdef Q_OS_LINUX

// Qt's FFmpeg plugin sends decoder warnings (e.g. Vorbis timestamp warnings)
// straight to stderr. Keep errors, drop the rest. The plugin is loaded lazily
// and with RTLD_LOCAL, so find the already loaded libavutil and resolve
// av_log_set_level from it.
bool applyDecoderWarningSilence() {
    void *handle = nullptr;
    dl_iterate_phdr(
        [](dl_phdr_info *info, size_t, void *data) -> int {
            if (!info->dlpi_name || !std::strstr(info->dlpi_name, "libavutil.so"))
                return 0;
            *static_cast<void **>(data) = dlopen(info->dlpi_name, RTLD_NOW | RTLD_NOLOAD);
            return *static_cast<void **>(data) ? 1 : 0;
        },
        &handle);

    if (!handle)
        return false;

    void *symbol = dlsym(handle, "av_log_set_level");
    if (!symbol)
        return false;

    constexpr int avLogError = 16;
    reinterpret_cast<void (*)(int)>(symbol)(avLogError);
    return true;
}

void silenceDecoderWarnings() {
    if (qEnvironmentVariableIsSet("QT_FFMPEG_DEBUG"))
        return;

    static bool applied = false;
    if (!applied)
        applied = applyDecoderWarningSilence();
}

#else

void silenceDecoderWarnings() {}

#endif

} // namespace

Player::Player(QObject *parent)
    : QObject(parent), m_media(new QMediaPlayer(this)), m_audioOutput(new QAudioOutput(this)) {
    m_media->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(m_volume / 100.0f);

    connect(m_media, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &errorString) { emit loadFailed(errorString); });
}

Player::~Player() = default;

bool Player::open(const QString &filePath) {
    if (filePath.isEmpty())
        return false;

    m_filePath = filePath;
    m_media->setSource(QUrl::fromLocalFile(filePath));
    m_media->setLoops(m_looping ? QMediaPlayer::Infinite : 1);
    silenceDecoderWarnings();
    return true;
}

void Player::play() {
    if (!m_filePath.isEmpty())
        m_media->play();
}

void Player::stop() { m_media->stop(); }

void Player::setLooping(bool enabled) {
    m_looping = enabled;
    m_media->setLoops(enabled ? QMediaPlayer::Infinite : 1);
}

void Player::setVolumePercent(int percent) {
    m_volume = std::clamp(percent, 0, 100);
    m_audioOutput->setVolume(m_volume / 100.0f);
}

int Player::volumePercent() const { return m_volume; }

bool Player::isPlaying() const { return m_media->playbackState() == QMediaPlayer::PlayingState; }

double Player::positionSec() const {
    const qint64 position = m_media->position();
    return position > 0 ? position / 1000.0 : 0.0;
}

double Player::lengthSec() const {
    const qint64 duration = m_media->duration();
    return duration > 0 ? duration / 1000.0 : 0.0;
}

QString Player::filePath() const { return m_filePath; }

} // namespace audio
