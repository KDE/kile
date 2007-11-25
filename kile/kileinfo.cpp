/***************************************************************************
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

// 2007-03-12 dani
//  - use KileDocument::Extensions

#include "kileinfo.h"

#include <qwidget.h>
#include <qfileinfo.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <kate/document.h>
#include <kate/view.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kilestructurewidget.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "kiledocumentinfo.h"
#include "kileproject.h"
#include "kileuntitled.h"
#include "kilejscript.h"
#include "editorkeysequencemanager.h"
#include "templates.h"

#include <kstandarddirs.h>
#include <qstringlist.h>
#include <qstring.h>

KileInfo::KileInfo(QWidget *parent) :
	m_manager(0L),
	m_jScriptManager(0L),
	m_toolFactory(0L),
	m_texKonsole(0L),
	m_edit(0L),
	m_parentWidget(parent),
	m_currentTarget(QString::null)
{
	m_docManager = new KileDocument::Manager(this, parent, "KileDocument::Manager");
	m_viewManager= new KileView::Manager(this, parent, "KileView::Manager");
	m_templateManager = new KileTemplate::Manager(this, parent, "KileTemplate::Manager");
	m_editorKeySequenceManager = new KileEditorKeySequence::Manager(this, parent, "KileEditorKeySequence::Manager");
	QObject::connect(m_docManager, SIGNAL(documentStatusChanged(Kate::Document*, bool, unsigned char)), m_viewManager, SLOT(reflectDocumentStatus(Kate::Document*, bool, unsigned char)));
}

KileInfo::~KileInfo()
{
}

Kate::Document * KileInfo::activeTextDocument() const
{
	Kate::View *view = viewManager()->currentTextView();
	if (view) return view->getDoc(); else return 0L;
}

QString KileInfo::getName(Kate::Document *doc, bool shrt)
{
	KILE_DEBUG() << "===KileInfo::getName(Kate::Document *doc, bool " << shrt << ")===" << endl;
	QString title=QString::null;
	
	if (!doc)
		doc = activeTextDocument();
	if (doc)
	{
		KILE_DEBUG() << "url " << doc->url().path() << endl;
		title = shrt ? doc->url().fileName() : doc->url().path();
               //work around for bug in KatePart, use docName and not url
               //reloading the file after is it changed on disc by another application
               //cause the URL to be empty for a short while
		if ( title.isEmpty() )
			title = shrt ? QFileInfo(doc->docName()).fileName() : doc->docName();
	}

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
				KUrl master = KUrl::fromPathOrUrl(project->masterDocument());
				if (shrt) return master.fileName();
				else return master.path();
			}
			else
			{
				KileProjectItem *item = project->rootItem(docManager()->activeProjectItem());
				if (item)
				{
					KUrl url = item->url();
					if (shrt) return url.fileName();
					else return url.path();
				}
				else
					return QString::null;
			}
		}
		else
			return getName(activeTextDocument(), shrt);
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
	if(name.isNull())
		return name;

	QString file = name;

	if (file.left(2) == "./" )
	{
		file = QFileInfo(outputFilter()->source()).absolutePath() + '/' + file.mid(2);
	}

	if (file[0] != '/' )
	{
		file = QFileInfo(outputFilter()->source()).absolutePath() + '/' + file;
	}

	QFileInfo fi(file);
	if ( file.isNull() || fi.isDir() || (! fi.exists()) || (! fi.isReadable()))
	{
		// - call from logwidget or error handling, which 
		//   tries to determine the LaTeX source file
		bool found = false;
		QStringList extlist = (m_extensions->latexDocuments()).split(" ");
		for ( QStringList::Iterator it=extlist.begin(); it!=extlist.end(); ++it )
		{
			QString name = file + (*it);
			if ( QFileInfo(name).exists() )
			{
				file = name;
				fi.setFile(name);
				found = true;
				break;
			}
		}
		if ( ! found )
			file = QString::null;
	}

	if ( ! fi.isReadable() ) return QString::null;

	return file;
}

KUrl::List KileInfo::getParentsFor(KileDocument::Info *info)
{
	KileProjectItemList *items = docManager()->itemsFor(info);
	KUrl::List list;
	for ( uint i = 0; i < items->count(); ++i)
		if (items->at(i)->parent()) list.append(items->at(i)->parent()->url());

	return list;
}

const QStringList* KileInfo::retrieveList(const QStringList* (KileDocument::Info::*getit)() const, KileDocument::Info * docinfo /* = 0L */)
{
	m_listTemp.clear();

	if (docinfo == 0L) docinfo = docManager()->getInfo();
	KileProjectItem *item = docManager()->itemFor(docinfo, docManager()->activeProject());

	KILE_DEBUG() << "Kile::retrieveList()" << endl;
	if (item)
	{
		const KileProject *project = item->project();
		const KileProjectItem *root = project->rootItem(item);
		if (root)
		{
			KILE_DEBUG() << "\tusing root item " << root->url().fileName() << endl;

			Q3PtrList<KileProjectItem> children;
			children.append(root);
			root->allChildren(&children);

			const QStringList *list;

			for (uint i=0; i < children.count(); ++i)
			{
				KILE_DEBUG() << "\t" << children.at(i)->url().fileName() << endl;
				list = (children.at(i)->getInfo()->*getit)();
				if (list)
				{
					for (uint i=0; i < list->count(); ++i)
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
	KILE_DEBUG() << "Kile::allLabels()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::labels;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allBibItems(KileDocument::Info * info)
{
	KILE_DEBUG() << "Kile::allBibItems()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::bibItems;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allBibliographies(KileDocument::Info * info)
{
	KILE_DEBUG() << "Kile::bibliographies()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::bibliographies;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allDependencies(KileDocument::Info * info)
{
	KILE_DEBUG() << "Kile::dependencies()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::dependencies;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allNewCommands(KileDocument::Info * info)
{
	KILE_DEBUG() << "Kile::newCommands()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::newCommands;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* KileInfo::allPackages(KileDocument::Info * info)
{
	KILE_DEBUG() << "Kile::allPackages()" << endl;
	const QStringList* (KileDocument::Info::*p)() const=&KileDocument::Info::packages;
	const QStringList* list = retrieveList(p, info);
	return list;
}

QString KileInfo::lastModifiedFile(KileDocument::Info * info)
{
	if (info == 0) info = docManager()->getInfo();
	const QStringList *list = allDependencies(info);
	return info->lastModifiedFile(list);
}

QString KileInfo::documentTypeToString(KileDocument::Type type)
{
	switch(type) {
		case KileDocument::Undefined:
			return i18n("Undefined");
		case KileDocument::Text:
			return i18n("Text");
		case KileDocument::LaTeX:
			return i18n("LaTeX");
		case KileDocument::BibTeX:	
			return i18n("BibTeX");
		case KileDocument::Script:
			return i18n("Script");
	}
	return QString();
}

bool KileInfo::similarOrEqualURL(const KUrl &validurl, const KUrl &testurl)
{
	if ( testurl.isEmpty() || testurl.path().isEmpty() ) return false;

	bool absolute = testurl.path().startsWith("/");
	return (
		     (validurl == testurl) ||
		     (!absolute && validurl.path().endsWith(testurl.path()))
		   );
}

bool KileInfo::isOpen(const KUrl & url)
{
	KILE_DEBUG() << "==bool KileInfo::isOpen(const KUrl & url)=============" << endl;
	uint cnt = viewManager()->textViews().count();
	
	for ( uint i = 0; i < cnt; ++i)
	{
		if ( viewManager()->textView(i)->getDoc() && similarOrEqualURL(viewManager()->textView(i)->getDoc()->url(), url) )
			return true;
	}

	return false;
}

bool KileInfo::projectIsOpen(const KUrl & url)
{
	KileProject *project = docManager()->projectFor(url);

	return project != 0 ;
}

QString KileInfo::getSelection() const
{
	Kate::Document *doc = activeTextDocument();
	
	if (doc && doc->hasSelection())
	{
		return doc->selection();
	}
	
	return QString::null;
}

void KileInfo::clearSelection() const
{
	Kate::Document *doc = activeTextDocument();
	
	if (doc && doc->hasSelection())
	{
		doc->removeSelectedText();
	}
}

QString KileInfo::expandEnvironmentVars(const QString &str)
{
	static QRegExp reEnvVars("\\$(\\w+)");
	QString result = str;
	int index = -1;
	while ( (index = str.find(reEnvVars, index + 1)) != -1 )
		result.replace(reEnvVars.cap(0),getenv(reEnvVars.cap(1).local8Bit()));

	return result;
}

QString KileInfo::checkOtherPaths(const QString &path,const QString &file, int type)
{
	KILE_DEBUG() << "QString KileInfo::checkOtherPaths(const QString &path,const QString &file, int type)" << endl;
	QStringList inputpaths;
	QString configpaths;
	QFileInfo info;

	switch(type)
	{
		case bibinputs:
			configpaths = KileConfig::bibInputPaths() + ":$BIBINPUTS";
			break;
		case texinputs:
			configpaths = KileConfig::teXPaths() + ":$TEXINPUTS";
			break;
		case bstinputs:
			configpaths = KileConfig::bstInputPaths() + ":$BSTINPUTS";
			break;
		default:
			KILE_DEBUG() << "Unknown type in checkOtherPaths" << endl;
			return QString::null;
			break;
	}

	inputpaths = expandEnvironmentVars(configpaths).split( ":");
	inputpaths.prepend(path);

		// the first match is supposed to be the correct one
	for ( QStringList::Iterator it = inputpaths.begin(); it != inputpaths.end(); ++it )
	{
		KILE_DEBUG() << "path is " << *it << "and file is " << file << endl;
		info.setFile((*it) + '/' + file);
		if(info.exists())
		{
			KILE_DEBUG() << "filepath after correction is: " << info.path() << endl;
			return info.absoluteFilePath();
		}
	}
	return QString::null;
}

QString KileInfo::relativePath(const QString basepath, const QString & file)
{
	KUrl url = KUrl::fromPathOrUrl(file);
	QString path = url.directory();
	QString filename = url.fileName();

	KILE_DEBUG() <<"===findRelativeURL==================" << endl;
	KILE_DEBUG() << "\tbasepath : " <<  basepath << " path: " << path << endl;

	QStringList basedirs = basepath.split("/", QString::SkipEmptyParts);
	QStringList dirs = path.split("/", QString::SkipEmptyParts);

	uint nDirs = dirs.count();
	//uint nBaseDirs = basedirs.count();

	while ( dirs.count() > 0 && basedirs.count() > 0 &&  dirs[0] == basedirs[0] )
	{
		dirs.pop_front();
		basedirs.pop_front();
	}

	/*KILE_DEBUG() << "\tafter" << endl;
	for (uint i=0; i < basedirs.count(); ++i)
	{
		KILE_DEBUG() << "\t\tbasedirs " << i << ": " << basedirs[i] << endl;
	}

	for (uint i=0; i < dirs.count(); ++i)
	{
		KILE_DEBUG() << "\t\tdirs " << i << ": " << dirs[i] << endl;
	}*/

	if (nDirs != dirs.count() )
	{
		path = dirs.join("/");

		//KILE_DEBUG() << "\tpath : " << path << endl;

		if (basedirs.count() > 0)
		{
			for (uint j=0; j < basedirs.count(); ++j)
			{
				path = "../" + path;
			}
		}

		if ( path.length()>0 && path.right(1) != "/" ) path = path + '/';

		path = path+filename;
	}
	else //assume an absolute path was requested
	{
		path = url.path();
	}

	KILE_DEBUG() << "\trelative path : " << path << endl;

	return path;
}
