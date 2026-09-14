#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>

class QWidget;

namespace ui::icons {

QIcon   mono(const QString &name);
void    setAutoRecolor(QWidget *widget);
QPixmap colorized(const QString &name, const QSize &size, const QColor &color);

// Renders any resource SVG, scaling proportionally when one dimension is zero
// and honoring the screen's device pixel ratio.
QPixmap pixmap(const QString &resourcePath, const QSize &size);

} // namespace ui::icons
