#pragma once

#include <QString>
#include <QVector>

#include <memory>

#include "audio/audiofile.h"

namespace catalog {

struct DirectoryNode
{
    QString path;
    DirectoryNode* parent = nullptr;
    QVector<DirectoryNode*> children;
    QVector<audio::AudioInfo> files;
    ~DirectoryNode()
    {
        qDeleteAll(children);
    }
};

struct FolderTree
{
    std::unique_ptr<DirectoryNode> root;
    int fileCount() const;
};

bool scanFolder(const QString& rootPath, FolderTree* out, QString* error = nullptr);

} // namespace catalog