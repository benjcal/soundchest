#include "foldertreemodel.h"

#include <QFileInfo>

FolderTreeModel::FolderTreeModel(QObject *parent) : QAbstractItemModel(parent) {}

void FolderTreeModel::setTree(const catalog::FolderTree *tree) {
    beginResetModel();
    m_tree = tree;
    endResetModel();
}

const QVector<audio::AudioInfo> *FolderTreeModel::filesFor(const QModelIndex &index) const {
    const catalog::DirectoryNode *node = nodeFor(index);
    return node ? &node->files : nullptr;
}

QModelIndex FolderTreeModel::index(int row, int column, const QModelIndex &parentIndex) const {
    if (!m_tree || !m_tree->root || column != 0)
        return {};

    if (!parentIndex.isValid()) {
        if (row != 0)
            return {};
        return createIndex(0, 0, m_tree->root.get());
    }

    const catalog::DirectoryNode *parent = nodeFor(parentIndex);
    if (!parent || row < 0 || row >= parent->children.size())
        return {};

    return createIndex(row, column, parent->children.at(row));
}

QModelIndex FolderTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid() || !m_tree || !m_tree->root)
        return {};

    const catalog::DirectoryNode *node = nodeFor(child);
    if (!node || node == m_tree->root.get() || !node->parent)
        return {};

    if (node->parent == m_tree->root.get())
        return createIndex(0, 0, m_tree->root.get());

    catalog::DirectoryNode *grandparent = node->parent->parent;
    const int               row         = grandparent->children.indexOf(node->parent);

    return createIndex(row, 0, node->parent);
}

int FolderTreeModel::rowCount(const QModelIndex &parentIndex) const {
    if (!m_tree || !m_tree->root)
        return 0;
    if (!parentIndex.isValid())
        return 1;

    const catalog::DirectoryNode *node = nodeFor(parentIndex);
    return node ? node->children.size() : 0;
}

int FolderTreeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant FolderTreeModel::data(const QModelIndex &index, int role) const {
    const catalog::DirectoryNode *node = nodeFor(index);
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

catalog::DirectoryNode *FolderTreeModel::nodeFor(const QModelIndex &index) const {
    if (!index.isValid() || !index.internalPointer())
        return nullptr;
    return static_cast<catalog::DirectoryNode *>(index.internalPointer());
}