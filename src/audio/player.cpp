#include "player.h"

#include "sf_data_source.h"

#include "miniaudio.h"

namespace audio {

Player::Player(QObject* parent)
    : QObject(parent)
    , m_engine(std::make_unique<ma_engine>())
    , m_sound(std::make_unique<ma_sound>())
{
    const ma_result result = ma_engine_init(nullptr, m_engine.get());
    m_engineReady = result == MA_SUCCESS;
    if (!m_engineReady)
        m_error = QString::fromUtf8(ma_result_description(result));
}

Player::~Player()
{
    close();
    if (m_engineReady)
        ma_engine_uninit(m_engine.get());
}

bool Player::open(const QString& filePath)
{
    if (!m_engineReady) {
        m_error = QStringLiteral("audio engine failed to initialize");
        return false;
    }

    close();

    auto* dataSource = new SfDataSource;
    QString error;
    if (!openSfDataSource(filePath, dataSource, &error)) {
        delete dataSource;
        m_error = error;
        return false;
    }

    const ma_result result =
        ma_sound_init_from_data_source(m_engine.get(), &dataSource->base, 0, nullptr,
                                       m_sound.get());
    if (result != MA_SUCCESS) {
        closeSfDataSource(dataSource);
        delete dataSource;
        m_error = QString::fromUtf8(ma_result_description(result));
        return false;
    }

    m_dataSource = dataSource;
    m_filePath = filePath;
    m_loaded = true;
    m_error.clear();

    m_channels = static_cast<int>(m_dataSource->channels);
    m_sampleRate = static_cast<int>(m_dataSource->sampleRate);
    m_lengthSec = m_sampleRate > 0
                      ? static_cast<double>(m_dataSource->lengthFrames) / m_sampleRate
                      : 0.0;

    ma_sound_set_looping(m_sound.get(), m_looping ? MA_TRUE : MA_FALSE);
    ma_sound_set_volume(m_sound.get(), m_volume);

    return true;
}

void Player::close()
{
    if (m_loaded) {
        ma_sound_uninit(m_sound.get());
        m_loaded = false;
    }
    if (m_dataSource) {
        closeSfDataSource(m_dataSource);
        delete m_dataSource;
        m_dataSource = nullptr;
    }
    m_filePath.clear();
    m_lengthSec = 0.0;
    m_sampleRate = 0;
    m_channels = 0;
    m_error.clear();
}

void Player::play()
{
    if (m_loaded)
        ma_sound_start(m_sound.get());
}

void Player::stop()
{
    if (!m_loaded)
        return;
    ma_sound_stop(m_sound.get());
    ma_sound_seek_to_pcm_frame(m_sound.get(), 0);
}

void Player::setLooping(bool enabled)
{
    m_looping = enabled;
    if (m_loaded)
        ma_sound_set_looping(m_sound.get(), enabled ? MA_TRUE : MA_FALSE);
}

void Player::setVolume(float linear)
{
    m_volume = qBound(0.0f, linear, 1.0f);
    if (m_loaded)
        ma_sound_set_volume(m_sound.get(), m_volume);
}

void Player::setVolumePercent(int percent)
{
    setVolume(percent / 100.0f);
}

int Player::volumePercent() const
{
    return qRound(m_volume * 100.0f);
}

bool Player::isPlaying() const
{
    return m_loaded && ma_sound_is_playing(m_sound.get()) != MA_FALSE;
}

double Player::positionSec() const
{
    if (!m_loaded)
        return 0.0;
    ma_uint64 cursor = 0;
    ma_sound_get_cursor_in_pcm_frames(m_sound.get(), &cursor);
    return m_sampleRate > 0 ? static_cast<double>(cursor) / m_sampleRate : 0.0;
}

double Player::lengthSec() const
{
    return m_lengthSec;
}

QString Player::filePath() const
{
    return m_filePath;
}

QString Player::errorString() const
{
    return m_error;
}

} // namespace audio