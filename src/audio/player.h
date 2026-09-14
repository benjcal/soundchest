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
    // and playback errors are reported through errorOccurred().
    bool open(const QString &filePath);

    void play();
    void stop();
    void setPositionSec(double seconds);
    void setLooping(bool enabled);
    void setVolumePercent(int percent);
    int  volumePercent() const;

    bool   hasMedia() const;
    bool   isPlaying() const;
    double lengthSec() const;

    QString filePath() const;

  signals:
    void errorOccurred(const QString &message);
    void positionChanged(double seconds);
    void playingChanged(bool playing);

  private:
    // Mirrors the state we set on QMediaPlayer/QAudioOutput because they do
    // not hand it back in the form the app needs: m_filePath backs hasMedia(),
    // m_looping has to be re-applied after every open(), and m_volume backs
    // volumePercent() for the slider.
    QMediaPlayer *m_media       = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QString       m_filePath;
    bool          m_looping = false;
    int           m_volume  = 80;
};

} // namespace audio
