/***************************************************************************
                          templates.h  -  description
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

#ifndef TEMPLATES_H
#define TEMPLATES_H


/**
  *@author Jeroen Wijnhout
  */

#include <qvaluelist.h>

struct TemplateInfo {
public:
   QString name;
   QString path;
   QString icon;
};

typedef QValueList<TemplateInfo> TemplateList;
typedef QValueListIterator<TemplateInfo> TemplateListIterator;

class Templates {
public:
	Templates();
	~Templates();

   int count()const { return m_TemplateList.count(); }

   //returns the i-th template
   TemplateListIterator at(int i) { return m_TemplateList.at(i);}

   //find the template with name name
   TemplateListIterator find(QString name);

   //add a template in $HOME/kile/templates/
   bool add(TemplateInfo ti);

   //remove a template from $HOME/kile/templates/
   bool remove(TemplateInfo ti);

private:
   bool copyAppData(const QString &src, const QString &subdir, const QString &file);
   bool removeAppData(const QString &file);

private:
   TemplateList m_TemplateList;
};

#endif
