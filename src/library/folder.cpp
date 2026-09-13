#include "library/folder.h"

namespace library {

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

} // namespace library
