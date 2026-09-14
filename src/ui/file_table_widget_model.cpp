#include "file_table_widget_model.h"

#include "format.h"

#include <QMimeData>
#include <QSet>
#include <QUrl>
#include <QVariant>

#include <algorithm>

namespace ui {

FileTableWidgetModel::FileTableWidgetModel(QObject *parent) : QAbstractTableModel(parent) {}

void FileTableWidgetModel::setFiles(QVector<library::AudioFile> files) {
    beginResetModel();
    m_files = std::move(files);
    endResetModel();
}

library::AudioFile FileTableWidgetModel::audioFile(int row) const {
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

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
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

Qt::ItemFlags FileTableWidgetModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QStringList FileTableWidgetModel::mimeTypes() const { return {QStringLiteral("text/uri-list")}; }

QMimeData *FileTableWidgetModel::mimeData(const QModelIndexList &indexes) const {
    QSet<int> rows;
    for (const QModelIndex &index : indexes) {
        if (index.isValid())
            rows.insert(index.row());
    }

    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end());

    QList<QUrl> urls;
    for (const int row : sortedRows) {
        if (row >= 0 && row < m_files.size())
            urls.append(QUrl::fromLocalFile(m_files.at(row).filePath));
    }

    if (urls.isEmpty())
        return nullptr;

    auto *data = new QMimeData;
    data->setUrls(urls);
    return data;
}

Qt::DropActions FileTableWidgetModel::supportedDragActions() const { return Qt::CopyAction; }

QVariant FileTableWidgetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
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
