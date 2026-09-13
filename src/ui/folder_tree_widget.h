#pragma once

#include <QWidget>

class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;
class QTreeView;

namespace ui {

class FolderTreeWidget : public QWidget {
    Q_OBJECT

  public:
    explicit FolderTreeWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    void setCurrentIndex(const QModelIndex &index);
    void expand(const QModelIndex &index);

  signals:
    void currentDirectoryChanged(const QModelIndex &index);

  private:
    QTreeView           *m_tree;
    QItemSelectionModel *m_selection = nullptr;
};

} // namespace ui
