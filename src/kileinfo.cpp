/*************************************************************************************
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007-2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

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

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "widgets/structurewidget.h"
#include "configurationmanager.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "documentinfo.h"
#include "kileproject.h"
#include "kileuntitled.h"
#include "scriptmanager.h"
#include "editorkeysequencemanager.h"
#include "templates.h"

#include <kstandarddirs.h>
#include <qstringlist.h>
#include <qstring.h>

KileInfo::KileInfo(QObject *parent) :
	m_mainWindow(NULL),
	m_manager(NULL),
	m_jScriptManager(NULL),
	m_toolFactory(NULL),
	m_texKonsole(NULL),
	m_edit(NULL)
{
	m_configurationManager = new KileConfiguration::Manager(this, parent, "KileConfiguration::Manager");
	m_docManager = new KileDocument::Manager(this, parent, "KileDocument::Manager");
	m_viewManager= new KileView::Manager(this, parent, "KileView::Manager");
	m_templateManager = new KileTemplate::Manager(this, parent, "KileTemplate::Manager");
	m_editorKeySequenceManager = new KileEditorKeySequence::Manager(this, parent, "KileEditorKeySequence::Manager");
	QObject::connect(m_docManager,
	                 SIGNAL(documentModificationStatusChanged(KTextEditor::Document*, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
	                 m_viewManager,
	                 SLOT(reflectDocumentModificationStatus(KTextEditor::Document*, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
}

KileInfo::~KileInfo()
{
}

KTextEditor::Document * KileInfo::activeTextDocument() const
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if (view) return view->document(); else return NULL;
}

QString KileInfo::getName(KTextEditor::Document *doc, bool shrt)
{
	KILE_DEBUG() << "===KileInfo::getName(KTextEditor::Document *doc, bool " << shrt << ")===" << endl;
	QString title;
	
	if (!doc) {
		doc = activeTextDocument();
	}
	if (doc) {
		KILE_DEBUG() << "url " << doc->url().toLocalFile() << endl;
		title = shrt ? doc->url().fileName() : doc->url().toLocalFile();
	}

	return title;
}

QString KileInfo::getCompileName(bool shrt /* = false */)
{
	KileProject *project = docManager()->activeProject();

	if (m_singlemode) {
		if (project) {
			if (project->masterDocument().length() > 0) {
				KUrl master = KUrl::fromPathOrUrl(project->masterDocument());
				if (shrt) return master.fileName();
				else return master.toLocalFile();
			}
			else {
				KileProjectItem *item = project->rootItem(docManager()->activeProjectItem());
				if (item) {
					KUrl url = item->url();
					if (shrt) return url.fileName();
					else return url.toLocalFile();
				}
				else
					return QString();
			}
		}
		else {
			return getName(activeTextDocument(), shrt);
		}
	}
	else {
		QFileInfo fi(m_masterName);
		if(shrt) {
			return fi.fileName();
		}
		else {
			return m_masterName;
		}
	}
}

QString KileInfo::getFullFromPrettyName(const QString& name)
{
	if(name.isEmpty()) {
		return name;
	}
	
	QString file = name;

	if(file.left(2) == "./") {
		file = QFileInfo(outputFilter()->source()).absolutePath() + '/' + file.mid(2);
	}

	if(QDir::isRelativePath(file)) {
		file = QFileInfo(outputFilter()->source()).absolutePath() + '/' + file;
	}

	QFileInfo fi(file);
	if(file.isEmpty() || fi.isDir() || (! fi.exists()) || (! fi.isReadable())) {
		// - call from logwidget or error handling, which 
		//   tries to determine the LaTeX source file
		bool found = false;
		QStringList extlist = (m_extensions->latexDocuments()).split(' ');
		for(QStringList::Iterator it=extlist.begin(); it!=extlist.end(); ++it) {
			QString name = file + (*it);
			if(QFileInfo(name).exists()) {
				file = name;
				fi.setFile(name);
				found = true;
				break;
			}
		}
		if(!found) {
			file.clear();
		}
	}

	if(!fi.isReadable()) {
		return QString();
	}

	return file;
}

KUrl::List KileInfo::getParentsFor(KileDocument::Info *info)
{
	QList<KileProjectItem*> items = docManager()->itemsFor(info);
	KUrl::List list;
	for(QList<KileProjectItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		if((*it)->parent()) {
			list.append((*it)->parent()->url());
		}
	}
	return list;
}

#ifdef __GNUC__
#warning Check why this function pointer is needed!
#endif
const QStringList* KileInfo::retrieveList(const QStringList* (KileDocument::Info::*getit)() const, KileDocument::Info * docinfo /* = 0L */)
{
	m_listTemp.clear();
	if (docinfo == 0L) docinfo = docManager()->getInfo();
	KileProjectItem *item = docManager()->itemFor(docinfo, docManager()->activeProject());

	KILE_DEBUG() << "Kile::retrieveList()";
	if (item) {
		KileProject *project = item->project();
		KileProjectItem *root = project->rootItem(item);
		if (root) {
			KILE_DEBUG() << "\tusing root item " << root->url().fileName();

			QList<KileProjectItem*> children;
			children.append(root);
			root->allChildren(&children);

			const QStringList *list;

			for(QList<KileProjectItem*>::iterator it = children.begin(); it != children.end(); ++it) {
				KILE_DEBUG() << "\t" << (*it)->url().fileName();

				list = ((*it)->getInfo()->*getit)();
				if (list) {
					for(int i = 0; i < list->count(); ++i) {
						m_listTemp << (*list)[i];
					}
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


	bool absolute = QDir::isAbsolutePath(testurl.toLocalFile());
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
		if (viewManager()->textView(i)->document() && similarOrEqualURL(viewManager()->textView(i)->document()->url(), url)) {
			return true;
		}
	}

	return false;
}

bool KileInfo::projectIsOpen(const KUrl & url)
{
	KileProject *project = docManager()->projectFor(url);

	return project != 0 ;
}

QString KileInfo::defaultGraphicExt()
{
	return m_defGraphicExt;
}

void KileInfo::setDefaultGraphicExt(QString ext){
	m_defGraphicExt = ext;
}

QString KileInfo::getSelection() const
{
	KTextEditor::View *view = viewManager()->currentTextView();

	if (view && view->selection()) {
		return view->selectionText();
	}
	
	return QString();
}

void KileInfo::clearSelection() const
{
	KTextEditor::View *view = viewManager()->currentTextView();
	
	if(view && view->selection()) {
		view->removeSelectionText();
	}
}

QString KileInfo::expandEnvironmentVars(const QString &str)
{
	static QRegExp reEnvVars("\\$(\\w+)");
	QString result = str;
	int index = -1;
	while ( (index = str.indexOf(reEnvVars, index + 1)) != -1 )
		result.replace(reEnvVars.cap(0),getenv(reEnvVars.cap(1).toLocal8Bit()));

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
			return QString();
			break;
	}

	inputpaths = expandEnvironmentVars(configpaths).split(':');
	inputpaths.prepend(path);

		// the first match is supposed to be the correct one
	foreach(const QString &string, inputpaths){
		KILE_DEBUG() << "path is " << string << "and file is " << file << endl;
		info.setFile(string + '/' + file);
		if(info.exists()) {
			KILE_DEBUG() << "filepath after correction is: " << info.path() << endl;
			return info.absoluteFilePath();
		}
	}
	return QString();
}

