#include "theme.h"

namespace {

QColor color(const char* hex)
{
    return QColor(QString::fromLatin1(hex));
}

} // namespace

namespace theme {

QPalette blenderPalette()
{
    const QColor primary   = color("#4772b3");
    const QColor secondary = color("#f1a355");

    const QColor text      = color("#ffffff");
    const QColor overlay2  = color("#B9B9B9");
    const QColor overlay1  = color("#989898");
    const QColor overlay0  = color("#848484");
    const QColor surface2  = color("#6C6C6C");
    const QColor surface1  = color("#545454");
    const QColor surface0  = color("#3d3d3d");
    const QColor base      = color("#303030");
    const QColor mantle    = color("#282828");
    const QColor crust     = color("#1d1d1d");

    const QColor highlightedText = primary.valueF() > 0.5 ? mantle : text;

    float h, s, v, a;
    text.getHsvF(&h, &s, &v, &a);
    const QColor brightText = QColor::fromHsvF(h, s, 1.0f - v, a);

    QPalette p;

    // Normal
    p.setColor(QPalette::Base, mantle);
    p.setColor(QPalette::AlternateBase, base);
    p.setColor(QPalette::Window, base);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::PlaceholderText, overlay1);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, base);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, brightText);
    p.setColor(QPalette::ToolTipBase, mantle);
    p.setColor(QPalette::ToolTipText, overlay2);
    p.setColor(QPalette::Highlight, primary);
    p.setColor(QPalette::HighlightedText, highlightedText);
    p.setColor(QPalette::Link, secondary);
    p.setColor(QPalette::LinkVisited, secondary);

    p.setColor(QPalette::Light, crust);
    p.setColor(QPalette::Midlight, mantle);
    p.setColor(QPalette::Mid, surface0);
    p.setColor(QPalette::Dark, surface1);
    p.setColor(QPalette::Shadow, overlay0);

    p.setColor(QPalette::Accent, secondary);

    // Inactive
    p.setColor(QPalette::Inactive, QPalette::Highlight, surface1);
    p.setColor(QPalette::Inactive, QPalette::Link, surface1);
    p.setColor(QPalette::Inactive, QPalette::LinkVisited, surface1);
    p.setColor(QPalette::Inactive, QPalette::Accent, surface1);

    // Disabled
    p.setColor(QPalette::Disabled, QPalette::WindowText, overlay1);
    p.setColor(QPalette::Disabled, QPalette::Base, base);
    p.setColor(QPalette::Disabled, QPalette::AlternateBase, base);
    p.setColor(QPalette::Disabled, QPalette::Text, overlay1);
    p.setColor(QPalette::Disabled, QPalette::PlaceholderText, overlay1);
    p.setColor(QPalette::Disabled, QPalette::Button, base);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, overlay1);
    p.setColor(QPalette::Disabled, QPalette::BrightText, mantle);
    p.setColor(QPalette::Disabled, QPalette::Highlight, surface2);
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, surface0);
    p.setColor(QPalette::Disabled, QPalette::Link, surface0);
    p.setColor(QPalette::Disabled, QPalette::LinkVisited, surface0);
    p.setColor(QPalette::Disabled, QPalette::Accent, surface2);

    return p;
}

} // namespace theme