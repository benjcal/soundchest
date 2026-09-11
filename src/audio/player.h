#pragma once

#include <QObject>
#include <QString>

#include <memory>

struct ma_engine;
struct ma_sound;

namespace audio {

class SndFileDataSource;

class Player : public QObject {
    Q_OBJECT

  public:
    explicit Player(QObject *parent = nullptr);
    ~Player() override;

    bool open(const QString &filePath);
    void close();

    void play();
    void stop();
    void setLooping(bool enabled);
    void setVolumePercent(int percent);
    int  volumePercent() const;

    bool   isPlaying() const;
    double positionSec() const;
    double lengthSec() const;

    QString filePath() const;
    QString errorString() const;

  private:
    void setVolume(float linear);

    std::unique_ptr<ma_engine>         m_engine;
    std::unique_ptr<ma_sound>          m_sound;
    std::unique_ptr<SndFileDataSource> m_dataSource;
    bool                               m_engineReady = false;
    bool                               m_loaded      = false;
    bool                               m_looping     = false;
    float                              m_volume      = 0.8f;
    double                             m_lengthSec   = 0.0;
    int                                m_sampleRate  = 0;
    QString                            m_filePath;
    QString                            m_error;
};

} // namespace audio
