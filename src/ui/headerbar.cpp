#include "headerbar.h"

#include "icons.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

#include <oclero/qlementine/widgets/Label.hpp>

namespace ui {

HeaderBar::HeaderBar(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("header"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(12);

    auto *logo = new QLabel(this);
    logo->setPixmap(icons::colorized(QStringLiteral("waveform"), QSize(30, 30), icons::accent()));

    auto *title = new oclero::qlementine::Label(QStringLiteral("Sound Chest"), oclero::qlementine::TextRole::H5, this);
    title->setObjectName(QStringLiteral("appTitle"));

    auto *openButton = new QToolButton(this);
    openButton->setIcon(icons::mono(QStringLiteral("folder-open")));
    openButton->setIconSize(QSize(24, 24));
    openButton->setMinimumSize(40, 36);
    openButton->setToolTip(QStringLiteral("Open Folder"));
    icons::setAutoRecolor(openButton);

    connect(openButton, &QToolButton::clicked, this, &HeaderBar::openFolderRequested);

    layout->addWidget(logo);
    layout->addWidget(title);
    layout->addStretch();
    layout->addWidget(openButton);
}

} // namespace ui
