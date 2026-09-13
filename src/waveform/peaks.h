#pragma once

#include <QMetaType>
#include <QVector>

namespace waveform {

struct Peaks {
    QVector<float> mins;
    QVector<float> maxs;

    bool valid() const { return !mins.isEmpty() && !maxs.isEmpty(); }
};

} // namespace waveform

Q_DECLARE_METATYPE(waveform::Peaks)
