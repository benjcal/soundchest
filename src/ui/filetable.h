#pragma once

#include <QWidget>

class QAbstractItemModel;
class QTableView;

class FileTable : public QWidget
{
    Q_OBJECT

public:
    explicit FileTable(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);

signals:
    void currentFileChanged(int row);

private:
    QTableView* m_table;
};