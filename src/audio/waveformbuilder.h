#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

#include "audio/waveform.h"

namespace audio {

WaveformData buildWaveform(const QString& filePath, int columnCount,
                           const std::atomic<int>* cancel = nullptr,
                           int generation = 0);

class WaveformBuilder : public QObject
{
    Q_OBJECT

public:
    explicit WaveformBuilder(QObject* parent = nullptr);

    void request(const QString& filePath, int columnCount);
    void cancel();

signals:
    void waveformReady(const QString& filePath, WaveformData data);

private:
    void onFinished();

    std::shared_ptr<std::atomic<int>> m_cancel;
    QFuture<WaveformData> m_future;
    QFutureWatcher<WaveformData>* m_watcher;
    QString m_requestedPath;
};

} // namespace audio