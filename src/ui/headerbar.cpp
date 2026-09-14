#include "headerbar.h"

#include "icons.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace ui {

HeaderBar::HeaderBar(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("header"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(12);

    auto *logo = new QLabel(this);
    logo->setPixmap(icons::pixmap(QStringLiteral(":/branding/lockup-on-dark.svg"), QSize(0, 24)));
    logo->setAccessibleName(QStringLiteral("Sound Chest"));

    auto *aboutButton = new QToolButton(this);
    aboutButton->setIcon(icons::mono(QStringLiteral("info")));
    aboutButton->setIconSize(QSize(22, 22));
    aboutButton->setMinimumSize(40, 36);
    aboutButton->setToolTip(QStringLiteral("About Sound Chest"));
    icons::setAutoRecolor(aboutButton);

    auto *openButton = new QToolButton(this);
    openButton->setIcon(icons::mono(QStringLiteral("folder-open")));
    openButton->setIconSize(QSize(24, 24));
    openButton->setMinimumSize(40, 36);
    openButton->setToolTip(QStringLiteral("Open Folder"));
    icons::setAutoRecolor(openButton);

    connect(aboutButton, &QToolButton::clicked, this, &HeaderBar::aboutRequested);
    connect(openButton, &QToolButton::clicked, this, &HeaderBar::openFolderRequested);

    layout->addWidget(logo);
    layout->addStretch();
    layout->addWidget(aboutButton);
    layout->addWidget(openButton);
}

} // namespace ui
