#include "mainwindow.h"
#include "mpv-player/MpvPlayer.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    m_mpvPlayer = new MpvPlayer(this);
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    m_openBtn = new QPushButton("Open");
    m_playBtn = new QPushButton("Pause");
    QHBoxLayout *hb = new QHBoxLayout();
    hb->addWidget(m_openBtn);
    hb->addWidget(m_playBtn);
    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(m_mpvPlayer);
    vl->addWidget(m_slider);
    vl->addLayout(hb);
    setLayout(vl);

    connect(m_slider, &QSlider::sliderMoved, m_mpvPlayer, &MpvPlayer::seek);
    connect(m_openBtn, &QPushButton::released, this, [=](){
        QString file = QFileDialog::getOpenFileName(0, "Open a video");
        if (file.isEmpty())
        {
            return;
        }
        m_mpvPlayer->openMedia(file);
    });

    connect(m_playBtn, &QPushButton::released, m_mpvPlayer, &MpvPlayer::pauseResume);

    // 播放位置改变
    connect(m_mpvPlayer, &MpvPlayer::signalPositionChanged, this, [=](double pos) {
        //qDebug()<<" time-pos:"<<pos;
        m_slider->setValue(pos);
    });

    // 视频时长改变
    connect(m_mpvPlayer, &MpvPlayer::signalUpdateDuration, this, [=](double duration){
        qDebug()<<" duration:"<<duration;
        setSliderRange(duration);
    });
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}
