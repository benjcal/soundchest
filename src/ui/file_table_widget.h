#pragma once

#include <QWidget>

class QAbstractItemModel;
class QTableView;

namespace ui {

class FileTableWidget : public QWidget {
    Q_OBJECT

  public:
    explicit FileTableWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

  signals:
    void currentFileChanged(int row);
    void visibleRowsChanged(int firstRow, int lastRow);

  private:
    void emitVisibleRows();

    QTableView *m_table;
};

} // namespace ui
