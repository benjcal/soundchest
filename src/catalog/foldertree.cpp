#include "foldertree.h"

#include <QCollator>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QSet>

#include <algorithm>
#include <memory>

namespace catalog {

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

int FolderTree::fileCount() const {
    int count = 0;
    if (!root)
        return 0;

    QVector<const DirectoryNode *> stack{root.get()};
    while (!stack.isEmpty()) {
        const DirectoryNode *node = stack.takeLast();
        count += node->files.size();
        for (const DirectoryNode *child : node->children)
            stack.append(child);
    }
    return count;
}

bool scanFolder(const QString &rootPath, FolderTree *out, QString *error) {
    if (!out || !QFileInfo::exists(rootPath)) {
        if (error)
            *error = QStringLiteral("folder does not exist");
        return false;
    }

    const QString root = QDir(rootPath).absolutePath();

    QStringList                               directories;
    QHash<QString, QVector<audio::AudioInfo>> filesByDir;

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

        audio::AudioInfo audioInfo;
        if (!audio::readInfo(info.absoluteFilePath(), &audioInfo))
            continue;

        filesByDir[info.absolutePath()].append(audioInfo);
    }

    for (QVector<audio::AudioInfo> &files : filesByDir)
        std::sort(files.begin(), files.end(), [](const audio::AudioInfo &a, const audio::AudioInfo &b) {
            return a.fileName.localeAwareCompare(b.fileName) < 0;
        });

    directories = sortedByName(directories);

    out->root       = std::make_unique<DirectoryNode>();
    out->root->path = root;

    QHash<QString, DirectoryNode *> byPath;
    byPath.insert(root, out->root.get());

    for (const QString &dir : directories) {
        auto *node = new DirectoryNode;
        node->path = dir;
        byPath.insert(dir, node);

        const QString  parentPath = QFileInfo(dir).absolutePath();
        DirectoryNode *parent     = byPath.value(parentPath);
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
        DirectoryNode *node = byPath.value(it.key());
        if (node)
            node->files = std::move(it.value());
    }

    return true;
}

} // namespace catalog