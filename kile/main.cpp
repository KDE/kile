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

#include "kile.h"

static KCmdLineOptions options[] =
{
  { "line <line>", "line", "0" },
  { "+[file]", "File to open", 0 },
  { 0, 0, 0 }
};

int main( int argc, char ** argv )
{
  KAboutData aboutData( "kile", "Kile",
    "1.5", "KDE Integrated LaTeX Environment", KAboutData::License_GPL,
    "Brachet Pascal 2002-2003",
    "Jeroen Wijnhout 2003-",
    0,
    "http://perso.club-internet.fr/pascal.brachet/kile/");
  aboutData.addAuthor("Brachet Pascal",0,"");
  aboutData.addCredit("David Ishee (Xgfe)");
  aboutData.addCredit("Unai Garro, Asokan, Maxim Azarov, Harald Fernengel, Stefan Kebekus");
  aboutData.addCredit("AceLan, Edward Lee, Haoxiang Lin (chinese translation)");
  aboutData.addCredit("Nikos Galanis (greek translation)");
  aboutData.addCredit("Emerson Ribeiro de Mello (portuguese-brazil translation)");
  aboutData.addCredit("Miguel Mingo (spanish translation)");
  aboutData.addCredit("Masayuki SANO (japanese translation)");
  aboutData.addCredit("Victor Kozyakin (russian translation)");
  aboutData.addCredit("Marian Janiga, Stanislav Visnovsky (slovak translation)");
  aboutData.addCredit("Roland Riegel (german translation)");
  aboutData.addCredit("Alfredo Beaumont (basque translation)");
  aboutData.addCredit("Marcel Hilzinger (hungarian translation)");
  aboutData.addCredit("Leopold Palomo Avellaneda (catalan translation)");
  aboutData.addCredit("Petr Sidlo (czech translation)");
  aboutData.addCredit("Daniele A.Morano (italian translation)");
  aboutData.addCredit("Kris Luyten (dutch translation)");
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
      if ( sa.left(5) == "file:" ) sa = sa.remove(0, 5);
      mw->load(sa);
      if (args->getOption("line")!="0") mw->setLine(args->getOption("line"));
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
      if ( sa.left(5) == "file:" ) sa = sa.remove(0, 5);
      arg_file << sa;
      client->send (appID, "Kile", "load(QString)", data_file);
      if (args->getOption("line")!="0")
         {
         QString li=args->getOption("line");
         arg_line << li;
         client->send (appID, "Kile", "setLine(QString)", data_line);
         }
      KStartupInfo::appStarted();
    }
}
return 0;
}

