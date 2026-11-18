// k9mpv: DVD title preview using libmpv's OpenGL render API.
// Copyright: GPL-2.0-or-later, see COPYING.
#include "k9mpv.h"
#include "k9dvd.h"
#include "k9dvdtitle.h"

#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <QGridLayout>
#include <QIcon>
#include <QMetaObject>
#include <QOpenGLContext>
#include <QSlider>
#include <QVBoxLayout>
#include <clocale>

static void *getProcAddress(void *, const char *name)
{
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    return glctx ? reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name))) : nullptr;
}

// ---------------------------------------------------------------- view

k9MpvView::k9MpvView(mpv_handle *mpv, QWidget *parent)
    : QOpenGLWidget(parent), m_mpv(mpv)
{
    setMinimumSize(160, 90);
}

k9MpvView::~k9MpvView()
{
    makeCurrent();
    if (m_ctx)
        mpv_render_context_free(m_ctx);
    doneCurrent();
}

void k9MpvView::initializeGL()
{
    mpv_opengl_init_params gl_init{getProcAddress, nullptr};
    int advanced = 1;
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced},
        {MPV_RENDER_PARAM_INVALID, nullptr}};
    if (mpv_render_context_create(&m_ctx, m_mpv, params) < 0) {
        qWarning("k9copy: failed to initialize mpv OpenGL render context");
        return;
    }
    mpv_render_context_set_update_callback(m_ctx, onUpdate, this);
}

void k9MpvView::paintGL()
{
    if (!m_ctx)
        return;
    const qreal dpr = devicePixelRatioF();
    mpv_opengl_fbo fbo{(int)defaultFramebufferObject(), (int)(width() * dpr), (int)(height() * dpr), 0};
    int flip = 1;
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip},
        {MPV_RENDER_PARAM_INVALID, nullptr}};
    mpv_render_context_render(m_ctx, params);
}

void k9MpvView::onUpdate(void *ctx)
{
    // Called from an mpv thread: hop to the GUI thread.
    QMetaObject::invokeMethod(static_cast<k9MpvView *>(ctx), "maybeUpdate", Qt::QueuedConnection);
}

void k9MpvView::maybeUpdate()
{
    if (m_ctx && (mpv_render_context_update(m_ctx) & MPV_RENDER_UPDATE_FRAME))
        update();
}

// ---------------------------------------------------------------- player

k9Mpv::k9Mpv(QWidget *parent)
    : QWidget(parent)
{
    // libmpv requires the C numeric locale.
    std::setlocale(LC_NUMERIC, "C");

    m_mpv = mpv_create();
    if (!m_mpv)
        qFatal("k9copy: cannot create mpv instance");
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "hwdec", "auto-safe");
    mpv_set_option_string(m_mpv, "keep-open", "yes");
    mpv_set_option_string(m_mpv, "idle", "yes");
    mpv_set_option_string(m_mpv, "input-default-bindings", "no");
    mpv_set_option_string(m_mpv, "config", "no");
    if (mpv_initialize(m_mpv) < 0)
        qFatal("k9copy: cannot initialize mpv");

    mpv_observe_property(m_mpv, 0, "percent-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "track-list/count", MPV_FORMAT_INT64);
    mpv_set_wakeup_callback(m_mpv, onWakeup, this);

    Ui_phonon.setupUi(this);
    auto *vl = new QVBoxLayout(Ui_phonon.wVideo);
    vl->setContentsMargins(0, 0, 0, 0);
    m_view = new k9MpvView(m_mpv, Ui_phonon.wVideo);
    vl->addWidget(m_view);

    m_seek = new QSlider(Qt::Horizontal, this);
    m_seek->setRange(0, 1000);
    m_volume = new QSlider(Qt::Horizontal, this);
    m_volume->setRange(0, 100);
    m_volume->setValue(100);
    m_volume->setMaximumWidth(100);

    Ui_phonon.bPlay->setIcon(QIcon::fromTheme("media-playback-start"));
    Ui_phonon.bStop->setIcon(QIcon::fromTheme("media-playback-stop"));

    auto *grid = new QGridLayout(Ui_phonon.frControl);
    grid->addWidget(Ui_phonon.bStop, 0, 0);
    grid->addWidget(Ui_phonon.bPlay, 0, 1);
    grid->addWidget(m_seek, 0, 2);
    grid->addWidget(m_volume, 0, 3);

    connect(Ui_phonon.bStop, &QAbstractButton::clicked, this, &k9Mpv::bStopClick);
    connect(Ui_phonon.bPlay, &QAbstractButton::clicked, this, &k9Mpv::bPlayClick);
    connect(m_seek, &QSlider::sliderMoved, this, &k9Mpv::seekSliderMoved);
    connect(m_volume, &QSlider::valueChanged, this, [this](int v) {
        double vol = v;
        mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    });
}

k9Mpv::~k9Mpv()
{
    // The render context must be freed before the mpv core.
    delete m_view;
    m_view = nullptr;
    mpv_terminate_destroy(m_mpv);
}

void k9Mpv::onWakeup(void *ctx)
{
    QMetaObject::invokeMethod(static_cast<k9Mpv *>(ctx), "handleEvents", Qt::QueuedConnection);
}

void k9Mpv::command(const QStringList &args)
{
    QList<QByteArray> utf8;
    QList<const char *> argv;
    for (const QString &a : args)
        utf8 << a.toUtf8();
    for (const QByteArray &a : utf8)
        argv << a.constData();
    argv << nullptr;
    mpv_command_async(m_mpv, 0, argv.data());
}

void k9Mpv::handleEvents()
{
    while (m_mpv) {
        mpv_event *ev = mpv_wait_event(m_mpv, 0);
        if (ev->event_id == MPV_EVENT_NONE)
            break;
        if (ev->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            auto *prop = static_cast<mpv_event_property *>(ev->data);
            if (qstrcmp(prop->name, "percent-pos") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                if (!m_seek->isSliderDown())
                    m_seek->setValue(int(*static_cast<double *>(prop->data) * 10));
            } else if (qstrcmp(prop->name, "track-list/count") == 0) {
                refreshTracks();
            }
        }
    }
}

void k9Mpv::refreshTracks()
{
    m_updatingTracks = true;
    Ui_phonon.cbAudio->clear();
    Ui_phonon.cbSubtitle->clear();
    Ui_phonon.cbSubtitle->addItem(tr("None"), QStringLiteral("no"));

    mpv_node list;
    if (mpv_get_property(m_mpv, "track-list", MPV_FORMAT_NODE, &list) >= 0) {
        if (list.format == MPV_FORMAT_NODE_ARRAY) {
            for (int i = 0; i < list.u.list->num; ++i) {
                mpv_node &t = list.u.list->values[i];
                if (t.format != MPV_FORMAT_NODE_MAP)
                    continue;
                QString type, lang, title;
                qint64 id = -1;
                bool selected = false;
                for (int k = 0; k < t.u.list->num; ++k) {
                    const char *key = t.u.list->keys[k];
                    mpv_node &v = t.u.list->values[k];
                    if (!qstrcmp(key, "type") && v.format == MPV_FORMAT_STRING) type = v.u.string;
                    else if (!qstrcmp(key, "lang") && v.format == MPV_FORMAT_STRING) lang = v.u.string;
                    else if (!qstrcmp(key, "title") && v.format == MPV_FORMAT_STRING) title = v.u.string;
                    else if (!qstrcmp(key, "id") && v.format == MPV_FORMAT_INT64) id = v.u.int64;
                    else if (!qstrcmp(key, "selected") && v.format == MPV_FORMAT_FLAG) selected = v.u.flag;
                }
                QString label = QStringLiteral("%1: %2").arg(id).arg(lang.isEmpty() ? (title.isEmpty() ? tr("unknown") : title) : lang);
                QComboBox *cb = (type == "audio") ? Ui_phonon.cbAudio : (type == "sub") ? Ui_phonon.cbSubtitle : nullptr;
                if (!cb)
                    continue;
                cb->addItem(label, QString::number(id));
                if (selected)
                    cb->setCurrentIndex(cb->count() - 1);
            }
        }
        mpv_free_node_contents(&list);
    }
    m_updatingTracks = false;
}

void k9Mpv::cbAudioChanged(int index)
{
    if (m_updatingTracks || index < 0)
        return;
    mpv_set_property_string(m_mpv, "aid", Ui_phonon.cbAudio->itemData(index).toString().toUtf8().constData());
}

void k9Mpv::cbSubtitleChanged(int index)
{
    if (m_updatingTracks || index < 0)
        return;
    mpv_set_property_string(m_mpv, "sid", Ui_phonon.cbSubtitle->itemData(index).toString().toUtf8().constData());
}

void k9Mpv::seekSliderMoved(int v)
{
    command({"seek", QString::number(v / 10.0, 'f', 1), "absolute-percent"});
}

void k9Mpv::open(k9DVD *_dvd, k9DVDTitle *_title, int chapter)
{
    // mpv numbers DVD titles from 0; k9copy from 1.
    mpv_set_property_string(m_mpv, "dvd-device", _dvd->getDevice().toUtf8().constData());
    m_url = QStringLiteral("dvd://%1").arg(_title->getnumTitle() - 1);
    // "start" is read when the file loads, so set it first.
    if (chapter > 1)
        mpv_set_property_string(m_mpv, "start", QStringLiteral("#%1").arg(chapter).toUtf8().constData());
    else
        mpv_set_property_string(m_mpv, "start", "none");
    command({"loadfile", m_url});
    int no = 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &no);
}

void k9Mpv::bPlayClick()
{
    if (!m_url.isEmpty())
        command({"cycle", "pause"});
}

void k9Mpv::bStopClick()
{
    command({"stop"});
    m_url.clear();
}
