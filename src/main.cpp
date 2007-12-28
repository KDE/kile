/***************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003-2005 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3CString>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstartupinfo.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include "kiledebug.h"

#include "kile.h"
#include "kileversion.h"
#include "kiledebug.h"

bool isProject(const QString &path)
{
	return path.right(7) == ".kilepr";
}

/**
 * Complete a relative paths to absolute ones.
 * Also accepts URLs of the form file:relativepath.
 **/
QString completePath(const QString &path)
{
	QString fullpath(path);

	KILE_DEBUG() << "==complete path is " << path << endl;
	if ( path.left(1) != "/" )
	{
		if ( path.left(5) == "file:" )
		{
			KUrl url = KUrl::fromPathOrUrl(path);
			url.setFileName(completePath(url.path()));
			fullpath = url.url();
		}
		else if ( path.indexOf(QRegExp("^[a-z]+:")) == -1 )
			fullpath = QDir::currentPath() + '/' + path;
	}

	KILE_DEBUG() << "\t" << fullpath << endl;
	return fullpath;
}

int main( int argc, char ** argv )
{
	KAboutData aboutData( "kile", "Kile", ki18n("Kile"), kileFullVersion.ascii(), ki18n("KDE Integrated LaTeX Environment"), KAboutData::License_GPL,
						ki18n("by the Kile Team (2003 - 2007)"),
						KLocalizedString(),
						"http://kile.sourceforge.net");
	aboutData.addAuthor(ki18n("Michel Ludwig"), ki18n("project management/developer (scripting & bug fixes)"), "michel.ludwig@kdemail.net");
	aboutData.addAuthor(ki18n("Holger Danielsson"), ki18n("former developer"), "holger.danielsson@versanet.de");
	aboutData.addAuthor(ki18n("Jeroen Wijnhout"), ki18n("former maintainer/developer"),"Jeroen.Wijnhout@kdemail.net");
	aboutData.addAuthor(ki18n("Brachet Pascal"));

        aboutData.addCredit(ki18n("Thomas Braun"), ki18n("Lots of bug fixes!"));
	aboutData.addCredit(ki18n("Simon Martin"), ki18n("KConfig XT, various improvements and bugfixing"));
	aboutData.addCredit(ki18n("Roland Schulz"), ki18n("KatePart integration"));
	aboutData.addCredit(ki18n("Thorsten Lück"), ki18n("Log Parsing"));
	aboutData.addCredit(ki18n("Jan-Marek Glogowski"), ki18n("Find in Files dialog"));
	aboutData.addCredit(ki18n("Thomas Basset"), ki18n("Translations"));
	aboutData.addCredit(ki18n("Please consult the webpage for up to date translation credits."));
	aboutData.addCredit(ki18n("Jonathan Pechta and Federico Zenith"), ki18n("Documentation"));

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineOptions options;
	options.add("line <line>", ki18n("Jump to line"), "0");
	options.add("new", ki18n("Start a new Kile mainwindow"), "0");
	options.add("+[file]", ki18n("File to open"), "0");
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	bool running = false;

#ifdef __GNUC__
#warning Comment the DCOP stuff out for now!
#endif
//FIXME: port for KDE4
/*
	DCOPClient *client=0L;
	Q3CString appID = "";
	client  = new DCOPClient ();
	client->attach();
	QCStringList apps = client->registeredApplications();

	for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
	{
		if  ((*it).contains ("kile") > 0)
		{
			appID = (*it);
			running = true;
			break;
		}
	}
*/
	if(!running || args->isSet("new")) {
//FIXME: port for KDE4
// 		a.dcopClient()->registerAs("kile", false);
		bool restore = (args->count() == 0);
		Kile app(restore);

		for(int i = 0; i < args->count(); ++i) {
			if ( isProject(args->arg(i)) )
				app.openProject(completePath(QFile::decodeName(args->arg(i))));
			else
				app.openDocument(completePath(QFile::decodeName(args->arg(i))));
		}

		QString line = args->getOption("line");
		if(line != "0") {
			app.setLine(line);
		}

		args->clear();
		return app.exec();
	}
//FIXME: port for KDE4
/*
	else
	{
		for ( int i = 0; i < args->count(); ++i )
		{
			if ( isProject(args->arg(i)) )
				client->send (appID, "Kile", "openProject(QString)", completePath(QFile::decodeName(args->arg(i))));
			else
				client->send (appID, "Kile", "openDocument(QString)", completePath(QFile::decodeName(args->arg(i))));
		}

		QString line = args->getOption("line");
		if (line != "0") client->send (appID, "Kile", "setLine(QString)", line);

		KStartupInfo::appStarted();
		QByteArray empty;
		client->send (appID, "Kile", "setActive()", empty);
	}
*/
	return 0;
}

