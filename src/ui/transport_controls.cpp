#include "transport_controls.h"

#include "icons.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolButton>

#include <oclero/qlementine/widgets/Switch.hpp>

namespace ui {

TransportControls::TransportControls(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("transport"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(10);

    m_playButton = new QToolButton(this);
    m_playButton->setIcon(icons::mono(QStringLiteral("play")));
    m_playButton->setIconSize(QSize(24, 24));
    m_playButton->setMinimumSize(40, 36);
    m_playButton->setToolTip(QStringLiteral("Play"));
    icons::setAutoRecolor(m_playButton);

    m_stopButton = new QToolButton(this);
    m_stopButton->setIcon(icons::mono(QStringLiteral("stop")));
    m_stopButton->setIconSize(QSize(24, 24));
    m_stopButton->setMinimumSize(40, 36);
    m_stopButton->setToolTip(QStringLiteral("Stop"));
    icons::setAutoRecolor(m_stopButton);

    m_loopButton = new QToolButton(this);
    m_loopButton->setIcon(icons::mono(QStringLiteral("repeat")));
    m_loopButton->setIconSize(QSize(24, 24));
    m_loopButton->setMinimumSize(40, 36);
    m_loopButton->setCheckable(true);
    m_loopButton->setToolTip(QStringLiteral("Loop"));
    icons::setAutoRecolor(m_loopButton);

    auto *autoplayLabel = new QLabel(QStringLiteral("Autoplay"), this);
    m_autoplaySwitch    = new oclero::qlementine::Switch(this);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setFixedWidth(180);

    m_volumeIcon = new QLabel(this);

    connect(m_playButton, &QToolButton::clicked, this, &TransportControls::playClicked);
    connect(m_stopButton, &QToolButton::clicked, this, &TransportControls::stopClicked);
    connect(m_loopButton, &QToolButton::toggled, this, &TransportControls::loopToggled);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &TransportControls::volumeChanged);

    layout->addWidget(m_playButton);
    layout->addWidget(m_stopButton);
    layout->addWidget(m_loopButton);
    layout->addSpacing(8);
    layout->addWidget(autoplayLabel);
    layout->addWidget(m_autoplaySwitch);
    layout->addStretch();
    layout->addWidget(m_volumeIcon);
    layout->addWidget(m_volumeSlider);

    setControlsEnabled(false);
}

void TransportControls::setControlsEnabled(bool enabled) {
    m_playButton->setEnabled(enabled);
    m_stopButton->setEnabled(enabled);
    m_loopButton->setEnabled(enabled);
    m_autoplaySwitch->setEnabled(enabled);
    m_volumeSlider->setEnabled(enabled);
    updateVolumeIcon();
}

void TransportControls::updateVolumeIcon() {
    const bool   enabled = m_volumeSlider->isEnabled();
    const QColor color   = palette().color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::WindowText);
    m_volumeIcon->setPixmap(icons::colorized(QStringLiteral("speaker-high"), QSize(24, 24), color));
}

void TransportControls::setVolume(int value) { m_volumeSlider->setValue(value); }

void TransportControls::setLooping(bool enabled) { m_loopButton->setChecked(enabled); }

bool TransportControls::looping() const { return m_loopButton->isChecked(); }

void TransportControls::setAutoplay(bool enabled) { m_autoplaySwitch->setChecked(enabled); }

bool TransportControls::autoplay() const { return m_autoplaySwitch->isChecked(); }

} // namespace ui
