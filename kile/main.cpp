/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstartupinfo.h>
#include <dcopclient.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kurl.h>

#include "kileapplication.h"
#include "kile.h"

#include <kdebug.h>

static KCmdLineOptions options[] =
{
	{ "line <line>", "line", "0" },
	{ "new", "Start a new Kile mainwindow.", 0 },
	{ "+[file]", "File to open", 0 },
	KCmdLineLastOption
};

bool isProject(const QString &path)
{
	return path.right(7) == ".kilepr";
}

int main( int argc, char ** argv )
{
	KAboutData aboutData( "kile", "Kile",
						"1.7a4", I18N_NOOP("KDE Integrated LaTeX Environment"), KAboutData::License_GPL,
						I18N_NOOP("by the Kile Team (2003 - 2004)"),
						0,
						"http://kile.sourceforge.net");
	aboutData.addAuthor("Jeroen Wijnhout",I18N_NOOP("maintainer/developer"),"Jeroen.Wijnhout@kdemail.net");
	aboutData.addAuthor("Brachet Pascal",0,"");
	aboutData.addCredit("Holger Danielsson", I18N_NOOP("Code Completion, Advanced Editing, Help system"));
	aboutData.addCredit("Simon Martin", I18N_NOOP("KConfig XT configuration system"));
	aboutData.addCredit("Roland Schulz", I18N_NOOP("KatePart integration"));
	aboutData.addCredit("Thorsten Lück", I18N_NOOP("Log Parsing"));
	aboutData.addCredit("Jan-Marek Glogowski", I18N_NOOP("Find in Files dialog"));
	aboutData.addCredit("Thomas Basset", I18N_NOOP("Translations"));
	aboutData.addCredit(I18N_NOOP("Please consult the webpage for up to date translation credits."));
	aboutData.addCredit("Jonathan Pechta and Federico Zenith", I18N_NOOP("Documentation"));

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	bool running = false;

	DCOPClient *client=0L;
	QCString appID = "";
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

	if ( !running || args->isSet("new") )
	{
		KileApplication a;
		a.dcopClient()->registerAs("kile", false);
		bool restore = (args->count() == 0);
		Kile * mw = new Kile(restore);
		a.setMainWidget(mw);

		for ( int i = 0; i < args->count(); i++)
		{
			if ( isProject(args->arg(i)) )
				mw->openProject(args->arg(i));
			else
				mw->openDocument(args->arg(i));
		}
		
		if (args->getOption("line") != "0")
			mw->setLine(args->getOption("line"));
	
		args->clear();
		return a.exec();
	}
	else
	{
		for ( int i = 0; i < args->count(); i++ )
		{
			if ( isProject(args->arg(i)) )
				client->send (appID, "Kile", "openProject(QString)", args->arg(i));
			else
				client->send (appID, "Kile", "openDocument(QString)", args->arg(i));
		}

		if (args->getOption("line") != "0")
			client->send (appID, "Kile", "setLine(QString)", args->getOption("line"));

		KStartupInfo::appStarted();
		QByteArray empty;
		client->send (appID, "Kile", "setActive()", empty);
	}	

	return 0;
}

