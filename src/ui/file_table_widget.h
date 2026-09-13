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
    void setCurrentRow(int row);

  signals:
    void currentFileChanged(int row);

  private:
    QTableView *m_table;
};

} // namespace ui
