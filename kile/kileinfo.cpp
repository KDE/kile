/***************************************************************************
                          kileinfointerface.cpp  -  description
                             -------------------
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#include <qfileinfo.h>

#include <kate/document.h>

#include "kiledocumentinfo.h"
#include "kileproject.h"
#include "kileinfo.h"

void KileInfo::trash(Kate::Document *doc)
{
	 infoFor(doc)->detach();
	 m_docList.remove(doc);
	 removeMap(doc);
	 delete doc;
}

KileDocumentInfo *KileInfo::infoFor(const QString & path)
{
	for (uint i=0; i < m_docList.count(); i++)
	{
		if ( m_docList.at(i)->url().path() == path)
			return infoFor(m_docList.at(i));
	}

	return 0;
}

Kate::Document* KileInfo::docFor(const KURL& url)
{
	for (uint i=0; i < m_docList.count(); i++)
	{
		if (m_docList.at(i)->url() == url)
			return m_docList.at(i);
	}

	return 0;
}
QString KileInfo::getName(Kate::Document *doc, bool shrt)
{
	QString title;
	if (doc == 0)
		doc = activeDocument();

	if (doc)
	{
		title = shrt ? doc->url().fileName() : doc->url().path();

		if (title == "") title = doc->docName();
	}
	else
		title="";

	return title;
}

QString KileInfo::getCompileName(bool shrt /* = false */)
{
	KileProject *project = activeProject();

	//TODO: handle the case where not master document is specified in a project (sick)
	if (project)
	{
		KileProjectItem *item = project->rootItem();
		if (item)
		{
			KURL url = item->url();
			if (shrt) return url.fileName();
			else return url.path();
		}
		else
			return QString::null;

	}
	else
	{
		if (m_singlemode)
			return getName(activeDocument(), shrt);
		else
		{
			QFileInfo fi(m_masterName);
			if (shrt)
				return fi.fileName();
			else
				return m_masterName;
		}
	}
}

bool	KileInfo::projectIsOpen(const KURL & url)
{
	for (uint i=0; i < m_projects.count(); i++)
	{
		if (m_projects.at(i)->url() == url)
			return true;
	}

	return false;
}

KileProject* KileInfo::activeProject()
{
	KileProject *curpr=0;
	Kate::Document *doc = activeDocument();

	if (doc)
	{
		for (uint i=0; i < m_projects.count(); i++)
		{
			if (m_projects.at(i)->contains(doc->url()) )
			{
				curpr = m_projects.at(i);
				break;
			}
		}
	}

	return curpr;
}

KileProjectItem* KileInfo::activeProjectItem()
{
	KileProject *curpr = activeProject();
	Kate::Document *doc = activeDocument();
	KileProjectItem *item = 0;

	if (curpr && doc)
	{
		KileProjectItemList *list = curpr->items();

		for (uint i=0; i < list->count(); i++)
		{
			if (list->at(i)->url() == doc->url())
			{
				item = list->at(i);
				break;
			}
		}
	}

	return item;
}

