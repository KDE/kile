/***************************************************************************
                          templates.cpp  -  description
                             -------------------
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kmessagebox.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qregexp.h>

#include "templates.h"

Templates::Templates()
{
	kdDebug() << "===Templates()===================" << endl;
   QStringList dirs = KGlobal::dirs()->findDirs("appdata","templates");
   QDir templates;
   TemplateInfo ti;
  
   for ( QValueListIterator<QString> i = dirs.begin(); i != dirs.end(); ++i)
   {

     templates = QDir(*i, "template_*.tex");
     for ( uint j=0; j< templates.count(); ++j)
	 {
        ti.path=templates.path()+"/"+templates[j];
        ti.name=templates[j].replace("template_","");
        ti.name.replace(".tex","");
	ti.icon=KGlobal::dirs()->findResource("appdata","pics/type_"+ti.name+".png");

		if (m_TemplateList.contains(ti))
			kdDebug() << "\tignoring: " << ti.path << endl;
		else
		{
			m_TemplateList.append(ti);
			kdDebug() << "\tadding: " << ti.name << " " << ti.path << endl;
		}

     }
   }
}

Templates::~Templates(){
}

bool Templates::copyAppData(const QString &src, const QString &subdir, const QString &file) {
   KIO::Job *job;
   QString dst,dir;

   //let saveLocation find and create the appropriate place to
   //store the templates (usually $HOME/.kde/share/apps/kile/templates)
   dir = KGlobal::dirs()->saveLocation("appdata",subdir,true);
   //if a directory is found
   if (!dir.isNull()) {
      dst = dir + "/"+ file;
      job = KIO::file_copy(KURL(src),KURL(dst),-1,true,false,false);
      //let KIO show the error messages
      job->setAutoErrorHandlingEnabled(true);
   }
   else {
      KMessageBox::error(0,i18n("Could not find a folder to save %1 to.\nCheck whether you have a .kde folder with write permissions in your home folder.").arg(file));
      return false;
   }

   return true;
}

bool Templates::removeAppData(const QString &file) {
	KIO::Job *job;
	QString src = KGlobal::dirs()->findResource("appdata",file);

	if (!src) return false;

	QFileInfo fi(src);
	if ( ! fi.exists() ) return true;
	
	job = KIO::file_delete(KURL(src),false);
	job->setAutoErrorHandlingEnabled(true);

	return true;
}

bool Templates::add(TemplateInfo ti) {
   
   return
   copyAppData(ti.path,"templates","template_"+ti.name+".tex") &&
   copyAppData(ti.icon,"pics","type_"+ti.name+".png");
   
}

bool Templates::remove(TemplateInfo ti) {
   
   return
   removeAppData(ti.path) && removeAppData(ti.icon);
}

TemplateListIterator Templates::find(const QString & name) {
   for (TemplateListIterator i=m_TemplateList.begin(); i != m_TemplateList.end(); ++i) {
      if ( (*i).name == name ) { return i; }      
   }

   return NULL;
}

