#ifndef MPVPLAYER_H
#define MPVPLAYER_H

#include <QWidget>
class MpvWidget;

class MpvPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MpvPlayer(QWidget *parent = nullptr);

    void openMedia(QString file);
    void seek(int msPos); // 单位毫秒
    void pauseResume();

signals:
//    void signalItemSet(int index);
    void signalUpdateDuration(int time);
    void signalPositionChanged(int time);
//    void signalStateChanged(int state);

private:
    MpvWidget *m_mpv;
};

#endif // MPVPLAYER_H
