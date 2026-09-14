#include "theme.h"

#include <QApplication>

#include <oclero/qlementine/style/QlementineStyle.hpp>

namespace ui::theme {

// These helpers read from the qlementine style when it is active. The literal
// fallbacks are gruvbox values, used only before the style is installed.
oclero::qlementine::QlementineStyle *qlementineStyle() {
    return qobject_cast<oclero::qlementine::QlementineStyle *>(qApp->style());
}

QColor workspaceBackground() {
    if (auto *style = qlementineStyle())
        return style->theme().backgroundColorWorkspace;
    return QColor(0x1d, 0x20, 0x21);
}

QColor border() {
    if (auto *style = qlementineStyle())
        return style->theme().borderColor;
    return QColor(0x50, 0x49, 0x45);
}

QColor waveformColor() {
    if (auto *style = qlementineStyle())
        return style->theme().secondaryColor;
    return QColor(0xeb, 0xdb, 0xb2);
}

} // namespace ui::theme
