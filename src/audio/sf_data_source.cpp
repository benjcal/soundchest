#include "sf_data_source.h"

#include <cstring>

namespace audio {

namespace {

ma_result sfDsRead(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount,
                   ma_uint64* pFramesRead)
{
    auto* ds = reinterpret_cast<SfDataSource*>(pDataSource);

    if (pFramesOut == nullptr) {
        if (sf_seek(ds->sf, static_cast<sf_count_t>(frameCount), SEEK_CUR) < 0)
            return MA_ERROR;
        *pFramesRead = frameCount;
        return MA_SUCCESS;
    }

    const sf_count_t read =
        sf_readf_float(ds->sf, static_cast<float*>(pFramesOut), static_cast<sf_count_t>(frameCount));
    if (read < 0)
        return MA_ERROR;
    *pFramesRead = static_cast<ma_uint64>(read);
    return MA_SUCCESS;
}

ma_result sfDsSeek(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    auto* ds = reinterpret_cast<SfDataSource*>(pDataSource);
    if (sf_seek(ds->sf, static_cast<sf_count_t>(frameIndex), SEEK_SET) < 0)
        return MA_ACCESS_DENIED;
    return MA_SUCCESS;
}

ma_result sfDsGetDataFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels,
                            ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
    Q_UNUSED(pChannelMap);
    Q_UNUSED(channelMapCap);

    auto* ds = reinterpret_cast<SfDataSource*>(pDataSource);
    if (pFormat)
        *pFormat = ma_format_f32;
    if (pChannels)
        *pChannels = ds->channels;
    if (pSampleRate)
        *pSampleRate = ds->sampleRate;
    return MA_SUCCESS;
}

ma_result sfDsGetCursor(ma_data_source* pDataSource, ma_uint64* pCursor)
{
    auto* ds = reinterpret_cast<SfDataSource*>(pDataSource);
    const sf_count_t pos = sf_seek(ds->sf, 0, SEEK_CUR);
    if (pos < 0)
        return MA_ERROR;
    *pCursor = static_cast<ma_uint64>(pos);
    return MA_SUCCESS;
}

ma_result sfDsGetLength(ma_data_source* pDataSource, ma_uint64* pLength)
{
    auto* ds = reinterpret_cast<SfDataSource*>(pDataSource);
    *pLength = ds->lengthFrames;
    return MA_SUCCESS;
}

ma_data_source_vtable g_sfDataSourceVtable = { sfDsRead, sfDsSeek, sfDsGetDataFormat, sfDsGetCursor,
                                               sfDsGetLength };

} // namespace

bool openSfDataSource(const QString& filePath, SfDataSource* out, QString* error)
{
    SF_INFO info;
    std::memset(&info, 0, sizeof(info));

    SNDFILE* sf = sf_open(filePath.toUtf8().constData(), SFM_READ, &info);
    if (!sf) {
        if (error)
            *error = QString::fromUtf8(sf_strerror(nullptr));
        return false;
    }

    ma_data_source_config config = ma_data_source_config_init();
    config.vtable = &g_sfDataSourceVtable;

    const ma_result result = ma_data_source_init(&config, &out->base);
    if (result != MA_SUCCESS) {
        sf_close(sf);
        if (error)
            *error = QString::fromUtf8(ma_result_description(result));
        return false;
    }

    out->sf = sf;
    out->channels = info.channels;
    out->sampleRate = info.samplerate;
    out->lengthFrames = static_cast<ma_uint64>(info.frames);
    return true;
}

void closeSfDataSource(SfDataSource* ds)
{
    if (!ds)
        return;
    if (ds->sf)
        sf_close(ds->sf);
    ds->sf = nullptr;
}

} // namespace audio