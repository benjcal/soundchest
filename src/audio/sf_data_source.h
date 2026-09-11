#pragma once

#include <sndfile.h>

#include <QString>

#include "miniaudio.h"

namespace audio {

struct SfDataSource
{
    ma_data_source_base base;
    SNDFILE* sf;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    ma_uint64 lengthFrames;
};

bool openSfDataSource(const QString& filePath, SfDataSource* out, QString* error = nullptr);
void closeSfDataSource(SfDataSource* ds);

} // namespace audio