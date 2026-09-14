#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace library {

enum class CollisionPolicy {
    Rename,
    Overwrite,
    Skip,
};

struct ExportResult {
    int         copied  = 0;
    int         skipped = 0;
    int         failed  = 0;
    QStringList errors;
};

// Counts source files whose destination path already exists and points at a
// different file (exporting a file onto itself is not a collision). Used to
// decide whether the export dialog needs to ask about overwriting.
int countCollisions(const QVector<QString> &sourcePaths, const QString &destinationDir);

// Copies files into destinationDir. Files that already exist there are
// renamed, overwritten, or skipped according to policy. The destination
// directory is created when missing. Never throws. Overwrite replaces the
// target only after the copy to a temporary file has succeeded.
ExportResult copyFiles(const QVector<QString> &sourcePaths, const QString &destinationDir, CollisionPolicy policy);

} // namespace library
