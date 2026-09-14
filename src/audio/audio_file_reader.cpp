#include "audio_file_reader.h"

#include <cstring>

namespace audio {

namespace {

QString containerLabel(int sfFormat) {
    switch (sfFormat & SF_FORMAT_TYPEMASK) {
    case SF_FORMAT_WAV:
    case SF_FORMAT_WAVEX:
    case SF_FORMAT_RF64:
        return QStringLiteral("WAV");
    case SF_FORMAT_AIFF:
        return QStringLiteral("AIFF");
    case SF_FORMAT_AU:
        return QStringLiteral("AU");
    case SF_FORMAT_FLAC:
        return QStringLiteral("FLAC");
    case SF_FORMAT_OGG:
        return QStringLiteral("OGG");
    case SF_FORMAT_OPUS:
        return QStringLiteral("Opus");
    case SF_FORMAT_MPEG:
        return QStringLiteral("MP3");
    case SF_FORMAT_CAF:
        return QStringLiteral("CAF");
    case SF_FORMAT_W64:
        return QStringLiteral("W64");
    default:
        return QStringLiteral("Unknown");
    }
}

int bitDepth(int sfFormat) {
    switch (sfFormat & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_U8:
        return 8;
    case SF_FORMAT_PCM_16:
        return 16;
    case SF_FORMAT_PCM_24:
        return 24;
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_FLOAT:
        return 32;
    case SF_FORMAT_DOUBLE:
        return 64;
    default:
        return 0;
    }
}

} // namespace

AudioFileReader::~AudioFileReader() { close(); }

bool AudioFileReader::open(const QString &filePath) {
    close();

    SF_INFO info;
    std::memset(&info, 0, sizeof(info));

    m_sf = sf_open(filePath.toUtf8().constData(), SFM_READ, &info);
    if (!m_sf)
        return false;

    m_sampleRate = info.samplerate;
    m_channels   = info.channels;
    m_frameCount = static_cast<qint64>(info.frames);
    m_format     = info.format;
    return true;
}

void AudioFileReader::close() {
    if (m_sf) {
        sf_close(m_sf);
        m_sf = nullptr;
    }
    m_sampleRate = 0;
    m_channels   = 0;
    m_frameCount = 0;
    m_format     = 0;
}

QString AudioFileReader::formatLabel() const {
    const QString container = containerLabel(m_format);
    const int     depth     = bitDepth(m_format);
    return depth > 0 ? QStringLiteral("%1 (%2-bit)").arg(container).arg(depth) : container;
}

double AudioFileReader::durationSec() const {
    return m_sampleRate > 0 ? static_cast<double>(m_frameCount) / m_sampleRate : 0.0;
}

qint64 AudioFileReader::readFrames(float *interleaved, qint64 frames) {
    if (!m_sf || frames <= 0)
        return 0;
    const sf_count_t read = sf_readf_float(m_sf, interleaved, static_cast<sf_count_t>(frames));
    return read < 0 ? 0 : static_cast<qint64>(read);
}

} // namespace audio
