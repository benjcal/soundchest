#pragma once

#include <QObject>
#include <QString>

class QAudioOutput;
class QMediaPlayer;

namespace audio {

class Player : public QObject {
    Q_OBJECT

  public:
    explicit Player(QObject *parent = nullptr);
    ~Player() override;

    // Starts loading the file. Returns false when the path is empty. Loading
    // and playback errors are reported through loadFailed().
    bool open(const QString &filePath);

    void play();
    void stop();
    void setLooping(bool enabled);
    void setVolumePercent(int percent);
    int  volumePercent() const;

    bool   isPlaying() const;
    double positionSec() const;
    double lengthSec() const;

    QString filePath() const;

  signals:
    void loadFailed(const QString &message);

  private:
    QMediaPlayer *m_media       = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QString       m_filePath;
    bool          m_looping     = false;
    int           m_volume      = 80;
};

} // namespace audio
