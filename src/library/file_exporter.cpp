#include "file_exporter.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace library {

namespace {

QString uniqueTargetPath(const QDir &directory, const QString &fileName) {
    if (!QFileInfo::exists(directory.filePath(fileName)))
        return directory.filePath(fileName);

    const QFileInfo info(fileName);
    const QString   base   = info.completeBaseName();
    const QString   suffix = info.completeSuffix();

    for (int n = 2;; ++n) {
        const QString candidate = suffix.isEmpty() ? QStringLiteral("%1 (%2)").arg(base).arg(n)
                                                   : QStringLiteral("%1 (%2).%3").arg(base).arg(n).arg(suffix);
        const QString path      = directory.filePath(candidate);
        if (!QFileInfo::exists(path))
            return path;
    }
}

QString temporaryTargetPath(const QDir &directory, const QString &fileName) {
    for (int n = 0;; ++n) {
        const QString candidate =
            n == 0 ? QStringLiteral("%1.part").arg(fileName) : QStringLiteral("%1.part%2").arg(fileName).arg(n);
        const QString path = directory.filePath(candidate);
        if (!QFileInfo::exists(path))
            return path;
    }
}

// Replaces targetPath with tempPath only after the copy to tempPath succeeded,
// so a failed copy never destroys the existing file.
bool replaceFile(const QString &tempPath, const QString &targetPath) {
    if (QFile::rename(tempPath, targetPath))
        return true;
    if (!QFileInfo::exists(targetPath) || !QFile::remove(targetPath))
        return false;
    return QFile::rename(tempPath, targetPath);
}

} // namespace

int countCollisions(const QVector<QString> &sourcePaths, const QString &destinationDir) {
    const QDir destination(destinationDir);
    int        conflicts = 0;

    for (const QString &sourcePath : sourcePaths) {
        const QString target = destination.filePath(QFileInfo(sourcePath).fileName());
        if (!QFileInfo::exists(target))
            continue;
        if (QFileInfo(sourcePath).canonicalFilePath() == QFileInfo(target).canonicalFilePath())
            continue;
        ++conflicts;
    }

    return conflicts;
}

ExportResult copyFiles(const QVector<QString> &sourcePaths, const QString &destinationDir, CollisionPolicy policy) {
    ExportResult result;

    if (destinationDir.isEmpty()) {
        result.failed = sourcePaths.size();
        result.errors.append(QStringLiteral("No destination folder."));
        return result;
    }

    const QDir destination(destinationDir);
    if (!destination.exists() && !QDir().mkpath(destinationDir)) {
        result.failed = sourcePaths.size();
        result.errors.append(QStringLiteral("Could not create %1").arg(destinationDir));
        return result;
    }

    for (const QString &sourcePath : sourcePaths) {
        const QFileInfo source(sourcePath);
        if (!source.exists() || !source.isFile()) {
            ++result.failed;
            result.errors.append(QStringLiteral("%1: not a file").arg(sourcePath));
            continue;
        }

        QString target = destination.filePath(source.fileName());

        if (QFileInfo(sourcePath).canonicalFilePath() == QFileInfo(target).canonicalFilePath()) {
            ++result.skipped;
            continue;
        }

        bool overwriting = false;
        if (QFileInfo::exists(target)) {
            switch (policy) {
            case CollisionPolicy::Skip:
                ++result.skipped;
                continue;
            case CollisionPolicy::Rename:
                target = uniqueTargetPath(destination, source.fileName());
                break;
            case CollisionPolicy::Overwrite:
                overwriting = true;
                break;
            }
        }

        const QString copyTarget = overwriting ? temporaryTargetPath(destination, source.fileName()) : target;
        if (!QFile::copy(sourcePath, copyTarget)) {
            ++result.failed;
            result.errors.append(QStringLiteral("%1: copy failed").arg(source.fileName()));
            if (overwriting)
                QFile::remove(copyTarget);
            continue;
        }

        if (overwriting && !replaceFile(copyTarget, target)) {
            QFile::remove(copyTarget);
            ++result.failed;
            result.errors.append(QStringLiteral("%1: could not replace %2").arg(source.fileName(), target));
            continue;
        }

        ++result.copied;
    }

    return result;
}

} // namespace library
