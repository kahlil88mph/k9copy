//
// C++ Implementation: k9videocodecs
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
#include "k9videocodecs.h"
#include <QStringList>

#include <KConfigGroup>

class _k9VideoCodec
{
public:
    _k9VideoCodec():name(""),fourcc(""),optPass1(""),optPass2(""),optOnePass(""),encoder(""){}
   _k9VideoCodec(QString _name,QString _fourcc,QString _optOnePass,QString _optPass1,QString _optPass2,QString _encoder) {
      name=_name;
      fourcc=_fourcc;
      optOnePass=_optOnePass;
      optPass1=_optPass1;
      optPass2=_optPass2; 
      encoder=_encoder;
   }
   QString name;
   QString fourcc;
   QString optPass1;
   QString optPass2;
   QString optOnePass;
   QString encoder;
}; 


k9VideoCodecs::k9VideoCodecs(QObject *parent)
 : QObject(parent)
{
   m_config=new k9Config();
   KConfig *config= m_config->getConfig();
   KConfigGroup group=config->group("codecs");
   int nbVideo=group.readEntry("VideoCount",0);




   bool bReset;
   bReset=(nbVideo==0);
   //adds default codecs
   if (bReset) {
      reset();
      m_config=new k9Config();
   }  
   for (int cpt=0;cpt <nbVideo;cpt++) {
        group=config->group(QString("videocodec%1").arg(cpt));
        m_codecs[cpt] = _k9VideoCodec(group.readEntry("name"), group.readEntry("fourcc",""),group.readEntry("opt1",""),group.readEntry("opt2",""),group.readEntry("opt3",""),group.readEntry("encoder","ffmpeg"));

   }


   delete m_config;

}

void k9VideoCodecs::reset() {
    // ffmpeg presets. Placeholders are filled by k9MP4Enc::replaceParams().
    // Filter order: deinterlace (only frames flagged interlaced), resize,
    // crop (coordinates already scaled to the resized frame), display aspect.
    const QString vf = QStringLiteral("-vf bwdif=deint=interlaced,scale=$WIDTH:$HEIGHT,crop=$CROPWIDTH:$CROPHEIGHT:$CROPLEFT:$CROPTOP,setdar=$ASPECT");
    const QString p1 = QStringLiteral(" -pass 1 -passlogfile $PASSLOGFILE");
    const QString p2 = QStringLiteral(" -pass 2 -passlogfile $PASSLOGFILE");
    auto add = [this](const QString &name, const QString &fourcc, const QString &one,
                      const QString &pass1, const QString &pass2) {
        m_codecs[m_codecs.count()] = _k9VideoCodec(name, fourcc, one, pass1, pass2, QStringLiteral("ffmpeg"));
    };
    auto std3 = [&](const QString &name, const QString &fourcc, const QString &opts) {
        add(name, fourcc, opts + " " + vf, opts + " " + vf + p1, opts + " " + vf + p2);
    };

    m_codecs.clear();
    std3("H.264 (x264)", "", "-c:v libx264 -preset medium -profile:v high -b:v $VIDBRk");
    add("H.265 / HEVC (x265)", "hvc1",
        "-c:v libx265 -preset medium -b:v $VIDBRk " + vf,
        "-c:v libx265 -preset medium -b:v $VIDBRk -x265-params pass=1:stats=$PASSLOGFILE " + vf,
        "-c:v libx265 -preset medium -b:v $VIDBRk -x265-params pass=2:stats=$PASSLOGFILE " + vf);
    // SVT-AV1 is single pass here; a "2-pass" run just encodes twice.
    {
        const QString av1 = "-c:v libsvtav1 -preset 8 -b:v $VIDBRk " + vf;
        add("AV1 (SVT-AV1)", "", av1, av1, av1);
    }
    std3("VP9 (libvpx)", "", "-c:v libvpx-vp9 -b:v $VIDBRk -row-mt 1 -deadline good -cpu-used 2");
    std3("Xvid (libxvid)", "XVID", "-c:v libxvid -b:v $VIDBRk");
    std3("MPEG-4 Part 2 (DivX compatible)", "DIVX", "-c:v mpeg4 -b:v $VIDBRk -mbd rd -trellis 2 -flags +mv4+aic");
    std3("MPEG-2", "", "-c:v mpeg2video -b:v $VIDBRk -maxrate 9800k -bufsize 1835k");
    add("copy (no re-encode)", "", "-c:v copy", "-c:v copy", "-c:v copy");

    save();
}

void k9VideoCodecs::save() {
   m_config=new k9Config();

    KConfig *config= m_config->getConfig();
   KConfigGroup group=config->group("codecs");

   

    int cpt=0;
    for (QMap<int,_k9VideoCodec>::iterator i=m_codecs.begin();i!=m_codecs.end();++i) {
	group=config->group(QString("videocodec%1").arg(cpt));
        group.writeEntry("name", i.value().name);
        group.writeEntry("fourcc", i.value().fourcc);
        group.writeEntry("opt1",i.value().optOnePass);
        group.writeEntry("opt2", i.value().optPass1);
        group.writeEntry("opt3",i.value().optPass2);
	group.writeEntry("encoder",i.value().encoder);

        cpt++;
    }

    group=config->group("codecs");
    group.writeEntry("VideoCount",cpt);

    m_config->save();
    delete m_config;
}




int k9VideoCodecs::count() {
   return m_codecs.count();
}

void k9VideoCodecs::setFourcc(int _num,QString _value) {
    m_codecs[_num].fourcc=_value;
}
void k9VideoCodecs::setOptions0(int _num,QString _value) {
    m_codecs[_num].optOnePass=_value;
}
void k9VideoCodecs::setOptions1(int _num,QString _value) {
    m_codecs[_num].optPass1=_value;
}
void k9VideoCodecs::setOptions2(int _num,QString _value) {
    m_codecs[_num].optPass2=_value;
}

void k9VideoCodecs::setCodecName(int _num,QString _value) {
    m_codecs[_num].name=_value;
}
void k9VideoCodecs::setEncoder(int _num,QString _value) {
   m_codecs[_num].encoder=_value;
}
QString k9VideoCodecs::getFourcc(int _num) {
   return m_codecs[_num].fourcc;
}

QString k9VideoCodecs::getOptions0(int _num) {
   return m_codecs[_num].optOnePass;
}

QString k9VideoCodecs::getOptions1(int _num) {
   return m_codecs[_num].optPass1;
}

QString k9VideoCodecs::getOptions2(int _num) {
   return m_codecs[_num].optPass2;
}
QString k9VideoCodecs::getCodecName(int _num) {
   return m_codecs[_num].name;
}

int k9VideoCodecs::getAbsCodecNum(int _num,const QString &_encoder) {
   k9VideoCodecs v(0);
   int nb=v.count();
   int num=0;
   for (int i=0;i<nb;i++) {
      if (v.getEncoder(i) == _encoder) {
	  if (_num==num)
	     return i;
	  else num++;

      }
   }
   return 0;
}


QString k9VideoCodecs::getEncoder(int _num) {
    return m_codecs[_num].encoder=="" ? QString("ffmpeg"): m_codecs[_num].encoder;

}

void k9VideoCodecs::remove(int _num) {
    int nb=count();
    if (nb>0) {
        for(int i=_num;i<nb-1;i++) {
            m_codecs[i]=m_codecs[i+1];
        }
        m_codecs.remove(nb-1);
    }
}

k9VideoCodecs::~k9VideoCodecs()
{

}

