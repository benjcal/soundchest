#include "file_table_widget.h"

#include "file_table_widget_model.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QDrag>
#include <QFontMetrics>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QMenu>
#include <QMimeData>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QShortcut>
#include <QTableView>
#include <QVBoxLayout>

namespace ui {

namespace {

// A table view that drags its selected rows out as file URLs (text/uri-list),
// which is what game engines and file managers accept. The custom drag pixmap
// keeps a 500-file selection from rendering 500 rows under the cursor.
class FileTableView : public QTableView {
  public:
    using QTableView::QTableView;

  protected:
    void startDrag(Qt::DropActions) override {
        const QModelIndexList indexes = selectionModel()->selectedRows();
        if (indexes.isEmpty())
            return;

        QMimeData *mimeData = model()->mimeData(indexes);
        if (!mimeData)
            return;

        auto *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(dragPixmap(indexes.size()));
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    }

  private:
    QPixmap dragPixmap(int count) const {
        const QString label = count == 1 ? QStringLiteral("1 sound") : QStringLiteral("%1 sounds").arg(count);

        const QFont        font = this->font();
        const QFontMetrics metrics(font);
        const QSize        textSize = metrics.size(Qt::TextSingleLine, label);

        QPixmap pixmap(textSize.width() + 20, textSize.height() + 12);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        QColor background = palette().color(QPalette::Highlight);
        background.setAlphaF(0.85f);
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        painter.drawRoundedRect(QRectF(0.5, 0.5, pixmap.width() - 1.0, pixmap.height() - 1.0), 4.0, 4.0);

        painter.setPen(palette().color(QPalette::HighlightedText));
        painter.setFont(font);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, label);
        return pixmap;
    }
};

} // namespace

FileTableWidget::FileTableWidget(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("filePane"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_table = new FileTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setDragEnabled(true);
    m_table->setDragDropMode(QAbstractItemView::DragOnly);
    m_table->setDefaultDropAction(Qt::CopyAction);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(34);
    m_table->horizontalHeader()->setStretchLastSection(true);

    // activated() cannot be used here: qlementine reports
    // SH_ItemView_ActivateItemOnSingleClick, so it fires on a single click.
    // Emit on double-click and on Return/Enter instead.
    connect(m_table, &QTableView::doubleClicked, this,
            [this](const QModelIndex &index) { emit fileActivated(index.row()); });

    auto *activateShortcut = new QShortcut(m_table);
    activateShortcut->setKeys({QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter)});
    activateShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(activateShortcut, &QShortcut::activated, this, [this] {
        const QModelIndex index = m_table->currentIndex();
        if (index.isValid())
            emit fileActivated(index.row());
    });

    auto *playShortcut = new QShortcut(QKeySequence(Qt::Key_Space), m_table);
    playShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(playShortcut, &QShortcut::activated, this, &FileTableWidget::playPauseRequested);

    auto *exportShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+E")), m_table);
    exportShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(exportShortcut, &QShortcut::activated, this, &FileTableWidget::exportRequested);

    connect(m_table, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        const QModelIndex index = m_table->indexAt(pos);
        if (index.isValid() && m_selection && !m_selection->isSelected(index))
            m_table->setCurrentIndex(index);

        QMenu    menu(this);
        QAction *exportAction = menu.addAction(QStringLiteral("Export to Folder…"));
        exportAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));
        connect(exportAction, &QAction::triggered, this, &FileTableWidget::exportRequested);
        menu.exec(m_table->viewport()->mapToGlobal(pos));
    });

    layout->addWidget(m_table);
}

void FileTableWidget::setModel(QAbstractItemModel *model) {
    if (m_selection)
        disconnect(m_selection, nullptr, this, nullptr);

    m_table->setModel(model);
    m_selection = m_table->selectionModel();

    if (m_selection) {
        connect(m_selection, &QItemSelectionModel::currentRowChanged, this,
                [this](const QModelIndex &current, const QModelIndex &) { emit currentFileChanged(current.row()); });
    }
}

QVector<library::AudioFile> FileTableWidget::selectedFiles() const {
    const auto *model = qobject_cast<const FileTableWidgetModel *>(m_table->model());
    if (!model || !m_selection)
        return {};

    const QModelIndexList       rows = m_selection->selectedRows();
    QVector<library::AudioFile> files;
    files.reserve(rows.size());
    for (const QModelIndex &index : rows) {
        const library::AudioFile file = model->audioFile(index.row());
        if (!file.filePath.isEmpty())
            files.append(file);
    }
    return files;
}

void FileTableWidget::setCurrentRow(int row) {
    m_table->setCurrentIndex(m_table->model()->index(row, FileTableWidgetModel::Name));
}

} // namespace ui
