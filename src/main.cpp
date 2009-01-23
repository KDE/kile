/********************************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet
                               2003 - 2005 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDir>
#include <QFile>
#include <QtDBus>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KLocale>
#include <KStartupInfo>
#include <KUrl>

#include "kile.h"
#include "kileversion.h"
#include "kiledebug.h"

bool isProject(const QString &path)
{
	return path.endsWith(".kilepr");
}

/*
 * Complete a relative paths to absolute ones.
 * Also accepts URLs of the form file:relativepath.
*/
QString completePath(const QString &path)
{
	QString fullpath(path);

	KILE_DEBUG() << "==complete path is " << path;
	if( QDir::isRelativePath(path) ) {
		if(path.startsWith("file:")) {
			KUrl url = KUrl::fromPathOrUrl(path);
			url.setFileName(completePath(url.path()));
			fullpath = url.url();
		}
		else if(path.indexOf(QRegExp("^[a-z]+:")) == -1) {
			fullpath = QDir::currentPath() + QDir::separator() + path;
		}
	}

	KILE_DEBUG() << "==fullpath=" << fullpath;
	return fullpath;
}

int main( int argc, char ** argv )
{
	KAboutData aboutData( "kile", QByteArray(), ki18n("Kile"), kileFullVersion.toAscii(),
				ki18n("KDE Integrated LaTeX Environment"),
				KAboutData::License_GPL,
				ki18n("by the Kile Team (2003 - 2009)"),
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

     	QDBusConnection dbus = QDBusConnection::sessionBus();
	running = dbus.interface()->isServiceRegistered("net.sourceforge.kile");

	if( !running  || args->isSet("new") ){

		bool restore = (args->count() == 0);
		Kile app(restore);

		for(int i = 0; i < args->count(); ++i) {
			//FIXME: check whether this can be used to open Urls
			if(isProject(args->arg(i))) {
				app.openProject(completePath(args->arg(i)));
			}
			else {
				app.openDocument(completePath(args->arg(i)));
			}
		}

		QString line = args->getOption("line");
		if(line != "0") {
			app.setLine(line);
		}

		args->clear();
		return app.exec();
	}
	else {
		QDBusInterface *interface = new QDBusInterface("net.sourceforge.kile","/main","net.sourceforge.kile.main");

		for ( int i = 0; i < args->count(); ++i ) {
			QString path = args->arg(i);
			path = completePath(path);

			if ( isProject(args->arg(i)) ){
                         	interface->call("openProject",path);
			}
			else {
				interface->call("openDocument",path);
			}
		}

		QString line = args->getOption("line");
		if (line != "0") {
			interface->call("setLine", line);
		}

		KStartupInfo::appStarted();
		interface->call("setActive"); 
		delete interface;
	}
	return 0;
}

