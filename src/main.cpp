/********************************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet
                               2003 - 2005 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2012 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include <QFileInfo>
#include <QTextCodec>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KLocale>
#include <KStartupInfo>
#include <KUrl>
#include <KStandardDirs>
#include <KEncodingProber>

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
QString completePath(const QString &path, const QString& currentPath)
{
	QString fullpath(path);

	KILE_DEBUG() << "==complete path is " << path;
	if(QDir::isRelativePath(path)) {
		if(path.startsWith("file:")) {
			KUrl url(path);
			url.setFileName(completePath(url.toLocalFile(), currentPath));
			fullpath = url.url();
		}
		else if(path.indexOf(QRegExp("^[a-z]+:")) == -1) {
			fullpath = currentPath + QDir::separator() + path;
		}
	}

	KILE_DEBUG() << "==fullpath=" << fullpath;
	return fullpath;
}

QString readDataFromStdin()
{
	KILE_DEBUG();

	QByteArray fileData;
	QFile qstdin;
	QTextCodec *codec = NULL;

	qstdin.open( stdin, QIODevice::ReadOnly );
	fileData = qstdin.readAll();
	qstdin.close();

	KTempDir *tempDir = new KTempDir(KStandardDirs::locateLocal("tmp", "kile-stdin"));
	QString tempFileName = QFileInfo(tempDir->name(), i18n("StandardInput.tex")).absoluteFilePath();
	KILE_DEBUG() << "tempFile is " << tempFileName;

	QFile tempFile(tempFileName);
	if(!tempFile.open(QIODevice::WriteOnly)) {
		return QString();
	}

	QTextStream stream(&tempFile);

	KEncodingProber prober(KEncodingProber::Universal);
 	KEncodingProber::ProberState state = prober.feed(fileData);
	KILE_DEBUG() << "KEncodingProber::state " << state;
	KILE_DEBUG() << "KEncodingProber::prober.confidence() " << prober.confidence();
	KILE_DEBUG() << "KEncodingProber::encoding " << prober.encodingName();

	codec = QTextCodec::codecForName(prober.encodingName());
	if(codec){
		stream.setCodec(codec);
	}

	stream << fileData;
	tempFile.close();

	return tempFileName;
}



int main( int argc, char ** argv )
{
	KAboutData aboutData( "kile", QByteArray(), ki18n("Kile"), kileFullVersion.toAscii(),
				ki18n("KDE Integrated LaTeX Environment"),
				KAboutData::License_GPL,
				ki18n("by the Kile Team (2003 - 2012)"),
				KLocalizedString(),
				"http://kile.sourceforge.net");
	aboutData.addAuthor(ki18n("Michel Ludwig"), ki18n("Project Management/Developer"), "michel.ludwig@kdemail.net");
        aboutData.addAuthor(ki18n("Thomas Braun"), ki18n("Former Developer"), "thomas.braun@virtuell-zuhause.de");
	aboutData.addAuthor(ki18n("Holger Danielsson"), ki18n("Former Developer"), "holger.danielsson@versanet.de");
	aboutData.addAuthor(ki18n("Jeroen Wijnhout"), ki18n("Former Maintainer/Developer"),"Jeroen.Wijnhout@kdemail.net");
	aboutData.addAuthor(ki18n("Brachet Pascal"));

	aboutData.addCredit(ki18n("Andrius Štikonas"), ki18n("Migration from Subversion to Git"), "stikonas@gmail.com");
	aboutData.addCredit(ki18n("Simon Martin"), ki18n("KConfig XT, Various Improvements and Bug-Fixing"));
	aboutData.addCredit(ki18n("Roland Schulz"), ki18n("KatePart Integration"));
	aboutData.addCredit(ki18n("Thorsten Lück"), ki18n("Log Parsing"));
	aboutData.addCredit(ki18n("Jan-Marek Glogowski"), ki18n("Find-in-Files Dialog"));
	aboutData.addCredit(ki18n("Jonathan Pechta"), ki18n("Documentation"));
	aboutData.addCredit(ki18n("Federico Zenith"), ki18n("Documentation"));

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineOptions options;
	options.add("line <line>", ki18n("Jump to line"));
	options.add("new", ki18n("Start a new Kile mainwindow"));
	options.add("+[files]", ki18n("Files to open"));
	options.add("+-", ki18n("Read from stdin"));

	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	bool running = false;

	// we save the current path here to avoid problems when it's (erroneously) changed later
	// (for instance, when a new KonsoleWidget is created, see #301808)
	const QString currentPath = QDir::currentPath();

	// this has to go before the DBus connection
	KApplication app;

	QDBusConnection dbus = QDBusConnection::sessionBus();
	running = dbus.interface()->isServiceRegistered("net.sourceforge.kile");

	if(!running  || args->isSet("new")) {
		bool restore = (args->count() == 0);
		Kile *kile = new Kile(restore);

		for(int i = 0; i < args->count(); ++i) {
			//FIXME: check whether this can be used to open Urls
			if(isProject(args->arg(i))) {
				kile->openProject(completePath(args->arg(i), currentPath));
			}
			else if(args->arg(i) == "-"){
				kile->openDocument(readDataFromStdin());
			}
			else {
				kile->openDocument(completePath(args->arg(i), currentPath));
			}
		}

		if(args->isSet("line")){
			QString line = args->getOption("line");
			kile->setLine(line);
		}

		args->clear();

		return app.exec();
	}
	else {
		QDBusInterface *interface = new QDBusInterface("net.sourceforge.kile","/main","net.sourceforge.kile.main");

		for ( int i = 0; i < args->count(); ++i ) {
			QString path = args->arg(i);
			path = completePath(path, currentPath);

			if (isProject(args->arg(i))) {
				interface->call("openProject", path);
			}
			else if(args->arg(i) == "-") {
				interface->call("openDocument", readDataFromStdin());
			}
			else {
				interface->call("openDocument", path);
			}
		}

		if(args->isSet("line")){
			QString line = args->getOption("line");
			interface->call("setLine", line);
		}

		KStartupInfo::appStarted();
		interface->call("setActive");
		delete interface;
	}
	return 0;
}
