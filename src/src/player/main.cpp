/***************************************************************************
* // Developer: Pauline123 <pauline123@users.sf.net>, (C) 2014

 *   Copyright (C) 2007 by Jean-Michel   *
 *   k9copy@free.fr             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//PM DVDNAV #include "src/dvdnav/common.h"
#include "dvdread/dvd_reader.h"
#include "k9copy.h"
#include "k9play.h"
#include <kaboutdata.h>
#include "k9i18n.h"


#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QDBusConnection>
#include <QDBusError>
#include <QTimer>
#include <QCommandLineParser>
#include <QCommandLineOption>

static const char description[] =
    I18N_NOOP("A KDE 4 Application");

static const char version[] = VERSION ;

int main(int argc, char **argv) {

    KAboutData about("k9player", i18n("k9copy"), version, i18n(description),
                     KAboutLicense::GPL,
		 i18n("<p>(C) 2014 Pauline123 </p>" 
			  "<p>(C) 2007 Jean-Michel</p>"),
        QStringLiteral("k9copy-reloaded.sourceforge.net"),
        QString(i18n("<p>Please report bugs to:</p>")) + QStringLiteral("<p>pauline123@users.sourceforge.net</p>"));
	about.addAuthor(i18n("Pauline123"), i18n("developer"), "pauline123@users.sourceforge.net", 0);
        about.addAuthor(i18n("Jean-Michel PETIT"), i18n("developer"), "k9copy@free.fr", 0);
	QList<KAboutPerson> people = about.authors();
	about.addCredit(people[0].name(), people[0].task());

     QApplication app(argc, argv);  
    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("gui"), i18n("no gui")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("input"), i18n("input device"), QLatin1String("device")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("output"), i18n("output device"), QLatin1String("device")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("dvdtitle"), i18n("title to play"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("play"), i18n("play title to stdout")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("startsector"), i18n("start sector"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("endsector"), i18n("end sector"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("audiofilter"), i18n("list of audio streams"), QLatin1String("number,number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("subpicturefilter"), i18n("list of spu streams"), QLatin1String("number,number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("vampsfactor"), i18n("shrink factor"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("ffactor"), i18n("shrink factor forced")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("inputsize"), i18n("size of the cell to be copied"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("totalsize"), i18n("total size of selected titles"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("chaptersize"), i18n("size of the chapters being copied"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("chapterlist"), i18n("selected chapters"), QLatin1String("list")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("dvdsize"), i18n("new dvd size"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("chapter"), i18n("selected chapter"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("cell"), i18n("cell number in selected chapter"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("inject"), i18n("status file name"), QLatin1String("filename")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("initstatus"), i18n("initialize status file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("continue"), i18n("continue playing from last sector")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("firstpass"), i18n("don't save status at end")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("usecache"), i18n("save cell in a temporary file before encoding")));

    QString TitleNumber(parser.value("dvdtitle"));
    QString InputOptionArg( parser.value("input"));
    QString OutputOptionArg( parser.value("output"));
    QString startSectorArg(parser.value("startsector"));
    QString endSectorArg(parser.value("endsector"));
    QString audioFilterArg(parser.value("audiofilter"));
    QString subpictureFilterArg(parser.value("subpicturefilter"));
    QString vampsFactorArg(parser.value("vampsfactor"));
    QString inputSizeArg(parser.value("inputsize"));
    QString chapterArg(parser.value("chapter"));
    QString cellArg(parser.value("cell"));
    QString injectArg(parser.value("inject"));
    QString totalSizeArg(parser.value("totalsize"));
    QString dvdSizeArg(parser.value("dvdsize"));
    QString chapterSizeArg(parser.value("chaptersize"));
    QString chapterListArg(parser.value("chapterlist"));

    bool gui=parser.isSet("gui");
    k9Tools::setBatchCopy(!gui);
    bool ffactor=parser.isSet("ffactor");
    bool play= parser.isSet("play");
    if (play) {
        QObject obj;
        k9play player(&obj);
        player.setinitStatus( parser.isSet("initstatus"));
        player.setcontinue( parser.isSet("continue"));
        player.setfirstPass(parser.isSet("firstpass"));
        player.setuseCache(parser.isSet("usecache"));
        player.setDevice(InputOptionArg);
        player.setTitle(TitleNumber.toInt());
        player.setstartSector(startSectorArg);
        player.setendSector(endSectorArg );
        player.setaudioFilter( audioFilterArg);
        player.setsubpictureFilter( subpictureFilterArg);
        player.setvampsFactor( vampsFactorArg);
        player.setinputSize( inputSizeArg);
        player.setchapter(chapterArg);
        player.setcell(cellArg);
        player.setinject(injectArg);
        player.settotalSize(totalSizeArg);
        player.setdvdSize(dvdSizeArg);
        player.setchapterSize( chapterSizeArg);
        player.setforcedFactor(ffactor);
        player.setchapterList( chapterListArg);
	player.setexitOnEnd(true);
        player.setMainThread(true);
        QDBusConnection::sessionBus().registerObject("/", &obj);
        if (!QDBusConnection::sessionBus().registerService("k9copy.free.fr.player")) {
            fprintf(stderr, "%s\n",
                    qPrintable(QDBusConnection::sessionBus().lastError().message()));
            //  exit(1);
        }

        //QTimer::singleShot (100, &player, SLOT (execute ()));
        player.execute();
        return app.exec();

        //return 0;
    }


    return app.exec();
}
