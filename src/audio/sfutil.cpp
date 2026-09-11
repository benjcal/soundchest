#include "sfutil.h"

namespace {

QString containerLabel(int sfFormat)
{
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

int bitDepth(int sfFormat)
{
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

namespace audio {

QString formatLabel(int sfFormat)
{
    const QString container = containerLabel(sfFormat);
    const int depth = bitDepth(sfFormat);
    return depth > 0 ? QStringLiteral("%1 (%2-bit)").arg(container).arg(depth) : container;
}

} // namespace audio