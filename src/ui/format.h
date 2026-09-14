#pragma once

#include <QString>

namespace ui {

inline QString formatChannels(int channels) {
    switch (channels) {
    case 1:
        return QStringLiteral("Mono");
    case 2:
        return QStringLiteral("Stereo");
    default:
        return QString::number(channels);
    }
}

inline QString formatDuration(double seconds) {
    if (seconds < 60.0)
        return QStringLiteral("%1 s").arg(seconds, 0, 'f', 2);

    const int totalSeconds = static_cast<int>(seconds);
    const int minutes      = totalSeconds / 60;
    const int secs         = totalSeconds % 60;
    if (minutes < 60)
        return QStringLiteral("%1:%2").arg(minutes).arg(secs, 2, 10, QLatin1Char('0'));

    const int hours = minutes / 60;
    return QStringLiteral("%1:%2:%3")
        .arg(hours)
        .arg(minutes % 60, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
}

} // namespace ui
