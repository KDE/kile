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
#include <klocale.h>
#include <kmessagebox.h>

#include "kiledocumentinfo.h"
#include "kileproject.h"
#include "kileinfo.h"

void KileInfo::mapItem(KileDocumentInfo *docinfo, KileProjectItem *item)
{
	item->setInfo(docinfo);
}

void KileInfo::trashDoc(KileDocumentInfo *docinfo)
{
	Kate::Document *doc = docFor(docinfo->url());

	kdDebug() << "\tTRASHING " <<  doc  << endl;
	if (doc == 0L) return;

	kdDebug() << "DETACHING " << docinfo << endl;
	docinfo->detach();

	kdDebug() << "DELETING " << doc << endl;
	m_docList.remove(doc);

	kdDebug() << "just checking: " << docinfo->getDoc() << endl;
	kdDebug() << "just checking: " << docFor(docinfo->url()) << endl;

	delete doc;
}

KileProject* KileInfo::projectFor(const KURL &projecturl)
{
	KileProject *project = 0;

	//find project with url = projecturl
	QPtrListIterator<KileProject> it(m_projects);
	while ( it.current() )
	{
		if ((*it)->url() == projecturl)
		{
			return *it;
		}
		++it;
	}

	return project;
}

KileProject* KileInfo::projectFor(const QString &name)
{
	KileProject *project = 0;

	//find project with url = projecturl
	QPtrListIterator<KileProject> it(m_projects);
	while ( it.current() )
	{
		if ((*it)->name() == name)
		{
			return *it;
		}
		++it;
	}

	return project;
}

KileProjectItem* KileInfo::itemFor(const KURL &url, KileProject *project /*=0L*/) const
{
	if (project == 0L)
	{
		QPtrListIterator<KileProject> it(m_projects);
		while ( it.current() )
		{
			kdDebug() << "looking in project " << (*it)->name() << endl;
			if ((*it)->contains(url))
			{
				kdDebug() << "\t\tfound!" << endl;
				return (*it)->item(url);
			}
			++it;
		}
		kdDebug() << "\t nothing found" << endl;
	}
	else
	{
		if ( project->contains(url) )
			return project->item(url);
	}

	return 0L;
}

KileProjectItem* KileInfo::itemFor(KileDocumentInfo *docinfo, KileProject *project /*=0*/) const
{
	return itemFor(docinfo->url(), project);
}

KileProjectItemList* KileInfo::itemsFor(KileDocumentInfo *docinfo) const
{
	kdDebug() << "==KileInfo::itemsFor(" << docinfo->url().fileName() << ")============" << endl;
	KileProjectItemList *list = new KileProjectItemList();
	list->setAutoDelete(false);

	QPtrListIterator<KileProject> it(m_projects);
	while ( it.current() )
	{
		kdDebug() << "\tproject: " << (*it)->name() << endl;
		if ((*it)->contains(docinfo->url()))
		{
			kdDebug() << "\t\tcontains" << endl;
			list->append((*it)->item(docinfo->url()));
		}
		++it;
	}

	return list;
}

KileDocumentInfo* KileInfo::getInfo() const
{
	Kate::Document *doc = activeDocument(); 
	if ( doc != 0L )
		return infoFor(doc);
	else
		return 0L;
}

KileDocumentInfo *KileInfo::infoFor(const QString & path) const
{
// 	kdDebug() << "==KileInfo::infoFor==========================" << endl;
	kdDebug() << "\t" << path << endl;
	QPtrListIterator<KileDocumentInfo> it(m_infoList);
	while ( true )
	{
// 		kdDebug() << "\tconsidering " << it.current()->url().path() << endl;
		if ( it.current()->url().path() == path)
			return it.current();

		if (it.atLast()) break;

		++it;
	}

	//kdDebug() << "\tCOULD NOT find info for " << path << endl;
	return 0L;
}

KileDocumentInfo* KileInfo::infoFor(Kate::Document* doc) const
{
	return infoFor(doc->url().path());
}

Kate::Document* KileInfo::docFor(const KURL& url)
{
	for (uint i=0; i < m_docList.count(); i++)
	{
		if (m_docList.at(i)->url() == url)
			return m_docList.at(i);
	}

	return 0L;
}

QString KileInfo::getName(Kate::Document *doc, bool shrt)
{
	QString title;
	if (doc == 0)
		doc = activeDocument();

	if (doc)
	{
		//kdDebug() << "getName: url " << doc->url().path() << " name " << doc->docName() << endl;
		title = shrt ? doc->url().fileName() : doc->url().path();

		if (title == "") title = i18n("Untitled");
	}
	else
		title=QString::null;

	return title;
}

QString KileInfo::getCompileName(bool shrt /* = false */)
{
	KileProject *project = activeProject();

	//TODO: handle the case where not master document is specified in a project (sick)
	if (project)
	{
		KileProjectItem *item = project->rootItem(activeProjectItem());
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
	KileProject *project = projectFor(url);

	return project != 0 ;
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
