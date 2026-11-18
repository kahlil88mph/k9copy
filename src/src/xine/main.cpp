/***************************************************************************
 *   Developer: Pauline123 <pauline123@users.sf.net>, (C) 2014

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


#include "config.h"
#include "k9xineplayer.h"

#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <kaboutdata.h>
#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QDBusConnection>
#include <QDBusError>
#include <QTimer>
#include <QApplication>


static const char description[] =
    I18N_NOOP("A KDE 4 Application");

static const char version[] = VERSION ;

int main(int argc, char **argv)
{
    KAboutData about("k9xineplayer", i18n("k9copy"), version, i18n(description),
                     KAboutLicense::GPL,
		 i18n("<p>(C) 2014 Pauline123 </p>" 
			  "<p>(C) 2007 Jean-Michel</p>"),
        QStringLiteral("k9copy-reloaded.sourceforge.net"),
        QString(i18n("<p>Please report bugs to:</p>")) + QStringLiteral("<p>pauline123@users.sourceforge.net</p>"));
	about.addAuthor(i18n("Pauline123"), i18n("developer"), "pauline123@users.sourceforge.net", 0);
        about.addAuthor(i18n("Jean-Michel PETIT"), i18n("developer"), "k9copy@free.fr", 0);
	QList<KAboutPerson> people = about.authors();
	about.addCredit(people[0].name(), people[0].task());
    QApplication  app(argc, argv,false);

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("wid"), i18n("wid"), QLatin1String("window id")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("vo"), i18n("vo"), QLatin1String("video output")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("ao"), i18n("ao"), QLatin1String("audio output")));
        // no session.. just start up normally
    QString wid(parser.value("wid"));
    QString vo(parser.value("vo"));  
    QString ao(parser.value("ao"));

    QObject obj;
    k9xinePlayer player(&obj);
    player.setAo(ao);
    player.setVo(vo);
    player.init(wid.toLong());

    if (!QDBusConnection::sessionBus().registerService("k9copy.free.fr.xineplayer")) {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));        
        exit(1);
    }
    QDBusConnection::sessionBus().registerObject("/", &obj);

    return app.exec();
}
