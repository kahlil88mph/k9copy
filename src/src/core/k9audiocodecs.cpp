//
// C++ Implementation: k9audiocodecs
//
// Description:
//
// Developer: Pauline123 <pauline123@users.sf.net>, (C) 2014
//
// Author: Jean-Michel PETIT <k9copy@free.fr>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "k9audiocodecs.h"

#include <QStringList>

#include <KConfigGroup>

class _k9AudioCodec {
public:
    _k9AudioCodec():name(""),options(""),encoder(""),extension("") {}
    _k9AudioCodec(QString _name,QString _options,QString _encoder,QString _extension="") {
        name=_name;
        options=_options;
	encoder=_encoder;
	extension=_extension;
    }
    QString name;
    QString options;
    QString encoder;
    QString extension;
};


k9AudioCodecs::k9AudioCodecs(QObject *parent)
        : QObject(parent) {
    m_config=new k9Config();

    KConfig *config= m_config->getConfig();
    KConfigGroup group=config->group("codecs");
    int nbAudio=group.readEntry("AudioCount",0);


    //adds default codecs
    if (nbAudio==0) {
        reset();
        m_config=new k9Config();
    }

    for (int cpt=0;cpt <nbAudio;cpt++) {
	group=config->group(QString("audiocodec%1").arg(cpt));
	m_codecs[cpt] = _k9AudioCodec(group.readEntry("name"),group.readEntry("opt",""),group.readEntry("encoder","ffmpeg"),group.readEntry("extension"));

    }

    delete m_config;

}

void k9AudioCodecs::reset() {
    // ffmpeg presets; the extension is used when extracting audio only.
    m_codecs.clear();
    m_codecs[0]=_k9AudioCodec("AAC","-c:a aac -b:a $AUDBRk","ffmpeg",".m4a");
    m_codecs[1]=_k9AudioCodec("MP3 (LAME)","-c:a libmp3lame -b:a $AUDBRk","ffmpeg",".mp3");
    m_codecs[2]=_k9AudioCodec("Opus","-c:a libopus -b:a $AUDBRk","ffmpeg",".opus");
    m_codecs[3]=_k9AudioCodec("Vorbis","-c:a libvorbis -b:a $AUDBRk","ffmpeg",".ogg");
    m_codecs[4]=_k9AudioCodec("AC-3","-c:a ac3 -b:a $AUDBRk","ffmpeg",".ac3");
    m_codecs[5]=_k9AudioCodec("MP2","-c:a mp2 -b:a $AUDBRk","ffmpeg",".mp2");
    m_codecs[6]=_k9AudioCodec("FLAC (lossless)","-c:a flac","ffmpeg",".flac");
    m_codecs[7]=_k9AudioCodec("copy (no re-encode)","-c:a copy","ffmpeg",".mka");

    save();
}

void k9AudioCodecs::save() {
    m_config=new k9Config();

    KConfig *config= m_config->getConfig();
    KConfigGroup group;

    
    int cpt=0;
    for (QMap<int,_k9AudioCodec>::iterator i=m_codecs.begin();i!=m_codecs.end();++i) {
	group=config->group(QString("audiocodec%1").arg(cpt));
	group.writeEntry("name",i.value().name);
        group.writeEntry("opt", i.value().options);
	group.writeEntry("encoder", i.value().encoder);
        group.writeEntry("extension",i.value().extension);

        cpt++;
    }
    group=config->group("codecs");
    group.writeEntry("AudioCount",cpt);

    m_config->save();

    delete m_config;
}




int k9AudioCodecs::count() {
    return m_codecs.count();
}

void k9AudioCodecs::setOptions(int _num,QString _value) {
    m_codecs[_num].options=_value;
}


void k9AudioCodecs::setEncoder(int _num,QString _value) {
   m_codecs[_num].encoder=_value;
}

void k9AudioCodecs::setCodecName(int _num,QString _value) {
    m_codecs[_num].name=_value;
}

void k9AudioCodecs::setExtension(int _num,QString _value) {
    m_codecs[_num].extension=_value;
}

QString k9AudioCodecs::getOptions(int _num) {
    return m_codecs[_num].options;
}

QString k9AudioCodecs::getCodecName(int _num) {
    return m_codecs[_num].name;
}

int k9AudioCodecs::getAbsCodecNum(int _num,const QString &_encoder) {
   k9AudioCodecs a(0);
   int nb=a.count();
   int num=0;
   for (int i=0;i<nb;i++) {
      if (a.getEncoder(i) == _encoder) {
	  if (_num==num)
	     return i;
	  else num++;

      }
   }
   return 0;
}



QString k9AudioCodecs::getEncoder(int _num) {
    return m_codecs[_num].encoder=="" ? QString("ffmpeg"): m_codecs[_num].encoder;

}

QString k9AudioCodecs::getExtension(int _num) {
    return m_codecs[_num].extension;

}

void k9AudioCodecs::remove(int _num) {
    int nb=count();
    if (nb>0) {
        for (int i=_num;i<nb-1;i++) {
            m_codecs[i]=m_codecs[i+1];
        }
        m_codecs.remove(nb-1);
    }
}

k9AudioCodecs::~k9AudioCodecs() {
}

