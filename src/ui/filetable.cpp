#include "filetable.h"

#include "filetablemodel.h"
#include "waveformdelegate.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QTableView>
#include <QVBoxLayout>

FileTable::FileTable(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("filePane"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(34);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setItemDelegateForColumn(FileTableModel::Waveform, new WaveformDelegate(m_table));

    layout->addWidget(m_table);
}

void FileTable::setModel(QAbstractItemModel* model)
{
    m_table->setModel(model);

    auto* header = m_table->horizontalHeader();
    header->setSectionResizeMode(FileTableModel::Waveform, QHeaderView::Fixed);
    m_table->setColumnWidth(FileTableModel::Waveform, 80);

    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                emit currentFileChanged(current.row());
            });
}