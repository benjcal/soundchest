#include "headerbar.h"

#include "icons.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

HeaderBar::HeaderBar(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("header"));
    setAttribute(Qt::WA_StyledBackground, true);

    const QColor text    = palette().color(QPalette::WindowText);
    const QColor primary = palette().color(QPalette::Highlight);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(12);

    auto *logo = new QLabel(this);
    logo->setPixmap(icons::themed(QStringLiteral("waveform"), primary).pixmap(30, 30));

    auto *title = new QLabel(QStringLiteral("Sound Chest"), this);
    title->setObjectName(QStringLiteral("appTitle"));
    QFont titleFont = font();
    titleFont.setPointSizeF(14);
    titleFont.setWeight(QFont::DemiBold);
    title->setFont(titleFont);

    auto *openButton = new QToolButton(this);
    openButton->setIcon(icons::themed(QStringLiteral("folder-open"), text));
    openButton->setText("Open Folder");
    openButton->setIconSize(QSize(24, 24));
    openButton->setMinimumSize(40, 36);
    openButton->setToolTip(QStringLiteral("Open Folder"));

    connect(openButton, &QToolButton::clicked, this, &HeaderBar::openFolderRequested);

    layout->addWidget(logo);
    layout->addWidget(title);
    layout->addStretch();
    layout->addWidget(openButton);
}