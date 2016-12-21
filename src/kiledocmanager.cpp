/*****************************************************************************
*   Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)      *
*             (C) 2006-2016 by Michel Ludwig (michel.ludwig@kdemail.net)     *
*             (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)  *
******************************************************************************/

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

#include "kiledocmanager.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDir>
#include <QDropEvent>
#include <QFile>
#include <QLabel>
#include <QList>
#include <QMimeData>
#include <QMimeType>
#include <QProgressDialog>
#include <QPushButton>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QUrl>

#include <KConfigGroup>
#include <KEncodingFileDialog>
#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include "dialogs/cleandialog.h"
#include "dialogs/listselector.h"
#include "dialogs/managetemplatesdialog.h"
#include "dialogs/newfilewizard.h"
#include "dialogs/projectdialogs.h"
#include "documentinfo.h"
#include "errorhandler.h"
#include "kileconfig.h"
#include "kiledebug.h"
#include "kileinfo.h"
#include "kileproject.h"
#include "kilestdtools.h"
#include "kiletool_enums.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "livepreview.h"
#include "parser/parsermanager.h"
#include "scriptmanager.h"
#include "templates.h"
#include "widgets/filebrowserwidget.h"
#include "widgets/konsolewidget.h"
#include "widgets/projectview.h"
#include "widgets/structurewidget.h"

/*
 * Newly created text documents have an empty URL and a non-empty document name.
 */

#define MAX_NUMBER_OF_STORED_SETTINGS 50

namespace KileDocument
{

Manager::Manager(KileInfo *info, QObject *parent, const char *name) :
	QObject(parent),
	m_ki(info),
	m_progressDialog(Q_NULLPTR),
	m_currentlySavingAll(false),
	m_currentlyOpeningFile(false)
{
	setObjectName(name);
	m_editor = KTextEditor::Editor::instance();
	if(!m_editor) {
		KMessageBox::error(m_ki->mainWindow(), i18n("No editor component found. Please check your KDE installation."),
		                                       i18n("No editor component found."));
	}
}

Manager::~Manager()
{
	KILE_DEBUG_MAIN << "==KileDocument::Manager::~Manager()=========";
	if(m_progressDialog.isNull()) {
		delete m_progressDialog.data();
	}
}

void Manager::readConfig()
{
}

void Manager::writeConfig()
{
}

void Manager::trashDoc(TextInfo *docinfo, KTextEditor::Document *doc /*= Q_NULLPTR */ )
{
	KILE_DEBUG_MAIN << "==void Manager::trashDoc(" << docinfo->url().toLocalFile() << ")=====";

	if(m_ki->isOpen(docinfo->url())) {
		return;
	}

	if(doc) {
		doc = docinfo->getDoc();
	}

	//look for doc before we detach the docinfo
	//if we do it the other way around, docFor will always return nil
 	if(!doc) {
		doc = docFor(docinfo->url());
	}

	KILE_DEBUG_MAIN << "DETACHING " << docinfo;
	docinfo->detach();

	KILE_DEBUG_MAIN << "\tTRASHING " <<  doc;
	if(!doc) {
		return;
	}

	KILE_DEBUG_MAIN << "just checking: docinfo->getDoc() =  " << docinfo->getDoc();
	KILE_DEBUG_MAIN << "just checking: docFor(docinfo->url()) = " << docFor(docinfo->url());

	for (int i = 0; i < m_textInfoList.count(); ++i) {
		if((m_textInfoList.at(i) != docinfo) && (m_textInfoList.at(i)->getDoc() == doc)) {
			KMessageBox::information(0, i18n("The internal structure of Kile is corrupted (probably due to a bug in Kile). Please select Save All from the File menu and close Kile.\nThe Kile team apologizes for any inconvenience and would appreciate a bug report."));
			qWarning() << "docinfo " << m_textInfoList.at(i) << " url " << m_textInfoList.at(i)->url().fileName() << " has a wild pointer!!!";
		}
	}

	KILE_DEBUG_MAIN << "DELETING doc";
	delete doc;
}

// update all Info's with changed user commands
void Manager::updateInfos()
{
	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		(*it)->updateStructLevelInfo();
	}
}

bool Manager::isOpeningFile()
{
	return m_currentlyOpeningFile;
}

KTextEditor::Editor* Manager::getEditor()
{
	return m_editor;
}

KTextEditor::Document* Manager::docFor(const QUrl &url)
{
	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		TextInfo *info = *it;

		if(m_ki->similarOrEqualURL(info->url(), url)) {
			return info->getDoc();
		}
	}

	return Q_NULLPTR;
}

TextInfo* Manager::getInfo() const
{
	KTextEditor::Document *doc = m_ki->activeTextDocument();
	if(doc) {
		return textInfoFor(doc);
	}
	else {
		return Q_NULLPTR;
	}
}

TextInfo* Manager::textInfoFor(const QUrl &url)
{
	if(url.isEmpty()) {
		return Q_NULLPTR;
	}

	KILE_DEBUG_MAIN << "==KileInfo::textInfoFor(" << url << ")==========================";

	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		TextInfo *info = *it;

		if (info->url() == url) {
			return info;
		}
	}

	// the URL might belong to a TextInfo* which currently doesn't have a KTextEditor::Document*
	// associated with it, i.e. a project item which isn't open in the editor
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProjectItem *item = (*it)->item(url);

		// all project items (across different projects) that share a URL have the same TextInfo*;
		// so, the first one we find is good enough
		if(item) {
			KileDocument::TextInfo *info = item->getInfo();
			if(info) {
				return info;
			}
		}
	}

	KILE_DEBUG_MAIN << "\tCOULD NOT find info for " << url;
	return Q_NULLPTR;
}

TextInfo* Manager::textInfoFor(KTextEditor::Document* doc) const
{
	if(!doc) {
		return Q_NULLPTR;
	}

	// TextInfo* objects that contain KTextEditor::Document* pointers must be open in the editor, i.e.
	// we don't have to look through the project items
	for(QList<TextInfo*>::const_iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		if((*it)->getDoc() == doc) {
			return (*it);
		}
	}

	KILE_DEBUG_MAIN << "\tCOULD NOT find info for" << doc->url() << "by searching via a KTextEditor::Document*";
	return Q_NULLPTR;
}

QUrl Manager::urlFor(TextInfo* textInfo)
{
	KileProjectItem *item = itemFor(textInfo);

	QUrl url;
	if(item) {
		url = item->url(); // all items with 'textInfo' share the same URL
	}
	else {
		KTextEditor::Document *document = textInfo->getDoc();
		if(document) {
			url = document->url();
		}
	}
	return url;
}

KileProject* Manager::projectForMember(const QUrl &memberUrl)
{
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;

		if(project->contains(memberUrl)) {
			return project;
		}
	}
	return Q_NULLPTR;
}

KileProject* Manager::projectFor(const QUrl &projecturl)
{
	//find project with url = projecturl
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;
		if(project->url() == projecturl) {
			return project;
		}
	}

	return Q_NULLPTR;
}

KileProject* Manager::projectFor(const QString &name)
{
	//find project with url = projecturl
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;

		if (project->name() == name) {
			return project;
		}
	}

	return Q_NULLPTR;
}

KileProjectItem* Manager::itemFor(const QUrl &url, KileProject *project /*=0L*/) const
{
	if (!project) {
		for(QList<KileProject*>::const_iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
			KileProject *project = *it;

			KileProjectItem *item = project->item(url);
			if(item) {
				return item;
			}
		}
		return Q_NULLPTR;
	}
	else {
		return project->item(url);
	}
}

KileProjectItem* Manager::itemFor(TextInfo *docinfo, KileProject *project /*=0*/) const
{
	if (!project) {
		for(QList<KileProject*>::const_iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
			KileProject *project = *it;

			KileProjectItem *item = project->item(docinfo);
			if(item) {
				return item;
			}
		}
		return Q_NULLPTR;
	}
	else {
		return project->item(docinfo);
	}
}

QList<KileProjectItem*> Manager::itemsFor(Info *docinfo) const
{
	if(!docinfo) {
		return QList<KileProjectItem*>();
	}

	KILE_DEBUG_MAIN << "==KileInfo::itemsFor(" << docinfo->url().fileName() << ")============";
	QList<KileProjectItem*> list;
	for(QList<KileProject*>::const_iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;

		KILE_DEBUG_MAIN << "\tproject: " << (*it)->name();
		if(project->contains(docinfo)) {
			KILE_DEBUG_MAIN << "\t\tcontains";
			list.append(project->item(docinfo));
		}
	}

	return list;
}

QList<KileProjectItem*> Manager::itemsFor(const QUrl &url) const
{
	QList<KileProjectItem*> list;
	for(QList<KileProject*>::const_iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;

		if(project->contains(url)) {
			list.append(project->item(url));
		}
	}

	return list;
}

bool Manager::isProjectOpen()
{
	return ( m_projects.count() > 0 );
}

KileProject* Manager::activeProject()
{
	KTextEditor::Document *doc = m_ki->activeTextDocument();

	if (doc) {
		return projectForMember(doc->url());
	}
	else {
		return Q_NULLPTR;
	}
}

KileProjectItem* Manager::activeProjectItem()
{
	KileProject *curpr = activeProject();
	KTextEditor::Document *doc = m_ki->activeTextDocument();

	if (curpr && doc) {
		QList<KileProjectItem*> list = curpr->items();

		for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
			KileProjectItem *item = *it;

			if (item->url() == doc->url()) {
				return item;
			}
		}
	}

	return Q_NULLPTR;
}

TextInfo* Manager::createTextDocumentInfo(KileDocument::Type type, const QUrl &url, const QUrl& baseDirectory)
{
	TextInfo *docinfo = Q_NULLPTR;

	// check whether this URL belongs to an opened project and a TextInfo* object has already
	// been created for that URL
	docinfo = textInfoFor(url);

	if(!docinfo) {
		switch(type) {
			case Undefined: // fall through
			case Text:
				KILE_DEBUG_MAIN << "CREATING TextInfo for " << url.url();
				docinfo = new TextInfo(m_ki->extensions(),
				                       m_ki->abbreviationManager(),
				                       m_ki->parserManager());
				break;
			case LaTeX:
				KILE_DEBUG_MAIN << "CREATING LaTeXInfo for " << url.url();
				docinfo = new LaTeXInfo(m_ki->extensions(),
				                        m_ki->abbreviationManager(),
				                        m_ki->latexCommands(),
				                        m_ki->editorExtension(),
				                        m_ki->configurationManager(),
				                        m_ki->codeCompletionManager(),
				                        m_ki->livePreviewManager(),
				                        m_ki->parserManager());
				break;
			case BibTeX:
				KILE_DEBUG_MAIN << "CREATING BibInfo for " << url.url();
				docinfo = new BibInfo(m_ki->extensions(),
				                      m_ki->abbreviationManager(),
				                      m_ki->parserManager(),
				                      m_ki->latexCommands());
				break;
			case Script:
				KILE_DEBUG_MAIN << "CREATING ScriptInfo for " << url.url();
				docinfo = new ScriptInfo(m_ki->extensions(),
				                         m_ki->abbreviationManager(),
				                         m_ki->parserManager());
				break;
		}
		docinfo->setBaseDirectory(baseDirectory);
		emit(documentInfoCreated(docinfo));
		m_textInfoList.append(docinfo);
	}

	KILE_DEBUG_MAIN << "DOCINFO: returning " << docinfo << " " << docinfo->url().fileName();
	return docinfo;
}

void Manager::recreateTextDocumentInfo(TextInfo *oldinfo)
{
	QList<KileProjectItem*> list = itemsFor(oldinfo);
	QUrl url = oldinfo->url();
	TextInfo *newinfo = createTextDocumentInfo(m_ki->extensions()->determineDocumentType(url), url, oldinfo->getBaseDirectory());

	newinfo->setDoc(oldinfo->getDoc());

	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		(*it)->setInfo(newinfo);
	}

	removeTextDocumentInfo(oldinfo);

	emit(updateStructure(false, newinfo));
}

bool Manager::removeTextDocumentInfo(TextInfo *docinfo, bool closingproject /* = false */)
{
	KILE_DEBUG_MAIN << "==Manager::removeTextDocumentInfo(Info *docinfo)=====";
	QList<KileProjectItem*> itms = itemsFor(docinfo);
	bool oneItem = false;
	if(itms.count() == 1) {
		oneItem = true;
	}

	if(itms.count() == 0 || ( closingproject && oneItem )) {
		KILE_DEBUG_MAIN << "\tremoving " << docinfo <<  " count = " << m_textInfoList.count();

		// we still have to stop parsing for 'docinfo'
		QUrl url = urlFor(docinfo);
		if(url.isValid()) {
			m_ki->parserManager()->stopDocumentParsing(url);
		}

		m_textInfoList.removeAll(docinfo);

		emit(closingDocument(docinfo));

		cleanupDocumentInfoForProjectItems(docinfo);
		delete docinfo;

		return true;
	}

	KILE_DEBUG_MAIN << "\tnot removing " << docinfo;
	return false;
}


KTextEditor::Document* Manager::createDocument(const QUrl &url, TextInfo *docinfo, const QString& encoding,
                                                                                   const QString& mode,
                                                                                   const QString& highlight)
{
	KILE_DEBUG_MAIN << "==KTextEditor::Document* Manager::createDocument()===========";

	KTextEditor::Document *doc = Q_NULLPTR;

	if(!m_editor) {
		return Q_NULLPTR;
	}

	doc = docFor(url);
	if (doc) {
		qWarning() << url << " already has a document!";
		return doc;
	}
	doc = m_editor->createDocument(Q_NULLPTR);
	KILE_DEBUG_MAIN << "appending document " <<  doc;

	docinfo->setDoc(doc); // do this here to set up all the signals correctly in 'TextInfo'
	doc->setEncoding(encoding);

	KILE_DEBUG_MAIN << "url is = " << docinfo->url();

	if(!url.isEmpty()) {
		bool r = doc->openUrl(url);
		// don't add scripts to the recent files
		if(r && docinfo->getType() != Script) {
        		emit(addToRecentFiles(url));
		}
	}

	//handle changes of the document
	connect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SIGNAL(documentNameChanged(KTextEditor::Document*)));
	connect(doc, SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SIGNAL(documentUrlChanged(KTextEditor::Document*)));
	if(doc->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("readWriteChanged(KTextEditor::Document*)")) >= 0) {
		// signal available in KDE 4.10
		connect(doc, SIGNAL(readWriteChanged(KTextEditor::Document*)),
		        this, SIGNAL(documentReadWriteStateChanged(KTextEditor::Document*)));
	}

	connect(doc, SIGNAL(modifiedChanged(KTextEditor::Document*)), this, SLOT(newDocumentStatus(KTextEditor::Document*)));
	KTextEditor::ModificationInterface *modificationInterface = qobject_cast<KTextEditor::ModificationInterface*>(doc);
	if(modificationInterface) {
		modificationInterface->setModifiedOnDiskWarning(true);
		connect(doc, SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                        this, SIGNAL(documentModificationStatusChanged(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
	}

	if(!mode.isEmpty()){
		docinfo->setMode(mode);     // this ensures that mode passed with the mode parameter is actually used
	}
	if(!highlight.isEmpty()){
		docinfo->setHighlightingMode(highlight);
	}
	// FIXME: the whole structure updating stuff needs to be rewritten; updates should originate from
	//        the docinfo only, i.e. the structure view should just react to changes!
	connect(docinfo, SIGNAL(completed(KileDocument::Info*)), m_ki->structureWidget(), SLOT(update(KileDocument::Info*)));

	KILE_DEBUG_MAIN << "createDocument: url " << doc->url();
	KILE_DEBUG_MAIN << "createDocument: SANITY check: " << (docinfo->getDoc() == docFor(docinfo->url()));
	return doc;
}

// WARNING: 'item' must have been set up with a TextInfo* object already
KTextEditor::View* Manager::loadItem(KileDocument::Type type, KileProjectItem *item, const QString & text, bool openProjectItemViews)
{
	KTextEditor::View *view = Q_NULLPTR;

	KILE_DEBUG_MAIN << "==loadItem(" << item->url() << ")======";

	if(item->type() != KileProjectItem::Image) {
		view = loadText(type, item->url(), item->encoding(), openProjectItemViews && item->isOpen(), item->mode(), item->highlight(), text);
		KILE_DEBUG_MAIN << "\tloadItem: docfor = " << docFor(item->url());

		TextInfo *docinfo = item->getInfo();

		KILE_DEBUG_MAIN << "\tloadItem: docinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url());
		if ( docinfo->getDoc() != docFor(docinfo->url()) ) qWarning() << "docinfo->getDoc() != docFor()";
	}
	else {
		KILE_DEBUG_MAIN << "\tloadItem: no document generated";
		TextInfo *docinfo = item->getInfo();

		if(!docFor(item->url())) {
			docinfo->detach();
			KILE_DEBUG_MAIN << "\t\t\tdetached";
		}
	}

	return view;
}

KTextEditor::View* Manager::loadText(KileDocument::Type type, const QUrl &url, const QString& encoding,
                                                                               bool create,
                                                                               const QString& mode,
                                                                               const QString& highlight,
                                                                               const QString& text,
                                                                               int index,
                                                                               const QUrl &baseDirectory)
{
	KILE_DEBUG_MAIN << "==loadText(" << url.url() << ")=================";
	//if doc already opened, update the structure view and return the view
	if(!url.isEmpty() && m_ki->isOpen(url)) {
		return m_ki->viewManager()->switchToTextView(url);
	}

	TextInfo *docinfo = createTextDocumentInfo(type, url, baseDirectory);
	KTextEditor::Document *doc = createDocument(url, docinfo, encoding, mode, highlight);

	m_ki->structureWidget()->clean(docinfo);

	if(!text.isEmpty()) {
		doc->setText(text);
	}

	if (doc && create) {
		return m_ki->viewManager()->createTextView(docinfo, index);
	}

	KILE_DEBUG_MAIN << "just after createView()";
	KILE_DEBUG_MAIN << "\tdocinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url());

	return Q_NULLPTR;
}

//FIXME: template stuff should be in own class
KTextEditor::View* Manager::loadTemplate(TemplateItem *sel)
{
	KILE_DEBUG_MAIN << "templateitem *sel = " << sel;
	QString text;

	if(!sel) {
		return Q_NULLPTR;
	}

	if (sel->name() != KileTemplate::Manager::defaultEmptyTemplateCaption()
	    && sel->name() != KileTemplate::Manager::defaultEmptyLaTeXTemplateCaption()
	    && sel->name() != KileTemplate::Manager::defaultEmptyBibTeXTemplateCaption()) {
		if(!m_editor) {
			return Q_NULLPTR;
		}
		//create a new document to open the template in
		KTextEditor::Document *tempdoc = m_editor->createDocument(Q_NULLPTR);

		if (!tempdoc->openUrl(QUrl::fromLocalFile(sel->path()))) {
			KMessageBox::error(m_ki->mainWindow(), i18n("Could not find template: %1", sel->name()), i18n("File Not Found"));
		}
		else {
			//substitute templates variables
			text = tempdoc->text();
			delete tempdoc;
			replaceTemplateVariables(text);
		}
	}

	KileDocument::Type type = sel->type();
	//always set the base directory for scripts
	return createDocumentWithText(text, type, QString(), (type == KileDocument::Script ? QUrl::fromLocalFile(m_ki->scriptManager()->getLocalScriptDirectory()) : QUrl()));
}

KTextEditor::View* Manager::createDocumentWithText(const QString& text, KileDocument::Type type /* = KileDocument::Undefined */, const QString& /* extension */, const QUrl &baseDirectory)
{
	KTextEditor::View *view = loadText(type, QUrl(), QString(), true, QString(), QString(), text, -1, baseDirectory);
	if(view) {
		//FIXME this shouldn't be necessary!!!
		view->document()->setModified(true);
		newDocumentStatus(view->document());
	}

	return view;
}

KTextEditor::View* Manager::createNewJScript()
{
	KTextEditor::View *view = createDocumentWithText(QString(), Script, "js", QUrl::fromLocalFile(m_ki->scriptManager()->getLocalScriptDirectory()));
	emit(updateStructure(false, Q_NULLPTR));
	emit(updateModeStatus());
	return view;
}

KTextEditor::View* Manager::createNewLaTeXDocument()
{
	KTextEditor::View *view = createDocumentWithText(QString(), LaTeX);
	emit(updateStructure(false, Q_NULLPTR));
	emit(updateModeStatus());
	return view;
}

void Manager::replaceTemplateVariables(QString &line)
{
	line=line.replace("$$AUTHOR$$", KileConfig::author());
	line=line.replace("$$DOCUMENTCLASSOPTIONS$$", KileConfig::documentClassOptions());
	if (!KileConfig::templateEncoding().isEmpty()) { line=line.replace("$$INPUTENCODING$$", "\\usepackage["+ KileConfig::templateEncoding() +"]{inputenc}");}
	else {
		line = line.remove("$$INPUTENCODING$$");
	}
}

void Manager::createTemplate()
{
	KTextEditor::View *view = m_ki->viewManager()->currentTextView();
	if (view) {
		if (view->document()->isModified()) {
			KMessageBox::information(m_ki->mainWindow(),i18n("Please save the file first."));
			return;
		}
	}
	else {
		KMessageBox::information(m_ki->mainWindow(),i18n("Open/create a document first."));
		return;
	}

	QUrl url = view->document()->url();
	KileDocument::Type type = m_ki->extensions()->determineDocumentType(url);

	if(type == KileDocument::Undefined || type == KileDocument::Text) {
		KMessageBox::information(m_ki->mainWindow(),i18n("A template for this type of document cannot be created."));
		return;
	}

	ManageTemplatesDialog mtd(m_ki->templateManager(), url, i18n("Create Template From Document"));
	mtd.exec();
}

void Manager::removeTemplate()
{
	ManageTemplatesDialog mtd(m_ki->templateManager(), i18n("Remove Template"));
	mtd.exec();
}

void Manager::fileNew(KileDocument::Type type)
{
	NewFileWizard *nfw = new NewFileWizard(m_ki->templateManager(), type, m_ki->mainWindow());
	if(nfw->exec()) {
		KTextEditor::View *view = loadTemplate(nfw->getSelection());
		if(view) {
			if(nfw->useWizard()) {
				emit(startWizard());
			}
			emit(updateStructure(false, Q_NULLPTR));
			emit(updateModeStatus());
		}
	}
	delete nfw;
}

void Manager::fileNewScript()
{
	fileNew(KileDocument::Script);
}

void Manager::fileNew(const QUrl &url)
{
	//create an empty file
	QFile file(url.toLocalFile());
	file.open(QIODevice::ReadWrite);
	file.close();

	fileOpen(url, QString());
}

void Manager::fileOpen()
{
	//determine the starting dir for the file dialog
	QString compileName = m_ki->getCompileName();
	QString currentDir;
	if(QFileInfo(compileName).exists()) {
		currentDir = QFileInfo(compileName).absolutePath();
	}
	else {
		currentDir = m_ki->fileSelector()->currentUrl().toLocalFile();
	}

	// use a filter for fileOpen dialog
	Extensions *extensions = m_ki->extensions();
	QString filter = extensions->fileFilterKDEStyle(true, {KileDocument::Extensions::TEX,
	                                                       KileDocument::Extensions::PACKAGES,
	                                                       KileDocument::Extensions::BIB,
	                                                       KileDocument::Extensions::METAPOST});

	// try to get the current encoding, this is kind of ugly ...
	QString encoding = m_ki->toolManager()->config()->group("Kate Document Defaults").readEntry("Encoding","");

	//get the URLs
	KEncodingFileDialog::Result result = KEncodingFileDialog::getOpenUrlsAndEncoding(encoding, QUrl::fromLocalFile(currentDir), filter, m_ki->mainWindow(), i18n("Open Files"));

	//open them
	const QList<QUrl>& urls = result.URLs;
	for (QList<QUrl>::ConstIterator i = urls.begin(); i != urls.end(); ++i) {
		const QUrl& url = *i;
		if(m_ki->extensions()->isProjectFile(url)) { // this can happen... (bug 317432)
			KILE_DEBUG_MAIN << "file is a project file:" << url;
			projectOpen(url);
			continue;
		}

		fileOpen(url, result.encoding);
	}
}

void Manager::fileSelected(const KFileItem& file)
{
	fileSelected(file.url());
}

void Manager::fileSelected(const KileProjectItem * item)
{
	fileOpen(item->url(), item->encoding());
}

void Manager::fileSelected(const QUrl &url)
{
	fileOpen(url, QString());
}

void Manager::saveURL(const QUrl &url)
{
	KTextEditor::Document *doc = docFor(url);
	if(doc) {
		doc->save();
	}
}

void Manager::newDocumentStatus(KTextEditor::Document *doc)
{
	KILE_DEBUG_MAIN << "void Manager::newDocumentStatus(Kate::Document)" << endl;
	if(!doc) {
		return;
	}

	// sync terminal
	m_ki->texKonsole()->sync();

	emit(documentModificationStatusChanged(doc, doc->isModified(), KTextEditor::ModificationInterface::OnDiskUnmodified));
}

bool Manager::fileSaveAll(bool disUntitled)
{
	// this can occur when autosaving should take place when we
	// are still busy with it (KIO::NetAccess keeps the event loop running)
	if(m_currentlySavingAll) {
		return true;
	}
	m_currentlySavingAll = true;
	KTextEditor::View *view = Q_NULLPTR;
	QFileInfo fi;
	bool oneSaveFailed = false;
	QUrl url, backupUrl;

	KILE_DEBUG_MAIN << "===Kile::fileSaveAll(disUntitled = " << disUntitled <<")";

	for(int i = 0; i < m_ki->viewManager()->textViewCount(); ++i) {
		view = m_ki->viewManager()->textView(i);

		if(view && view->document()->isModified()) {
			url = view->document()->url();
			fi.setFile(url.toLocalFile());

			if(!disUntitled || !(disUntitled && url.isEmpty()))  { // either we don't disregard untitled docs, or the doc has a title
				KILE_DEBUG_MAIN << "trying to save: " << url.toLocalFile();
				bool saveResult = view->document()->documentSave();
				fi.refresh();

				if(!saveResult) {
					oneSaveFailed = true;
					m_ki->errorHandler()->printMessage(KileTool::Error,
					                                   i18n("Kile encountered problems while saving the file %1. Do you have enough free disk space left?", url.toDisplayString()),
					                                   i18n("Saving"));
				}
			}
		}
	}

	/*
	 This may look superfluos but actually it is not, in the case of multiple modified docs it ensures that the structure view keeps synchronized with the currentTextView
	 And if we only have one masterdoc or none nothing goes wrong.
	*/
	emit(updateStructure(false, Q_NULLPTR));
	m_currentlySavingAll = false;
	return !oneSaveFailed;
}

TextInfo* Manager::fileOpen(const QUrl &url, const QString& encoding, int index)
{
	m_currentlyOpeningFile = true;
	KILE_DEBUG_MAIN << "==Kile::fileOpen==========================";

	KILE_DEBUG_MAIN << "url is " << url.url();
	const QUrl realurl = symlinkFreeURL(url);
	KILE_DEBUG_MAIN << "symlink free url is " << realurl.url();

	bool isopen = m_ki->isOpen(realurl);
	if(isopen) {
		m_currentlyOpeningFile = false; // has to be before the 'switchToTextView' call as
		                                // it emits signals that are handled by the live preview manager
		m_ki->viewManager()->switchToTextView(realurl);
		return textInfoFor(realurl);
	}

	KTextEditor::View *view = loadText(m_ki->extensions()->determineDocumentType(realurl), realurl, encoding, true, QString(), QString(), QString(), index);
	QList<KileProjectItem*> itemList = itemsFor(realurl);
	TextInfo *textInfo = textInfoFor(realurl);

	for(QList<KileProjectItem*>::iterator it = itemList.begin(); it != itemList.end(); ++it) {
		(*it)->setInfo(textInfo);
	}

	if(itemList.isEmpty()) {
		emit addToProjectView(realurl);
		loadDocumentAndViewSettings(textInfo);
	}
	else if(view) {
		KileProjectItem *item = itemList.first();
		item->loadDocumentAndViewSettings();
	}

	emit(updateStructure(false, Q_NULLPTR));
	emit(updateModeStatus());
	// update undefined references in this file
	emit(updateReferences(textInfoFor(realurl)));
	m_currentlyOpeningFile = false;
	emit documentOpened(textInfo);
	return textInfo;
}

bool Manager::fileSave(KTextEditor::View *view)
{
	// the 'data' property can be set by the view manager
	QAction *action = dynamic_cast<QAction*>(QObject::sender());
	if(action) {
		QVariant var = action->data();
		if(!view && var.isValid()) {
			view = var.value<KTextEditor::View*>();
			// the 'data' property for the relevant actions is cleared
			// inside the view manager
		}
	}
	if(!view) {
		view = m_ki->viewManager()->currentTextView();
	}
	if(!view) {
		return false;
	}
	QUrl url = view->document()->url();
	if(url.isEmpty()) { // newly created document
		return fileSaveAs(view);
	}
	else {
		bool ret = view->document()->documentSave();
		emit(updateStructure(false, textInfoFor(view->document())));
		return ret;
	}
}

bool Manager::fileSaveAs(KTextEditor::View* view)
{
	// the 'data' property can be set by the view manager
	QAction *action = dynamic_cast<QAction*>(QObject::sender());
	if(action) {
		QVariant var = action->data();
		if(!view && var.isValid()) {
			view = var.value<KTextEditor::View*>();
			// the 'data' property for the relevant actions is cleared
			// inside the view manager
		}
	}
	if(!view) {
		view = m_ki->viewManager()->currentTextView();
	}
	if(!view) {
		return false;
	}

	KTextEditor::Document* doc = view->document();
	Q_ASSERT(doc);
	KileDocument::TextInfo* info = textInfoFor(doc);
	Q_ASSERT(info);
	QUrl startUrl = info->url();
	QUrl oldURL = info->url();
	if(startUrl.isEmpty()) {
		QUrl baseDirectory = info->getBaseDirectory();
		if(baseDirectory.isEmpty()) {
			startUrl = QUrl("kfiledialog:///KILE_LATEX_SAVE_DIR");
		}
		else {
			startUrl = baseDirectory;
		}
	}

	KILE_DEBUG_MAIN << "startUrl is " << startUrl;
	KEncodingFileDialog::Result result;
	QUrl saveURL;
	while(true) {
		QString filter = m_ki->extensions()->fileFilterKDEStyle(true, info->getFileFilter());

		result = KEncodingFileDialog::getSaveUrlAndEncoding(doc->encoding(), startUrl, filter, m_ki->mainWindow(), i18n("Save File"));
		if(result.URLs.isEmpty() || result.URLs.first().isEmpty()) {
			return false;
		}
		saveURL = result.URLs.first();
		if(info->getType() == KileDocument::LaTeX) {
			saveURL = Info::makeValidTeXURL(saveURL, m_ki->mainWindow(),
			                                m_ki->extensions()->isTexFile(saveURL), false); // don't check for file existence
		}

		auto statJob = KIO::stat(saveURL, KIO::StatJob::SourceSide, 0);
		KJobWidgets::setWindow(statJob, m_ki->mainWindow());
		if (statJob->exec()) { // check for writing possibility
			int r =  KMessageBox::warningContinueCancel(m_ki->mainWindow(), i18n("A file with the name \"%1\" exists already. Do you want to overwrite it?", saveURL.fileName()), i18n("Overwrite File?"), KStandardGuiItem::overwrite());
			if(r != KMessageBox::Continue) {
				continue;
			}
		}
		break;
	}
	doc->setEncoding(result.encoding);
	if(!doc->saveAs(saveURL)) {
		return false;
	}
	if(oldURL != saveURL) {
		if(info->isDocumentTypePromotionAllowed()) {
			recreateTextDocumentInfo(info);
			info = textInfoFor(doc);
		}
		m_ki->structureWidget()->updateUrl(info);
		emit addToRecentFiles(saveURL);
		emit addToProjectView(doc->url());
	}
	emit(documentSavedAs(view, info));
	return true;
}

void Manager::fileSaveCopyAs()
{
	KTextEditor::View *view = Q_NULLPTR;
	// the 'data' property can be set by the view manager
	QAction *action = dynamic_cast<QAction*>(QObject::sender());
	if(action) {
		QVariant var = action->data();
		if(var.isValid()) {
			view = var.value<KTextEditor::View*>();
			// the 'data' property for the relevant actions is cleared
			// inside the view manager
		}
	}
	if(!view) {
		view = m_ki->viewManager()->currentTextView();
	}
	if(!view) {
		return;
	}

	KTextEditor::Document *doc = view->document();

	if(!doc) {
		return;
	}

	KileDocument::TextInfo *originalInfo = textInfoFor(doc);

	if(!originalInfo) {
		return;
	}

	view = createDocumentWithText(doc->text(),originalInfo->getType());

	KileDocument::TextInfo *newInfo = textInfoFor(view->document());

	if(originalInfo->url().isEmpty()) { // untitled doc
		newInfo->setBaseDirectory(m_ki->fileSelector()->currentUrl());
	}
	else {
		newInfo->setBaseDirectory(originalInfo->url());
	}

	fileSaveAs(view);

	doc = view->document();
	if(doc && !doc->isModified()) { // fileSaveAs was successful
		fileClose(doc);
	}
}

bool Manager::fileCloseAllOthers(KTextEditor::View *currentView)
{
	// the 'data' property can be set by the view manager
	QAction *action = dynamic_cast<QAction*>(QObject::sender());
	if(action) {
		QVariant var = action->data();
		if(!currentView && var.isValid()) {
			// the 'data' property for the relevant actions is cleared
			// inside the view manager
			currentView = var.value<KTextEditor::View*>();
		}
	}
	if(!currentView) {
		currentView = m_ki->viewManager()->currentTextView();
	}
	if(!currentView) {
		return false;
	}

	QList<KTextEditor::View*> viewList;
	for(int i = 0; i < m_ki->viewManager()->textViewCount(); ++i) {
		KTextEditor::View *view = m_ki->viewManager()->textView(i);
		if(currentView == view) {
			continue;
		}
		viewList.push_back(view);

	}
	for(QList<KTextEditor::View*>::iterator it = viewList.begin();
	    it != viewList.end(); ++it) {
		if (!fileClose(*it)) {
			return false;
		}
	}
	return true;
}

bool Manager::fileCloseAll()
{
	KTextEditor::View * view = m_ki->viewManager()->currentTextView();

	//assumes one view per doc here
	while(m_ki->viewManager()->textViewCount() > 0) {
		view = m_ki->viewManager()->textView(0);
		if (!fileClose(view->document())) {
			return false;
		}
	}
	return true;
}

bool Manager::fileClose(const QUrl &url)
{
	KTextEditor::Document *doc = docFor(url);
	if(!doc) {
		return true;
	}
	else {
		return fileClose(doc);
	}
}

bool Manager::fileClose(KTextEditor::View *view)
{
	// the 'data' property can be set by the view manager
	QAction *action = dynamic_cast<QAction*>(QObject::sender());
	if(action) {
		QVariant var = action->data();
		if(!view && var.isValid()) {
			view = var.value<KTextEditor::View*>();
			// the 'data' property for the relevant actions is cleared
			// inside the view manager
		}
	}
	if(!view) {
		view = m_ki->viewManager()->currentTextView();
	}
	if(!view) {
		return false;
	}
	return fileClose(view->document());
}

bool Manager::fileClose(KTextEditor::Document *doc /* = 0L*/, bool closingproject /*= false*/)
{
	KILE_DEBUG_MAIN << "==Kile::fileClose==========================";

	if(!doc) {
		doc = m_ki->activeTextDocument();
	}

	if(!doc) {
		return true;
	}

	//FIXME: remove from docinfo map, remove from dirwatch
	KILE_DEBUG_MAIN << "doc->url().toLocalFile()=" << doc->url().toLocalFile();

	const QUrl url = doc->url();

	TextInfo *docinfo= textInfoFor(doc);
	if(!docinfo) {
		qWarning() << "no DOCINFO for " << url.url();
		return true;
	}
	bool inProject = false;
	QList<KileProjectItem*> items = itemsFor(docinfo);
	for(QList<KileProjectItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		KileProjectItem *item = *it;

		//FIXME: refactor here
		if(item && doc) {
			storeProjectItem(item, doc);
			inProject = true;
		}
	}

	if(!inProject) {
	KILE_DEBUG_MAIN << "not in project";
		saveDocumentAndViewSettings(docinfo);
	}

	if(doc->closeUrl()) {
		// docinfo may have been recreated from 'Untitled' doc to a named doc
		if(url.isEmpty()) {
			docinfo= textInfoFor(doc);
		}

		if(KileConfig::cleanUpAfterClose()) {
			cleanUpTempFiles(url, true); // yes we pass here url and not docinfo->url()
		}

		//FIXME: use signal/slot
		if( doc->views().count() > 0){
			m_ki->viewManager()->removeView(doc->views().first());
		}
		//remove the decorations

		trashDoc(docinfo, doc);
		m_ki->structureWidget()->clean(docinfo);
		removeTextDocumentInfo(docinfo, closingproject);

		emit removeFromProjectView(url);
		emit updateModeStatus();
	}
	else {
		return false;
	}

	return true;
}

void Manager::buildProjectTree(const QUrl &url)
{
	KileProject * project = projectFor(url);

	if (project) {
		buildProjectTree(project);
	}
}

void Manager::buildProjectTree(KileProject *project)
{
	if(!project) {
		project = activeProject();
	}

	if(!project) {
		project = selectProject(i18n("Refresh Project Tree"));
	}

	if (project) {
		//TODO: update structure for all docs
		project->buildProjectTree();
	}
	else if (m_projects.count() == 0) {
		KMessageBox::error(m_ki->mainWindow(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to build the tree for, then choose Refresh Project Tree again."),i18n( "Could Not Refresh Project Tree"));
	}
}

void Manager::projectNew()
{
	KileNewProjectDialog *dlg = new KileNewProjectDialog(m_ki->templateManager(), m_ki->extensions(), m_ki->mainWindow());

	if (dlg->exec())
	{
		TextInfo *newTextInfo = Q_NULLPTR;

		KileProject *project = dlg->project();

		//add the project file to the project
		KileProjectItem *item = new KileProjectItem(project, project->url());
		createTextInfoForProjectItem(item);
		item->setOpenState(false);
		projectOpenItem(item);

		if(dlg->createNewFile()) {
			m_currentlyOpeningFile = true; // don't let live preview interfere

			QString filename = dlg->file();

			//create the new document and fill it with the template
			KTextEditor::View *view = loadTemplate(dlg->getSelection());

			if(view) {
				//derive the URL from the base url of the project
				QUrl url = project->baseURL();
				url = url.adjusted(QUrl::StripTrailingSlash);
				url.setPath(url.path() + '/' + filename);

				newTextInfo = textInfoFor(view->document());

				//save the new file
				//FIXME: this needs proper error handling
				view->document()->saveAs(url);
				emit(documentModificationStatusChanged(view->document(),
				     false, KTextEditor::ModificationInterface::OnDiskUnmodified));

				//add this file to the project
				item = new KileProjectItem(project, url);
				item->setInfo(newTextInfo);

				//docinfo->updateStruct(m_kwStructure->level());
				emit(updateStructure(false, newTextInfo));
			}

			m_currentlyOpeningFile = false;
		}

		project->buildProjectTree();
		project->save();
		addProject(project);

		emit(updateModeStatus());
		emit(addToRecentProjects(project->url()));

		if(newTextInfo) {
			emit documentOpened(newTextInfo);
		}
	}
}

void Manager::addProject(KileProject *project)
{
	KILE_DEBUG_MAIN << "==void Manager::addProject(const KileProject *project)==========";
	m_projects.append(project);
	KILE_DEBUG_MAIN << "\tnow " << m_projects.count() << " projects";
	emit addToProjectView(project);
	connect(project, SIGNAL(projectTreeChanged(const KileProject *)), this, SIGNAL(projectTreeChanged(const KileProject *)));
}

KileProject* Manager::selectProject(const QString& caption)
{
	QStringList list;
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		list.append((*it)->name());
	}

	KileProject *project = Q_NULLPTR;
	QString name;
	if (list.count() > 1) {
		KileListSelector *dlg  = new KileListSelector(list, caption, i18n("Select Project"), true, m_ki->mainWindow());
		if (dlg->exec()) {
			if(!dlg->hasSelection()) {
				return Q_NULLPTR;
			}
			name = dlg->selectedItems().first();
		}
		delete dlg;
	}
	else if (list.count() == 0) {
		return Q_NULLPTR;
	}
	else {
		name = m_projects.first()->name();
	}

	project = projectFor(name);

	return project;
}

void Manager::addToProject(const QUrl &url)
{
	KILE_DEBUG_MAIN << "===Kile::addToProject(const QUrl &url =" << url.url() << ")";

	KileProject *project = selectProject(i18n("Add to Project"));

	if (project) {
		addToProject(project, url);
	}
}

void Manager::addToProject(KileProject* project, const QUrl &url)
{
	const QUrl realurl = symlinkFreeURL(url);
	QFileInfo fi(realurl.toLocalFile());

	if (project->contains(realurl)) {
		m_ki->errorHandler()->printMessage(KileTool::Info,
		                                   i18n("The file %1 is already member of the project %2", realurl.fileName(), project->name()),
		                                   i18n("Add to Project"));
		return;
	}
	else if(!fi.exists() || !fi.isReadable())
	{
		m_ki->errorHandler()->printMessage(KileTool::Info,
		                                   i18n("The file %1 can not be added because it does not exist or is not readable", realurl.fileName()),
		                                   i18n("Add to Project"));
		return;
	}

	KileProjectItem *item = new KileProjectItem(project, realurl);
	createTextInfoForProjectItem(item);
	item->setOpenState(m_ki->isOpen(realurl));
	projectOpenItem(item);
	emit addToProjectView(item);
	buildProjectTree(project);
}

void Manager::removeFromProject(KileProjectItem *item)
{
	if (item && item->project()) {
		KILE_DEBUG_MAIN << "\tprojecturl = " << item->project()->url().toLocalFile() << ", url = " << item->url().toLocalFile();

		if (item->project()->url() == item->url()) {
			KMessageBox::error(m_ki->mainWindow(), i18n("This file is the project file, which holds all the information about your project.  As such, it cannot be removed from the project."), i18n("Cannot Remove File From Project"));
			return;
		}

		emit removeItemFromProjectView(item, m_ki->isOpen(item->url()));

		KileProject *project = item->project();
		project->remove(item);

		// update undefined references in all project files
		updateProjectReferences(project);
		project->buildProjectTree();
	}
}

// WARNING: 'item' must have been set up with a TextInfo* object already
void Manager::projectOpenItem(KileProjectItem *item, bool openProjectItemViews)
{
	KILE_DEBUG_MAIN << "==Kile::projectOpenItem==========================";
	KILE_DEBUG_MAIN << "\titem:" << item->url().toLocalFile();

	if (m_ki->isOpen(item->url())) { //remove item from projectview (this file was opened before as a normal file)
		emit removeFromProjectView(item->url());
	}

	KileDocument::TextInfo* itemInfo = item->getInfo();
	Q_ASSERT(itemInfo);

	if(item->isOpen()) {
		KTextEditor::View *view = loadItem(m_ki->extensions()->determineDocumentType(item->url()), item, QString(), openProjectItemViews);
		if (view) {
			item->loadDocumentAndViewSettings();
		}
		// make sure that the item has been parsed, even if it isn't shown;
		// this is necessary to identify the correct LaTeX root document (bug 233667);
		m_ki->structureWidget()->update(itemInfo, true);
	}
	else { // 'item' is not shown, i.e. its contents won't be loaded into a KTextEditor::Document;
		// so, we have to do it: we are loading the contents of the project item into the docinfo
		// for a moment
		itemInfo->setDocumentContents(loadTextURLContents(item->url(), item->encoding()));
		// in order to pass the contents to the parser
		m_ki->structureWidget()->update(itemInfo, true);
		// now we don't need the contents anymore
		itemInfo->setDocumentContents(QStringList());
	}
}

void Manager::createTextInfoForProjectItem(KileProjectItem *item)
{
	item->setInfo(createTextDocumentInfo(m_ki->extensions()->determineDocumentType(item->url()),
	                                     item->url(), item->project()->baseURL()));
}

void Manager::projectOpen(const QUrl &url, int step, int max, bool openProjectItemViews)
{
	KILE_DEBUG_MAIN << "==Kile::projectOpen==========================";
	KILE_DEBUG_MAIN << "\tfilename: " << url.fileName();

	const QUrl realurl = symlinkFreeURL(url);

	if(m_ki->projectIsOpen(realurl)) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		KMessageBox::information(m_ki->mainWindow(), i18n("<p>The project \"%1\" is already open.</p>"
		                                                  "<p>If you wanted to reload the project, close the project before you re-open it.</p>", url.fileName()),
			                                     i18n("Project Already Open"));
		return;
	}

	QFileInfo fi(realurl.toLocalFile());
	if(!fi.isReadable()) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		if (KMessageBox::warningYesNo(m_ki->mainWindow(), i18n("<p>The project file for the project \"%1\" does not exist or it is not readable.</p>"
		                                                       "<p>Do you want to remove this project from the recent projects list?</p>",
		                                                       url.fileName()),
		                                                  i18n("Could Not Open Project"))  == KMessageBox::Yes) {
			emit(removeFromRecentProjects(realurl));
		}
		return;
	}

	if(!m_progressDialog) {
		createProgressDialog();
	}

	KileProject *kp = new KileProject(realurl, m_ki->extensions());

	if(!kp->appearsToBeValidProjectFile()) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		KMessageBox::sorry(m_ki->mainWindow(), i18n("<p>The file \"%1\" cannot be opened as it does not appear to be a project file.</p>",
		                                            url.fileName()),
		                   i18n("Impossible to Open Project File"));
		delete kp;
		return;
	}

	if(kp->getProjectFileVersion() > KILE_PROJECTFILE_VERSION) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		KMessageBox::sorry(m_ki->mainWindow(), i18n("<p>The project \"%1\" cannot be opened as it was created <br/>by a newer version of Kile.</p>",
		                                            url.fileName()),
		                   i18n("Impossible to Open Project"));
		delete kp;
		return;
	}

	if(!kp->isOfCurrentVersion()) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		if(KMessageBox::questionYesNo(m_ki->mainWindow(), i18n("<p>The project file \"%1\" was created by a previous version of Kile.<br/>"
		                                                       "It needs to be updated before it can be opened.</p>"
		                                                       "<p>Do you want to update it?</p>", url.fileName()),
		                                                  i18n("Project File Needs to be Updated"))  == KMessageBox::No) {
			delete kp;
			return;
		}

		if(!kp->migrateProjectFileToCurrentVersion()) {
			if (KMessageBox::warningYesNo(m_ki->mainWindow(), i18n("<p>The project file \"%1\" could be not updated.</p>"
			                                                       "<p>Do you want to remove this project from the recent projects list?</p>", url.fileName()),
			                                                  i18n("Could Not Update Project File"))  == KMessageBox::Yes) {
				emit(removeFromRecentProjects(realurl));
			}
			delete kp;
			return;
		}
	}

	m_progressDialog->show();

	kp->load();

	if(kp->isInvalid()) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}
		delete kp;
		return;
	}

	emit(addToRecentProjects(realurl));

	QList<KileProjectItem*> list = kp->items();
	int project_steps = list.count();
	m_progressDialog->setMaximum(project_steps * max);
	project_steps *= step;
	m_progressDialog->setValue(project_steps);

	// open the project files in the correct order
	QVector<KileProjectItem*> givenPositionVector(list.count(), Q_NULLPTR);
	QList<KileProjectItem*> notCorrectlyOrderedList;
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		KileProjectItem *item = *it;
		int order = item->order();

		if(order >= 0 && order >= list.count()) {
			order = -1;
		}
		if(!item->isOpen() || order < 0 || givenPositionVector[order] != Q_NULLPTR) {
			notCorrectlyOrderedList.push_back(item);
		}
		else {
			givenPositionVector[order] = item;
		}
	}

	QList<KileProjectItem*> orderedList;
	for(int i = 0; i < givenPositionVector.size(); ++i) {
		KileProjectItem *item = givenPositionVector[i];
		if(item) {
			orderedList.push_back(item);
		}
	}
	for(QList<KileProjectItem*>::iterator i = notCorrectlyOrderedList.begin(); i != notCorrectlyOrderedList.end(); ++i) {
		orderedList.push_back(*i);
	}

	addProject(kp);

	// for the parsing to work correctly, all ProjectItems need to have TextInfo* objects, but
	// the URL of 'item' might already be associated with a TextInfo* object; for example, through
	// a stand-alone document currently being open already, or through a project item that belongs to
	// a different project
	// => 'createTextDocumentInfo' will take care of that situation as well
	for (QList<KileProjectItem*>::iterator i = orderedList.begin(); i != orderedList.end(); ++i) {
		createTextInfoForProjectItem(*i);
	}

	unsigned int counter = 1;
	for (QList<KileProjectItem*>::iterator i = orderedList.begin(); i != orderedList.end(); ++i) {
		projectOpenItem(*i, openProjectItemViews);
		m_progressDialog->setValue(counter + project_steps);
		qApp->processEvents();
		++counter;
	}

	kp->buildProjectTree();

	emit(updateStructure(false, Q_NULLPTR));
	emit(updateModeStatus());

	// update undefined references in all project files
	updateProjectReferences(kp);

	m_ki->viewManager()->switchToTextView(kp->lastDocument());

	emit(projectOpened(kp));
}

// as all labels are gathered in the project, we can check for unsolved references
void Manager::updateProjectReferences(KileProject *project)
{
	QList<KileProjectItem*> list = project->items();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		emit(updateReferences((*it)->getInfo()));
	}
}

void Manager::projectOpen()
{
	KILE_DEBUG_MAIN << "==Kile::projectOpen==========================";
	QUrl url = QFileDialog::getOpenFileUrl(m_ki->mainWindow(), i18n("Open Project"),
	                                       QUrl::fromLocalFile(KileConfig::defaultProjectLocation()),
	                                       m_ki->extensions()->fileFilterQtStyle(false, {KileDocument::Extensions::KILE_PROJECT}));

	if(!url.isEmpty()) {
		projectOpen(url);
	}
}


void Manager::projectSave(KileProject *project /* = 0 */)
{
	KILE_DEBUG_MAIN << "==Kile::projectSave==========================";
	if (!project) {
		//find the project that corresponds to the active doc
		project= activeProject();
	}

	if(!project) {
		project = selectProject(i18n("Save Project"));
	}

	if(project) {
		QList<KileProjectItem*> list = project->items();
		KTextEditor::Document *doc = Q_NULLPTR;
		KileProjectItem *item = Q_NULLPTR;
		TextInfo *docinfo = Q_NULLPTR;

		// determine the order in which the project items are opened
		QVector<KileProjectItem*> viewPositionVector(m_ki->viewManager()->getTabCount(), Q_NULLPTR);
		for(QList<KileProjectItem*>::iterator i = list.begin(); i != list.end(); ++i) {
			docinfo = (*i)->getInfo();
			if(docinfo) {
				KTextEditor::View *view = m_ki->viewManager()->textView(docinfo);
				if(view) {
					int position = m_ki->viewManager()->tabIndexOf(view);
					if(position >= 0 && position < viewPositionVector.size()) {
						viewPositionVector[position] = *i;
					}
				}
			}
		}
		int position = 0;
		for(int i = 0; i < viewPositionVector.size(); ++i) {
			if(viewPositionVector[i] != Q_NULLPTR) {
				viewPositionVector[i]->setOrder(position);
				++position;
			}
		}

		//update the open-state of the items
		for (QList<KileProjectItem*>::iterator i = list.begin(); i != list.end(); ++i) {
			item = *i;
			KILE_DEBUG_MAIN << "\tsetOpenState(" << (*i)->url().toLocalFile() << ") to " << m_ki->isOpen(item->url());
			item->setOpenState(m_ki->isOpen(item->url()));
			docinfo = item->getInfo();

			if(docinfo) {
				doc = docinfo->getDoc();
			}
			if(doc) {
				storeProjectItem(item, doc);
			}

			doc = Q_NULLPTR;
			docinfo = Q_NULLPTR;
		}

		project->save();
	}
	else {
		KMessageBox::error(m_ki->mainWindow(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to save, then choose Save Project again."),i18n( "Could Determine Active Project"));
	}
}

void Manager::projectAddFiles(const QUrl &url)
{
	KileProject *project = projectFor(url);

	if (project) {
		projectAddFiles(project,url);
	}
}

void Manager::projectAddFiles(KileProject *project,const QUrl &fileUrl)
{
	KILE_DEBUG_MAIN << "==Kile::projectAddFiles()==========================";
 	if(project == 0) {
		project = activeProject();
	}

	if(project == 0) {
		project = selectProject(i18n("Add Files to Project"));
	}

	if (project) {
		QString currentDir;
		if(fileUrl.isEmpty()) {
			currentDir = QFileInfo(project->url().path()).dir().dirName();
		}
		else {
			currentDir = fileUrl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path();
		}

		KILE_DEBUG_MAIN << "currentDir is " << currentDir;
		QFileDialog *dlg = new QFileDialog(m_ki->mainWindow(), i18n("Add Files"), currentDir, m_ki->extensions()->fileFilterQtStyle(true, {}));
		dlg->setModal(true);
		dlg->setFileMode(QFileDialog::ExistingFiles);
		dlg->setLabelText(QFileDialog::Accept, i18n("Add"));

		if(dlg->exec()) {
			QList<QUrl> urls = dlg->selectedUrls();
			for(int i=0; i < urls.count(); ++i) {
				addToProject(project, urls[i]);
			}
			// update undefined references in all project files
			updateProjectReferences(project);
		}
		delete dlg;

		//open them
	}
	else if (m_projects.count() == 0) {
		KMessageBox::error(m_ki->mainWindow(), i18n("There are no projects opened. Please open the project you want to add files to, then choose Add Files again."),i18n( "Could Not Determine Active Project"));
	}
}

void Manager::toggleArchive(KileProjectItem *item)
{
	item->setArchive(!item->archive());
}

void Manager::projectOptions(const QUrl &url)
{
	KileProject *project = projectFor(url);

	if (project) {
		projectOptions(project);
	}
}

void Manager::projectOptions(KileProject *project /* = 0*/)
{
	KILE_DEBUG_MAIN << "==Kile::projectOptions==========================";
	if(!project) {
		project = activeProject();
	}

	if(!project) {
		project = selectProject(i18n("Project Options For"));
	}

	if (project) {
		KILE_DEBUG_MAIN << "\t" << project->name();
		KileProjectOptionsDialog *dlg = new KileProjectOptionsDialog(project, m_ki->extensions(), m_ki->mainWindow());
		dlg->exec();
	}
	else if (m_projects.count() == 0) {
		KMessageBox::error(m_ki->mainWindow(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to modify, then choose Project Options again."),i18n( "Could Not Determine Active Project"));
	}
}

bool Manager::projectCloseAll()
{
	KILE_DEBUG_MAIN << "==Kile::projectCloseAll==========================";

	while(m_projects.size() > 0) {
		if(!projectClose(m_projects.first()->url())) {
			return false;
		}
	}

	return true;
}

bool Manager::projectClose(const QUrl &url)
{
	KILE_DEBUG_MAIN << "==Kile::projectClose==========================";
	KileProject *project = 0;

	if (url.isEmpty()) {
		 project = activeProject();

		if (!project) {
			project = selectProject(i18n("Close Project"));
		}
	}
	else {
		project = projectFor(url);
	}

	if(project) {
		KILE_DEBUG_MAIN << "\tclosing:" << project->name();
		project->setLastDocument(QUrl::fromLocalFile(m_ki->getName()));

		projectSave(project);

		QList<KileProjectItem*> list = project->items();

		bool close = true;
		KTextEditor::Document *doc = Q_NULLPTR;
		TextInfo *docinfo = Q_NULLPTR;
		for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
			KileProjectItem *item = *it;

			doc = Q_NULLPTR;
			docinfo = item->getInfo();
			if (docinfo) {
				doc = docinfo->getDoc();
			}
			else {
				continue;
			}
			if (doc) {
				KILE_DEBUG_MAIN << "\t\tclosing item " << doc->url().toLocalFile();
				bool r = fileClose(doc, true);
				close = close && r;
				if (!close) {
					break;
				}
			}
			else {
				// we still need to delete the TextInfo object
				removeTextDocumentInfo(docinfo, true);
			}
		}

		if (close) {
			m_projects.removeAll(project);
			emit removeFromProjectView(project);
			delete project;
			emit(updateModeStatus());
			return true;
		}
		else
			return false;
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->mainWindow(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to close, then choose Close Project again."),i18n( "Could Not Close Project"));

	return true;
}

void Manager::storeProjectItem(KileProjectItem *item, KTextEditor::Document *doc)
{
	KILE_DEBUG_MAIN << "===Kile::storeProjectItem==============";
	KILE_DEBUG_MAIN << "\titem = " << item << ", doc = " << doc;
	item->setEncoding(doc->encoding());
	item->setMode(doc->mode());
	item->setHighlight(doc->highlightingMode());
	item->saveDocumentAndViewSettings();
}

void Manager::cleanUpTempFiles(const QUrl &url, bool silent)
{
	KILE_DEBUG_MAIN << "===void Manager::cleanUpTempFiles(const QUrl " << url.toLocalFile() << ", bool " << silent << ")===";

	if( url.isEmpty() )
		return;

	QStringList extlist;
	QFileInfo fi(url.toLocalFile());
	const QStringList templist = KileConfig::cleanUpFileExtensions().split(' ');
	const QString fileName = fi.fileName();
	const QString dirPath = fi.absolutePath();
	const QString baseName = fi.completeBaseName();

	for (int i = 0; i < templist.count(); ++i) {
		fi.setFile( dirPath + '/' + baseName + templist[i] );
		if(fi.exists()) {
			extlist.append(templist[i]);
		}
	}

	if(!silent && fileName.isEmpty()) {
		return;
	}

	if (!silent && extlist.count() > 0) {
		KILE_DEBUG_MAIN << "not silent";
		KileDialog::Clean *dialog = new KileDialog::Clean(m_ki->mainWindow(), fileName, extlist);
		if (dialog->exec() == QDialog::Accepted) {
			extlist = dialog->cleanList();
		}
		else {
			delete dialog;
			return;
		}

		delete dialog;
	}

	if(extlist.count() == 0) {
		m_ki->errorHandler()->printMessage(KileTool::Warning, i18n("Nothing to clean for %1", fileName),
		                                                           i18n("Clean"));
	}
	else {
		for(int i = 0; i < extlist.count(); ++i) {
			QFile file(dirPath + '/' + baseName + extlist[i]);
			KILE_DEBUG_MAIN << "About to remove file = " << file.fileName();
			file.remove();
		}
		m_ki->errorHandler()->printMessage(KileTool::Info,
		                                   i18n("Cleaning %1: %2", fileName, extlist.join(" ")),
		                                   i18n("Clean"));
	}
}

void Manager::openDroppedURLs(QDropEvent *e) {
	QList<QUrl> urls = e->mimeData()->urls();
	Extensions *extensions = m_ki->extensions();

	for(QList<QUrl>::iterator i = urls.begin(); i != urls.end(); ++i) {
		QUrl url = *i;
		if(extensions->isProjectFile(url)) {
			projectOpen(url);
		}
		else {
			fileOpen(url);
		}
	}
}

void Manager::reloadXMLOnAllDocumentsAndViews()
{
	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		KTextEditor::Document *doc = (*it)->getDoc();
		// FIXME: 'doc' can be null, for example if it belongs to a project item
		//        which has been closed, but this should be improved in the sense
		//        that 'm_textInfoList' should only contain 'TextInfo' objects which
		//        contain valid pointers to 'KTextEditor::Document' objects.
		if(!doc) {
			continue;
		}
		doc->reloadXML();
		QList<KTextEditor::View*> views = doc->views();
		for(QList<KTextEditor::View*>::iterator viewIt = views.begin(); viewIt != views.end(); ++viewIt) {
			(*viewIt)->reloadXML();
		}
	}
}


void Manager::handleParsingComplete(const QUrl &url, KileParser::ParserOutput* output)
{
	KILE_DEBUG_MAIN << url << output;
	if(!output) {
		KILE_DEBUG_MAIN << "NULL output given";
		return;
	}
	KileDocument::TextInfo *textInfo = textInfoFor(url);
	if(!textInfo) {
		KileProjectItem* item = itemFor(url);
		if(item) {
			textInfo = item->getInfo();
		}
		if(!textInfo) {
			// this can happen for instance when the document is closed
			// while the parser is still running
			KILE_DEBUG_MAIN << "no TextInfo object found for" << url << "found";
			return;
		}
	}
	textInfo->installParserOutput(output);
	m_ki->structureWidget()->updateAfterParsing(textInfo, output->structureViewItems);
	delete(output);
}

// Show all opened projects and switch to another one, if you want

void Manager::projectShow()
{
	if(m_projects.count() <= 1) {
		return;
	}

	// select the new project
	KileProject *project = selectProject(i18n("Switch Project"));
	if(!project || project==activeProject()) {
		return;
	}

	// get last opened document
	const QUrl lastdoc = project->lastDocument();
	KileProjectItem *docitem = (!lastdoc.isEmpty()) ? itemFor(lastdoc, project) : Q_NULLPTR;

	// if not, we search for the first opened tex file of this project
	// if no file is opened, we take the first tex file mentioned in the list
	KileProjectItem *first_texitem = Q_NULLPTR;
	if(!docitem) {
		QList<KileProjectItem*> list = project->items();
		for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
			KileProjectItem *item = *it;

			QString itempath = item->path();

			// called from QAction 'Show projects...': find the first opened
			// LaTeX document or, if that fails, any other opened file
			QStringList extlist = (m_ki->extensions()->latexDocuments() + ' ' + m_ki->extensions()->latexPackages()).split(' ');
			for(QStringList::Iterator it=extlist.begin(); it!=extlist.end(); ++it) {
				if(itempath.indexOf( (*it), -(*it).length() ) >= 0)  {
					if (m_ki->isOpen(item->url()))  {
						docitem = item;
						break;
					}
					else if(!first_texitem) {
						first_texitem = item;
					}
				}
			}
			if(docitem) {
				break;
			}
		}
	}

	// did we find one opened file or must we take the first entry
	if(!docitem) {
		if(!first_texitem) {
			return;
		}
		docitem = first_texitem;
	}

	// ok, we can switch to another project now
	if (m_ki->isOpen(docitem->url())) {
		m_ki->viewManager()->switchToTextView(docitem->url());
	}
	else {
		fileOpen(docitem->url(), docitem->encoding());
	}
}

void Manager::projectRemoveFiles()
{
	QList<KileProjectItem*> itemsList = selectProjectFileItems(i18n("Select Files to Remove"));
	if(itemsList.count() > 0) {
		for(QList<KileProjectItem*>::iterator it = itemsList.begin(); it != itemsList.end(); ++it) {
			removeFromProject(*it);
		}
	}
}

void Manager::projectShowFiles()
{
	KileProjectItem *item = selectProjectFileItem( i18n("Select File") );
	if(item) {
		if (item->type() == KileProjectItem::ProjectFile) {
			dontOpenWarning(item, i18n("Show Project Files"), i18n("project configuration file"));
		}
		else if(item->type() == KileProjectItem::Image) {
			dontOpenWarning(item, i18n("Show Project Files"), i18n("graphics file"));
		}
		else { // ok, we can switch to another file
			if  (m_ki->isOpen(item->url())) {
				m_ki->viewManager()->switchToTextView(item->url());
			}
			else {
				fileOpen(item->url(), item->encoding() );
			}
		}
	}
}

void Manager::projectOpenAllFiles()
{
	KileProject *project = selectProject(i18n("Select Project"));
	if(project) {
		projectOpenAllFiles(project->url());
	}
}

void Manager::projectOpenAllFiles(const QUrl &url)
{
	KileProject* project;
	KTextEditor::Document* doc = Q_NULLPTR;

	if(!url.isValid()) {
		return;
	}
	project = projectFor(url);

	if(!project)
		return;


	if(m_ki->viewManager()->currentTextView()) {
		doc = m_ki->viewManager()->currentTextView()->document();
	}
	// we remember the actual view, so the user gets the same view back after opening

	QList<KileProjectItem*> list = project->items();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		KileProjectItem *item = *it;

		if (item->type()==KileProjectItem::ProjectFile) {
			dontOpenWarning( item, i18n("Open All Project Files"), i18n("project configuration file") );
		}
		else if(item->type()==KileProjectItem::Image) {
			dontOpenWarning( item, i18n("Open All Project Files"), i18n("graphics file") );
		}
		else if(!m_ki->isOpen(item->url())) {
			fileOpen(item->url(), item->encoding());
		}
	}

	if(doc) { // we have a doc so switch back to original view
		m_ki->viewManager()->switchToTextView(doc->url());
	}
}

QStringList Manager::getProjectFiles()
{
	QStringList filelist;

	KileProject *project = activeProject();
	if ( project )
	{
		QList<KileProjectItem*> list = project->items();
		for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
			KileProjectItem *item = *it;

			if(item->type() != KileProjectItem::ProjectFile && item->type() != KileProjectItem::Image) {
				filelist << item->url().toLocalFile();
			}
		}
	}
	return filelist;
}

void Manager::dontOpenWarning(KileProjectItem *item, const QString &action, const QString &filetype)
{
	m_ki->errorHandler()->printMessage(KileTool::Info,
	                                   i18n("not opened: %1 (%2)", item->url().toLocalFile(), filetype),
	                                   action);
}

KileProjectItem* Manager::selectProjectFileItem(const QString &label)
{
	// select a project
	KileProject *project = selectProject(i18n("Select Project"));
	if(!project) {
		return Q_NULLPTR;
	}

	// get a list of files
	QStringList filelist;
	QMap<QString, KileProjectItem*> map;
	QList<KileProjectItem*> list = project->items();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		KileProjectItem *item = *it;

		filelist << item->path();
		map[item->path()] = item;
	}

	// select one of these files
	KileProjectItem *item = Q_NULLPTR;
	KileListSelector *dlg  = new KileListSelector(filelist, i18n("Project Files"), label, true, m_ki->mainWindow());
	if(dlg->exec()) {
		if(dlg->hasSelection()) {
			QString name = dlg->selectedItems().first();
			if(map.contains(name)) {
				item = map[name];
			}
			else {
				KMessageBox::error(m_ki->mainWindow(), i18n("Could not determine the selected file."),i18n( "Project Error"));
			}
		}
	}
	delete dlg;

	return item;
}

QList<KileProjectItem*> Manager::selectProjectFileItems(const QString &label)
{
	KileProject *project = selectProject(i18n("Select Project"));
	if(!project) {
		return QList<KileProjectItem*>();
	}

	QStringList filelist;
	QMap<QString,KileProjectItem *> map;

	QList<KileProjectItem*> list = project->items();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		KileProjectItem *item = *it;

		filelist << item->path();
		map[item->path()] = item;
	}

	QList<KileProjectItem*> itemsList;

	KileListSelector *dlg  = new KileListSelector(filelist, i18n("Project Files"), label, true, m_ki->mainWindow());
	dlg->setSelectionMode(QAbstractItemView::ExtendedSelection);
	if(dlg->exec()) {
		if(dlg->hasSelection()) {
			QStringList selectedfiles = dlg->selectedItems();
			for(QStringList::Iterator it = selectedfiles.begin(); it != selectedfiles.end(); ++it ){
				if(map.contains(*it)) {
					itemsList.append(map[(*it)]);
				}
				else {
					KMessageBox::error(m_ki->mainWindow(), i18n("Could not determine the selected file."), i18n( "Project Error"));
				}
			}
		}
	}
	delete dlg;

	return itemsList;
}

// add a new file to the project
//  - only when there is an active project
//  - if the file doesn't already belong to it (checked by addToProject)

void Manager::projectAddFile(QString filename, bool graphics)
{
	KILE_DEBUG_MAIN << "===Kile::projectAddFile==============";
	KileProject *project = activeProject();
	if(!project) {
		return;
	}

	QFileInfo fi(filename);
	if(!fi.exists()) {
		if(graphics) {
			return;
		}

		// called from InputDialog after a \input- or \include command:
		//  - if the chosen file has an extension: accept
		//  - if not we add the default TeX extension: accept if it exists else reject
		QString ext = fi.completeSuffix();
		if ( ! ext.isEmpty() ) {
			return;
                }

		filename += m_ki->extensions()->latexDocumentDefault();
		if ( QFileInfo(filename).exists() ) {
			return;
                }
	}

	//ok, we have a project and an existing file
	KILE_DEBUG_MAIN << "\tadd file: " << filename;
	m_ki->viewManager()->updateStructure(false);

	QUrl url;
	url.setPath(filename);
	addToProject(project, url);
}

const QUrl Manager::symlinkFreeURL(const QUrl &url)
{
#ifdef Q_OS_WIN
	//TODO: maybe actually do something here?  Seems unncecessary given Windows' lack of symlinks though...
	//Also: the else'd code below fails badly on Windows
	return url;
#else
	KILE_DEBUG_MAIN << "===symlinkFreeURL==";

	if(!url.isLocalFile()) {
		return url;
	}

	QDir dir(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path());
	QString filename = url.toLocalFile(); // if the directory does not exist we return the old url (just to be sure)

	if(dir.exists()) {
		filename= dir.canonicalPath() + '/' + url.fileName();
	}
	else {
		KILE_DEBUG_MAIN << "directory " << url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path() << "does not exist";
	}

	QFileInfo fi(filename);
	if (fi.isSymLink()) {
		filename = fi.symLinkTarget();
	}

	return QUrl::fromLocalFile(filename);
#endif //def Q_OS_WIN
}

void Manager::cleanupDocumentInfoForProjectItems(KileDocument::Info *info)
{
	QList<KileProjectItem*> itemsList = itemsFor(info);
	for(QList<KileProjectItem*>::iterator it = itemsList.begin(); it != itemsList.end(); ++it) {
		(*it)->setInfo(Q_NULLPTR);
	}
}

void Manager::createProgressDialog()
{
	//TODO this is a dangerous dialog and should be removed in the long-term:
	// the dialog disables all close events unless all files are loaded,
	// thus if there is a loading error, the only way to abort loading gracefully is to
	// terminate the application
	m_progressDialog = new KileWidget::ProgressDialog(m_ki->mainWindow());
	QLabel *label = new QLabel(m_progressDialog);
	label->setText(i18n("Opening Project..."));
	m_progressDialog->setLabel(label);
	m_progressDialog->setModal(true);
	m_progressDialog->setLabelText(i18n("Scanning project files..."));
	m_progressDialog->setAutoClose(true);
	m_progressDialog->setMinimumDuration(2000);
	m_progressDialog->hide();
}

void Manager::loadDocumentAndViewSettings(KileDocument::TextInfo *textInfo)
{
	KTextEditor::Document *document = textInfo->getDoc();
	if(!document) {
		return;
	}

	KConfigGroup configGroup = configGroupForDocumentSettings(document);
	if(!configGroup.exists()) {
		return;
	}

	document->readSessionConfig(configGroup, QSet<QString>() << "SkipEncoding" << "SkipUrl");
	{
		LaTeXInfo *latexInfo = dynamic_cast<LaTeXInfo*>(textInfo);
		if(latexInfo) {
			KileTool::LivePreviewManager::readLivePreviewStatusSettings(configGroup, latexInfo);
		}
	}

	{
		LaTeXOutputHandler *h = dynamic_cast<LaTeXOutputHandler*>(textInfo);
		if(h) {
			h->readBibliographyBackendSettings(configGroup);
		}
	}

	QList<KTextEditor::View*> viewList = document->views();
	int i = 0;
	for(QList<KTextEditor::View*>::iterator it = viewList.begin(); it != viewList.end(); ++it) {
		KTextEditor::View *view = *it;
		configGroup = configGroupForViewSettings(document, i);
		view->readSessionConfig(configGroup);
		++i;
	}

}

void Manager::saveDocumentAndViewSettings(KileDocument::TextInfo *textInfo)
{
	KTextEditor::Document *document = textInfo->getDoc();
	if(!document) {
		return;
	}

	KConfigGroup configGroup = configGroupForDocumentSettings(document);

	QUrl url = document->url();
	url.setPassword(""); // we don't want the password to appear in the configuration file
	deleteDocumentAndViewSettingsGroups(url);

	document->writeSessionConfig(configGroup, QSet<QString>() << "SkipEncoding" << "SkipUrl");
	{
		LaTeXInfo *latexInfo = dynamic_cast<LaTeXInfo*>(textInfo);
		if(latexInfo) {
			KileTool::LivePreviewManager::writeLivePreviewStatusSettings(configGroup, latexInfo);
		}
	}

	{
		LaTeXOutputHandler *h = dynamic_cast<LaTeXOutputHandler*>(textInfo);
		if(h) {
			h->writeBibliographyBackendSettings(configGroup);
		}
	}

	QList<KTextEditor::View*> viewList = document->views();
	int i = 0;
	for(QList<KTextEditor::View*>::iterator it = viewList.begin(); it != viewList.end(); ++it) {
		configGroup = configGroupForViewSettings(document, i);
		(*it)->writeSessionConfig(configGroup);
		++i;
	}
	// finally remove the config groups for the oldest documents that exceed MAX_NUMBER_OF_STORED_SETTINGS
	configGroup = KSharedConfig::openConfig()->group("Session Settings");
	QList<QUrl> urlList = QUrl::fromStringList(configGroup.readEntry("Saved Documents", QStringList()));
	urlList.removeAll(url);
	urlList.push_front(url);
	// remove excess elements
	if(urlList.length() > MAX_NUMBER_OF_STORED_SETTINGS) {
		int excessNumber = urlList.length() - MAX_NUMBER_OF_STORED_SETTINGS;
		for(; excessNumber > 0; --excessNumber) {
			QUrl url = urlList.takeLast();
			deleteDocumentAndViewSettingsGroups(url);
		}
	}
	configGroup.writeEntry("Documents", url);
	configGroup.writeEntry("Saved Documents", QUrl::toStringList(urlList));
}

KConfigGroup Manager::configGroupForDocumentSettings(KTextEditor::Document *doc) const
{
	return KSharedConfig::openConfig()->group(configGroupNameForDocumentSettings(doc->url()));
}

QString Manager::configGroupNameForDocumentSettings(const QUrl &url) const
{
	QUrl url2 = url;
	url2.setPassword("");
	return "Document-Settings,URL=" + url2.url();
}

KConfigGroup Manager::configGroupForViewSettings(KTextEditor::Document *doc, int viewIndex) const
{
	return KSharedConfig::openConfig()->group(configGroupNameForViewSettings(doc->url(), viewIndex));
}

QString Manager::configGroupNameForViewSettings(const QUrl &url, int viewIndex) const
{
	QUrl url2 = url;
	url2.setPassword("");
	return "View-Settings,View=" + QString::number(viewIndex) + ",URL=" + url2.url();
}

void Manager::deleteDocumentAndViewSettingsGroups(const QUrl &url)
{
	QString urlString = url.url();
	QStringList groupList = KSharedConfig::openConfig()->groupList();
	for(QStringList::iterator i = groupList.begin(); i != groupList.end(); ++i) {
		QString groupName = *i;
		if(groupName.startsWith("Document-Settings")
		|| groupName.startsWith("View-Settings")) {
			int urlIndex = groupName.indexOf("URL=");
			if(urlIndex >= 0 && groupName.mid(urlIndex + 4) == urlString) {
				KSharedConfig::openConfig()->deleteGroup(groupName);
			}
		}
	}
}

QStringList Manager::loadTextURLContents(const QUrl &url, const QString& encoding)
{
	QTemporaryFile file;
	if(!file.open()) {
		KILE_DEBUG_MAIN << "Cannot create temporary file for" << url;
		return QStringList();
	}
	auto downloadJob = KIO::file_copy(url, QUrl::fromLocalFile(file.fileName()), 0600, KIO::Overwrite);
	KJobWidgets::setWindow(downloadJob, m_ki->mainWindow());
	// FIXME: 'exec' should not be used!
	if (!downloadJob->exec()) {
		KILE_DEBUG_MAIN << "Cannot download resource: " << url;
		KILE_DEBUG_MAIN << downloadJob->errorString();
		file.close();
		return QStringList();
	}

	QFile localFile(file.fileName());

	if (!localFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		KILE_DEBUG_MAIN << "Cannot open source file: " << file.fileName();
		file.close();
		return QStringList();
	}

	QStringList res;
	QTextStream stream(&localFile);
	if(!encoding.isEmpty()) {
		stream.setCodec(encoding.toLatin1());
	}
	while(!stream.atEnd()) {
		res.append(stream.readLine());
	}
	file.close();
	return res;
}

}
