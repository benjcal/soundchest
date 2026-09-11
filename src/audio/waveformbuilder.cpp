#include "waveformbuilder.h"

#include <QtConcurrent>
#include <QFileInfo>

#include <sndfile.h>

#include <algorithm>
#include <cstring>

namespace audio {

WaveformData buildWaveform(const QString& filePath, int columnCount,
                           const std::atomic<int>* cancel, int generation)
{
    WaveformData data;
    if (columnCount <= 0)
        return data;

    const auto cancelled = [cancel, generation] {
        return cancel && cancel->load() != generation;
    };

    SF_INFO info;
    std::memset(&info, 0, sizeof(info));

    SNDFILE* sf = sf_open(filePath.toUtf8().constData(), SFM_READ, &info);
    if (!sf)
        return data;

    data.mins.fill(0.0f, columnCount);
    data.maxs.fill(0.0f, columnCount);

    const int channels = info.channels;
    const sf_count_t frameStep = std::max<sf_count_t>(1, info.frames / columnCount);

    QVector<float> buffer(static_cast<qint64>(65536) * channels);
    sf_count_t frameIndex = 0;
    bool anyData = false;

    for (;;) {
        if (cancelled()) {
            data.mins.clear();
            data.maxs.clear();
            break;
        }

        const sf_count_t read =
            sf_readf_float(sf, buffer.data(), 65536);
        if (read <= 0)
            break;

        for (sf_count_t f = 0; f < read; ++f) {
            float mono = 0.0f;
            for (int c = 0; c < channels; ++c)
                mono += buffer.at(static_cast<qint64>(f) * channels + c);
            mono /= channels;

            const sf_count_t globalFrame = frameIndex + f;
            int column = static_cast<int>(globalFrame / frameStep);
            column = std::clamp(column, 0, columnCount - 1);

            data.mins[column] = std::min(data.mins[column], mono);
            data.maxs[column] = std::max(data.maxs[column], mono);
            anyData = true;
        }
        frameIndex += read;
    }

    sf_close(sf);

    if (!anyData || cancelled()) {
        data.mins.clear();
        data.maxs.clear();
        return data;
    }

    float peak = 0.0f;
    for (int i = 0; i < columnCount; ++i) {
        peak = std::max(peak, std::abs(data.mins[i]));
        peak = std::max(peak, std::abs(data.maxs[i]));
    }
    if (peak > 0.0f) {
        for (int i = 0; i < columnCount; ++i) {
            data.mins[i] /= peak;
            data.maxs[i] /= peak;
        }
    }

    return data;
}

WaveformBuilder::WaveformBuilder(QObject* parent)
    : QObject(parent)
    , m_cancel(std::make_shared<std::atomic<int>>(0))
    , m_watcher(new QFutureWatcher<WaveformData>(this))
{
    connect(m_watcher, &QFutureWatcher<WaveformData>::finished, this,
            &WaveformBuilder::onFinished);
}

void WaveformBuilder::request(const QString& filePath, int columnCount)
{
    if (columnCount <= 0)
        return;

    const int generation = ++(*m_cancel);
    m_requestedPath = filePath;

    const auto cancel = m_cancel;
    m_future = QtConcurrent::run([filePath, columnCount, cancel, generation] {
        return buildWaveform(filePath, columnCount, cancel.get(), generation);
    });
    m_watcher->setFuture(m_future);
}

void WaveformBuilder::cancel()
{
    ++(*m_cancel);
}

void WaveformBuilder::onFinished()
{
    if (!m_future.isFinished())
        return;
    emit waveformReady(m_requestedPath, m_future.result());
}

} // namespace audio