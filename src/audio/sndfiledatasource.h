#pragma once

#include <QString>

#include <sndfile.h>

#include "miniaudio.h"

namespace audio {

class SndFileDataSource {
  public:
    SndFileDataSource() = default;
    ~SndFileDataSource();

    SndFileDataSource(const SndFileDataSource &)            = delete;
    SndFileDataSource &operator=(const SndFileDataSource &) = delete;

    bool open(const QString &filePath, QString *error = nullptr);
    void close();

    ma_data_source *dataSource() { return &m_base; }
    ma_uint32       channels() const { return m_channels; }
    ma_uint32       sampleRate() const { return m_sampleRate; }
    ma_uint64       lengthFrames() const { return m_lengthFrames; }

  private:
    static ma_result read(ma_data_source *pDataSource, void *pFramesOut, ma_uint64 frameCount, ma_uint64 *pFramesRead);
    static ma_result seek(ma_data_source *pDataSource, ma_uint64 frameIndex);
    static ma_result getDataFormat(ma_data_source *pDataSource, ma_format *pFormat, ma_uint32 *pChannels,
                                   ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap);
    static ma_result getCursor(ma_data_source *pDataSource, ma_uint64 *pCursor);
    static ma_result getLength(ma_data_source *pDataSource, ma_uint64 *pLength);

    static const ma_data_source_vtable s_vtable;

    ma_data_source_base m_base{};
    SNDFILE            *m_sf           = nullptr;
    bool                m_initialized  = false;
    ma_uint32           m_channels     = 0;
    ma_uint32           m_sampleRate   = 0;
    ma_uint64           m_lengthFrames = 0;
};

} // namespace audio
