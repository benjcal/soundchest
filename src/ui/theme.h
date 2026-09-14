#pragma once

#include <QColor>

namespace oclero::qlementine {
class QlementineStyle;
} // namespace oclero::qlementine

namespace ui::theme {

oclero::qlementine::QlementineStyle *qlementineStyle();

QColor workspaceBackground();
QColor border();
QColor waveformColor();

} // namespace ui::theme
