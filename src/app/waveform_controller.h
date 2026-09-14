#pragma once

#include <QObject>
#include <QString>

#include "library/audio_file.h"

namespace ui {
class Window;
} // namespace ui

namespace waveform {
class PeaksBuilder;
} // namespace waveform

namespace app {

class WaveformController : public QObject {
    Q_OBJECT

  public:
    WaveformController(ui::Window *window, waveform::PeaksBuilder *peaksBuilder, QObject *parent = nullptr);

  public slots:
    void onSoundsShown();
    void onSoundSelected(const library::AudioFile &file);

  private slots:
    void onPeaksReady(const QString &filePath);

  private:
    ui::Window             *m_window;
    waveform::PeaksBuilder *m_peaksBuilder;
    QString                 m_currentFilePath;
};

} // namespace app
