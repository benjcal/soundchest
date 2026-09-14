#include "peaks_builder.h"

#include "audio/audio_file_reader.h"

#include <QMetaObject>
#include <QRunnable>

#include <algorithm>
#include <utility>

namespace waveform {

namespace {

constexpr qint64 kReadFrames = 65536;
constexpr int    kMaxThreads = 4;

Peaks buildPeaks(const QString &filePath, int columns, const std::atomic<int> *generation, int buildGeneration) {
    Peaks data;
    if (columns <= 0)
        return data;

    const auto isStale = [generation, buildGeneration] { return generation && generation->load() != buildGeneration; };

    audio::AudioFileReader reader;
    if (!reader.open(filePath))
        return data;

    data.mins.fill(0.0f, columns);
    data.maxs.fill(0.0f, columns);

    const int    channels = reader.channels();
    const qint64 frames   = reader.frameCount();

    QVector<float> buffer(kReadFrames * channels);
    qint64         frameIndex = 0;
    bool           anyData    = false;

    for (;;) {
        if (isStale()) {
            data.mins.clear();
            data.maxs.clear();
            break;
        }

        const qint64 read = reader.readFrames(buffer.data(), kReadFrames);
        if (read <= 0)
            break;

        for (qint64 f = 0; f < read; ++f) {
            float mono = 0.0f;
            for (int c = 0; c < channels; ++c)
                mono += buffer.at(f * channels + c);
            mono /= channels;

            const qint64 globalFrame = frameIndex + f;
            // Spread frames evenly across columns; a fixed frameStep leaves a
            // zero (flat) tail when there are fewer frames than columns.
            int column = frames > 0 ? static_cast<int>(globalFrame * columns / frames) : 0;
            column     = std::clamp(column, 0, columns - 1);

            data.mins[column] = std::min(data.mins[column], mono);
            data.maxs[column] = std::max(data.maxs[column], mono);
            anyData           = true;
        }
        frameIndex += read;
    }

    if (!anyData || isStale()) {
        data.mins.clear();
        data.maxs.clear();
        return data;
    }

    float peak = 0.0f;
    for (int i = 0; i < columns; ++i) {
        peak = std::max(peak, std::abs(data.mins[i]));
        peak = std::max(peak, std::abs(data.maxs[i]));
    }
    if (peak > 0.0f) {
        for (int i = 0; i < columns; ++i) {
            data.mins[i] /= peak;
            data.maxs[i] /= peak;
        }
    }

    return data;
}

} // namespace

class BuildTask : public QRunnable {
  public:
    BuildTask(PeaksBuilder *builder, QString filePath, int columns, int buildGeneration,
              std::shared_ptr<std::atomic<int>> generation)
        : m_builder(builder), m_filePath(std::move(filePath)), m_columns(columns), m_buildGeneration(buildGeneration),
          m_generation(std::move(generation)) {}

    void run() override {
        Peaks peaks = buildPeaks(m_filePath, m_columns, m_generation.get(), m_buildGeneration);
        QMetaObject::invokeMethod(
            m_builder,
            [builder = m_builder, filePath = m_filePath, peaks = std::move(peaks), generation = m_buildGeneration,
             columns = m_columns]() mutable { builder->storeResult(filePath, std::move(peaks), generation, columns); },
            Qt::QueuedConnection);
    }

  private:
    PeaksBuilder                     *m_builder;
    QString                           m_filePath;
    int                               m_columns;
    int                               m_buildGeneration;
    std::shared_ptr<std::atomic<int>> m_generation;
};

PeaksBuilder::PeaksBuilder(QObject *parent) : QObject(parent), m_generation(std::make_shared<std::atomic<int>>(0)) {
    m_pool.setMaxThreadCount(kMaxThreads);
}

PeaksBuilder::~PeaksBuilder() {
    ++(*m_generation);
    m_pool.waitForDone();
}

Peaks PeaksBuilder::peaks(const QString &filePath) const { return m_cache.value(filePath); }

void PeaksBuilder::request(const QString &filePath, int columns) {
    if (filePath.isEmpty() || columns <= 0)
        return;

    const int cached = m_cachedColumns.value(filePath, 0);
    if (cached >= columns)
        return;

    const int pending = m_pendingColumns.value(filePath, 0);
    if (pending >= columns)
        return;

    m_pendingColumns.insert(filePath, columns);

    const int  buildGeneration = m_generation->load();
    const auto generation      = m_generation;
    m_pool.start(new BuildTask(this, filePath, columns, buildGeneration, generation));
}

void PeaksBuilder::clear() {
    ++(*m_generation);
    m_cache.clear();
    m_cachedColumns.clear();
    m_pendingColumns.clear();
}

void PeaksBuilder::storeResult(const QString &filePath, Peaks peaks, int buildGeneration, int columns) {
    if (buildGeneration != m_generation->load())
        return;

    // A finer build is already pending: let its result own the pending slot,
    // so this coarser result is dropped instead of replacing it.
    if (columns < m_pendingColumns.value(filePath, 0))
        return;

    if (columns <= m_cachedColumns.value(filePath, 0))
        return;

    m_pendingColumns.remove(filePath);

    if (!peaks.valid())
        return;

    m_cache.insert(filePath, std::move(peaks));
    m_cachedColumns.insert(filePath, columns);
    emit ready(filePath);
}

} // namespace waveform
