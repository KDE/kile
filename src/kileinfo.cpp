/*************************************************************************************
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007-2011 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include <QFileInfo>
#include <QObject>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "parser/parsermanager.h"
#include "widgets/structurewidget.h"
#include "configurationmanager.h"
#include "editorcommands.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "documentinfo.h"
#include "kileproject.h"
#include "scriptmanager.h"
#include "abbreviationmanager.h"
#include "editorkeysequencemanager.h"
#include "templates.h"
#include "utilities.h"

#include <kstandarddirs.h>
#include <QStringList>
#include <QString>

/*
 * Class KileInfo.
 */

KileInfo::KileInfo(KParts::MainWindow *parent) :
	m_mainWindow(parent),
	m_viewManager(NULL),
	m_manager(NULL),
	m_jScriptManager(NULL),
	m_toolFactory(NULL),
	m_texKonsole(NULL),
	m_errorHandler(NULL),
	m_edit(NULL)
{
	m_configurationManager = new KileConfiguration::Manager(this, parent, "KileConfiguration::Manager");
	m_docManager = new KileDocument::Manager(this, parent, "KileDocument::Manager");
	m_templateManager = new KileTemplate::Manager(this, parent, "KileTemplate::Manager");
	m_editorKeySequenceManager = new KileEditorKeySequence::Manager(this, parent, "KileEditorKeySequence::Manager");
	m_abbreviationManager = new KileAbbreviation::Manager(this, parent);
	m_parserManager = new KileParser::Manager(this, parent);
	m_editorCommands = new EditorCommands(this);
}

KileInfo::~KileInfo()
{
	// this has to be deleted before the editor component is destroyed
	delete m_editorCommands;
}

KTextEditor::Document * KileInfo::activeTextDocument() const
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if (view) return view->document(); else return NULL;
}

QString KileInfo::getName(KTextEditor::Document *doc, bool shrt) const
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

LaTeXOutputHandler* KileInfo::findCurrentLaTeXOutputHandler() const
{
	LaTeXOutputHandler *h = NULL;

	getCompileName(false, &h);
	return h;
}

QString KileInfo::getCompileName(bool shrt /* = false */, LaTeXOutputHandler** h /* = NULL */) const
{
	KileProject *project = docManager()->activeProject();

	if (m_singlemode) {
		if (project) {
			if(h) {
				*h = project;
			}
			return getCompileNameForProject(project, shrt);
		}
		else {
			KTextEditor::Document *doc = activeTextDocument();
			if(h) {
				*h = dynamic_cast<KileDocument::LaTeXInfo*>(m_docManager->textInfoFor(doc));
			}
			return getName(doc, shrt);
		}
	}
	else {
		QFileInfo fi(m_masterDocumentFileName);
		if(h) {
			*h = dynamic_cast<KileDocument::LaTeXInfo*>(m_docManager->textInfoFor(m_masterDocumentFileName));
		}
		if(shrt) {
			return fi.fileName();
		}
		else {
			return m_masterDocumentFileName;
		}
	}
}

QString KileInfo::getCompileNameForProject(KileProject *project, bool shrt) const
{
	if (!project->masterDocument().isEmpty()) {
		KUrl master(project->masterDocument());
		if(shrt) {
			return master.fileName();
		}
		else {
			return master.toLocalFile();
		}
	}
	else {
		KileProjectItem *item = project->rootItem(docManager()->activeProjectItem());
		if (item) {
			KUrl url = item->url();
			if(shrt) {
				return url.fileName();
			}
			else {
				return url.toLocalFile();
			}
		}
		else {
			return QString();
		}
	}
}

QString KileInfo::getFullFromPrettyName(const OutputInfo& info, const QString& name) const
{
	if(name.isEmpty()) {
		return name;
	}

	QString file = name;

	if(file.left(2) == "./") {
		file = QFileInfo(info.mainSourceFile()).absolutePath() + '/' + file.mid(2);
	}

	if(QDir::isRelativePath(file)) {
		file = QFileInfo(info.mainSourceFile()).absolutePath() + '/' + file;
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

QStringList KileInfo::retrieveList(QStringList (KileDocument::Info::*getit)() const, KileDocument::TextInfo *docinfo)
{
	if(!docinfo) {
		docinfo = docManager()->getInfo();
	}
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

			QStringList toReturn;
			for(QList<KileProjectItem*>::iterator it = children.begin(); it != children.end(); ++it) {
				const KileProjectItem *item = *it;
				KileDocument::TextInfo *textInfo = item->getInfo();
				KILE_DEBUG() << "\t" << item->url();

				if(textInfo) {
					toReturn << (textInfo->*getit)();
				}
			}
			return toReturn;
		}
		else {
			return QStringList();
		}
	}
	else if (docinfo) {
		return (docinfo->*getit)();
	}
	else {
		return QStringList();
	}
}

QStringList KileInfo::allLabels(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::allLabels()" << endl;
	return retrieveList(&KileDocument::Info::labels, info);
}

QStringList KileInfo::allBibItems(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::allBibItems()" << endl;
	return retrieveList(&KileDocument::Info::bibItems, info);
}

QStringList KileInfo::allBibliographies(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::bibliographies()" << endl;
	return retrieveList(&KileDocument::Info::bibliographies, info);
}

QStringList KileInfo::allDependencies(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::dependencies()" << endl;
	return retrieveList(&KileDocument::Info::dependencies, info);
}

QStringList KileInfo::allNewCommands(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::newCommands()" << endl;
	return retrieveList(&KileDocument::Info::newCommands, info);
}

QStringList KileInfo::allAsyFigures(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::asyFigures()" << endl;
	return retrieveList(&KileDocument::Info::asyFigures, info);
}

QStringList KileInfo::allPackages(KileDocument::TextInfo *info)
{
	KILE_DEBUG() << "Kile::allPackages()" << endl;
	return retrieveList(&KileDocument::Info::packages, info);
}

QString KileInfo::lastModifiedFile(KileDocument::TextInfo* info)
{
	if(!info) {
		info = docManager()->getInfo();
	}
	QStringList list = allDependencies(info);
	QFileInfo fileinfo(info->url().toLocalFile());
	list.append(fileinfo.fileName());

	return KileUtilities::lastModifiedFile(list, fileinfo.absolutePath());
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

	for (int i = 0; i < viewManager()->textViewCount(); ++i) {
		KTextEditor::View *view = viewManager()->textView(i);
		if (view->document() && similarOrEqualURL(view->document()->url(), url)) {
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
		result.replace(reEnvVars.cap(0),qgetenv(reEnvVars.cap(1).toLocal8Bit()));

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
			configpaths = KileConfig::bibInputPaths() + PATH_SEPARATOR + "$BIBINPUTS";
			break;
		case texinputs:
			configpaths = KileConfig::teXPaths() + PATH_SEPARATOR + "$TEXINPUTS";
			break;
		case bstinputs:
			configpaths = KileConfig::bstInputPaths() + PATH_SEPARATOR + "$BSTINPUTS";
			break;
		default:
			KILE_DEBUG() << "Unknown type in checkOtherPaths" << endl;
			return QString();
			break;
	}

	inputpaths = expandEnvironmentVars(configpaths).split(PATH_SEPARATOR);
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

