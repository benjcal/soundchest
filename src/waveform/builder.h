#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

#include "peaks.h"

namespace waveform {

Peaks buildPeaks(const QString &filePath, int columnCount, const std::atomic<int> *cancel = nullptr,
                           int generation = 0);

class PeaksBuilder : public QObject {
    Q_OBJECT

  public:
    explicit PeaksBuilder(QObject *parent = nullptr);

    void request(const QString &filePath, int columnCount);
    void cancel();

  signals:
    void peaksReady(const QString &filePath, Peaks data);

  private:
    void onFinished();

    std::shared_ptr<std::atomic<int>> m_cancel;
    QFuture<Peaks>             m_future;
    QFutureWatcher<Peaks>     *m_watcher;
    QString                           m_requestedPath;
};

} // namespace waveform
