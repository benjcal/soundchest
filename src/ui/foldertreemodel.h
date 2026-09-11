#pragma once

#include <QAbstractItemModel>

#include <memory>

#include "catalog/foldertree.h"

namespace ui {

class FolderTreeModel : public QAbstractItemModel {
    Q_OBJECT

  public:
    explicit FolderTreeModel(QObject *parent = nullptr);

    void                             setTree(std::shared_ptr<const catalog::FolderTree> tree);
    const QVector<audio::AudioInfo> *filesFor(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int         rowCount(const QModelIndex &parent = {}) const override;
    int         columnCount(const QModelIndex &parent = {}) const override;
    QVariant    data(const QModelIndex &index, int role) const override;

  private:
    catalog::DirectoryNode *nodeFor(const QModelIndex &index) const;

    std::shared_ptr<const catalog::FolderTree> m_tree;
};

} // namespace ui
