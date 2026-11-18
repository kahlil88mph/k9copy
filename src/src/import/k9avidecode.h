//
// C++ Interface: k9avidecode
//
// Author: Jean-Michel PETIT <k9copy@free.fr>, (C) 2007
// Ported to the modern FFmpeg API (send/receive, AVCodecParameters), 2026.
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef K9AVIDECODE_H
#define K9AVIDECODE_H

#include "k9common.h"
#include <QObject>
#include <QImage>
#include <QString>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

struct SwsContext;

class k9AviDecode : public QObject
{
    Q_OBJECT
public:
    k9AviDecode(QObject *parent = nullptr, const char *name = nullptr);
    ~k9AviDecode();

    bool open(const QString &_fileName);
    void readFrame(double _seconds);
    void seek(double _seconds);
    void close();
    double getDuration() const;
    bool opened() const;
    QString getFileName() const;
    QString getError() const;

private:
    AVFormatContext *m_FormatCtx = nullptr;
    AVCodecContext *m_CodecCtx = nullptr;
    AVFrame *m_Frame = nullptr;
    AVPacket *m_Packet = nullptr;
    SwsContext *m_Sws = nullptr;
    int m_videoStream = -1;
    bool m_opened = false;
    double m_duration = 0;
    QString m_fileName;
    QString m_error;

    int64_t calcDuration();
    void emitFrame(const AVFrame *frame);

Q_SIGNALS:
    void drawFrame(QImage);
};

#endif
