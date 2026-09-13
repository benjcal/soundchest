#include "peaks_builder.h"

#include "audio/audio_file_reader.h"

#include <QMetaObject>
#include <QRunnable>

#include <algorithm>
#include <utility>

namespace waveform {

namespace {

Peaks buildPeaks(const QString &filePath, int resolution, const std::atomic<int> *cancel, int generation) {
    Peaks data;
    if (resolution <= 0)
        return data;

    const auto cancelled = [cancel, generation] { return cancel && cancel->load() != generation; };

    audio::AudioFileReader reader;
    if (!reader.open(filePath))
        return data;

    data.mins.fill(0.0f, resolution);
    data.maxs.fill(0.0f, resolution);

    const int      channels  = reader.channels();
    const qint64   frames    = reader.frameCount();
    const qint64   frameStep = std::max<qint64>(1, frames / resolution);

    QVector<float> buffer(static_cast<qint64>(65536) * channels);
    qint64         frameIndex = 0;
    bool           anyData    = false;

    for (;;) {
        if (cancelled()) {
            data.mins.clear();
            data.maxs.clear();
            break;
        }

        const qint64 read = reader.readFrames(buffer.data(), 65536);
        if (read <= 0)
            break;

        for (qint64 f = 0; f < read; ++f) {
            float mono = 0.0f;
            for (int c = 0; c < channels; ++c)
                mono += buffer.at(f * channels + c);
            mono /= channels;

            const qint64 globalFrame = frameIndex + f;
            int          column      = static_cast<int>(globalFrame / frameStep);
            column                   = std::clamp(column, 0, resolution - 1);

            data.mins[column] = std::min(data.mins[column], mono);
            data.maxs[column] = std::max(data.maxs[column], mono);
            anyData           = true;
        }
        frameIndex += read;
    }

    if (!anyData || cancelled()) {
        data.mins.clear();
        data.maxs.clear();
        return data;
    }

    float peak = 0.0f;
    for (int i = 0; i < resolution; ++i) {
        peak = std::max(peak, std::abs(data.mins[i]));
        peak = std::max(peak, std::abs(data.maxs[i]));
    }
    if (peak > 0.0f) {
        for (int i = 0; i < resolution; ++i) {
            data.mins[i] /= peak;
            data.maxs[i] /= peak;
        }
    }

    return data;
}

class BuildTask : public QRunnable {
  public:
    BuildTask(PeaksBuilder *builder, QString filePath, int resolution, int generation,
              std::shared_ptr<std::atomic<int>> cancel)
        : m_builder(builder), m_filePath(std::move(filePath)), m_resolution(resolution), m_generation(generation),
          m_cancel(std::move(cancel)) {}

    void run() override {
        Peaks peaks = buildPeaks(m_filePath, m_resolution, m_cancel.get(), m_generation);
        QMetaObject::invokeMethod(
            m_builder,
            [builder = m_builder, filePath = m_filePath, peaks = std::move(peaks), generation = m_generation,
             resolution = m_resolution]() mutable {
                builder->storeResult(filePath, std::move(peaks), generation, resolution);
            },
            Qt::QueuedConnection);
    }

  private:
    PeaksBuilder                     *m_builder;
    QString                           m_filePath;
    int                               m_resolution;
    int                               m_generation;
    std::shared_ptr<std::atomic<int>> m_cancel;
};

} // namespace

PeaksBuilder::PeaksBuilder(QObject *parent) : QObject(parent), m_cancel(std::make_shared<std::atomic<int>>(0)) {
    m_pool.setMaxThreadCount(4);
}

PeaksBuilder::~PeaksBuilder() {
    ++(*m_cancel);
    m_pool.waitForDone();
}

Peaks PeaksBuilder::peaks(const QString &filePath) const { return m_cache.value(filePath); }

void PeaksBuilder::request(const QString &filePath, int resolution) {
    if (filePath.isEmpty() || resolution <= 0)
        return;

    const int cached = m_cachedResolution.value(filePath, 0);
    if (cached >= resolution)
        return;

    const int pending = m_pendingResolution.value(filePath, 0);
    if (pending >= resolution)
        return;

    m_pending.insert(filePath);
    m_pendingResolution.insert(filePath, resolution);

    const int  generation = m_cancel->load();
    const auto cancel     = m_cancel;
    m_pool.start(new BuildTask(this, filePath, resolution, generation, cancel));
}

void PeaksBuilder::clear() {
    ++(*m_cancel);
    m_cache.clear();
    m_cachedResolution.clear();
    m_pending.clear();
    m_pendingResolution.clear();
}

void PeaksBuilder::storeResult(const QString &filePath, Peaks peaks, int generation, int resolution) {
    m_pending.remove(filePath);

    if (generation != m_cancel->load())
        return;

    if (resolution < m_pendingResolution.value(filePath, 0))
        return;

    m_pendingResolution.remove(filePath);

    if (!peaks.valid())
        return;

    m_cache.insert(filePath, std::move(peaks));
    m_cachedResolution.insert(filePath, resolution);
    emit ready(filePath);
}

} // namespace waveform
