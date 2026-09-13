#include "file_table_widget.h"

#include "file_table_widget_model.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QTableView>
#include <QVBoxLayout>

namespace ui {

FileTableWidget::FileTableWidget(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("filePane"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(34);
    m_table->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_table);
}

void FileTableWidget::setModel(QAbstractItemModel *model) {
    m_table->setModel(model);

    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &) { emit currentFileChanged(current.row()); });
}

void FileTableWidget::setCurrentRow(int row) {
    m_table->setCurrentIndex(m_table->model()->index(row, FileTableWidgetModel::Name));
}

} // namespace ui
