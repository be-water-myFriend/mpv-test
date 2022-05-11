#include "mainwindow.h"
#include "mpv-player/MpvPlayer.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QDateTime>

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

    m_slider->setFocusPolicy(Qt::NoFocus);
    m_openBtn->setFocusPolicy(Qt::NoFocus);
    m_playBtn->setFocusPolicy(Qt::NoFocus);

    connect(m_slider, &QSlider::sliderMoved, m_mpvPlayer, &MpvPlayer::seek);
    connect(m_openBtn, &QPushButton::released, this, [=](){
        // D:\tmp\handMake1\2019年1月26日汇总\0420
//        QString file = QFileDialog::getOpenFileName(0, "Open a video");
//        if (file.isEmpty())
//        {
//            return;
//        }
        m_mpvPlayer->openMedia("D:\\tmp\\handMake1\\0420-abp108HD_cut2.mp4");
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

    installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{

    // 按键压下
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key_event            = static_cast<QKeyEvent *>(event);
        int key                         = key_event->key();
        Qt::KeyboardModifiers modifiers = key_event->modifiers();
        switch (key)
        {
            case Qt::Key_Left:

                m_mpvPlayer->seek(m_mpvPlayer->position() - 10*1000);
                break;
            case Qt::Key_Right:
            {
                // 延时打印
//                static qint64 oldTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
//                qint64 newTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
//                if(newTime - oldTime >= 100)
//                {
//                    qDebug()<<"AAAAA:"<<newTime - oldTime;
//                }
//                oldTime = newTime;

                int pos = m_mpvPlayer->position();
                m_mpvPlayer->seek(pos + 10*1000);
                break;
            }

            default:
                return false;
        }
        return true;
    }
    else if (event->type() == QEvent::KeyRelease)  // 按键抬起
    {
        QKeyEvent *key_event            = static_cast<QKeyEvent *>(event);
        int key                         = key_event->key();
        Qt::KeyboardModifiers modifiers = key_event->modifiers();
        switch (key)
        {
            case Qt::Key_Left:
                if (!key_event->isAutoRepeat())
                {
                }
                break;
            case Qt::Key_Right:
                if (!key_event->isAutoRepeat())
                {
                }
                break;
            default:
                return false;
        }
        return true;
    }

    return false;
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}
