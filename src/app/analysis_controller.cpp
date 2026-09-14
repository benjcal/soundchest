#include "app/analysis_controller.h"

#include "analysis/sound_analyzer.h"
#include "ui/window.h"

namespace app {

AnalysisController::AnalysisController(ui::Window *window, analysis::SoundAnalyzer *analyzer, QObject *parent)
    : QObject(parent), m_window(window), m_analyzer(analyzer) {
    connect(m_analyzer, &analysis::SoundAnalyzer::ready, this, &AnalysisController::onStatsReady);
}

void AnalysisController::onSoundSelected(const library::AudioFile &file) {
    m_currentFile = file;

    if (m_analyzer->hasStats(file.filePath)) {
        showStatsForCurrentFile();
        return;
    }

    m_analyzer->request(file.filePath);
}

void AnalysisController::onStatsReady(const QString &filePath) {
    if (filePath != m_currentFile.filePath)
        return;

    showStatsForCurrentFile();
}

void AnalysisController::showStatsForCurrentFile() {
    const analysis::SoundStats stats = m_analyzer->stats(m_currentFile.filePath);
    if (stats.valid)
        m_window->statusAudioInfo(m_currentFile, &stats);
}

} // namespace app
