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
#include <kdebug.h>

/*
 * KileProjectItem
 */


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
	kdDebug() << "loading..." <<endl;

	QStringList groups = m_config->groupList();

	//load general settings/options
	m_config->setGroup("General");
	m_name = m_config->readEntry("name", "untitled");

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
			item = new KileProjectItem(url);

			m_config->setGroup(groups[i]);
			item->setOpenState(m_config->readBoolEntry("open", true));
			item->changePath(groups[i].mid(5));

			connect(item, SIGNAL(urlChanged(KileProjectItem*)), this, SLOT(itemRenamed(KileProjectItem*)) );

			m_projectitems.append(item);
		}
	}

	dump();

	return true;
}

bool KileProject::save()
{
	kdDebug() << "saving..." <<endl;

	m_config->setGroup("General");
	m_config->writeEntry("name", m_name);

	KileProjectItem *item;
	for (uint i=0; i < m_projectitems.count(); i++)
	{
		item = m_projectitems.at(i);
		m_config->setGroup("item:"+item->path());
		m_config->writeEntry("open", item->isOpen());
	}

	m_config->sync();

	dump();

	return true;
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
	m_projectitems.remove(item);

	kdDebug() << "KileProject::remove" << endl;

	dump();
}

void KileProject::itemRenamed(KileProjectItem *item)
{
	m_config->deleteGroup("item:"+item->path());
	//config.sync();

	item->changePath(findRelativePath(item->url()));
}

QString KileProject::findRelativePath(const KURL &url)
{
	QString basepath = m_baseurl.path();
	QString path = url.directory();
	QString filename = url.fileName();

	kdDebug() <<"findRelativeURL " << basepath << " ; " << path << endl;

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

	kdDebug() << "findRelativeURL " << path << endl;

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

void KileProject::dump()
{
	kdDebug() << "KileProject::dump() " << m_name << endl;
	for ( uint i=0; i < m_projectitems.count(); i++)
		kdDebug() << "item " << i << " : "  << m_projectitems.at(i)->path() << endl;
}

#include "kileproject.moc"
