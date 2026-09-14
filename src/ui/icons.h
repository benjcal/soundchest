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
QColor  accent();

} // namespace ui::icons
