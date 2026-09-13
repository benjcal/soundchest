#pragma once

#include <QString>
#include <QVector>

#include <memory>

#include "library/audio_file.h"

namespace library {

struct FolderNode {
    QString                   path;
    FolderNode            *parent = nullptr;
    QVector<FolderNode *>  children;
    QVector<library::AudioFile> files;
    ~FolderNode() { qDeleteAll(children); }
};

struct Folder {
    std::unique_ptr<FolderNode> root;
    int                            fileCount() const;
};

bool scan(const QString &rootPath, Folder *out, QString *error = nullptr);

} // namespace library