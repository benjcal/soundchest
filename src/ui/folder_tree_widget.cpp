#include "folder_tree_widget.h"

#include "icons.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVBoxLayout>

namespace ui {

namespace {

class FolderItemDelegate : public QStyledItemDelegate {
  public:
    FolderItemDelegate(QTreeView *view, QIcon closed, QIcon open)
        : m_view(view), m_closed(std::move(closed)), m_open(std::move(open)) {}

  protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        option->icon = m_view->isExpanded(index) ? m_open : m_closed;
        option->features |= QStyleOptionViewItem::HasDecoration;
    }

  private:
    QTreeView *m_view;
    QIcon      m_closed;
    QIcon      m_open;
};

} // namespace

FolderTreeWidget::FolderTreeWidget(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("folderPane"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_tree = new QTreeView(this);
    m_tree->setHeaderHidden(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setExpandsOnDoubleClick(true);

    m_tree->setIconSize(QSize(18, 18));
    m_tree->setItemDelegate(new FolderItemDelegate(m_tree, icons::mono(QStringLiteral("folder-closed")),
                                                   icons::mono(QStringLiteral("folder-open"))));
    icons::setAutoRecolor(m_tree);

    layout->addWidget(m_tree);
}

void FolderTreeWidget::setModel(QAbstractItemModel *model) {
    if (m_selection) {
        disconnect(m_selection, &QItemSelectionModel::currentChanged, this, &FolderTreeWidget::currentDirectoryChanged);
    }

    m_tree->setModel(model);
    m_selection = m_tree->selectionModel();

    if (m_selection) {
        connect(m_selection, &QItemSelectionModel::currentChanged, this, &FolderTreeWidget::currentDirectoryChanged);
    }
}

void FolderTreeWidget::setCurrentIndex(const QModelIndex &index) { m_tree->setCurrentIndex(index); }

void FolderTreeWidget::expand(const QModelIndex &index) { m_tree->expand(index); }

} // namespace ui
