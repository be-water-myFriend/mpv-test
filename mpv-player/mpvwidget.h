#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QtWidgets/QOpenGLWidget>
#include <client.h>
#include <render_gl.h>
#include "qthelper.hpp"
#include <QThread>

class MpvController : public QObject
{
    Q_OBJECT

public:
    MpvController() {
        mpv = mpv_create();
        if (!mpv) {
            throw std::runtime_error("could not create mpv context");
        }
    }

    void setEventCallback(void (*cb)(void *d), void *d)
    {
        mpv_set_option_string(mpv, "terminal", "yes");
        mpv_set_option_string(mpv, "msg-level", "all=error");
        if (mpv_initialize(mpv) < 0) {
            throw std::runtime_error("could not initialize mpv context");
            return;
        }

        // Request hw decoding, just for testing.
        mpv::qt::set_option_variant(mpv, "hwdec", "auto");

        mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
        mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
        mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);
        mpv_set_wakeup_callback(mpv, cb, d);
    }

public slots:
    int setCommand(const QVariant& params)
    {
        return mpv::qt::get_error(mpv::qt::command_variant(mpv, params));
    }

    void setProperty(const QString& name, const QVariant& value)
    {
        mpv::qt::set_property_variant(mpv, name, value);
    }

    QVariant getProperty(const QString &name) const
    {
        return mpv::qt::get_property_variant(mpv, name);
    }

signals:

public:
    mpv_handle *mpv;
};

class MpvWidget Q_DECL_FINAL: public QOpenGLWidget
{
    Q_OBJECT
public:
    MpvWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~MpvWidget();
    int setCommand(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;

    void asyncSetCommand(const QVariant& params);
    void asyncSetProperty(const QString& name, const QVariant& value);
    QSize sizeHint() const override { return QSize(480, 270);}

Q_SIGNALS:
    void durationChanged(double value);
    void positionChanged(double value);
    void pauseChanged(bool value);
    void eofReachedChanged(int value);

    void signalMpvEventStartFile();
    void signalMpvEventFileLoaded();
    void signalMpvEventEndFile(int reason);
    void signalMpvEventIdling();

    void signalAsyncSetCommand(const QVariant& params);
    void signalAsyncSetProperty(const QString& name, const QVariant& value);
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
private Q_SLOTS:
    void on_mpv_events();
    void maybeUpdate();
private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    QThread workerThread;
    MpvController m_mpvCtl;
    mpv_render_context *mpv_gl;
};



#endif // PLAYERWINDOW_H
