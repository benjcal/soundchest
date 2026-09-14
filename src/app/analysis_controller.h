#pragma once

#include <QObject>
#include <QString>

#include "library/audio_file.h"

namespace analysis {
class SoundAnalyzer;
} // namespace analysis

namespace ui {
class Window;
} // namespace ui

namespace app {

class AnalysisController : public QObject {
    Q_OBJECT

  public:
    AnalysisController(ui::Window *window, analysis::SoundAnalyzer *analyzer, QObject *parent = nullptr);

  public slots:
    void onSoundSelected(const library::AudioFile &file);

  private slots:
    void onStatsReady(const QString &filePath);

  private:
    void showStatsForCurrentFile();

    ui::Window              *m_window;
    analysis::SoundAnalyzer *m_analyzer;

    // Holds the whole file, not just the path, because the status line needs
    // the metadata alongside the stats (WaveformController only needs a path).
    library::AudioFile m_currentFile;
};

} // namespace app
