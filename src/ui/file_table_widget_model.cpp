#include "file_table_widget_model.h"

#include "waveform/peaks_cache.h"

#include <QVariant>

namespace ui {

namespace {

QString formatDuration(double seconds) {
    if (seconds < 60.0)
        return QStringLiteral("%1 s").arg(seconds, 0, 'f', 2);

    const int totalSeconds = static_cast<int>(seconds);
    const int minutes      = totalSeconds / 60;
    const int secs         = totalSeconds % 60;
    if (minutes < 60)
        return QStringLiteral("%1:%2").arg(minutes).arg(secs, 2, 10, QLatin1Char('0'));

    const int hours = minutes / 60;
    return QStringLiteral("%1:%2:%3")
        .arg(hours)
        .arg(minutes % 60, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
}

QString formatChannels(int channels) {
    switch (channels) {
    case 1:
        return QStringLiteral("Mono");
    case 2:
        return QStringLiteral("Stereo");
    default:
        return QString::number(channels);
    }
}

} // namespace

FileTableWidgetModel::FileTableWidgetModel(QObject *parent) : QAbstractTableModel(parent) {}

void FileTableWidgetModel::setFiles(QVector<library::AudioFile> files) {
    beginResetModel();
    m_files = std::move(files);
    endResetModel();
}

void FileTableWidgetModel::setPeaksCache(waveform::PeaksCache *cache) {
    if (m_waveformCache == cache)
        return;

    if (m_waveformCache)
        disconnect(m_waveformCache, nullptr, this, nullptr);

    m_waveformCache = cache;

    if (m_waveformCache) {
        connect(m_waveformCache, &waveform::PeaksCache::ready, this, [this](const QString &filePath) {
            for (int row = 0; row < m_files.size(); ++row) {
                if (m_files.at(row).filePath != filePath)
                    continue;
                const QModelIndex changed = index(row, Waveform);
                emit              dataChanged(changed, changed, {WaveformRole});
            }
        });
    }
}

library::AudioFile FileTableWidgetModel::audioInfo(int row) const {
    if (row < 0 || row >= m_files.size())
        return {};
    return m_files.at(row);
}

int FileTableWidgetModel::rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_files.size(); }

int FileTableWidgetModel::columnCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : ColumnCount; }

QVariant FileTableWidgetModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_files.size())
        return {};

    const library::AudioFile &info = m_files.at(index.row());

    if (role == WaveformRole) {
        if (!m_waveformCache || info.filePath.isEmpty())
            return {};
        return QVariant::fromValue(m_waveformCache->get(info.filePath));
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Waveform:
            return {};
        case Name:
            return info.fileName;
        case Duration:
            return formatDuration(info.durationSec);
        case BitRate:
            return info.bitRateKbps > 0.0 ? QStringLiteral("%1 kbps").arg(qRound(info.bitRateKbps))
                                          : QStringLiteral("—");
        case Channels:
            return formatChannels(info.channels);
        case Format:
            return info.format;
        default:
            return {};
        }
    }

    if (role == Qt::ToolTipRole && index.column() == Name)
        return info.filePath;

    return {};
}

QVariant FileTableWidgetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case Waveform:
        return {};
    case Name:
        return QStringLiteral("Name");
    case Duration:
        return QStringLiteral("Duration");
    case BitRate:
        return QStringLiteral("Bit rate");
    case Channels:
        return QStringLiteral("Channels");
    case Format:
        return QStringLiteral("Format");
    default:
        return {};
    }
}

} // namespace ui
