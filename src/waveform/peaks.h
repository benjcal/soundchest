#pragma once

#include <QVector>

namespace waveform {

struct Peaks {
    QVector<float> mins;
    QVector<float> maxs;

    bool valid() const { return !mins.isEmpty() && !maxs.isEmpty(); }
};

} // namespace waveform
