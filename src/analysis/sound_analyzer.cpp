#include "sound_analyzer.h"

#include "audio/audio_file_reader.h"

#include <QMetaObject>
#include <QRunnable>

#include <algorithm>
#include <cmath>
#include <ebur128.h>
#include <limits>

namespace analysis {

namespace {

constexpr int kMaxThreads = 2;

// ebur128 needs explicit channel mapping for anything but plain stereo.
void mapChannels(ebur128_state *state, int channels) {
    if (channels == 1) {
        ebur128_set_channel(state, 0, EBUR128_CENTER);
        return;
    }

    ebur128_set_channel(state, 0, EBUR128_LEFT);
    ebur128_set_channel(state, 1, EBUR128_RIGHT);
    for (int c = 2; c < channels; ++c)
        ebur128_set_channel(state, static_cast<unsigned int>(c), c == 2 ? EBUR128_CENTER : EBUR128_UNUSED);
}

double integratedLufs(ebur128_state *state) {
    double lufs = 0.0;
    if (ebur128_loudness_global(state, &lufs) != EBUR128_SUCCESS)
        return -std::numeric_limits<double>::infinity();
    return lufs;
}

double loudestTruePeak(ebur128_state *state, int channels) {
    double peak = 0.0;
    for (int c = 0; c < channels; ++c) {
        double channelPeak = 0.0;
        if (ebur128_true_peak(state, static_cast<unsigned int>(c), &channelPeak) == EBUR128_SUCCESS)
            peak = std::max(peak, channelPeak);
    }
    return peak;
}

SoundStats analyzeFile(const QString &filePath, const std::atomic<int> *generation, int buildGeneration) {
    SoundStats stats;

    const auto isStale = [generation, buildGeneration] { return generation && generation->load() != buildGeneration; };

    audio::AudioFileReader reader;
    if (!reader.open(filePath))
        return stats;

    const int channels   = reader.channels();
    const int sampleRate = reader.sampleRate();
    if (channels <= 0 || sampleRate <= 0)
        return stats;

    ebur128_state *state = ebur128_init(static_cast<unsigned int>(channels), static_cast<unsigned long>(sampleRate),
                                        EBUR128_MODE_I | EBUR128_MODE_TRUE_PEAK);
    if (!state)
        return stats;

    mapChannels(state, channels);

    constexpr qint64 kBlockFrames = 32768;

    QVector<float> buffer(kBlockFrames * channels);
    double         sumOfSquares = 0.0;
    qint64         sampleCount  = 0;
    double         samplePeak   = 0.0;
    bool           anyData      = false;

    for (;;) {
        if (isStale())
            break;

        const qint64 read = reader.readFrames(buffer.data(), kBlockFrames);
        if (read <= 0)
            break;

        anyData = true;

        if (ebur128_add_frames_float(state, buffer.constData(), static_cast<size_t>(read)) != EBUR128_SUCCESS) {
            anyData = false;
            break;
        }

        const qint64 samples = read * channels;
        for (qint64 i = 0; i < samples; ++i) {
            const double sample = buffer.at(i);
            samplePeak          = std::max(samplePeak, std::abs(sample));
            sumOfSquares += sample * sample;
        }
        sampleCount += samples;
    }

    if (!anyData || isStale() || sampleCount == 0) {
        ebur128_destroy(&state);
        return stats;
    }

    const double lufs     = integratedLufs(state);
    const double truePeak = loudestTruePeak(state, channels);
    ebur128_destroy(&state);

    if (samplePeak <= 0.0 || !std::isfinite(lufs))
        return stats;

    stats.samplePeakDbfs = 20.0 * std::log10(samplePeak);
    stats.rmsDbfs        = 20.0 * std::log10(std::sqrt(sumOfSquares / static_cast<double>(sampleCount)));
    // ebur128 reports a zero true peak only when it could not measure one;
    // fall back to the sample peak so the row is never blank.
    stats.truePeakDbtp = truePeak > 0.0 ? 20.0 * std::log10(truePeak) : stats.samplePeakDbfs;
    stats.lufs         = lufs;
    stats.valid        = true;
    return stats;
}

} // namespace

class AnalyzeTask : public QRunnable {
  public:
    AnalyzeTask(SoundAnalyzer *analyzer, QString filePath, int buildGeneration,
                std::shared_ptr<std::atomic<int>> generation)
        : m_analyzer(analyzer), m_filePath(std::move(filePath)), m_buildGeneration(buildGeneration),
          m_generation(std::move(generation)) {}

    void run() override {
        SoundStats stats = analyzeFile(m_filePath, m_generation.get(), m_buildGeneration);
        QMetaObject::invokeMethod(
            m_analyzer,
            [analyzer = m_analyzer, filePath = m_filePath, stats = std::move(stats),
             buildGeneration = m_buildGeneration]() mutable {
                analyzer->storeResult(filePath, std::move(stats), buildGeneration);
            },
            Qt::QueuedConnection);
    }

  private:
    SoundAnalyzer                    *m_analyzer;
    QString                           m_filePath;
    int                               m_buildGeneration;
    std::shared_ptr<std::atomic<int>> m_generation;
};

SoundAnalyzer::SoundAnalyzer(QObject *parent) : QObject(parent), m_generation(std::make_shared<std::atomic<int>>(0)) {
    m_pool.setMaxThreadCount(kMaxThreads);
}

SoundAnalyzer::~SoundAnalyzer() {
    ++(*m_generation);
    m_pool.waitForDone();
}

SoundStats SoundAnalyzer::stats(const QString &filePath) const { return m_cache.value(filePath); }

bool SoundAnalyzer::hasStats(const QString &filePath) const { return m_cache.contains(filePath); }

void SoundAnalyzer::request(const QString &filePath) {
    if (filePath.isEmpty() || m_cache.contains(filePath) || m_pending.contains(filePath))
        return;

    m_pending.insert(filePath);

    const int  buildGeneration = m_generation->load();
    const auto generation      = m_generation;
    m_pool.start(new AnalyzeTask(this, filePath, buildGeneration, generation));
}

void SoundAnalyzer::storeResult(const QString &filePath, SoundStats stats, int buildGeneration) {
    if (buildGeneration != m_generation->load())
        return;

    m_pending.remove(filePath);

    // Cache failures too: invalid stats mean the file is silent or unreadable,
    // and re-running the analysis on every selection would be wasted work.
    m_cache.insert(filePath, stats);

    if (stats.valid)
        emit ready(filePath);
}

} // namespace analysis
