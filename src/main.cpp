/********************************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet
                               2003 - 2005 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2011 - 2014 by Michel Ludwig (michel.ludwig@kdemail.net)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFile>
#include <QtDBus>
#include <QFileInfo>
#include <QTextCodec>
#include <QUrl>

#include <KAboutData>
#include <KDBusService>
#include <KEncodingProber>
#include <KLocalizedString>
#include <KStartupInfo>

#include "kile.h"
#include "kileversion.h"
#include "kiledebug.h"

Q_LOGGING_CATEGORY(LOG_KILE_MAIN, "kile.main")

bool isProject(const QUrl url)
{
	return url.fileName().endsWith(".kilepr");
}

QString readDataFromStdin()
{
	KILE_DEBUG_MAIN;

	QByteArray fileData;
	QFile qstdin;
	QTextCodec *codec = NULL;

	qstdin.open( stdin, QIODevice::ReadOnly );
	fileData = qstdin.readAll();
	qstdin.close();

	QTemporaryDir *tempDir = new QTemporaryDir(QDir::tempPath() + QLatin1Char('/') +  "kile-stdin");
	QString tempFileName = QFileInfo(tempDir->path(), i18n("StandardInput.tex")).absoluteFilePath();
	KILE_DEBUG_MAIN << "tempFile is " << tempFileName;

	QFile tempFile(tempFileName);
	if(!tempFile.open(QIODevice::WriteOnly)) {
		return QString();
	}

	QTextStream stream(&tempFile);

	KEncodingProber prober(KEncodingProber::Universal);
 	KEncodingProber::ProberState state = prober.feed(fileData);
	KILE_DEBUG_MAIN << "KEncodingProber::state " << state;
	KILE_DEBUG_MAIN << "KEncodingProber::prober.confidence() " << prober.confidence();
	KILE_DEBUG_MAIN << "KEncodingProber::encoding " << prober.encoding();

	codec = QTextCodec::codecForName(prober.encoding());
	if(codec){
		stream.setCodec(codec);
	}

	stream << fileData;
	tempFile.close();

	return tempFileName;
}

extern "C" Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
	QApplication app(argc, argv);

	KAboutData aboutData("kile", i18n("Kile"), kileFullVersion.toAscii(),
	                     i18n("KDE Integrated LaTeX Environment"),
	                     KAboutLicense::GPL,
	                     i18n("by the Kile Team (2003 - 2014)"),
	                     QString(),
	                     QStringLiteral("http://kile.sourceforge.net"));
	aboutData.addAuthor(i18n("Michel Ludwig"), i18n("Project Management/Developer"), "michel.ludwig@kdemail.net");
	aboutData.addAuthor(i18n("Holger Danielsson"), i18n("Developer"), "holger.danielsson@versanet.de");
	aboutData.addAuthor(i18n("Thomas Braun"), i18n("Former Developer"), "thomas.braun@virtuell-zuhause.de");
	aboutData.addAuthor(i18n("Jeroen Wijnhout"), i18n("Former Maintainer/Developer"),"Jeroen.Wijnhout@kdemail.net");
	aboutData.addAuthor(i18n("Brachet Pascal"));

	aboutData.addCredit(i18n("Andrius Štikonas"), i18n("Migration from Subversion to Git"), "andrius@stikonas.eu");
	aboutData.addCredit(i18n("Simon Martin"), i18n("KConfig XT, Various Improvements and Bug-Fixing"));
	aboutData.addCredit(i18n("Roland Schulz"), i18n("KatePart Integration"));
	aboutData.addCredit(i18n("Thorsten Lück"), i18n("Log Parsing"));
	aboutData.addCredit(i18n("Jan-Marek Glogowski"), i18n("Find-in-Files Dialog"));
	aboutData.addCredit(i18n("Jonathan Pechta"), i18n("Documentation"));
	aboutData.addCredit(i18n("Federico Zenith"), i18n("Documentation"));

	aboutData.setOrganizationDomain(QByteArray("sourceforge.net"));

	aboutData.setProductName(QByteArray("kile"));

	KAboutData::setApplicationData(aboutData);

	app.setApplicationName(aboutData.componentName());
	app.setApplicationDisplayName(aboutData.displayName());
	app.setOrganizationDomain(aboutData.organizationDomain());
	app.setApplicationVersion(aboutData.version());

	QCommandLineParser parser;
	parser.addVersionOption();
	parser.addHelpOption();
	aboutData.setupCommandLine(&parser);

	parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("line"), i18n("Jump to line"), QLatin1String("line")));
	parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("new"), i18n("Start a new Kile mainwindow")));
//TODO KF5 VERIFY THAT '-' STILL WORKS
	parser.addPositionalArgument("urls", i18n("Files to open / specify '-' to read from standard input"), QLatin1String("[urls...]"));

	parser.process(app);
	aboutData.processCommandLine(&parser);

	app.setApplicationName(aboutData.componentName());
	app.setApplicationDisplayName(aboutData.displayName());
	app.setOrganizationDomain(aboutData.organizationDomain());
	app.setApplicationVersion(aboutData.version());


	bool running = false;

	const KDBusService dbusService(KDBusService::Multiple);

	QDBusConnection dbus = QDBusConnection::sessionBus();
	running = dbus.interface()->isServiceRegistered("net.sourceforge.kile");

	if(!running  || parser.isSet("new")) {
		bool restore = (parser.positionalArguments().count() == 0);

		Kile *kile = new Kile(restore);

		Q_FOREACH(QString argument, parser.positionalArguments()) {
			if(argument == "-") {
				kile->openDocument(readDataFromStdin());
			}
			else {
				const QUrl url = QUrl::fromUserInput(argument);

				if(isProject(url)) {
					kile->openProject(url);
				}
				else {
					kile->openDocument(url);
				}
			}
		}

		if(parser.isSet("line")){
			QString line = parser.value("line");
			kile->setLine(line);
		}

		return app.exec();
	}
	else {
		QDBusInterface *interface = new QDBusInterface("net.sourceforge.kile","/main","net.sourceforge.kile.main");

		Q_FOREACH(QString argument, parser.positionalArguments()) {
			if(argument == "-") {
				interface->call("openDocument", readDataFromStdin());
			}
			else {
				const QUrl url = QUrl::fromUserInput(argument);

				if(isProject(url)) {
					interface->call("openProject", url.url());
				}
				else {
					interface->call("openDocument", url.url());
				}
			}
		}

		if(parser.isSet("line")){
			QString line = parser.value("line");
			interface->call("setLine", line);
		}

		KStartupInfo::appStarted();
		interface->call("setActive");
		delete interface;
	}

	return EXIT_SUCCESS;
}
