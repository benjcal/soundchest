#include "peaks_cache.h"

#include "builder.h"

#include <QMetaObject>
#include <QRunnable>

#include <utility>

namespace waveform {

namespace {

constexpr int previewColumns = 64;

class BuildTask : public QRunnable {
  public:
    BuildTask(PeaksCache *cache, QString filePath, int columnCount, int generation,
              std::shared_ptr<std::atomic<int>> cancel)
        : m_cache(cache), m_filePath(std::move(filePath)), m_columnCount(columnCount), m_generation(generation),
          m_cancel(std::move(cancel)) {}

    void run() override {
        Peaks data = buildPeaks(m_filePath, m_columnCount, m_cancel.get(), m_generation);
        QMetaObject::invokeMethod(
            m_cache,
            [cache = m_cache, filePath = m_filePath, data = std::move(data), generation = m_generation]() mutable {
                cache->storeResult(filePath, std::move(data), generation);
            },
            Qt::QueuedConnection);
    }

  private:
    PeaksCache                    *m_cache;
    QString                           m_filePath;
    int                               m_columnCount;
    int                               m_generation;
    std::shared_ptr<std::atomic<int>> m_cancel;
};

} // namespace

PeaksCache::PeaksCache(QObject *parent) : QObject(parent), m_cancel(std::make_shared<std::atomic<int>>(0)) {
    m_pool.setMaxThreadCount(4);
}

PeaksCache::~PeaksCache() {
    ++(*m_cancel);
    m_pool.waitForDone();
}

Peaks PeaksCache::get(const QString &filePath) const { return m_cache.value(filePath); }

void PeaksCache::request(const QString &filePath) {
    if (filePath.isEmpty() || m_cache.contains(filePath) || m_pending.contains(filePath))
        return;

    m_pending.insert(filePath);
    m_pool.start(new BuildTask(this, filePath, previewColumns, m_cancel->load(), m_cancel));
}

void PeaksCache::clear() {
    ++(*m_cancel);
    m_cache.clear();
    m_pending.clear();
}

void PeaksCache::storeResult(const QString &filePath, Peaks data, int generation) {
    m_pending.remove(filePath);

    if (generation != m_cancel->load())
        return;

    if (!data.valid())
        return;

    m_cache.insert(filePath, std::move(data));
    emit ready(filePath);
}

} // namespace waveform
