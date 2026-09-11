#pragma once

#include <QString>

namespace audio {

struct AudioInfo
{
    QString fileName;
    QString filePath;
    double durationSec = 0.0;
    int sampleRate = 0;
    int channels = 0;
    QString format;
    qint64 fileSizeBytes = 0;
    double bitRateKbps = 0.0;
};

bool readInfo(const QString& filePath, AudioInfo* out, QString* error = nullptr);

} // namespace audio