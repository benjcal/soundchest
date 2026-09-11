#include "folderbrowser.h"

#include <QLabel>
#include <QVBoxLayout>

FolderBrowser::FolderBrowser(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("folderPane"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    auto* placeholder = new QLabel(tr("Folder browser — arrives in Stage 3"), this);
    placeholder->setAlignment(Qt::AlignCenter);
    layout->addWidget(placeholder);
}