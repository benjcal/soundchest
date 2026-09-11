#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QThreadPool>

#include <atomic>
#include <memory>

#include "waveform.h"

namespace waveform {

class WaveformCache : public QObject {
    Q_OBJECT

  public:
    explicit WaveformCache(QObject *parent = nullptr);
    ~WaveformCache() override;

    WaveformData get(const QString &filePath) const;

    void request(const QString &filePath);
    void clear();

    void storeResult(const QString &filePath, WaveformData data, int generation);

  signals:
    void ready(const QString &filePath);

  private:
    QHash<QString, WaveformData>      m_cache;
    QSet<QString>                     m_pending;
    QThreadPool                       m_pool;
    std::shared_ptr<std::atomic<int>> m_cancel;
};

} // namespace waveform
