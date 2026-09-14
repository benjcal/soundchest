#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QThreadPool>

#include <atomic>
#include <memory>

#include "sound_stats.h"

namespace analysis {

class SoundAnalyzer : public QObject {
    Q_OBJECT

  public:
    explicit SoundAnalyzer(QObject *parent = nullptr);
    ~SoundAnalyzer() override;

    // Returns cached stats; check SoundStats::valid before using them.
    SoundStats stats(const QString &filePath) const;
    bool       hasStats(const QString &filePath) const;

    // Starts an analysis pass unless the file is already cached or pending.
    // Failures are cached as invalid stats, so they are analyzed only once.
    void request(const QString &filePath);

  signals:
    void ready(const QString &filePath);

  private:
    friend class AnalyzeTask;

    // Worker protocol: an analysis captures the generation number when it is
    // requested and is compared against it again when it finishes. clear() and
    // the destructor bump the generation, so in-flight work becomes stale and
    // its result is dropped.
    void storeResult(const QString &filePath, SoundStats stats, int buildGeneration);

    QHash<QString, SoundStats>        m_cache;
    QSet<QString>                     m_pending;
    QThreadPool                       m_pool;
    std::shared_ptr<std::atomic<int>> m_generation;
};

} // namespace analysis
