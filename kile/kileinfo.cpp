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
	m_parentWidget(parent),
	m_currentTarget(QString::null)
{
	m_docManager = new KileDocument::Manager(this, 0L, "KileDocument::Manager");
	m_viewManager= new KileView::Manager(this, 0L, "KileView::Manager");
}

KileInfo::~KileInfo()
{
	delete m_docManager; m_docManager = 0L;
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
	KileProject *project = docManager()->activeProject();

	//TODO: handle the case where not master document is specified in a project (sick)
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

bool KileInfo::isOpen(const KURL & url)
{
	kdDebug() << "==bool KileInfo::isOpen(const KURL & url)=============" << endl;
	uint cnt = viewManager()->views().count();
	kdDebug() << "\t" << cnt << " views" << endl;
	for ( uint i = 0; i < cnt; i++)
	{
		kdDebug() << "\t" << i << " " << viewManager()->view(i) << " url " << &url << endl;
		if ( viewManager()->view(i)->getDoc() && (url == viewManager()->view(i)->getDoc()->url()) )
			return true;
	}

	return false;
}

bool	KileInfo::projectIsOpen(const KURL & url)
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
