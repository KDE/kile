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
#include <iostream>

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kmessagebox.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
 
#include "templates.h"

Templates::Templates(){
   //get the template directories
   //typically: /home/user/.kde/share/apps/kile/templates
   //and /kderoot/share/apps/kile/template
   //WARNING: I assume that the /home/user/.../templates directory is the
   //first int the list.
   QStringList dirs = KGlobal::dirs()->findDirs("appdata","templates");
   QDir templates;
   TemplateInfo ti;
  
   for ( QValueListIterator<QString> i = dirs.begin(); i != dirs.end(); i++) {
   
     templates = QDir(*i, "template_*.tex");
     for ( int j=0; j< templates.count(); j++) {
        ti.path=templates.path()+"/"+templates[j];
        ti.name=templates[j].replace("template_","");
        ti.name.replace(".tex","");
        ti.icon=ti.path;
        ti.icon.replace("templates","pics");
        ti.icon.replace("template_","type_");
        ti.icon.replace(".tex",".png");
        //NOTE: off limit strings in the templates names are
        //templates,template_,.tex.,.png,_template

        m_TemplateList.append(ti);
     }
   }

   
}

Templates::~Templates(){
}

bool Templates::copyAppData(QString src, QString subdir, QString file) {
   KIO::Job *job;
   QString dst,dir;

   //let saveLocation find and create the appropriate place to
   //store the templates (usually $HOME/.kde/share/apps/kile/templates)
   dir = KGlobal::dirs()->saveLocation("appdata",subdir,true);
   //if a directory if found
   if (dir != QString::null ) {
      dst = dir + "/"+ file;
      job = KIO::file_copy(KURL(src),KURL(dst),-1,true,false,false);
      //let KIO show the error messages
      job->setAutoErrorHandlingEnabled(true);
   }
   else {
      KMessageBox::error(0,i18n("Could not find a directory to save %1 to.\nCheck whether you have a .kde directory with write permissions in your home directory.").arg(file));
      return false;
   }

   return true;
}
   
bool Templates::add(TemplateInfo ti) {
   
   return
   copyAppData(ti.path,"templates","template_"+ti.name+".tex") &&
   copyAppData(ti.icon,"pics","type_"+ti.name+".png");
   
}

bool Templates::remove(TemplateInfo ti) {
   return 0;
}

TemplateListIterator Templates::find(QString name) {
   for (TemplateListIterator i=m_TemplateList.begin(); i != m_TemplateList.end(); i++) {
      if ( (*i).name == name ) { return i; }      
   }

   return NULL;
}

