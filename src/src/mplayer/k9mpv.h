// k9mpv: DVD title preview using libmpv's OpenGL render API.
// Replaces the former xine (k9xineplayer), Phonon and mplayer backends.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9MPV_H
#define K9MPV_H

#include "k9common.h"
#include "ui_phonon.h"
#include <QOpenGLWidget>
#include <QWidget>

struct mpv_handle;
struct mpv_render_context;
class k9DVD;
class k9DVDTitle;
class QSlider;

class k9MpvView : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit k9MpvView(mpv_handle *mpv, QWidget *parent = nullptr);
    ~k9MpvView() override;

protected:
    void initializeGL() override;
    void paintGL() override;

private Q_SLOTS:
    void maybeUpdate();

private:
    static void onUpdate(void *ctx);
    mpv_handle *m_mpv;
    mpv_render_context *m_ctx = nullptr;
};

class k9Mpv : public QWidget
{
    Q_OBJECT
public:
    explicit k9Mpv(QWidget *parent = nullptr);
    ~k9Mpv() override;

public Q_SLOTS:
    void bStopClick();
    void bPlayClick();
    void open(k9DVD *_dvd, k9DVDTitle *_title, int chapter);
    void cbSubtitleChanged(int);
    void cbAudioChanged(int);

private Q_SLOTS:
    void handleEvents();
    void seekSliderMoved(int);

private:
    static void onWakeup(void *ctx);
    void command(const QStringList &args);
    void refreshTracks();

    Ui::phonon Ui_phonon;
    mpv_handle *m_mpv = nullptr;
    k9MpvView *m_view = nullptr;
    QSlider *m_seek = nullptr;
    QSlider *m_volume = nullptr;
    bool m_updatingTracks = false;
    QString m_url;
};

#endif
