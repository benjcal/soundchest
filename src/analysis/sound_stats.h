#pragma once

namespace analysis {

// All levels are in decibels relative to full scale. `truePeakDbtp` is the
// oversampled inter-sample peak, `lufs` the EBU R128 integrated loudness.
struct SoundStats {
    double samplePeakDbfs = 0.0;
    double truePeakDbtp   = 0.0;
    double rmsDbfs        = 0.0;
    double lufs           = 0.0;
    bool   valid          = false;
};

} // namespace analysis
