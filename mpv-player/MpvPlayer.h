#ifndef MPVPLAYER_H
#define MPVPLAYER_H

#include <QWidget>
class MpvWidget;

enum PlayState
{
    Idle,
    Opening,
    Buffering,
    Play,
    Pause,
    Stop,
    EndReached
};

enum LoopState
{
    LoopPrevious = -1, // 上一部
    LoopNormal,
    LoopNext
};

class MpvPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MpvPlayer(QWidget *parent = nullptr);
    ~MpvPlayer();

    void initSignalConnect();
    bool openMedia(QString filepath);

    void pauseResume();

    // 播放控制
    void play();
    void pause();
    void stop();
    void seek(int msPos); // 单位毫秒
    bool isPlaying();

    // 音量控制
    int volume();
    void setVolume(int vol);
    void setMute(bool mute);
    bool isMute();

    // 视频时间
    int position();
    int duration();

    // 列表播放控制
    void setMediaListPlayMode(int mode);
    int mediaListAdd(QString videoPath);
    void mediaListPlay(int index = 0);
    void mediaListStop();
    void mediaListPrevious();
    void mediaListNext();

    QWidget *renderWidget() const;
    int state() const;


    void setState(PlayState newState);

signals:
    void signalItemSet(int index);
    void signalUpdateDuration(int time);
    void signalPositionChanged(int time);
    void signalStateChanged(int state);

    bool signalPlayError(int index);

private:
    MpvWidget *m_mpv;

    PlayState m_state;
    int m_position{};
    int m_playIndex{};
    LoopState m_loopState;
    QList<QString> m_Playlist;
};

#endif // MPVPLAYER_H
