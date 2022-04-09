#include "MpvPlayer.h"
#include "mpvwidget.h"
#include <QVBoxLayout>
#include <QDebug>

MpvPlayer::MpvPlayer(QWidget *parent) : QWidget(parent)
{
    m_mpv = new MpvWidget(this);

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
}

void MpvPlayer::openMedia(QString file)
{
    m_mpv->command(QStringList() << "loadfile" << file);
}

void MpvPlayer::seek(int msPos)
{
    double pos = (msPos*1.0)/1000;

    // mpv seek单位为秒，支持double类型
    m_mpv->command(QVariantList() << "seek" << pos << "absolute");
}

void MpvPlayer::pauseResume()
{
    const bool paused = m_mpv->getProperty("pause").toBool();
    m_mpv->setProperty("pause", !paused);
}
