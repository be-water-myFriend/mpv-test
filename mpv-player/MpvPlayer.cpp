#include "MpvPlayer.h"
#include "mpvwidget.h"
#include <QVBoxLayout>
#include <QDebug>

MpvPlayer::MpvPlayer(QWidget *parent) : QWidget(parent)
{
    m_mpv = new MpvWidget(this);

    // 设置鼠标跟随
    setMouseTracking(true);

    // 设置渲染控件布局
    QVBoxLayout *vl = new QVBoxLayout();
    vl->setSpacing(0);
    vl->setContentsMargins(0, 0, 0, 0);
    setLayout(vl);
    vl->addWidget(m_mpv);

    // 播放位置改变
    connect(m_mpv, &MpvWidget::positionChanged, this, [=](double pos) {
        emit signalPositionChanged(pos * 1000);
    });

    // 视频时长改变
    connect(m_mpv, &MpvWidget::durationChanged, this, [=](double duration){
        emit signalUpdateDuration(duration * 1000);
    });

    connect(m_mpv, &MpvWidget::pauseChanged, this, [=](int value){
        if(value) {
            setState(PlayState::Pause);
        } else {
            setState(PlayState::Play);
        }
    });

    connect(m_mpv, &MpvWidget::eofReachedChanged, this, [=](int value){
        if(value) {
            setState(PlayState::EndReached);

            // 循环播放
            mediaListNext();
        }
    });
}

bool MpvPlayer::openMedia(QString filepath)
{
    setState(PlayState::Opening);
//    if(!m_mpv->setCommand(QStringList() << "loadfile" << filepath))
//    {
//        m_mpv->setProperty("pause", false);
//        setState(PlayState::Play);
//        return true;
//    }
//    return false;

    m_mpv->asyncSetCommand(QStringList() << "loadfile" << filepath);
        m_mpv->setProperty("pause", false);
        setState(PlayState::Play);
        return true;
}

void MpvPlayer::seek(int msPos)
{
    double pos = (msPos*1.0)/1000;

    // mpv seek单位为秒，支持double类型
    m_mpv->asyncSetCommand(QVariantList() << "seek" << pos << "absolute");
}

void MpvPlayer::pauseResume()
{
    const bool paused = m_mpv->getProperty("pause").toBool();
    m_mpv->setProperty("pause", !paused);
}

void MpvPlayer::play()
{
    m_mpv->setProperty("pause", false);
}

void MpvPlayer::pause()
{
    m_mpv->setProperty("pause", true);
}

void MpvPlayer::stop()
{
    m_mpv->setCommand(QVariantList()<<"stop");
    setState(PlayState::Stop);
}

bool MpvPlayer::isPlaying()
{
    return !m_mpv->getProperty("pause").toBool();
}

int MpvPlayer::volume()
{
    return m_mpv->getProperty("volume").toInt();
}

void MpvPlayer::setVolume(int vol)
{
    m_mpv->setProperty("volume",vol);
}

void MpvPlayer::setMute(bool mute)
{
    m_mpv->setProperty("mute",mute);
}

bool MpvPlayer::isMute()
{
    return m_mpv->getProperty("mute").toBool();
}

int MpvPlayer::position()
{
    double time = m_mpv->getProperty("playback-time").toDouble();
    return time*1000;
}

int MpvPlayer::duration()
{
    double time = m_mpv->getProperty("duration").toDouble();
    return time*1000;
}

void MpvPlayer::setMediaListPlayMode(int mode)
{

}

int MpvPlayer::mediaListAdd(QString videoPath)
{
    m_Playlist.push_back(videoPath);
    return m_Playlist.length();
}

void MpvPlayer::mediaListPlay(int index)
{
    m_playIndex = index;
    emit signalItemSet(m_playIndex);
    openMedia(m_Playlist.at(m_playIndex));
}

void MpvPlayer::mediaListStop()
{
    stop();
}

void MpvPlayer::mediaListPrevious()
{
    m_playIndex--;
    if(m_playIndex < 0)
    {
        m_playIndex = m_Playlist.length();
    }

    emit signalItemSet(m_playIndex);
    openMedia(m_Playlist.at(m_playIndex));
}

void MpvPlayer::mediaListNext()
{
    m_playIndex++;
    if(m_playIndex > (m_Playlist.length()-1))
    {
        m_playIndex = 0;
    }

    emit signalItemSet(m_playIndex);
    openMedia(m_Playlist.at(m_playIndex));
}

QWidget *MpvPlayer::renderWidget() const
{
    return m_mpv;
}

int MpvPlayer::state() const
{
    return m_state;
}

QString stateToStr(PlayState newState) {
    QString stateStr = "Unknown";

    switch (newState) {
    case PlayState::Idle:
        stateStr = "Idle";
        break;

    case PlayState::Opening:
        stateStr = "Opening";
        break;

    case PlayState::Buffering:
        stateStr = "Buffering";
        break;

    case PlayState::Play:
        stateStr = "Play";
        break;

    case PlayState::Pause:
        stateStr = "Pause";
        break;

    case PlayState::Stop:
        stateStr = "Stop";
        break;

    case PlayState::EndReached:
        stateStr = "EndReached";
        break;
    }

    return stateStr;
}

void MpvPlayer::setState(PlayState newState)
{
    qDebug()<<"MpvPlayer::oldState "<<stateToStr(m_state)<<" to newState "<<stateToStr(newState);

    if(m_state != newState) {
        m_state = newState;

        emit signalStateChanged(newState);
    }
}
