//
// C++ Implementation: k9avidecode
//
// Author: Jean-Michel PETIT <k9copy@free.fr>, (C) 2007
// Ported to the modern FFmpeg API (send/receive, AVCodecParameters), 2026.
// Libraries are now linked directly instead of dlopen()ed.
//
// Copyright: See COPYING file that comes with this distribution
//
#include "k9avidecode.h"
#include "k9common.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

k9AviDecode::k9AviDecode(QObject *parent, const char *)
    : QObject(parent)
{
}

k9AviDecode::~k9AviDecode()
{
    close();
}

bool k9AviDecode::open(const QString &_fileName)
{
    m_error.clear();
    close();

    if (avformat_open_input(&m_FormatCtx, _fileName.toUtf8().constData(), nullptr, nullptr) != 0) {
        m_error = i18n("Couldn't open the file %1", _fileName);
        m_FormatCtx = nullptr;
        return false;
    }
    if (avformat_find_stream_info(m_FormatCtx, nullptr) < 0) {
        m_error = i18n("Couldn't find stream information");
        avformat_close_input(&m_FormatCtx);
        return false;
    }

    const AVCodec *codec = nullptr;
    m_videoStream = av_find_best_stream(m_FormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (m_videoStream < 0) {
        m_error = (m_videoStream == AVERROR_DECODER_NOT_FOUND)
                      ? i18n("Unsupported codec")
                      : i18n("The file doesn't contain any video stream");
        avformat_close_input(&m_FormatCtx);
        return false;
    }

    m_CodecCtx = avcodec_alloc_context3(codec);
    if (!m_CodecCtx
        || avcodec_parameters_to_context(m_CodecCtx, m_FormatCtx->streams[m_videoStream]->codecpar) < 0
        || avcodec_open2(m_CodecCtx, codec, nullptr) < 0) {
        m_error = i18n("Could'nt open the codec");
        avcodec_free_context(&m_CodecCtx);
        avformat_close_input(&m_FormatCtx);
        return false;
    }

    m_Frame = av_frame_alloc();
    m_Packet = av_packet_alloc();
    if (!m_Frame || !m_Packet) {
        m_error = i18n("Unable to allocate memory for frames");
        m_opened = true; // let close() release what was allocated
        close();
        return false;
    }

    int64_t start = (m_FormatCtx->start_time == AV_NOPTS_VALUE) ? 0 : m_FormatCtx->start_time;
    m_duration = (double)(calcDuration() + start) / AV_TIME_BASE;
    m_opened = true;
    m_fileName = _fileName;
    return true;
}

int64_t k9AviDecode::calcDuration()
{
    if (m_FormatCtx->duration > 0)
        return m_FormatCtx->duration;

    // No container duration: sum packet durations of the video stream.
    AVStream *st = m_FormatCtx->streams[m_videoStream];
    int64_t duration = 0;
    av_seek_frame(m_FormatCtx, m_videoStream, 0, AVSEEK_FLAG_BACKWARD);
    while (av_read_frame(m_FormatCtx, m_Packet) >= 0) {
        if (m_Packet->stream_index == m_videoStream)
            duration += m_Packet->duration;
        av_packet_unref(m_Packet);
    }
    return av_rescale_q(duration, st->time_base, AV_TIME_BASE_Q);
}

void k9AviDecode::seek(double _seconds)
{
    if (!m_opened)
        return;
    AVRational time_base = m_FormatCtx->streams[m_videoStream]->time_base;
    int64_t fspos = av_rescale_q((int64_t)(_seconds * AV_TIME_BASE), AV_TIME_BASE_Q, time_base);
    av_seek_frame(m_FormatCtx, m_videoStream, fspos, AVSEEK_FLAG_BACKWARD);
}

void k9AviDecode::readFrame(double _seconds)
{
    if (!m_opened)
        return;
    AVRational time_base = m_FormatCtx->streams[m_videoStream]->time_base;
    int64_t fspos = av_rescale_q((int64_t)(_seconds * AV_TIME_BASE), AV_TIME_BASE_Q, time_base);
    av_seek_frame(m_FormatCtx, m_videoStream, fspos, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(m_CodecCtx);

    bool found = false;
    while (!found && av_read_frame(m_FormatCtx, m_Packet) >= 0) {
        if (m_Packet->stream_index == m_videoStream
            && avcodec_send_packet(m_CodecCtx, m_Packet) >= 0) {
            while (!found && avcodec_receive_frame(m_CodecCtx, m_Frame) >= 0) {
                int64_t ts = m_Frame->best_effort_timestamp;
                if (ts == AV_NOPTS_VALUE)
                    ts = m_Frame->pts;
                if (ts == AV_NOPTS_VALUE || ts >= fspos) {
                    emitFrame(m_Frame);
                    found = true;
                }
                av_frame_unref(m_Frame);
            }
        }
        av_packet_unref(m_Packet);
    }
}

void k9AviDecode::emitFrame(const AVFrame *frame)
{
    const int w = frame->width;
    const int h = frame->height;
    m_Sws = sws_getCachedContext(m_Sws, w, h, (AVPixelFormat)frame->format,
                                 w, h, AV_PIX_FMT_RGB24, SWS_BICUBIC,
                                 nullptr, nullptr, nullptr);
    if (!m_Sws)
        return;

    QImage img(w, h, QImage::Format_RGB888);
    uint8_t *dst[4] = { img.bits(), nullptr, nullptr, nullptr };
    int dstStride[4] = { (int)img.bytesPerLine(), 0, 0, 0 };
    sws_scale(m_Sws, frame->data, frame->linesize, 0, h, dst, dstStride);
    Q_EMIT drawFrame(img);
}

void k9AviDecode::close()
{
    sws_freeContext(m_Sws);
    m_Sws = nullptr;
    av_packet_free(&m_Packet);
    av_frame_free(&m_Frame);
    avcodec_free_context(&m_CodecCtx);
    if (m_FormatCtx)
        avformat_close_input(&m_FormatCtx);
    m_opened = false;
}

double k9AviDecode::getDuration() const { return m_duration; }
bool k9AviDecode::opened() const { return m_opened; }
QString k9AviDecode::getFileName() const { return m_fileName; }
QString k9AviDecode::getError() const { return m_error; }
