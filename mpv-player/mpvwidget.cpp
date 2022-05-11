#include "mpvwidget.h"
#include <stdexcept>
#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>
#include <QDebug>

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    m_mpvCtl.setEventCallback(wakeup, this);
}

MpvWidget::~MpvWidget()
{
    makeCurrent();
    if (mpv_gl) {
        mpv_render_context_free(mpv_gl);
    }

    mpv_terminate_destroy(m_mpvCtl.mpv);
}

int MpvWidget::setMpvCommand(const QVariant& params)
{
    return m_mpvCtl.setMpvCommand(params);
}

void MpvWidget::setMpvProperty(const QString& name, const QVariant& value)
{
    m_mpvCtl.setMpvProperty(name, value);
}

QVariant MpvWidget::getMpvProperty(const QString &name) const
{
    return m_mpvCtl.getMpvProperty(name);
}

void MpvWidget::asyncMpvSetCommand(const QVariant &params)
{
    m_mpvCtl.setMpvCommandAsync(params);
}

void MpvWidget::asyncMpvSetProperty(const QString &name, const QVariant &value)
{
    m_mpvCtl.setMpvPropertyAsync(name, value);
}

void MpvWidget::initializeGL()
{
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    if (mpv_render_context_create(&mpv_gl, m_mpvCtl.mpv, params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_gl, MpvWidget::on_update, reinterpret_cast<void *>(this));
}

void MpvWidget::paintGL()
{
    mpv_opengl_fbo mpfbo{static_cast<int>(defaultFramebufferObject()), width(), height(), 0};
    int flip_y{1};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpv_gl, params);
}

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (m_mpvCtl.mpv) {
        mpv_event *event = mpv_wait_event(m_mpvCtl.mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        } else if (strcmp(prop->name, "pause") == 0) {
            if (prop->format == MPV_FORMAT_FLAG) {
                int flag = *(static_cast<int*>(prop->data));
                if(m_mpvCtl.getMpvProperty("idle-active").toBool() == false)
                {
                    Q_EMIT pauseChanged(flag);
                }

//                qDebug()<<"handle_mpv_event:"<<"MPV_EVENT_PROPERTY_CHANGE pause="
//                        <<flag
//                        <<" idle-active="
//                        <<m_mpvCtl.getProperty("idle-active").toBool();
            }
        }
        break;
    }

    case MPV_EVENT_START_FILE: {
//        qDebug()<<"handle_mpv_event:"<<"start file";
        emit signalMpvEventStartFile();
        break;
    }
    case MPV_EVENT_FILE_LOADED: {
//        qDebug()<<"handle_mpv_event:"<<"file loaded";
        emit signalMpvEventFileLoaded();
        break;
    }
    case MPV_EVENT_END_FILE: {
        const mpv_event_end_file* end_file_data = (mpv_event_end_file*)event->data;
        emit signalMpvEventEndFile(end_file_data->reason);

//        qDebug()<<"handle_mpv_event:"<<"end file reason="<<end_file_data->reason;
        break;
    }
    case MPV_EVENT_IDLE: {
//        qDebug()<<"handle_mpv_event:"<<"idling";
        emit signalMpvEventIdling();
        break;
    }

    default: ;
        // Ignore uninteresting or unknown events.
        //        qDebug()<<"handle_mpv_event:"<<"unknown event id="<<event->event_id;
        break;
    }
}

// Make Qt invoke mpv_render_context_render() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's render API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    } else {
        update();
    }
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}
