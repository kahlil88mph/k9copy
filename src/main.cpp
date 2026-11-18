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

#include "dvdread/dvd_reader.h"
#include "k9copy.h"
#include "k9common.h"



#include "k9assistant.h"
#include "k9dialogs.h"

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QDBusConnection>
#include <QDBusError>
#include <QTimer>
#include <QApplication>
#include <QIcon>
#include "k9i18n.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "k9batch.h"

static const char description[] =
        I18N_NOOP("A DVD backup tool");

static const char version[] = VERSION ;

int main(int argc, char **argv) {
    k9batch batch;
    if (batch.exec(argc,argv)) 
      return 1;
    QApplication app(argc, argv);
    k9i18n::init();
    QApplication::setApplicationName(QStringLiteral("k9copy"));
    QApplication::setApplicationDisplayName(i18n("K9copy"));
    QApplication::setApplicationVersion(QStringLiteral(VERSION));
    QApplication::setOrganizationName(QStringLiteral("k9copy"));
    QApplication::setDesktopFileName(QStringLiteral("k9copy"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("k9copy")));

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n(description));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("input"), i18n("input device"), QLatin1String("device")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("output"), i18n("output device"), QLatin1String("device")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("assistant"), i18n("the k9copy backup assistant")));
    parser.process(app);
    k9Tools::setMainThread();

    {
        // no session.. just start up normally
        QString InputOptionArg( parser.value("input"));
        QString OutputOptionArg( parser.value("output"));
        k9Config::checkCodecs();
        if (!parser.isSet("assistant")) {
            k9Copy  *widget = new k9Copy;
            if (InputOptionArg !="") {
		widget->setInput( InputOptionArg);
		widget->fileOpen();
            }
            if (OutputOptionArg !="")
		widget->setOutput( OutputOptionArg);

            if ((InputOptionArg !="") && (OutputOptionArg!=""))
		widget->clone( InputOptionArg,OutputOptionArg);
            widget->show();
        } else {
            k9Assistant *ast=k9Assistant::createAssistant();
            if (InputOptionArg !="")
                ast->setPath(InputOptionArg);
            k9Dialogs::setMainWidget(ast);
            QTimer::singleShot (10, ast, SLOT (run ()));

	}	

        int res=app.exec();
        k9Config config;
        if (config.getPrefDelTmpFiles())
            k9Tools::clearOutput();

        return res;
    }
}
