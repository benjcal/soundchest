#include "waveformdelegate.h"

#include "filetablemodel.h"
#include "waveform/waveformrenderer.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>

#include <algorithm>

namespace ui {

namespace {

constexpr int horizontalPadding = 4;
constexpr int verticalPadding   = 5;

} // namespace

WaveformDelegate::WaveformDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void WaveformDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    const QRectF area = option.rect.adjusted(horizontalPadding, verticalPadding, -horizontalPadding, -verticalPadding);
    if (area.width() <= 0 || area.height() <= 0)
        return;

    const auto data = index.data(FileTableModel::WaveformRole).value<waveform::WaveformData>();

    if (!data.valid()) {
        waveform::paintMidline(painter, area, option.palette.color(QPalette::Mid));
        return;
    }

    const bool selected = option.state & QStyle::State_Selected;
    QColor     barColor =
        selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::WindowText);
    barColor.setAlpha(190);

    const int   columns     = data.mins.size();
    const qreal columnWidth = waveform::barWidth(area, columns);

    painter->setPen(Qt::NoPen);
    for (int i = 0; i < columns; ++i) {
        painter->fillRect(waveform::barRect(area, i, columnWidth, data.mins.at(i), data.maxs.at(i)), barColor);
    }

    waveform::paintMidline(painter, area, option.palette.color(QPalette::Mid));
}

QSize WaveformDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(150, 34);
}

} // namespace ui
