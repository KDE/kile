/***************************************************************************
                          kileproject.cpp -  description
                             -------------------
    begin                : Fri Aug 1 2003
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
#include "kileproject.h"

#include <qstringlist.h>
#include <qfileinfo.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include "kiledocumentinfo.h"

/*
 * KileURLTree
 */
KileURLTree::KileURLTree(KileURLTree *parent, const KURL & url)  : m_parent(parent), m_url(url), m_sibling(0), m_child(0)
{
	if (m_parent)
	{
		//set this item as child, if parent didn't have a child yet
		if (m_parent->firstChild() == 0)
			m_parent->setChild(this);
		else
		{
			KileURLTree *item = m_parent->firstChild();
			while (item->sibling())
				item = item->sibling();

			item->setSibling(this);
		}
	}
}

KileURLTree::~KileURLTree()
{
	kdDebug() << "~KileURLTree() " << m_url.path() << endl;
	delete m_sibling;
	delete m_child;
}

/*
 * KileProjectItem
 */
KileProjectItem::KileProjectItem(KileProject *project, const KURL & url) :
	m_project(project),
	m_url(url),
	m_docinfo(0)
{
	m_highlight=m_encoding=QString::null; m_bOpen = true;

	if (project)
		project->add(this);
}

void KileProjectItem::setInfo(KileDocumentInfo *docinfo)
{
	m_docinfo = docinfo;
	connect(docinfo,SIGNAL(nameChanged(const KURL &)), this, SLOT(changeURL(const KURL &)));
}

/*
 * KileProject
 */
KileProject::KileProject(const QString& name, const KURL& url) : QObject(0,name.ascii())
{
	init(name,url);
}

KileProject::KileProject(const KURL& url) :  QObject(0,url.fileName().ascii())
{
	init(url.fileName(), url);
}

void KileProject::init(const QString& name, const KURL& url)
{
	m_rootItem = 0;
    m_name = name;
	m_projecturl = url;
	m_projectitems.setAutoDelete(true);

	m_config = new KSimpleConfig(m_projecturl.path());

	m_baseurl = m_projecturl.directory();
	m_baseurl.cleanPath(true);

	kdDebug() << "KileProject m_baseurl = " << m_baseurl.path() << endl;

	if (QFileInfo(url.path()).exists())
	{
		load();
	}
	else
	{
		save();
	}
}

bool KileProject::load()
{
	kdDebug() << "KileProject: loading..." <<endl;

	QStringList groups = m_config->groupList();

	//load general settings/options
	m_config->setGroup("General");
	m_name = m_config->readEntry("name", i18n("Untitled"));

	KURL url;
	KileProjectItem *item;

	//retrieve all the project files and create and initialize project items for them
	for (uint i=0; i < groups.count(); i++)
	{
		if (groups[i].left(5) == "item:")
		{
			url = m_baseurl;
			url.addPath(groups[i].mid(5));
			url.cleanPath(true);
			item = new KileProjectItem(this, url);

			m_config->setGroup(groups[i]);
			item->setOpenState(m_config->readBoolEntry("open", true));
			item->setEncoding(m_config->readEntry("encoding", QString::null));
			item->setHighlight(m_config->readEntry("highlight",QString::null));
			if (m_config->readBoolEntry("master", false)) m_rootItem = item;
			item->changePath(groups[i].mid(5));

			connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );

			//m_projectitems.append(item);
		}
	}

	dump();

	return true;
}

//TODO: restore encoding for documents
bool KileProject::save()
{
	kdDebug() << "KileProject: saving..." <<endl;

	m_config->setGroup("General");
	m_config->writeEntry("name", m_name);

	KileProjectItem *item;
	for (uint i=0; i < m_projectitems.count(); i++)
	{
		item = m_projectitems.at(i);
		m_config->setGroup("item:"+item->path());
		m_config->writeEntry("open", item->isOpen());
		m_config->writeEntry("encoding", item->encoding());
		m_config->writeEntry("highlight", item->highlight());
		kdDebug() << "\tsaving " << item->path() << " " << item->isOpen() << " " << item->encoding() << " " << item->highlight()<< endl;
	}

	m_config->sync();

	dump();

	return true;
}

void KileProject::buildProjectTree()
{
}

KileProjectItem* KileProject::item(const KURL & url)
{
	QPtrListIterator<KileProjectItem> it(m_projectitems);
	while (it.current())
	{
		if ((*it)->url() == url)
			return *it;
		++it;
	}

	return 0;
}

void KileProject::add(KileProjectItem* item)
{
	kdDebug() << "KileProject::add " << item->url().path() << endl;

	item->changePath(findRelativePath(item->url()));
	connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );

	m_projectitems.append(item);

	dump();
}

void KileProject::remove(KileProjectItem* item)
{
	if (m_config->hasGroup("item:"+item->path()))
		m_config->deleteGroup("item:"+item->path());
	else
		kdWarning() << "KileProject::remove() Failed to delete the group corresponding to this item!!!" <<endl;

	m_projectitems.remove(item);
	
	kdDebug() << "KileProject::remove" << endl;

	dump();
}

void KileProject::itemRenamed(KileProjectItem *item)
{
	kdDebug() << "==KileProject::itemRenamed==========================" << endl;
	kdDebug() << "\t" << item->url().fileName() << endl;
	m_config->deleteGroup("item:"+item->path());
	//config.sync();

	item->changePath(findRelativePath(item->url()));
}

QString KileProject::findRelativePath(const KURL &url)
{
	QString basepath = m_baseurl.path();
	QString path = url.directory();
	QString filename = url.fileName();

	//kdDebug() <<"findRelativeURL " << basepath << " ; " << path << endl;

	QStringList basedirs = QStringList::split("/", basepath);
	QStringList dirs = QStringList::split("/", path);

	while ( dirs.count() > 0 && basedirs.count() > 0 &&  dirs[0] == basedirs[0] )
	{
		dirs.pop_front();
		basedirs.pop_front();
	}

	int diff = basedirs.count()  - dirs.count() ;

	path = dirs.join("/");

	if (diff > 0)
	{
		for (int j=0; j < diff; j++)
		{
			path = "../" + path;
		}
	}

	if (diff <0)
		path += "/";

	path += filename;

	//kdDebug() << "findRelativeURL " << path << endl;

	return path;
}

bool KileProject::contains(const KURL &url)
{
	for (uint i=0; i < m_projectitems.count(); i++)
	{
		if ( m_projectitems.at(i)->url() == url )
			return true;
	}

	return false;
}

KileProjectItem *KileProject::rootItem()
{
	//FIXME: don't set the master document statically, but
	//use the updateStructure() to build the project tree

	QPtrListIterator<KileProjectItem> it(m_projectitems);
	KileDocumentInfo *docinfo;
	while (it.current())
	{
		docinfo = (*it)->getInfo();
		//kdDebug() << "rootItem()  " << docinfo->url().path() << "is root? " << docinfo->isLaTeXRoot() << endl;
		if (docinfo && docinfo->isLaTeXRoot())
		{
			return *it;
		}
		++it;
	}

	return 0;

	return m_rootItem;
}

void KileProject::dump()
{
	kdDebug() << "KileProject::dump() " << m_name << endl;
	for ( uint i=0; i < m_projectitems.count(); i++)
		kdDebug() << "\titem " << i << " : "  << m_projectitems.at(i)->path() << endl;
}

#include "kileproject.moc"
