#pragma once

#include <QString>

#include <memory>

#include "library/audio_file.h"
#include "library/folder.h"

namespace library {

class AudioLibraryScanner {
  public:
    static std::shared_ptr<const Folder> scan(const QString &rootPath, QString *error = nullptr);
};

} // namespace library
