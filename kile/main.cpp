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

static KCmdLineOptions options[] =
{
  { "line <line>", "line", "0" },
  { "+[file]", "File to open", 0 },
  KCmdLineLastOption
};

int main( int argc, char ** argv )
{
  KAboutData aboutData( "kile", "Kile",
    "1.5.2-patch1", I18N_NOOP("KDE Integrated LaTeX Environment"), KAboutData::License_GPL,
    "Jeroen Wijnhout 2003",
    0,
    "http://kile.sourceforge.net");
  aboutData.addAuthor("Jeroen Wijnhout",I18N_NOOP("maintainer/developer"),"Jeroen.Wijnhout@kdemail.net");
  aboutData.addAuthor("Brachet Pascal",0,"");
  aboutData.addCredit("Thanks to all the patchers, packagers from all over the world!","");
  aboutData.addCredit("David Ishee", I18N_NOOP("Xgfe"));
  aboutData.addCredit("Unai Garro, Asokan, Maxim Azarov, Harald Fernengel, Stefan Kebekus");
  aboutData.addCredit("Goon", I18N_NOOP("french translation"));
  aboutData.addCredit("Kris Luyten", I18N_NOOP("dutch translation"));
  aboutData.addCredit("Alexander Hunziger, Roland Riegel", I18N_NOOP("german translation"));
  aboutData.addCredit("Stefan Asserhall", I18N_NOOP("swedish translation"));
  aboutData.addCredit("Alfredo Beaumont", I18N_NOOP("basque translation"));
  aboutData.addCredit("Leopold Palomo Avellaneda", I18N_NOOP("catalan translation"));
  aboutData.addCredit("AceLan, Edward Lee, Haoxiang Lin", I18N_NOOP("chinese translation"));
  aboutData.addCredit("Nikos Galanis", I18N_NOOP("greek translation"));
  aboutData.addCredit("Emerson Ribeiro de Mello", I18N_NOOP("portuguese-brazil translation"));
  aboutData.addCredit("Miguel Mingo", I18N_NOOP("spanish translation"));
  aboutData.addCredit("Masayuki SANO", I18N_NOOP("japanese translation"));
  aboutData.addCredit("Victor Kozyakin", I18N_NOOP("russian translation"));
  aboutData.addCredit("Marian Janiga, Stanislav Visnovsky", I18N_NOOP("slovak translation"));
  aboutData.addCredit("Marcel Hilzinger", I18N_NOOP("hungarian translation"));
  aboutData.addCredit("Petr Sidlo", I18N_NOOP("czech translation"));
  aboutData.addCredit("Daniele A.Morano", I18N_NOOP("italian translation"));

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

delete client;
return 0;
}

