#include "style.h"

namespace ui {

const QColor &Style::sliderHandleColor(oclero::qlementine::MouseState mouse) const {
    if (mouse == oclero::qlementine::MouseState::Disabled)
        return theme().secondaryColorDisabled;
    if (mouse == oclero::qlementine::MouseState::Pressed)
        return theme().secondaryColorPressed;
    if (mouse == oclero::qlementine::MouseState::Hovered)
        return theme().secondaryColorHovered;
    return theme().secondaryColor;
}

} // namespace ui
