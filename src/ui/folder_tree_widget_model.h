#pragma once

#include <QAbstractItemModel>

#include <memory>

#include "library/folder.h"

namespace ui {

class FolderTreeWidgetModel : public QAbstractItemModel {
    Q_OBJECT

  public:
    explicit FolderTreeWidgetModel(QObject *parent = nullptr);

    void                             setFolder(std::shared_ptr<const library::Folder> folder);
    const QVector<library::AudioFile> *filesFor(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int         rowCount(const QModelIndex &parent = {}) const override;
    int         columnCount(const QModelIndex &parent = {}) const override;
    QVariant    data(const QModelIndex &index, int role) const override;

  private:
    library::FolderNode *nodeFor(const QModelIndex &index) const;

    std::shared_ptr<const library::Folder> m_tree;
};

} // namespace ui
