#include "transportbar.h"

#include "icons.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolButton>

TransportBar::TransportBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("transport"));

    const QColor text = palette().color(QPalette::WindowText);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(10);

    m_playButton = new QToolButton(this);
    m_playButton->setIcon(icons::themed(QStringLiteral("play"), text));
    m_playButton->setIconSize(QSize(24, 24));
    m_playButton->setMinimumSize(40, 36);
    m_playButton->setToolTip(tr("Play (audio arrives in Stage 2)"));

    m_stopButton = new QToolButton(this);
    m_stopButton->setIcon(icons::themed(QStringLiteral("stop"), text));
    m_stopButton->setIconSize(QSize(24, 24));
    m_stopButton->setMinimumSize(40, 36);
    m_stopButton->setToolTip(tr("Stop (audio arrives in Stage 2)"));

    m_loopButton = new QToolButton(this);
    m_loopButton->setIcon(icons::themed(QStringLiteral("repeat"), text));
    m_loopButton->setIconSize(QSize(24, 24));
    m_loopButton->setMinimumSize(40, 36);
    m_loopButton->setCheckable(true);
    m_loopButton->setToolTip(tr("Loop (audio arrives in Stage 2)"));

    m_autoplayCheck = new QCheckBox(tr("Autoplay"), this);
    m_autoplayCheck->setToolTip(tr("Autoplay selected file (arrives in Stage 4)"));

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(180);
    m_volumeSlider->setToolTip(tr("Volume (arrives in Stage 2)"));

    auto* volumeIcon = new QLabel(this);
    volumeIcon->setPixmap(icons::themed(QStringLiteral("speaker-high"), text).pixmap(24, 24));

    connect(m_playButton, &QToolButton::clicked, this, &TransportBar::playClicked);
    connect(m_stopButton, &QToolButton::clicked, this, &TransportBar::stopClicked);
    connect(m_loopButton, &QToolButton::toggled, this, &TransportBar::loopToggled);
    connect(m_autoplayCheck, &QCheckBox::toggled, this, &TransportBar::autoplayChanged);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &TransportBar::volumeChanged);

    layout->addWidget(m_playButton);
    layout->addWidget(m_stopButton);
    layout->addWidget(m_loopButton);
    layout->addSpacing(8);
    layout->addWidget(m_autoplayCheck);
    layout->addStretch();
    layout->addWidget(volumeIcon);
    layout->addWidget(m_volumeSlider);

    setControlsEnabled(false);
}

void TransportBar::setControlsEnabled(bool enabled)
{
    m_playButton->setEnabled(enabled);
    m_stopButton->setEnabled(enabled);
    m_loopButton->setEnabled(enabled);
    m_autoplayCheck->setEnabled(enabled);
    m_volumeSlider->setEnabled(enabled);
}

int TransportBar::volume() const
{
    return m_volumeSlider->value();
}

void TransportBar::setVolume(int value)
{
    m_volumeSlider->setValue(value);
}