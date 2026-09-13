#include "file_table_widget.h"

#include "file_table_widget_model.h"
#include "file_table_widget_waveform_delegate.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QScrollBar>
#include <QTableView>
#include <QVBoxLayout>

#include <algorithm>

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
    m_table->setItemDelegateForColumn(FileTableWidgetModel::Waveform, new FileTableWidgetWaveformDelegate(m_table));

    connect(m_table->verticalScrollBar(), &QScrollBar::valueChanged, this, &FileTableWidget::emitVisibleRows);

    layout->addWidget(m_table);
}

void FileTableWidget::setModel(QAbstractItemModel *model) {
    m_table->setModel(model);

    auto *header = m_table->horizontalHeader();
    header->setSectionResizeMode(FileTableWidgetModel::Waveform, QHeaderView::Fixed);
    m_table->setColumnWidth(FileTableWidgetModel::Waveform, 80);

    connect(model, &QAbstractItemModel::modelReset, this, &FileTableWidget::emitVisibleRows);
    connect(model, &QAbstractItemModel::rowsInserted, this, &FileTableWidget::emitVisibleRows);

    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &) { emit currentFileChanged(current.row()); });
}

void FileTableWidget::emitVisibleRows() {
    QAbstractItemModel *model = m_table->model();
    if (!model)
        return;

    const int rowCount = model->rowCount();
    if (rowCount == 0)
        return;

    const QRect viewport = m_table->viewport()->rect();
    const int   first    = m_table->rowAt(viewport.top());
    const int   last     = m_table->rowAt(viewport.bottom());
    if (first < 0 && last < 0)
        return;

    const int normalizedFirst = std::max(0, first < 0 ? 0 : first);
    const int normalizedLast  = std::min(rowCount - 1, last < 0 ? rowCount - 1 : last);

    emit visibleRowsChanged(normalizedFirst, normalizedLast);
}

} // namespace ui
