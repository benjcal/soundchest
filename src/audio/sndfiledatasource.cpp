#include "sndfiledatasource.h"

#include <cstring>

namespace audio {

namespace {

SndFileDataSource *self(ma_data_source *pDataSource) { return reinterpret_cast<SndFileDataSource *>(pDataSource); }

} // namespace

ma_result SndFileDataSource::read(ma_data_source *pDataSource, void *pFramesOut, ma_uint64 frameCount,
                                  ma_uint64 *pFramesRead) {
    auto *ds = self(pDataSource);

    if (pFramesOut == nullptr) {
        if (sf_seek(ds->m_sf, static_cast<sf_count_t>(frameCount), SEEK_CUR) < 0)
            return MA_ERROR;
        *pFramesRead = frameCount;
        return MA_SUCCESS;
    }

    const sf_count_t read =
        sf_readf_float(ds->m_sf, static_cast<float *>(pFramesOut), static_cast<sf_count_t>(frameCount));
    if (read < 0)
        return MA_ERROR;
    *pFramesRead = static_cast<ma_uint64>(read);
    return MA_SUCCESS;
}

ma_result SndFileDataSource::seek(ma_data_source *pDataSource, ma_uint64 frameIndex) {
    auto *ds = self(pDataSource);
    if (sf_seek(ds->m_sf, static_cast<sf_count_t>(frameIndex), SEEK_SET) < 0)
        return MA_ACCESS_DENIED;
    return MA_SUCCESS;
}

ma_result SndFileDataSource::getDataFormat(ma_data_source *pDataSource, ma_format *pFormat, ma_uint32 *pChannels,
                                           ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap) {
    Q_UNUSED(pChannelMap);
    Q_UNUSED(channelMapCap);

    auto *ds = self(pDataSource);
    if (pFormat)
        *pFormat = ma_format_f32;
    if (pChannels)
        *pChannels = ds->m_channels;
    if (pSampleRate)
        *pSampleRate = ds->m_sampleRate;
    return MA_SUCCESS;
}

ma_result SndFileDataSource::getCursor(ma_data_source *pDataSource, ma_uint64 *pCursor) {
    auto            *ds  = self(pDataSource);
    const sf_count_t pos = sf_seek(ds->m_sf, 0, SEEK_CUR);
    if (pos < 0)
        return MA_ERROR;
    *pCursor = static_cast<ma_uint64>(pos);
    return MA_SUCCESS;
}

ma_result SndFileDataSource::getLength(ma_data_source *pDataSource, ma_uint64 *pLength) {
    auto *ds = self(pDataSource);
    *pLength = ds->m_lengthFrames;
    return MA_SUCCESS;
}

const ma_data_source_vtable SndFileDataSource::s_vtable = {
    &SndFileDataSource::read,      &SndFileDataSource::seek,      &SndFileDataSource::getDataFormat,
    &SndFileDataSource::getCursor, &SndFileDataSource::getLength,
};

SndFileDataSource::~SndFileDataSource() { close(); }

bool SndFileDataSource::open(const QString &filePath, QString *error) {
    close();

    SF_INFO info;
    std::memset(&info, 0, sizeof(info));

    SNDFILE *sf = sf_open(filePath.toUtf8().constData(), SFM_READ, &info);
    if (!sf) {
        if (error)
            *error = QString::fromUtf8(sf_strerror(nullptr));
        return false;
    }

    ma_data_source_config config = ma_data_source_config_init();
    config.vtable                = &s_vtable;

    const ma_result result = ma_data_source_init(&config, &m_base);
    if (result != MA_SUCCESS) {
        sf_close(sf);
        if (error)
            *error = QString::fromUtf8(ma_result_description(result));
        return false;
    }

    m_sf           = sf;
    m_initialized  = true;
    m_channels     = info.channels;
    m_sampleRate   = info.samplerate;
    m_lengthFrames = static_cast<ma_uint64>(info.frames);
    return true;
}

void SndFileDataSource::close() {
    if (m_sf) {
        sf_close(m_sf);
        m_sf = nullptr;
    }
    if (m_initialized) {
        ma_data_source_uninit(&m_base);
        m_initialized = false;
    }
    m_channels     = 0;
    m_sampleRate   = 0;
    m_lengthFrames = 0;
}

} // namespace audio
