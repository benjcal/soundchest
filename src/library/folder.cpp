#include "folder.h"

#include <QCollator>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QSet>

#include <algorithm>
#include <memory>

namespace library {

namespace {

const QSet<QString> &audioExtensions() {
    static const QSet<QString> extensions{
        QStringLiteral("wav"),  QStringLiteral("aiff"), QStringLiteral("aif"),  QStringLiteral("au"),
        QStringLiteral("flac"), QStringLiteral("ogg"),  QStringLiteral("opus"), QStringLiteral("mp3"),
        QStringLiteral("caf"),  QStringLiteral("w64"),  QStringLiteral("rf64"),
    };
    return extensions;
}

QStringList sortedByName(const QStringList &names) {
    QStringList result = names;
    QCollator   collator;
    collator.setNumericMode(true);
    std::sort(result.begin(), result.end(), collator);
    return result;
}

} // namespace

int Folder::fileCount() const {
    int count = 0;
    if (!root)
        return 0;

    QVector<const FolderNode *> stack{root.get()};
    while (!stack.isEmpty()) {
        const FolderNode *node = stack.takeLast();
        count += node->files.size();
        for (const FolderNode *child : node->children)
            stack.append(child);
    }
    return count;
}

bool scan(const QString &rootPath, Folder *out, QString *error) {
    if (!out || !QFileInfo::exists(rootPath)) {
        if (error)
            *error = QStringLiteral("folder does not exist");
        return false;
    }

    const QString root = QDir(rootPath).absolutePath();

    QStringList                               directories;
    QHash<QString, QVector<library::AudioFile>> filesByDir;

    QDirIterator it(rootPath, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo info = it.fileInfo();

        if (info.isDir()) {
            directories.append(info.absoluteFilePath());
            continue;
        }

        if (!audioExtensions().contains(info.suffix().toLower()))
            continue;

        library::AudioFile audioInfo;
        if (!library::readAudioFile(info.absoluteFilePath(), &audioInfo))
            continue;

        filesByDir[info.absolutePath()].append(audioInfo);
    }

    for (QVector<library::AudioFile> &files : filesByDir)
        std::sort(files.begin(), files.end(), [](const library::AudioFile &a, const library::AudioFile &b) {
            return a.fileName.localeAwareCompare(b.fileName) < 0;
        });

    directories = sortedByName(directories);

    out->root       = std::make_unique<FolderNode>();
    out->root->path = root;

    QHash<QString, FolderNode *> byPath;
    byPath.insert(root, out->root.get());

    for (const QString &dir : directories) {
        auto *node = new FolderNode;
        node->path = dir;
        byPath.insert(dir, node);

        const QString  parentPath = QFileInfo(dir).absolutePath();
        FolderNode *parent     = byPath.value(parentPath);
        if (!parent) {
            delete node;
            out->root.reset();
            if (error)
                *error = QStringLiteral("folder tree has a missing parent");
            return false;
        }

        node->parent = parent;
        parent->children.append(node);
    }

    for (auto it = filesByDir.begin(); it != filesByDir.end(); ++it) {
        FolderNode *node = byPath.value(it.key());
        if (node)
            node->files = std::move(it.value());
    }

    return true;
}

} // namespace library