#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

namespace audio {

struct WaveformData
{
    QVector<float> mins;
    QVector<float> maxs;

    bool valid() const { return !mins.isEmpty() && !maxs.isEmpty(); }
};

} // namespace audio

Q_DECLARE_METATYPE(audio::WaveformData)