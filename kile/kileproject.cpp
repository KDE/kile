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
 * KileProjectItem
 */
KileProjectItem::KileProjectItem(KileProject *project, const KURL & url, int type) :
	m_project(project),
	m_url(url),
	m_type(type),
	m_docinfo(0),
	m_parent(0),
	m_child(0),
	m_sibling(0)
{
	m_highlight=m_encoding=QString::null; m_bOpen = m_archive = true;

	if (project)
		project->add(this);
}

void KileProjectItem::setParent(KileProjectItem * item)
{
	m_parent = item;

	//update parent info
	if (m_parent)
	{
		if (m_parent->firstChild())
		{
			//get last child
			KileProjectItem *sib = m_parent->firstChild();
			while (sib->sibling())
				sib = sib->sibling();

			sib->setSibling(this);
		}
		else
		{
			m_parent->setChild(this);
		}
	}
	else
	{
		setChild(0);
		setSibling(0);
	}
}

KileProjectItem* KileProjectItem::print(int level)
{
	QString str;
	str.fill('\t', level);
	kdDebug() << str << "+" << url().fileName() << endl;

	if (firstChild())
		return firstChild()->print(++level);

	if (sibling())
		return sibling()->print(level);

	if (parent())
	{
		if (parent()->sibling())
			parent()->sibling()->print(--level);
	}

	return 0;
}

void KileProjectItem::allChildren(QPtrList<KileProjectItem> *list) const
{
	KileProjectItem *item = firstChild();

	kdDebug() << "\tKileProjectItem::allChildren(" << list->count() << ")" << endl;
	while(item != 0)
	{
		list->append(item);
		kdDebug() << "\t\tappending " << item->url().fileName() << endl;
		item->allChildren(list);
		item = item->sibling();
	}
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
//	m_rootItem = 0;
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
		//create the project file
		m_config->setGroup("General");
		m_config->writeEntry("name", m_name);
		m_config->sync();
	}
}

void KileProject::setExtensions(const QString & ext)
{
	bool retype = false;
	if (ext != m_extensions)
		retype = true; //determine the types of the project items again

	m_extensions = ext;
	QStringList lst = QStringList::split(" ", ext);
	QString pattern = lst.join("|");
	pattern.replace(".","\\.");
	pattern ="("+ pattern +")$";
	kdDebug() << "==KileProject::setExtensions"<<endl;
	kdDebug() << "\tsetting pattern to: " << pattern << endl;
	m_reExtensions.setPattern(pattern);

	if (retype)
	{
		buildProjectTree();
	}
}

void KileProject::setType(KileProjectItem *item)
{
	kdDebug() << "==KileProject::setType()================" << endl;
	if (m_reExtensions.search(item->url().fileName()) != -1)
		item->setType(KileProjectItem::Other);
	else
		item->setType(KileProjectItem::Source);
	kdDebug() <<"\tsetting type of " << item->url().fileName() << " to " << item->type() << endl;
}

bool KileProject::load()
{
	kdDebug() << "KileProject: loading..." <<endl;

	QStringList groups = m_config->groupList();

	//load general settings/options
	m_config->setGroup("General");
	m_name = m_config->readEntry("name", i18n("Untitled"));
	setExtensions(m_config->readEntry("extensions", ".eps .pdf .dvi .ps .fig .log .aux .gif .jpg .png .fig"));
	m_archiveCommand = m_config->readEntry("archive", "tar zcvf '%S'.tar.gz %F");

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
			//item->setType(m_config->readNumEntry("type", KileProjectItem::Source));
			item->setArchive(m_config->readBoolEntry("archive", true));
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
	m_config->writeEntry("extensions", m_extensions);
	m_config->writeEntry("archive", m_archiveCommand);

	KileProjectItem *item;
	for (uint i=0; i < m_projectitems.count(); i++)
	{
		item = m_projectitems.at(i);
		m_config->setGroup("item:"+item->path());
		m_config->writeEntry("open", item->isOpen());
		m_config->writeEntry("encoding", item->encoding());
		m_config->writeEntry("highlight", item->highlight());
		//m_config->writeEntry("type", item->type());
		m_config->writeEntry("archive", item->archive());
		kdDebug() << "\tsaving " << item->path() << " " << item->isOpen() << " " << item->encoding() << " " << item->highlight()<< endl;
	}

	m_config->sync();

	dump();

	return true;
}

void KileProject::buildProjectTree()
{
	kdDebug() << "==KileProject::buildProjectTree==========================" << endl;

	//determine the parent doc for each item (TODO:an item can only have one parent, not necessarily true for LaTeX docs)

	const QStringList *deps;
	KileProjectItem *itm;
	KURL url;
	QPtrListIterator<KileProjectItem> it(m_projectitems);

	//clean first
	while (it.current())
	{
		(*it)->setParent(0);
		++it;
	}

	//use the dependencies list of the documentinfo object to determine the parent
	it.toFirst();
	while (it.current())
	{
		deps = (*it)->getInfo()->dependencies();
		for (uint i=0; i < deps->count(); i++)
		{
			url = m_baseurl;
			url.addPath((*deps)[i]);
			itm = item(url);
			if (itm && (itm->parent() == 0)) itm->setParent(*it);
			else kdDebug() << "\tcould not find " << url.path() << " in projectlist"<< endl;
		}
		++it;
	}

	//make a list of all the root items (items with parent == 0)
	m_rootItems.clear();
	it.toFirst();
	while (it.current())
	{
		if ((*it)->parent() == 0)
			m_rootItems.append(*it);

		++it;
	}

	QPtrListIterator<KileProjectItem> rit(m_rootItems);
	while (rit.current())
	{
		(*rit)->print(1);
		++rit;
	}

	emit(projectTreeChanged(this));
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
	kdDebug() << "KileProject::add projectitem" << item->url().path() << endl;

	setType(item);

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

	kdDebug() << "KileProject::remove" << endl;
	m_projectitems.remove(item);

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

KileProjectItem *KileProject::rootItem(KileProjectItem *item) const
{
	if (item)
	{
		kdDebug() << "\trootItem use buildProjectTree results" << endl;
		while ( item->parent() != 0)
			item = item->parent();

		if (item)
			kdDebug() << "\troot is " << item->url().fileName() << endl;
		else
			kdDebug() << "\tno root found" << endl;

		return item;
	}
	else
	{
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
	}
	return 0;
}

void KileProject::dump()
{
	kdDebug() << "KileProject::dump() " << m_name << endl;
	for ( uint i=0; i < m_projectitems.count(); i++)
		kdDebug() << "\titem " << i << " : "  << m_projectitems.at(i)->path() << endl;
}

#include "kileproject.moc"
