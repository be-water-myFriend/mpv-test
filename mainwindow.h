#ifndef MainWindow_H
#define MainWindow_H

#include <QtWidgets/QWidget>

class MpvPlayer;
class QSlider;
class QPushButton;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

protected:
  virtual bool eventFilter(QObject *, QEvent *);

private Q_SLOTS:
    void setSliderRange(int duration);
private:
    MpvPlayer *m_mpvPlayer;
    QSlider *m_slider;
    QPushButton *m_openBtn;
    QPushButton *m_playBtn;
};

#endif // MainWindow_H
