#pragma once

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>

#include "library/audio_file.h"

class QMimeData;

namespace ui {

class FileTableWidgetModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    enum Column {
        Name,
        Duration,
        BitRate,
        Channels,
        Format,
        ColumnCount,
    };

    explicit FileTableWidgetModel(QObject *parent = nullptr);

    void               setFiles(QVector<library::AudioFile> files);
    library::AudioFile audioFile(int row) const;

    int      rowCount(const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags   flags(const QModelIndex &index) const override;
    QStringList     mimeTypes() const override;
    QMimeData      *mimeData(const QModelIndexList &indexes) const override;
    Qt::DropActions supportedDragActions() const override;

  private:
    QVector<library::AudioFile> m_files;
};

} // namespace ui
