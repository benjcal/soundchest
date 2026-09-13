#pragma once

#include <QStyledItemDelegate>

namespace ui {

class FileTableWidgetWaveformDelegate : public QStyledItemDelegate {
    Q_OBJECT

  public:
    explicit FileTableWidgetWaveformDelegate(QObject *parent = nullptr);

    void  paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

} // namespace ui
