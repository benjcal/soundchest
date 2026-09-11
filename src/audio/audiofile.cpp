#include "audiofile.h"

#include "sfutil.h"

#include <QFileInfo>
#include <cstring>

namespace audio {

bool readInfo(const QString& filePath, AudioInfo* out, QString* error)
{
    SF_INFO info;
    std::memset(&info, 0, sizeof(info));

    SNDFILE* sf = sf_open(filePath.toUtf8().constData(), SFM_READ, &info);
    if (!sf) {
        if (error)
            *error = QString::fromUtf8(sf_strerror(nullptr));
        return false;
    }
    sf_close(sf);

    if (out) {
        out->fileName = QFileInfo(filePath).fileName();
        out->filePath = filePath;
        out->durationSec =
            info.samplerate > 0 ? static_cast<double>(info.frames) / info.samplerate : 0.0;
        out->sampleRate = info.samplerate;
        out->channels = info.channels;
        out->format = formatLabel(info.format);
        out->fileSizeBytes = QFileInfo(filePath).size();
        out->bitRateKbps = out->durationSec > 0.0
                               ? out->fileSizeBytes * 8.0 / out->durationSec / 1000.0
                               : 0.0;
    }

    return true;
}

} // namespace audio