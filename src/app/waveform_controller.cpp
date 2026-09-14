#include "app/waveform_controller.h"

#include "ui/waveform_widget.h"
#include "ui/window.h"
#include "waveform/peaks_builder.h"

#include <algorithm>

namespace app {

WaveformController::WaveformController(ui::Window *window, waveform::PeaksBuilder *peaksBuilder, QObject *parent)
    : QObject(parent), m_window(window), m_peaksBuilder(peaksBuilder) {
    connect(m_peaksBuilder, &waveform::PeaksBuilder::ready, this, &WaveformController::onPeaksReady);
}

void WaveformController::onSoundsShown() {
    m_currentFilePath.clear();
    m_window->waveform()->clear();

    // Folder changed: cancel in-flight builds and drop cached peaks so the
    // previous folder's waveforms cannot reappear.
    m_peaksBuilder->clear();
}

void WaveformController::onSoundSelected(const library::AudioFile &file) {
    m_currentFilePath = file.filePath;
    m_window->waveform()->clear();

    // request() is a no-op when peaks are already cached, so show them now.
    const waveform::Peaks cached = m_peaksBuilder->peaks(file.filePath);
    if (cached.valid())
        m_window->waveform()->setData(cached);

    const int pixelColumns =
        std::max(1, qRound(m_window->waveform()->width() * m_window->waveform()->devicePixelRatioF()));
    m_peaksBuilder->request(file.filePath, pixelColumns);
}

void WaveformController::onPeaksReady(const QString &filePath) {
    if (filePath != m_currentFilePath)
        return;

    const waveform::Peaks peaks = m_peaksBuilder->peaks(filePath);
    if (peaks.valid())
        m_window->waveform()->setData(peaks);
}

} // namespace app
