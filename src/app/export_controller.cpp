#include "app/export_controller.h"

#include "library/file_exporter.h"
#include "ui/file_table_widget.h"
#include "ui/window.h"

#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

namespace app {

namespace {

constexpr auto kLastExportDirKey = "export/lastDir";

} // namespace

ExportController::ExportController(ui::Window *window, ui::FileTableWidget *fileTable, QObject *parent)
    : QObject(parent), m_window(window), m_fileTable(fileTable) {
    connect(m_fileTable, &ui::FileTableWidget::exportRequested, this, &ExportController::exportSelection);
}

void ExportController::exportSelection() {
    const QVector<library::AudioFile> files = m_fileTable->selectedFiles();
    if (files.isEmpty()) {
        m_window->setStatusMessage(QStringLiteral("Select one or more sounds to export."));
        return;
    }

    const QSettings settings;
    const QString   startDir = settings.value(kLastExportDirKey, QDir::homePath()).toString();

    const QString destinationDir = m_window->chooseExportFolderPath(startDir);
    if (destinationDir.isEmpty())
        return;

    QVector<QString> sourcePaths;
    sourcePaths.reserve(files.size());
    for (const library::AudioFile &file : files)
        sourcePaths.append(file.filePath);

    const int conflicts = library::countCollisions(sourcePaths, destinationDir);

    library::CollisionPolicy policy = library::CollisionPolicy::Rename;
    if (conflicts > 0) {
        QMessageBox  box(QMessageBox::Question, QStringLiteral("Files already exist"),
                         QStringLiteral("%1 of %2 selected files already exist in the destination.")
                             .arg(conflicts)
                             .arg(files.size()),
                         QMessageBox::NoButton, m_window);
        QPushButton *renameButton    = box.addButton(QStringLiteral("Rename copies"), QMessageBox::AcceptRole);
        QPushButton *overwriteButton = box.addButton(QStringLiteral("Overwrite"), QMessageBox::DestructiveRole);
        QPushButton *skipButton      = box.addButton(QStringLiteral("Skip"), QMessageBox::RejectRole);
        QPushButton *cancelButton    = box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(renameButton);
        box.exec();

        if (box.clickedButton() == cancelButton)
            return;
        if (box.clickedButton() == overwriteButton)
            policy = library::CollisionPolicy::Overwrite;
        else if (box.clickedButton() == skipButton)
            policy = library::CollisionPolicy::Skip;
    }

    const library::ExportResult result = library::copyFiles(sourcePaths, destinationDir, policy);

    QString summary;
    if (result.skipped == 0 && result.failed == 0)
        summary = QStringLiteral("Exported %1 file(s) to %2").arg(result.copied).arg(destinationDir);
    else
        summary = QStringLiteral("Exported %1, skipped %2, failed %3 — %4")
                      .arg(result.copied)
                      .arg(result.skipped)
                      .arg(result.failed)
                      .arg(destinationDir);

    if (!result.errors.isEmpty())
        summary += QStringLiteral(" (%1)").arg(result.errors.first());

    m_window->setStatusMessage(summary);

    QSettings writableSettings;
    writableSettings.setValue(kLastExportDirKey, destinationDir);
}

} // namespace app
