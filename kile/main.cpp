/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 by Pascal Brachet
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kileapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstartupinfo.h>
#include <dcopclient.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "kile.h"

#include <kdebug.h>

static KCmdLineOptions options[] =
    {
        { "line <line>", "line", "0" },
        { "+[file]", "File to open", 0 },
        KCmdLineLastOption
    };

int main( int argc, char ** argv )
{
   KAboutData aboutData( "kile", "Kile",
                          "1.6.1", I18N_NOOP("KDE Integrated LaTeX Environment"), KAboutData::License_GPL,
                          I18N_NOOP("by the Kile Team (2003)"),
                          0,
                          "http://kile.sourceforge.net");
    aboutData.addAuthor("Jeroen Wijnhout",I18N_NOOP("maintainer/developer"),"Jeroen.Wijnhout@kdemail.net");
    aboutData.addAuthor("Brachet Pascal",0,"");
    aboutData.addCredit("Roland Schulz", I18N_NOOP("KatePart integration"));
    aboutData.addCredit("Thorsten Lück", I18N_NOOP("log parsing"));
    aboutData.addCredit("Jan-Marek Glogowski", I18N_NOOP("find in files"));
    aboutData.addCredit("Thomas Basset", I18N_NOOP("translations"));
    aboutData.addCredit(I18N_NOOP("Please consult the webpage for up to date translation credits."));
    aboutData.addCredit("Jonathan Pechta and Federico Zenith", I18N_NOOP("documentation"));

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

    if (!running)
    {
        KileApplication a;
        a.dcopClient()->registerAs("kile", false);
        Kile * mw = new Kile();
        a.setMainWidget(mw);
        if (args->count()>0)
        {
            QString sa = args->arg(0);
            if ( sa.left(5) == "file:" )
                sa = sa.remove(0, 5);
            kdDebug() << QString("main: load(%1)").arg(sa) << endl;
            QFileInfo fi(sa);
	    if (sa.right(7) == ".kilepr")
	    	mw->projectOpen(fi.absFilePath());
	    else
            	mw->load(KURL::fromPathOrURL(fi.absFilePath()));
            if (args->getOption("line")!="0")
                mw->setLine(args->getOption("line"));
        }
        args->clear();
        return a.exec();
    }
    else
    {
        if (args->count()>0)
        {
            QByteArray data_file, data_line;
            QDataStream arg_file(data_file, IO_WriteOnly);
            QDataStream arg_line(data_line, IO_WriteOnly);
            QString sa = args->arg(0);
            if ( sa.left(5) == "file:" )
                sa = sa.remove(0, 5);
	    QFileInfo fi(sa);
            arg_file << fi.absFilePath();
            kdDebug() << QString("main: load(%1)").arg(sa) << endl;
	    
	    if (sa.right(7) == ".kilepr")
	    	client->send (appID, "Kile", "projectOpen(QString)", data_file);
	    else
            	client->send (appID, "Kile", "load(QString)", data_file);
		
            if (args->getOption("line")!="0")
            {
                QString li=args->getOption("line");
                arg_line << li;
                client->send (appID, "Kile", "setLine(QString)", data_line);
            }
            KStartupInfo::appStarted();
	    QByteArray empty;
	    client->send (appID, "Kile", "setActive()", empty);
        }
    }
    return 0;
}

