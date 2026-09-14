#pragma once

#include <QVector>
#include <QWidget>

#include "library/audio_file.h"

class QAbstractItemModel;
class QItemSelectionModel;
class QTableView;

namespace ui {

class FileTableWidget : public QWidget {
    Q_OBJECT

  public:
    explicit FileTableWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    void setCurrentRow(int row);

    QVector<library::AudioFile> selectedFiles() const;

  signals:
    void currentFileChanged(int row);
    void fileActivated(int row);
    void playPauseRequested();
    void exportRequested();

  private:
    QTableView          *m_table;
    QItemSelectionModel *m_selection = nullptr;
};

} // namespace ui
