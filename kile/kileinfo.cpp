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

#include <qwidget.h>
#include <qfileinfo.h>
#include <qobject.h>

#include <kate/document.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kilefileselect.h"
#include "kilestructurewidget.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "kiledocumentinfo.h"
#include "kileproject.h"
#include "kileinfo.h"

KileInfo::KileInfo(QWidget *parent) :
	m_manager(0L),
	m_toolFactory(0L),
	m_texKonsole(0L),
	m_edit(0L),
	m_parentWidget(parent),
	m_currentTarget(QString::null)
{
	m_docManager = new KileDocument::Manager(this, parent, "KileDocument::Manager");
	m_viewManager= new KileView::Manager(this, parent, "KileView::Manager");
	QObject::connect(m_docManager, SIGNAL(documentStatusChanged(Kate::Document*, bool, unsigned char)), m_viewManager, SLOT(reflectDocumentStatus(Kate::Document*, bool, unsigned char)));
}

KileInfo::~KileInfo()
{
}

Kate::Document * KileInfo::activeDocument() const
{
	Kate::View *view = viewManager()->currentView();
	if (view) return view->getDoc(); else return 0L;
}

QString KileInfo::getName(Kate::Document *doc, bool shrt)
{
	QString title;
	if (doc == 0)
		doc = activeDocument();

	if (doc)
	{
		kdDebug() << "getName: url " << doc->url().path() << " name " << doc->docName() << endl;
		//work around for bug in KatePart, use docName and not url
		//reloading the file after is it changed on disc by another application
		//cause the URL to be empty for a short while
		title = shrt ? QFileInfo(doc->docName()).fileName() : doc->url().path();
		if (title == "") title = i18n("Untitled");
	}
	else
		title=QString::null;

	return title;
}

QString KileInfo::getCompileName(bool shrt /* = false */)
{
	KileProject *project = docManager()->activeProject();

	if (m_singlemode)
	{
		if (project)
		{
			if (project->masterDocument().length() > 0)
			{
				KURL master = KURL::fromPathOrURL(project->masterDocument());
				if (shrt) return master.fileName();
				else return master.path();
			}
			else
			{
				KileProjectItem *item = project->rootItem(docManager()->activeProjectItem());
				if (item)
				{
					KURL url = item->url();
					if (shrt) return url.fileName();
					else return url.path();
				}
				else
					return QString::null;
			}
		}
		else
			return getName(activeDocument(), shrt);
	}
	else
	{
		QFileInfo fi(m_masterName);
		if (shrt)
			return fi.fileName();
		else
			return m_masterName;
	}
}

QString KileInfo::getFullFromPrettyName(const QString & name)
{
	QString file = name;

	if (file.left(2) == "./" )
	{
		file = QFileInfo(outputFilter()->source()).dirPath(true) + "/" + file.mid(2);
	}

	if (file[0] != '/' )
	{
		file = QFileInfo(outputFilter()->source()).dirPath(true) + "/" + file;
	}

	QFileInfo fi(file);
	if ( (file == QString::null) || fi.isDir() || (! fi.exists()) || (! fi.isReadable()))
	{
		if ( QFileInfo(file+".tex").exists() )
		{
			file += ".tex";
			fi.setFile(file);
		}
		else
			file = QString::null;
	}

	if ( ! fi.isReadable() ) return QString::null;

	return file;
}

KURL::List KileInfo::getParentsFor(KileDocument::Info *info)
{
	KileProjectItemList *items = docManager()->itemsFor(info);
	KURL::List list;
	for ( uint i = 0; i < items->count(); i++)
		if (items->at(i)->parent()) list.append(items->at(i)->parent()->url());

	return list;
}

const QStringList* KileInfo::retrieveList(const QStringList* (KileDocument::Info::*getit)() const, KileDocument::Info * docinfo /* = 0L */)
{
	m_listTemp.clear();

	if (docinfo == 0L) docinfo = docManager()->getInfo();
	KileProjectItem *item = docManager()->itemFor(docinfo, docManager()->activeProject());

	kdDebug() << "Kile::retrieveList()" << endl;
	if (item)
	{
		const KileProject *project = item->project();
		const KileProjectItem *root = project->rootItem(item);
		if (root)
		{
			kdDebug() << "\tusing root item " << root->url().fileName() << endl;

			QPtrList<KileProjectItem> children;
			children.append(root);
			root->allChildren(&children);

			const QStringList *list;

			for (uint i=0; i < children.count(); i++)
			{
				kdDebug() << "\t" << children.at(i)->url().fileName() << endl;
				list = (children.at(i)->getInfo()->*getit)();
				if (list)
				{
					for (uint i=0; i < list->count(); i++)
						m_listTemp << (*list)[i];
				}
			}

			return &m_listTemp;
		}
		else
			return &m_listTemp;
	}
	else	if (docinfo)
	{
		m_listTemp = *((docinfo->*getit)());
		return &m_listTemp;
	}
	else
		return &m_listTemp;
}

const QStringList* KileInfo::allLabels(KileDocument::Info * info)
{
	kdDebug() << "Kile::allLabels()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::labels;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allBibItems(KileDocument::Info * info)
{
	kdDebug() << "Kile::allBibItems()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::bibItems;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allBibliographies(KileDocument::Info * info)
{
	kdDebug() << "Kile::bibliographies()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::bibliographies;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allDependencies(KileDocument::Info * info)
{
	kdDebug() << "Kile::dependencies()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::dependencies;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allNewCommands(KileDocument::Info * info)
{
	kdDebug() << "Kile::newCommands()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::newCommands;
	const QStringList* list = retrieveList(p, info);
	return list;
}

QString KileInfo::lastModifiedFile(KileDocument::Info * info)
{
	if (info == 0) info = docManager()->getInfo();
	const QStringList *list = allDependencies(info);
	return info->lastModifiedFile(list);
}

bool KileInfo::similarOrEqualURL(const KURL &validurl, const KURL &testurl)
{
	if ( testurl.isEmpty() ) return false;

	kdDebug() << "==KileInfo::similarOrEqualURL(" << validurl.url() << "," << testurl.url() << ")===========" << endl;
	bool absolute = testurl.path().startsWith("/");
	return (
		     (validurl == testurl) ||
		     (!absolute && validurl.path().endsWith(testurl.path()))
		   );
}

bool KileInfo::isOpen(const KURL & url)
{
	kdDebug() << "==bool KileInfo::isOpen(const KURL & url)=============" << endl;
	uint cnt = viewManager()->views().count();
	
	for ( uint i = 0; i < cnt; i++)
	{
		if ( viewManager()->view(i)->getDoc() && similarOrEqualURL(viewManager()->view(i)->getDoc()->url(), url) )
			return true;
	}

	return false;
}

bool KileInfo::projectIsOpen(const KURL & url)
{
	KileProject *project = docManager()->projectFor(url);

	return project != 0 ;
}

QString KileInfo::getSelection() const
{
	Kate::Document *doc = activeDocument();
	
	if (doc && doc->hasSelection())
	{
		return doc->selection();
	}
	
	return QString::null;
}

void KileInfo::clearSelection() const
{
	Kate::Document *doc = activeDocument();
	
	if (doc && doc->hasSelection())
	{
		doc->removeSelectedText();
	}
}

QString KileInfo::relativePath(const QString basepath, const QString & file)
{
	KURL url = KURL::fromPathOrURL(file);
	QString path = url.directory();
	QString filename = url.fileName();

	kdDebug() <<"===findRelativeURL==================" << endl;
	kdDebug() << "\tbasepath : " <<  basepath << " path: " << path << endl;

	QStringList basedirs = QStringList::split("/", basepath, false);
	QStringList dirs = QStringList::split("/", path, false);

	uint nDirs = dirs.count();
	//uint nBaseDirs = basedirs.count();

	while ( dirs.count() > 0 && basedirs.count() > 0 &&  dirs[0] == basedirs[0] )
	{
		dirs.pop_front();
		basedirs.pop_front();
	}

	/*kdDebug() << "\tafter" << endl;
	for (uint i=0; i < basedirs.count(); i++)
	{
		kdDebug() << "\t\tbasedirs " << i << ": " << basedirs[i] << endl;
	}

	for (uint i=0; i < dirs.count(); i++)
	{
		kdDebug() << "\t\tdirs " << i << ": " << dirs[i] << endl;
	}*/

	if (nDirs != dirs.count() )
	{
		path = dirs.join("/");

		//kdDebug() << "\tpath : " << path << endl;

		if (basedirs.count() > 0)
		{
			for (uint j=0; j < basedirs.count(); j++)
			{
				path = "../" + path;
			}
		}

		if ( path.length()>0 && path.right(1) != "/" ) path = path + "/";

		path = path+filename;
	}
	else //assume an absolute path was requested
	{
		path = url.path();
	}

	kdDebug() << "\trelative path : " << path << endl;

	return path;
}
