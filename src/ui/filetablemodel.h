#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "audio/audioinfo.h"
#include "waveform/waveform.h"

namespace waveform {
class WaveformCache;
} // namespace waveform

namespace ui {

class FileTableModel : public QAbstractTableModel {
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

    explicit FileTableModel(QObject *parent = nullptr);

    void             setFiles(QVector<audio::AudioInfo> files);
    void             setWaveformCache(waveform::WaveformCache *cache);
    audio::AudioInfo audioInfo(int row) const;

    int      rowCount(const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  private:
    QVector<audio::AudioInfo> m_files;
    waveform::WaveformCache  *m_waveformCache = nullptr;
};

} // namespace ui
