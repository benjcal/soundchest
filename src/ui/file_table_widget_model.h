#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "library/audio_file.h"
#include "waveform/peaks.h"

namespace waveform {
class PeaksCache;
} // namespace waveform

namespace ui {

class FileTableWidgetModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    enum Column {
        Waveform,
        Name,
        Duration,
        BitRate,
        Channels,
        Format,
        ColumnCount,
    };

    enum {
        WaveformRole = Qt::UserRole + 1,
    };

    explicit FileTableWidgetModel(QObject *parent = nullptr);

    void             setFiles(QVector<library::AudioFile> files);
    void             setPeaksCache(waveform::PeaksCache *cache);
    library::AudioFile audioInfo(int row) const;

    int      rowCount(const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  private:
    QVector<library::AudioFile> m_files;
    waveform::PeaksCache    *m_waveformCache = nullptr;
};

} // namespace ui
