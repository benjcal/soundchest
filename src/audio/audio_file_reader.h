#pragma once

#include <QString>

#include <sndfile.h>

namespace audio {

class AudioFileReader {
  public:
    AudioFileReader() = default;
    ~AudioFileReader();

    AudioFileReader(const AudioFileReader &)            = delete;
    AudioFileReader &operator=(const AudioFileReader &) = delete;

    bool open(const QString &filePath);
    void close();

    QString formatLabel() const;
    int     sampleRate() const { return m_sampleRate; }
    int     channels() const { return m_channels; }
    qint64  frameCount() const { return m_frameCount; }
    double  durationSec() const;

    // Reads interleaved float frames. Returns frames read; 0 means end of file.
    qint64 readFrames(float *interleaved, qint64 frames);

  private:
    SNDFILE *m_sf         = nullptr;
    int      m_sampleRate = 0;
    int      m_channels   = 0;
    qint64   m_frameCount = 0;
    int      m_format     = 0;
};

} // namespace audio
