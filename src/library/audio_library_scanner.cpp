#include "library/audio_library_scanner.h"

#include "audio/audio_file_reader.h"

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

const QCollator &nameCollator() {
    static const QCollator collator = [] {
        QCollator result;
        result.setNumericMode(true);
        return result;
    }();
    return collator;
}

QStringList sortedByPath(const QStringList &paths) {
    QStringList result = paths;
    std::sort(result.begin(), result.end(),
              [](const QString &a, const QString &b) { return nameCollator().compare(a, b) < 0; });
    return result;
}

bool readAudioFile(const QString &filePath, AudioFile *out) {
    audio::AudioFileReader reader;
    if (!reader.open(filePath))
        return false;

    const QFileInfo fileInfo(filePath);

    out->fileName    = fileInfo.fileName();
    out->filePath    = filePath;
    out->durationSec = reader.durationSec();
    out->sampleRate  = reader.sampleRate();
    out->channels    = reader.channels();
    out->format      = reader.formatLabel();

    // Average bit rate derived from the file size; for uncompressed formats
    // this is the raw PCM rate.
    const qint64 fileSizeBytes = fileInfo.size();
    out->bitRateKbps           = out->durationSec > 0.0 ? fileSizeBytes * 8.0 / out->durationSec / 1000.0 : 0.0;
    return true;
}

} // namespace

std::shared_ptr<const Folder> AudioLibraryScanner::scan(const QString &rootPath, QString *error) {
    if (!QFileInfo::exists(rootPath)) {
        if (error)
            *error = QStringLiteral("folder does not exist");
        return nullptr;
    }

    const QString root = QDir(rootPath).absolutePath();

    QStringList                        directories;
    QHash<QString, QVector<AudioFile>> filesByDir;

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

        AudioFile audioFile;
        if (!readAudioFile(info.absoluteFilePath(), &audioFile))
            continue;

        filesByDir[info.absolutePath()].append(audioFile);
    }

    for (QVector<AudioFile> &files : filesByDir)
        std::sort(files.begin(), files.end(), [](const AudioFile &a, const AudioFile &b) {
            return nameCollator().compare(a.fileName, b.fileName) < 0;
        });

    directories = sortedByPath(directories);

    auto folder        = std::make_shared<Folder>();
    folder->root       = std::make_unique<FolderNode>();
    folder->root->path = root;

    QHash<QString, FolderNode *> byPath;
    byPath.insert(root, folder->root.get());

    // The sorted list is parent-before-child: a parent's absolute path is a
    // strict prefix of every descendant's path, so byPath always has the parent.
    for (const QString &dir : directories) {
        auto *node = new FolderNode;
        node->path = dir;
        byPath.insert(dir, node);

        const QString parentPath = QFileInfo(dir).absolutePath();
        FolderNode   *parent     = byPath.value(parentPath);
        if (!parent) {
            delete node;
            if (error)
                *error = QStringLiteral("folder tree has a missing parent");
            return nullptr;
        }

        node->parent = parent;
        parent->children.append(node);
    }

    for (auto it = filesByDir.begin(); it != filesByDir.end(); ++it) {
        FolderNode *node = byPath.value(it.key());
        if (node)
            node->files = std::move(it.value());
    }

    return folder;
}

} // namespace library
