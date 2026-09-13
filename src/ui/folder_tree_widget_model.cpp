#include "folder_tree_widget_model.h"

#include <QFileInfo>

namespace ui {

FolderTreeWidgetModel::FolderTreeWidgetModel(QObject *parent) : QAbstractItemModel(parent) {}

void FolderTreeWidgetModel::setFolder(std::shared_ptr<const library::Folder> folder) {
    beginResetModel();
    m_tree = std::move(folder);
    endResetModel();
}

const QVector<library::AudioFile> *FolderTreeWidgetModel::filesFor(const QModelIndex &index) const {
    const library::FolderNode *node = nodeFor(index);
    return node ? &node->files : nullptr;
}

QModelIndex FolderTreeWidgetModel::index(int row, int column, const QModelIndex &parentIndex) const {
    if (!m_tree || !m_tree->root || column != 0)
        return {};

    if (!parentIndex.isValid()) {
        if (row != 0)
            return {};
        return createIndex(0, 0, m_tree->root.get());
    }

    const library::FolderNode *parent = nodeFor(parentIndex);
    if (!parent || row < 0 || row >= parent->children.size())
        return {};

    return createIndex(row, column, parent->children.at(row));
}

QModelIndex FolderTreeWidgetModel::parent(const QModelIndex &child) const {
    if (!child.isValid() || !m_tree || !m_tree->root)
        return {};

    const library::FolderNode *node = nodeFor(child);
    if (!node || node == m_tree->root.get() || !node->parent)
        return {};

    if (node->parent == m_tree->root.get())
        return createIndex(0, 0, m_tree->root.get());

    library::FolderNode *grandparent = node->parent->parent;
    const int               row         = grandparent->children.indexOf(node->parent);

    return createIndex(row, 0, node->parent);
}

int FolderTreeWidgetModel::rowCount(const QModelIndex &parentIndex) const {
    if (!m_tree || !m_tree->root)
        return 0;
    if (!parentIndex.isValid())
        return 1;

    const library::FolderNode *node = nodeFor(parentIndex);
    return node ? node->children.size() : 0;
}

int FolderTreeWidgetModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant FolderTreeWidgetModel::data(const QModelIndex &index, int role) const {
    const library::FolderNode *node = nodeFor(index);
    if (!node)
        return {};

    if (role == Qt::DisplayRole) {
        const QString name = QFileInfo(node->path).fileName();
        return name.isEmpty() ? node->path : name;
    }
    if (role == Qt::ToolTipRole)
        return node->path;

    return {};
}

library::FolderNode *FolderTreeWidgetModel::nodeFor(const QModelIndex &index) const {
    if (!index.isValid() || !index.internalPointer())
        return nullptr;
    return static_cast<library::FolderNode *>(index.internalPointer());
}

} // namespace ui
