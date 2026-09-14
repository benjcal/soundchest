#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QThreadPool>

#include <atomic>
#include <memory>

#include "peaks.h"

namespace waveform {

class PeaksBuilder : public QObject {
    Q_OBJECT

  public:
    explicit PeaksBuilder(QObject *parent = nullptr);
    ~PeaksBuilder() override;

    // Returns cached peaks. May be coarser than requested.
    Peaks peaks(const QString &filePath) const;

    // Builds peaks with the requested number of columns unless the file
    // already has an equal or larger column count cached or pending.
    void request(const QString &filePath, int columns);

    void clear();

  signals:
    void ready(const QString &filePath);

  private:
    friend class BuildTask;

    // Worker protocol: a build captures the current generation number when it
    // is requested and is compared against it again when it finishes. clear()
    // and the destructor bump the generation, marking every in-flight build
    // stale. Workers post back through storeResult() on the GUI thread; stale
    // results are dropped, as are results no finer than what is already cached.
    void storeResult(const QString &filePath, Peaks peaks, int buildGeneration, int columns);

    QHash<QString, Peaks>             m_cache;
    QHash<QString, int>               m_cachedColumns;
    QHash<QString, int>               m_pendingColumns;
    QThreadPool                       m_pool;
    std::shared_ptr<std::atomic<int>> m_generation;
};

} // namespace waveform
