#include "filetable.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

FileTable::FileTable(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("filePane"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    auto* table = new QTableView(this);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);

    auto* model = new QStandardItemModel(0, 5, table);
    model->setHorizontalHeaderLabels({ tr("Name"), tr("Duration"), tr("Bit rate"),
                                       tr("Channels"), tr("Format") });
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(table);
}