#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
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

    // Builds peaks at the given resolution unless an equal or finer
    // resolution for this file is already cached or pending.
    void request(const QString &filePath, int resolution);

    void clear();

    void storeResult(const QString &filePath, Peaks peaks, int generation, int resolution);

  signals:
    void ready(const QString &filePath);

  private:
    QHash<QString, Peaks>             m_cache;
    QHash<QString, int>               m_cachedResolution;
    QHash<QString, int>               m_pendingResolution;
    QSet<QString>                     m_pending;
    QThreadPool                       m_pool;
    std::shared_ptr<std::atomic<int>> m_cancel;
};

} // namespace waveform
