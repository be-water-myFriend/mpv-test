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

    initSignalConnect();

    // 初始化循环状态
    m_loopState =  LoopState::LoopNormal;
}

MpvPlayer::~MpvPlayer()
{
    qDebug()<<"~MpvPlayer()";
}

void MpvPlayer::initSignalConnect()
{
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

    connect(m_mpv, &MpvWidget::signalMpvEventStartFile, this, [=](){
        setState(PlayState::Opening);
    });
    connect(m_mpv, &MpvWidget::signalMpvEventFileLoaded, this, [=](){
        if (true == m_mpv->getMpvProperty("pause").toBool()) {
            play();
        } else {
            setState(PlayState::Play);
        }
    });
    connect(m_mpv, &MpvWidget::signalMpvEventEndFile, this, [=](int reason){
        switch (reason) {
        case MPV_END_FILE_REASON_EOF:
            setState(PlayState::EndReached);

            // 播放完成时默认正循环播放
            mediaListNext();
            break;
        case MPV_END_FILE_REASON_ERROR:
            // 等到出错信号槽返回值
            if(false == signalPlayError(m_playIndex))
            {
                return;
            }

            // 跳过错误视频播放
            if(m_loopState == LoopState::LoopNext)
            {
                mediaListNext();
            }
            else if(m_loopState == LoopState::LoopPrevious)
            {
                mediaListPrevious();
            }
            break;
        case MPV_END_FILE_REASON_STOP:
            setState(PlayState::Stop);
            break;

        case MPV_END_FILE_REASON_QUIT:
            setState(PlayState::Stop);
            break;

        case MPV_END_FILE_REASON_REDIRECT:
            setState(PlayState::Stop);
            break;
        default:
            break;
        }
    });

    connect(m_mpv, &MpvWidget::signalMpvEventIdling, this, [=](){
        setState(PlayState::Idle);
    });
}

bool MpvPlayer::openMedia(QString filepath)
{
    //    if(!m_mpv->setCommand(QStringList() << "loadfile" << filepath))
    //    {
    //        return true;
    //    }
    //    return false;

    m_mpv->asyncMpvSetCommand(QStringList() << "loadfile" << filepath);
    return true;
}

void MpvPlayer::seek(int msPos)
{
    double pos = (msPos*1.0)/1000;

    // mpv seek单位为秒，支持double类型
    m_mpv->asyncMpvSetCommand(QVariantList() << "seek" << pos << "absolute");
}

void MpvPlayer::pauseResume()
{
    const bool paused = m_mpv->getMpvProperty("pause").toBool();
    m_mpv->setMpvProperty("pause", !paused);
}

void MpvPlayer::play()
{
    m_mpv->setMpvProperty("pause", false);
}

void MpvPlayer::pause()
{
    m_mpv->setMpvProperty("pause", true);
}

void MpvPlayer::stop()
{
    m_mpv->setMpvCommand(QVariantList()<<"stop");
    setState(PlayState::Stop);
}

bool MpvPlayer::isPlaying()
{
    return !m_mpv->getMpvProperty("pause").toBool();
}

int MpvPlayer::volume()
{
    return m_mpv->getMpvProperty("volume").toInt();
}

void MpvPlayer::setVolume(int vol)
{
    m_mpv->setMpvProperty("volume",vol);
}

void MpvPlayer::setMute(bool mute)
{
    m_mpv->setMpvProperty("mute",mute);
}

bool MpvPlayer::isMute()
{
    return m_mpv->getMpvProperty("mute").toBool();
}

int MpvPlayer::position()
{
    double time = m_mpv->getMpvProperty("playback-time").toDouble();
    return time*1000;
}

int MpvPlayer::duration()
{
    double time = m_mpv->getMpvProperty("duration").toDouble();
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
    if(index < 0 || index >= m_Playlist.length())
    {
        qDebug()<<"MpvPlayer::mediaListPlay outof index!";
        return;
    }

    m_playIndex = index;
    emit signalItemSet(m_playIndex);
    openMedia(m_Playlist.at(m_playIndex));

    // 默认下循环
    m_loopState = LoopState::LoopNext;
}

void MpvPlayer::mediaListStop()
{
    stop();
}

void MpvPlayer::mediaListPrevious()
{
    m_loopState = LoopState::LoopPrevious;

    m_playIndex--;
    if(m_playIndex < 0)
    {
        m_playIndex = m_Playlist.length()-1;
    }

    emit signalItemSet(m_playIndex);
    openMedia(m_Playlist.at(m_playIndex));
}

void MpvPlayer::mediaListNext()
{
    m_loopState = LoopState::LoopNext;

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
    //qDebug()<<"MpvPlayer::oldState "<<stateToStr(m_state)<<" to newState "<<stateToStr(newState);

    // 触发状态变更信号
    if(m_state != newState) {
        m_state = newState;
        emit signalStateChanged(newState);
    }

    // 重置循环状态
    if(newState == PlayState::Play)
    {
       m_loopState =  LoopState::LoopNormal;
    }
}
