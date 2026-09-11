#include "waveformdelegate.h"

#include "filetablemodel.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>

#include <algorithm>

namespace {

constexpr int horizontalPadding = 4;
constexpr int verticalPadding = 5;

} // namespace

WaveformDelegate::WaveformDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void WaveformDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const QRect area = option.rect.adjusted(horizontalPadding, verticalPadding,
                                            -horizontalPadding, -verticalPadding);
    if (area.width() <= 0 || area.height() <= 0)
        return;

    const auto data = index.data(FileTableModel::WaveformRole).value<audio::WaveformData>();
    const qreal midY = area.center().y();

    if (!data.valid()) {
        painter->setPen(QPen(option.palette.color(QPalette::Mid), 1));
        painter->drawLine(QPointF(area.left(), midY), QPointF(area.right(), midY));
        return;
    }

    const int columns = data.mins.size();
    const qreal columnWidth = static_cast<qreal>(area.width()) / columns;
    const qreal amplitude = area.height() * 0.42;

    const bool selected = option.state & QStyle::State_Selected;
    QColor barColor = selected ? option.palette.color(QPalette::HighlightedText)
                               : option.palette.color(QPalette::WindowText);
    barColor.setAlpha(190);

    painter->setPen(Qt::NoPen);
    for (int i = 0; i < columns; ++i) {
        const qreal x = area.left() + i * columnWidth;
        const qreal top = midY - data.maxs.at(i) * amplitude;
        const qreal bottom = midY - data.mins.at(i) * amplitude;
        const qreal barTop = std::min(top, bottom);
        const qreal barHeight = std::max(1.0, std::abs(bottom - top));
        painter->fillRect(QRectF(x, barTop, std::max(1.0, columnWidth), barHeight), barColor);
    }

    painter->setPen(QPen(option.palette.color(QPalette::Mid), 1));
    painter->drawLine(QPointF(area.left(), midY), QPointF(area.right(), midY));
}

QSize WaveformDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(150, 34);
}
