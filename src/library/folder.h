#pragma once

#include <QString>
#include <QVector>

#include <memory>

#include "library/audio_file.h"

namespace library {

struct FolderNode {
    FolderNode()                              = default;
    FolderNode(const FolderNode &)            = delete;
    FolderNode &operator=(const FolderNode &) = delete;
    ~FolderNode() { qDeleteAll(children); }

    QString                     path;
    FolderNode                 *parent = nullptr;
    QVector<FolderNode *>       children;
    QVector<library::AudioFile> files;
};

struct Folder {
    std::unique_ptr<FolderNode> root;
    int                         fileCount() const;
};

} // namespace library