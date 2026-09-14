#pragma once

#include <oclero/qlementine/style/QlementineStyle.hpp>

namespace ui {

// Slider handles in qlementine use the theme's primaryColorForeground, which
// in the gruvbox theme is as dark as the background. Use the secondary
// (cream) colors instead so the volume knob stays visible on both the filled
// value and the groove, enabled and disabled.
class Style : public oclero::qlementine::QlementineStyle {
  public:
    using QlementineStyle::QlementineStyle;

    const QColor &sliderHandleColor(oclero::qlementine::MouseState mouse) const override;
};

} // namespace ui
