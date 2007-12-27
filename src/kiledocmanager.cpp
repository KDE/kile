//
// C++ Implementation: kiledocmanager
//
// Description:
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//         Michel Ludwig <michel.ludwig@kdemail.net>, (C) 2006, 2007
//         Holger Danielsson <holger.danielsson@versanet.de>, (C) 2007

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

#include <qtextcodec.h>
#include <qfile.h>
#include <qdir.h>

#include <QList>
#include <QDropEvent>

#include <k3urldrag.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/view.h>
#include <kapplication.h>
#include "kiledebug.h"
#include <kencodingfiledialog.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <KProgressDialog>
#include <kfile.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kpushbutton.h>
#include <kurl.h>
#include <kfileitem.h>

#include "kileeventfilter.h"
#include "kileuntitled.h"
#include "templates.h"
#include "newfilewizard.h"
#include "managetemplatesdialog.h"
#include "kileinfo.h"
#include "kilejscript.h"
#include "kileproject.h"
#include "kiledocumentinfo.h"
#include "kileviewmanager.h"
#include "kilefileselect.h"
#include "kileprojectview.h"
#include "kilestructurewidget.h"
#include "dialogs/projectdialogs.h"
#include "kiletool.h"
#include "kiletool_enums.h"
#include "kilestdtools.h"
#include "kilelistselector.h"
#include "kiletoolmanager.h"
#include "widgets/konsolewidget.h"
#include "kileconfig.h"
#include "kilelogwidget.h"
#include "cleandialog.h"

/*
 * Newly created text documents have an empty URL and a non-empty document name.
 */

namespace KileDocument
{

Manager::Manager(KileInfo *info, QObject *parent, const char *name) :
	QObject(parent, name),
	m_ki(info),
	m_progressDialog(NULL)
{
//FIXME: check whether this is still needed
// 	KTextEditor::Document::setFileChangedDialogsActivated (true);

	if (KileConfig::defaultEncoding() == "invalid") {
		KileConfig::setDefaultEncoding(QString::fromLatin1(QTextCodec::codecForLocale()->name()));
	}
}

Manager::~Manager()
{
	KILE_DEBUG() << "==KileDocument::Manager::~Manager()=========" << endl;
}

void Manager::trashDoc(TextInfo *docinfo, KTextEditor::Document *doc /*= 0L*/ )
{
	KILE_DEBUG() << "==void Manager::trashDoc(" << docinfo->url().path() << ")=====" << endl;

	if ( m_ki->isOpen(docinfo->url()) ) return;

	if ( doc == 0L ) doc = docinfo->getDoc();
	//look for doc before we detach the docinfo
	//if we do it the other way around, docFor will always return nil
 	if ( doc == 0L ) doc = docFor(docinfo->url());

	KILE_DEBUG() << "DETACHING " << docinfo << endl;
	docinfo->detach();

	KILE_DEBUG() << "\tTRASHING " <<  doc  << endl;
	if ( doc == 0L ) return;

	KILE_DEBUG() << "just checking: docinfo->getDoc() =  " << docinfo->getDoc() << endl;
	KILE_DEBUG() << "just checking: docFor(docinfo->url()) = " << docFor(docinfo->url()) << endl;

	for (int i = 0; i < m_textInfoList.count(); ++i) {
		if((m_textInfoList.at(i) != docinfo) && (m_textInfoList.at(i)->getDoc() == doc)) {
			KMessageBox::information(0, i18n("The internal structure of Kile is corrupted (probably due to a bug in Kile). Please select Save All from the File menu and close Kile.\nThe Kile team apologizes for any inconvenience and would appreciate a bug report."));
			kWarning() << "docinfo " << m_textInfoList.at(i) << " url " << m_textInfoList.at(i)->url().fileName() << " has a wild pointer!!!"<< endl;
		}
	}

	KILE_DEBUG() << "DELETING doc" << endl;
	delete doc;
}

// update all Info's with changed user commands
void Manager::updateInfos()
{
	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		(*it)->updateStructLevelInfo();
	}
}

KTextEditor::Document* Manager::docFor(const KUrl & url)
{
	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		TextInfo *info = *it;

		if(m_ki->similarOrEqualURL(info->url(), url)) {
			return info->getDoc();
		}
	}

	return NULL;
}

Info* Manager::getInfo() const
{
	KTextEditor::Document *doc = m_ki->activeTextDocument();
	if ( doc != 0L )
		return textInfoFor(doc);
	else
		return 0L;
}

TextInfo* Manager::textInfoFor(const QString & path) const
{
	if(path.isEmpty()) {
		return NULL;
	}

	KILE_DEBUG() << "==KileInfo::textInfoFor(" << path << ")==========================";
	for(QList<TextInfo*>::const_iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		TextInfo *info = *it;

		if(info->url().path() == path) {
			return info;
		}
	}

	KILE_DEBUG() << "\tCOULD NOT find info for " << path;
	return NULL;
}

TextInfo* Manager::textInfoForURL(const KUrl& url)
{
	if(url.isEmpty()) {
		return NULL;
	}

	KILE_DEBUG() << "==KileInfo::textInfoFor(" << url << ")==========================";

	for(QList<TextInfo*>::iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		TextInfo *info = *it;	

		if (info->url() == url) {
			return info;
		}
	}

	KILE_DEBUG() << "\tCOULD NOT find info for " << url;
	return NULL;
}

TextInfo* Manager::textInfoFor(KTextEditor::Document* doc) const
{
	if(!doc) {
		return NULL;
	}

	for(QList<TextInfo*>::const_iterator it = m_textInfoList.begin(); it != m_textInfoList.end(); ++it) {
		if((*it)->getDoc() == doc) {
			return (*it);
		}
	}
	
	return NULL;
}

void Manager::mapItem(TextInfo *docinfo, KileProjectItem *item)
{
	item->setInfo(docinfo);
}

KileProject* Manager::projectFor(const KUrl &projecturl)
{
	//find project with url = projecturl
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;
		if(project->url() == projecturl) {
			return project;
		}
	}

	return NULL;
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

	return NULL;
}

KileProjectItem* Manager::itemFor(const KUrl &url, KileProject *project /*=0L*/) const
{
	if (!project) {
		for(QList<KileProject*>::const_iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
			KileProject *project = *it;

			KILE_DEBUG() << "looking in project " << project->name();
			if (project->contains(url)) {
				KILE_DEBUG() << "\t\tfound!";
				return project->item(url);
			}
		}
		KILE_DEBUG() << "\t nothing found";
	}
	else {
		if(project->contains(url)) {
			return project->item(url);
		}
	}

	return NULL;
}

KileProjectItem* Manager::itemFor(Info *docinfo, KileProject *project /*=0*/) const
{
	if(docinfo)
		return itemFor(docinfo->url(), project);
	else
		return NULL;
}

QList<KileProjectItem*> Manager::itemsFor(Info *docinfo) const
{
	if(!docinfo) {
		return QList<KileProjectItem*>();
	}

	KILE_DEBUG() << "==KileInfo::itemsFor(" << docinfo->url().fileName() << ")============";
	QList<KileProjectItem*> list;
	for(QList<KileProject*>::const_iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		KileProject *project = *it;

		KILE_DEBUG() << "\tproject: " << (*it)->name();
		if(project->contains(docinfo)) {
			KILE_DEBUG() << "\t\tcontains";
			list.append(project->item(docinfo));
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
		for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
			KileProject *project = *it;

			if(project->contains(doc->url())) {
				return project;
			}
		}
	}

	return NULL;
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

	return NULL;
}

TextInfo* Manager::createTextDocumentInfo(KileDocument::Type type, const KUrl & url, const KUrl& baseDirectory)
{
	TextInfo *docinfo = 0L;

	//see if this file belongs to an opened project
	KileProjectItem *item = itemFor(url);
	if ( item != 0L ) docinfo = item->getInfo();

	if ( docinfo == 0L )
	{
		switch(type)
		{
			case Undefined: // fall through
			case Text:
				KILE_DEBUG() << "CREATING TextInfo for " << url.url() << endl;
				docinfo = new TextInfo(0L,m_ki->extensions());
				break;
			case LaTeX:
				KILE_DEBUG() << "CREATING LaTeXInfo for " << url.url() << endl;
				docinfo = new LaTeXInfo(0L, m_ki->extensions(), m_ki->latexCommands(), m_ki->eventFilter());
				break;
			case BibTeX:
				KILE_DEBUG() << "CREATING BibInfo for " << url.url() << endl;
				docinfo = new BibInfo(0L, m_ki->extensions(), m_ki->latexCommands());
				break;
			case Script:
				KILE_DEBUG() << "CREATING ScriptInfo for " << url.url() << endl;
				docinfo = new ScriptInfo(0L, m_ki->extensions());
				break;
		}
		docinfo->setBaseDirectory(baseDirectory);
		emit(documentInfoCreated(docinfo));
		m_textInfoList.append(docinfo);
	}

	KILE_DEBUG() << "DOCINFO: returning " << docinfo << " " << docinfo->url().fileName() << endl;
	return docinfo;
}

void Manager::recreateTextDocumentInfo(TextInfo *oldinfo)
{
	QList<KileProjectItem*> list = itemsFor(oldinfo);
	KUrl url = oldinfo->url();
	TextInfo *newinfo = createTextDocumentInfo(m_ki->extensions()->determineDocumentType(url), url, oldinfo->getBaseDirectory());

	newinfo->setDoc(oldinfo->getDoc());

	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		(*it)->setInfo(newinfo);
	}

	removeTextDocumentInfo(oldinfo);

	emit(updateStructure(true, newinfo));
}

bool Manager::removeTextDocumentInfo(TextInfo *docinfo, bool closingproject /* = false */)
{
	KILE_DEBUG() << "==Manager::removeTextDocumentInfo(Info *docinfo)=====" << endl;
	QList<KileProjectItem*> itms = itemsFor(docinfo);
	bool oneItem = false;
	if(itms.count() == 1) {
		oneItem = true;
	}

	if(itms.count() == 0 || ( closingproject && oneItem )) {
		KILE_DEBUG() << "\tremoving " << docinfo <<  " count = " << m_textInfoList.count() << endl;
		m_textInfoList.remove(docinfo);

		emit ( closingDocument ( docinfo ) );

		cleanupDocumentInfoForProjectItems(docinfo);
		delete docinfo;

		return true;
	}

	KILE_DEBUG() << "\tnot removing " << docinfo << endl;
	return false;
}

KTextEditor::Document* Manager::createDocument(const QString& name, const KUrl& url, TextInfo *docinfo, const QString & encoding, const QString & highlight)
{
	KILE_DEBUG() << "==KTextEditor::Document* Manager::createDocument()===========" << endl;
	//FIXME: this shouldn't be needed twice
	KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
	if(!editor) {
#ifdef __GNUC__
#warning Check for errors at line 471!
#endif
	}
	KTextEditor::Document *doc = editor->createDocument(NULL);
	if ( docFor(url) != 0L )
		kWarning() << url << " already has a document!" << endl;
	else
		KILE_DEBUG() << "\tappending document " <<  doc << endl;

	//set the default encoding
	QString enc = encoding.isNull() ? KileConfig::defaultEncoding() : encoding;
	doc->setEncoding(enc);

    KILE_DEBUG() << "url is = " << docinfo->url() << endl;

	if(!url.isEmpty())
	{
		bool r = doc->openUrl(url);
		// don't add scripts to the recent files
		if(r && docinfo->getType() != Script)
		{
        		emit(addToRecentFiles(url));
		}
	}

	//handle changes of the document
	connect(doc, SIGNAL(nameChanged(KTextEditor::Document *)), m_ki->parentWidget(), SLOT(newCaption()));
	connect(doc, SIGNAL(fileNameChanged()), m_ki->parentWidget(), SLOT(newCaption()));
	connect(doc, SIGNAL(modStateChanged(KTextEditor::Document*)), this, SLOT(newDocumentStatus(KTextEditor::Document*)));
	connect(doc, SIGNAL(modifiedOnDisc(KTextEditor::Document*, bool, unsigned char)), this, SIGNAL(documentStatusChanged(KTextEditor::Document*, bool, unsigned char)));

	docinfo->setDoc(doc);
	docinfo->setHighlightMode(highlight);
	// FIXME: the whole structure updating stuff needs to be rewritten; updates should originate from
	//        the docinfo only, i.e. the structure view should just react to changes!
	connect(docinfo, SIGNAL(completed(KileDocument::Info*)), m_ki->structureWidget(), SLOT(update(KileDocument::Info*)));

	KILE_DEBUG() << "createDocument: url " << doc->url();
	KILE_DEBUG() << "createDocument: SANITY check: " << (docinfo->getDoc() == docFor(docinfo->url())) << endl;
	return doc;
}

KTextEditor::View* Manager::loadItem(KileDocument::Type type, KileProjectItem *item, const QString & text, bool openProjectItemViews)
{
	KTextEditor::View *view = 0L;

	KILE_DEBUG() << "==loadItem(" << item->url().path() << ")======" << endl;

	if ( item->type() != KileProjectItem::Image )
	{
		view = loadText(type, QString::null, item->url(), item->encoding(), openProjectItemViews && item->isOpen(), item->highlight(), text);
		KILE_DEBUG() << "\tloadItem: docfor = " << docFor(item->url().path()) << endl;

		TextInfo *docinfo = textInfoFor(item->url().path());
		item->setInfo(docinfo);

		KILE_DEBUG() << "\tloadItem: docinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url().path()) << endl;
		if ( docinfo->getDoc() != docFor(docinfo->url().path()) ) kWarning() << "docinfo->getDoc() != docFor()" << endl;
	}
	else
	{
		KILE_DEBUG() << "\tloadItem: no document generated" << endl;
		TextInfo *docinfo = createTextDocumentInfo(m_ki->extensions()->determineDocumentType(item->url()), item->url());
		item->setInfo(docinfo);

		if ( docFor(item->url()) == 0L)
		{
			docinfo->detach();
			KILE_DEBUG() << "\t\t\tdetached" << endl;
		}
	}

	return view;
}

KTextEditor::View* Manager::loadText(KileDocument::Type type, const QString& name, const KUrl &url , const QString & encoding /* = QString::null */, bool create /* = true */, const QString & highlight /* = QString::null */, const QString & text /* = QString::null */, int index /* = - 1 */, const KUrl& baseDirectory /* = KUrl() */)
{
	KILE_DEBUG() << "==loadText(" << url.url() << ")=================" << endl;
	//if doc already opened, update the structure view and return the view
	if ( !url.isEmpty() && m_ki->isOpen(url))
		return m_ki->viewManager()->switchToTextView(url);

	TextInfo *docinfo = createTextDocumentInfo(type, url, baseDirectory);
	KTextEditor::Document *doc = createDocument(name, url, docinfo, encoding, highlight);

	m_ki->structureWidget()->clean(docinfo);
	docinfo->updateStruct();

	if ( !text.isNull() ) doc->setText(text);

	//FIXME: use signal/slot
	if (doc && create)
		return m_ki->viewManager()->createTextView(docinfo, index);

	KILE_DEBUG() << "just after createView()" << endl;
	KILE_DEBUG() << "\tdocinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url().path()) << endl;

	return 0L;
}

//FIXME: template stuff should be in own class
KTextEditor::View* Manager::loadTemplate(TemplateItem *sel)
{
	QString text = QString::null;

	if (sel && sel->name() != DEFAULT_EMPTY_CAPTION && sel->name() != DEFAULT_EMPTY_LATEX_CAPTION && sel->name() != DEFAULT_EMPTY_BIBTEX_CAPTION)
	{
		KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
		if(!editor) {
#ifdef __GNUC__
#warning Check for errors at line 576!
#endif
		}
		//create a new document to open the template in
		KTextEditor::Document *tempdoc = editor->createDocument(NULL);

		if (!tempdoc->openUrl(KUrl(sel->path())))
		{
			KMessageBox::error(m_ki->parentWidget(), i18n("Could not find template: %1").arg(sel->name()),i18n("File Not Found"));
		}
		else
		{
			//substitute templates variables
			text = tempdoc->text();
			delete tempdoc;
			replaceTemplateVariables(text);
		}
	}

	KileDocument::Type type = sel->type();
	//always set the base directory for scripts
	return createDocumentWithText(text, type, QString(), (type == KileDocument::Script ? m_ki->scriptManager()->getLocalJScriptDirectory() : QString()));
}

KTextEditor::View* Manager::createDocumentWithText(const QString& text, KileDocument::Type type /* = KileDocument::Undefined */, const QString& extension, const KUrl& baseDirectory)
{
	KTextEditor::View *view = loadText(type, KileUntitled::next() + (extension.isEmpty() ? QString::null : '.' + extension), KUrl(), QString::null, true, QString::null, text, -1, baseDirectory);
	if (view)
	{
		//FIXME this shouldn't be necessary!!!
		view->document()->setModified(true);
		newDocumentStatus(view->document());
	}

	return view;
}

KTextEditor::View* Manager::createNewJScript()
{
	KTextEditor::View *view = createDocumentWithText(QString::null, Script, "js", m_ki->scriptManager()->getLocalJScriptDirectory());
	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	return view;
}

KTextEditor::View* Manager::createNewLaTeXDocument()
{
	KTextEditor::View *view = createDocumentWithText(QString::null, LaTeX);
	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	return view;
}

void Manager::replaceTemplateVariables(QString &line)
{
	line=line.replace("$$AUTHOR$$", KileConfig::author());
	line=line.replace("$$DOCUMENTCLASSOPTIONS$$", KileConfig::documentClassOptions());
	if (!KileConfig::templateEncoding().isEmpty()) { line=line.replace("$$INPUTENCODING$$", "\\usepackage["+ KileConfig::templateEncoding() +"]{inputenc}");}
	else { line = line.replace("$$INPUTENCODING$$","");}
}

void Manager::createTemplate()
{
	KTextEditor::View *view = m_ki->viewManager()->currentTextView();
	if (view)
	{
		if (view->document()->isModified() )
		{
			KMessageBox::information(m_ki->parentWidget(),i18n("Please save the file first."));
			return;
		}
	}
	else
	{
		KMessageBox::information(m_ki->parentWidget(),i18n("Open/create a document first."));
		return;
	}

	KUrl url = view->document()->url();
	KileDocument::Type type = m_ki->extensions()->determineDocumentType(url);

	if(type == KileDocument::Undefined || type == KileDocument::Text)
	{
		KMessageBox::information(m_ki->parentWidget(),i18n("Sorry, but a template for this type of document cannot be created."));
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

void Manager::fileNew()
{
	NewFileWizard *nfw = new NewFileWizard(m_ki->templateManager(), m_ki->parentWidget());
	if (nfw->exec())
	{
		loadTemplate(nfw->getSelection());
		if ( nfw->useWizard() ) emit ( startWizard() );
		emit(updateStructure(false, 0L));
		emit(updateModeStatus());
	}
	delete nfw;
}

void Manager::fileNew(const KUrl & url)
{
	//create an empty file
	QFile file(url.path());
	file.open(QIODevice::ReadWrite);
	file.close();

	fileOpen(url, QString::null);
}

void Manager::fileOpen()
{
	//determine the starting dir for the file dialog
    QString compileName = m_ki->getCompileName();
	QString currentDir;
    if ( QFileInfo(compileName).exists() )
        currentDir = QFileInfo(compileName).absolutePath();
    else
        currentDir = m_ki->fileSelector()->dirOperator()->url().path();

	// use a filter for fileOpen dialog
	Extensions *extensions = m_ki->extensions();
	QString filter = extensions->latexDocumentFileFilter() + '\n'
	                 + extensions->latexPackageFileFilter() + '\n'
	                 + extensions->bibtexFileFilter() + '\n'
	                 + extensions->metapostFileFilter() + '\n'
	                 + "*|" + i18n("All Files");

	//get the URLs
	KEncodingFileDialog::Result result = KEncodingFileDialog::getOpenUrlsAndEncoding( KileConfig::defaultEncoding(), currentDir, filter, m_ki->parentWidget(), i18n("Open Files") );

	//open them
	KUrl::List urls = result.URLs;
	for (KUrl::List::Iterator i=urls.begin(); i != urls.end(); ++i)
		fileOpen(*i, result.encoding);
}

void Manager::fileSelected(const KFileItem *file)
{
	fileSelected(file->url());
#ifdef __GNUC__
#warning Things left to be ported at line 724!
#endif
//FIXME: port for KDE4
// 	m_ki->fileSelector()->dirOperator()->view()->setSelected(file,false);
}

void Manager::fileSelected(const KileProjectItem * item)
{
	fileOpen(item->url(), item->encoding());
}

void Manager::fileSelected(const KUrl & url)
{
	fileOpen(url, m_ki->fileSelector()->comboEncoding()->lineEdit()->text());
}

void Manager::saveURL(const KUrl & url)
{
	KTextEditor::Document *doc = docFor(url);
	if (doc) doc->save();
}

void Manager::newDocumentStatus(KTextEditor::Document *doc)
{
	if (doc == 0L) return;

	//sync terminal
	m_ki->texKonsole()->sync();

	emit(documentStatusChanged(doc,  doc->isModified(), 0));

	//updatestructure if active document changed from modified to unmodified (typically after a save)
	if (!doc->isModified())
		emit(updateStructure(true, textInfoFor(doc)));
}

void Manager::fileSaveAll(bool amAutoSaving, bool disUntitled )
{
	KTextEditor::View *view= 0L;
	QFileInfo fi;
	bool saveResult;
	KUrl url, backupUrl;
	
	KILE_DEBUG() << "===Kile::fileSaveAll(amAutoSaving = " <<  amAutoSaving << ",disUntitled = " << disUntitled <<")" << endl;

	for(int i = 0; i < m_ki->viewManager()->textViews().count(); ++i) {
		view = m_ki->viewManager()->textView(i);

		if ( view && view->document()->isModified() )
		{
			url = view->document()->url();
			fi.setFile(url.path());
			
			if	( 	( !amAutoSaving && !(disUntitled && url.isEmpty() ) ) // DisregardUntitled is true and we have an untitled doc and don't autosave
					|| ( amAutoSaving && !url.isEmpty() ) //don't save untitled documents when autosaving
					|| ( !amAutoSaving && !disUntitled )	// both false, so we want to save everything
				)
			{
	
				KILE_DEBUG() << "The files _" << autosaveWarnings.join(", ") <<  "_ have autosaveWarnings" <<endl;
				
				if ( amAutoSaving)
				{
					if( !fi.isWritable() )
					{
						if ( autosaveWarnings.contains(url.path()) )
						{
							KILE_DEBUG() << "File " << url.prettyUrl() << " is not writeable (again), trying next file" << endl;
							continue;
						}
						else
						{
							autosaveWarnings.append(url.path());
							KILE_DEBUG() << "File " << url.prettyUrl() << " is not writeable (first time)" << endl;
						}
					}
					else
					{	
						autosaveWarnings.remove(url.path());	
					}
				}
				if (amAutoSaving && fi.size() > 0) // the size check ensures that we don't save empty files (to prevent something like #125809 in the future).
				{
					KUrl backupUrl = KUrl::fromPathOrUrl(url.path()+ ".backup");
					
				 	// patch for secure permissions, slightly modified for kile by Thomas Braun, taken from #103331
					
    					// get the right permissions, start with safe default
					mode_t  perms = 0600;
					KIO::UDSEntry fentry;
					if (KIO::NetAccess::stat (url, fentry, kapp->mainWidget()))
					{
						KILE_DEBUG () << "stating successful: " << url.prettyUrl() << endl;
						KFileItem item (fentry, url);
						perms = item.permissions();
					}

    					// first del existing file if any, than copy over the file we have
					// failure if a: the existing file could not be deleted, b: the file could not be copied
					if ( (!KIO::NetAccess::exists( backupUrl, false, kapp->mainWidget() )
					      || KIO::NetAccess::del( backupUrl, kapp->mainWidget() ) )
					      && KIO::NetAccess::file_copy(url, backupUrl, kapp->mainWidget() ) )
					{
						KILE_DEBUG()<<"backing up successful ("<<url.prettyUrl()<<" -> "<<backupUrl.prettyUrl()<<")"<<endl;
					}
					else
					{
						KILE_DEBUG()<<"backing up failed ("<<url.prettyUrl()<<" -> "<<backupUrl.prettyUrl()<<")"<<endl;
						emit printMsg(KileTool::Error,i18n("The file %1 could not be saved, check the permissions and the free disk space!").arg(backupUrl.prettyUrl()),i18n("Autosave"));
					}
				}
				
				KILE_DEBUG() << "trying to save: " << url.path() << endl;
				saveResult = view->document()->documentSave();
				fi.refresh();
			
				if(!saveResult) {
					emit printMsg(KileTool::Error,i18n("Kile encountered problems while saving the file %1. Do you have enough free disk space left?").arg(url.url()),i18n("Saving"));
				}
			}
		}
	}
}

void Manager::fileOpen(const KUrl & url, const QString & encoding, int index)
{
	KILE_DEBUG() << "==Kile::fileOpen==========================" << endl;
	
	//don't want to receive signals from the fileselector since
	//that would allow the user to open a single file twice by double-clicking on it
	m_ki->fileSelector()->blockSignals(true);

	KILE_DEBUG() << "url is " << url.url() << endl;
	const KUrl realurl = symlinkFreeURL(url);
	KILE_DEBUG() << "symlink free url is " << realurl.url() << endl;
	
	bool isopen = m_ki->isOpen(realurl);

	KTextEditor::View *view = loadText(m_ki->extensions()->determineDocumentType(realurl), QString::null, realurl, encoding, true, QString::null, QString::null, index);
	KileProjectItem *item = itemFor(realurl);

	if(!isopen)
	{
		if(!item)
			emit addToProjectView(realurl);
		else if(view)
			view->setCursorPosition(KTextEditor::Cursor(item->lineNumber(),item->columnNumber()));
	}

	emit(updateStructure(true, 0L));
	emit(updateModeStatus());
	// update undefined references in this file
	emit(updateReferences(textInfoFor(realurl.path())) );
	m_ki->fileSelector()->blockSignals(false);
}

void Manager::fileSave()
{
	KTextEditor::View* view = m_ki->viewManager()->currentTextView();
	if(!view)
	{
		return;
	}
 	KUrl url = view->document()->url();
	if(url.isEmpty()) // newly created document
	{
		fileSaveAs();
		return;
	}
	else
	{
		view->document()->documentSave();
	}
}

void Manager::fileSaveAs(KTextEditor::View* view)
{
	
	if(!view)
		view = m_ki->viewManager()->currentTextView();
	if(!view)
		return;
	
	KTextEditor::Document* doc = view->document();
	Q_ASSERT(doc);
	KileDocument::TextInfo* info = textInfoFor(doc);
	Q_ASSERT(info);
	KUrl startURL = info->url();
	KUrl oldURL = startURL;
	if(startURL.isEmpty())
	{
		if((info->getBaseDirectory()).isEmpty())
		{
			startURL = ":KILE_LATEX_SAVE_DIR";
		}
		else
		{
			startURL = doc->url();
		}
	}
	
	KILE_DEBUG() << "startURL is " << startURL.path() << endl;
	
	KEncodingFileDialog::Result result;
	KUrl saveURL;
	while(true)
	{
		QString filter = info->getFileFilter() + "\n* |" + i18n("All Files");
		result = KEncodingFileDialog::getSaveUrlAndEncoding(KileConfig::defaultEncoding(), startURL.url(), filter, m_ki->parentWidget(), i18n("Save File"));
		if(result.URLs.isEmpty() || result.URLs.first().isEmpty())
		{
			return;
		}
		saveURL = result.URLs.first();
		if(info->getType() == KileDocument::LaTeX) {
			saveURL = Info::makeValidTeXURL(saveURL, m_ki->extensions()->isTexFile(saveURL), false); // don't check for file existence
		}
		if(KIO::NetAccess::exists(saveURL, true, kapp->mainWidget())) // check for writing possibility
		{
			int r =  KMessageBox::warningContinueCancel(m_ki->parentWidget(), i18n("A file with the name \"%1\" exists already. Do you want to overwrite it ?").arg(saveURL.fileName()), i18n("Overwrite File ?"), KGuiItem(i18n("&Overwrite")));
			if(r != KMessageBox::Continue)
			{
				continue;
			}
		}
		break;
	}
	doc->setEncoding(result.encoding);
	if(!doc->saveAs(saveURL))
	{
		return;
	}
	if(oldURL != saveURL)
	{
		if(info->isDocumentTypePromotionAllowed())
		{
			recreateTextDocumentInfo(info);
			info = textInfoFor(doc);
		}
		m_ki->structureWidget()->updateUrl(info);
		emit addToRecentFiles(saveURL);
		emit addToProjectView(doc->url());
	}
}

void Manager::fileSaveCopyAs()
{
	KTextEditor::Document *doc= m_ki->activeTextDocument();
	KTextEditor::View *view = 0L;
	if(doc)
	{
		KileDocument::TextInfo *originalInfo = textInfoFor(doc);
		
		if(!originalInfo)
			return;
		
		view = createDocumentWithText(doc->text(),originalInfo->getType());
		
		KileDocument::TextInfo *newInfo = textInfoFor(view->document());
		
		if(originalInfo->url().isEmpty()) // untitled doc
			newInfo->setBaseDirectory(m_ki->fileSelector()->dirOperator()->url().path());
		else
			newInfo->setBaseDirectory(originalInfo->url().path());
		
		fileSaveAs(view);
		
		doc = view->document();
		if(doc && !doc->isModified()) // fileSaveAs was successful
			fileClose(doc);
	}
}

bool Manager::fileCloseAllOthers()
{
	KTextEditor::View * currentView = m_ki->viewManager()->currentTextView();
	QList<KTextEditor::View*> list = m_ki->viewManager()->textViews();
	list.removeAll(currentView);

	for(QList<KTextEditor::View*>::iterator i =  list.begin(); i != list.end(); ++i) {
		if (!fileClose((*i)->document())) {
			return false;
		}
	}

	return true;
}

bool Manager::fileCloseAll()
{
	KTextEditor::View * view = m_ki->viewManager()->currentTextView();

	//assumes one view per doc here
	while( ! m_ki->viewManager()->textViews().isEmpty() )
	{
		view = m_ki->viewManager()->textView(0);
		if (! fileClose(view->document())) return false;
	}

	return true;
}

bool Manager::fileClose(const KUrl & url)
{
	KTextEditor::Document *doc = docFor(url);
	if ( doc == 0L )
		return true;
	else
		return fileClose(doc);
}

bool Manager::fileClose(KTextEditor::Document *doc /* = 0L*/, bool closingproject /*= false*/)
{
	KILE_DEBUG() << "==Kile::fileClose==========================" << endl;

	if (doc == 0L)
		doc = m_ki->activeTextDocument();

	if (doc == 0L)
		return true;
	else
	//FIXME: remove from docinfo map, remove from dirwatch
	{
		KILE_DEBUG() << "doc->url().path()=" << doc->url().path() << endl;

		const KUrl url = doc->url();

		TextInfo *docinfo= textInfoFor(doc);
		if (docinfo == 0L)
		{
			kWarning() << "no DOCINFO for " << url.url() << endl;
			return true;
		}
		QList<KileProjectItem*> items = itemsFor(docinfo);
		for(QList<KileProjectItem*>::iterator it = items.begin(); it != items.end(); ++it) {
			KileProjectItem *item = *it;

			//FIXME: refactor here
 			if(item && doc) {
				storeProjectItem(item, doc);
			}
		}

		if(doc->closeUrl()) {
			// docinfo may have been recreated from 'Untitled' doc to a named doc
			if ( url.isEmpty() )
				docinfo= textInfoFor(doc);
			
			if ( KileConfig::cleanUpAfterClose() )
				cleanUpTempFiles(url, true); // yes we pass here url and not docinfo->url()

			//FIXME: use signal/slot
			m_ki->viewManager()->removeView(doc->views().first());
			//remove the decorations

			trashDoc(docinfo, doc);
            		m_ki->structureWidget()->clean(docinfo);
 			removeTextDocumentInfo(docinfo, closingproject);

			emit removeFromProjectView(url);
            		emit updateModeStatus();
		}
		else
			return false;
	}

	return true;
}

void Manager::buildProjectTree(const KUrl & url)
{
	KileProject * project = projectFor(url);

	if (project)
		buildProjectTree(project);
}

void Manager::buildProjectTree(KileProject *project)
{
	if (project == 0)
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Refresh Project Tree"));

	if (project)
	{
		//TODO: update structure for all docs
		project->buildProjectTree();
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->parentWidget(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to build the tree for, then choose Refresh Project Tree again."),i18n( "Could Not Refresh Project Tree"));
}

void Manager::projectNew()
{
	KILE_DEBUG() << "==Kile::projectNew==========================" << endl;
	KileNewProjectDlg *dlg = new KileNewProjectDlg(m_ki->templateManager(), m_ki->extensions(), m_ki->parentWidget());
	KILE_DEBUG()<< "\tdialog created" << endl;

	if (dlg->exec())
	{
		KILE_DEBUG()<< "\tdialog executed" << endl;
		KILE_DEBUG() << "\t" << dlg->name() << " " << dlg->location() << endl;

		KileProject *project = dlg->project();

		//add the project file to the project
		//TODO: shell expand the filename
		KileProjectItem *item = new KileProjectItem(project, project->url());
		item->setOpenState(false);
		projectOpenItem(item);

		if (dlg->createNewFile())
		{
			QString filename = dlg->file();

			//create the new document and fill it with the template
			//TODO: shell expand the filename
			KTextEditor::View *view = loadTemplate(dlg->getSelection());

			//derive the URL from the base url of the project
			KUrl url = project->baseURL();
			url.addPath(filename);

			TextInfo *docinfo = textInfoFor(view->document());

			//save the new file
			view->document()->saveAs(url);
            		emit documentStatusChanged(view->document(), false, 0);

			//add this file to the project
			item = new KileProjectItem(project, url);
			//project->add(item);
			mapItem(docinfo, item);

			//docinfo->updateStruct(m_kwStructure->level());
			emit(updateStructure(true, docinfo));
		}

		project->buildProjectTree();
		//project->save();
		addProject(project);
		emit(updateModeStatus());
		emit(addToRecentProjects(project->url()));
	}
}

void Manager::addProject(KileProject *project)
{
	KILE_DEBUG() << "==void Manager::addProject(const KileProject *project)==========";
	m_projects.append(project);
	KILE_DEBUG() << "\tnow " << m_projects.count() << " projects";
	emit addToProjectView(project);
	connect(project, SIGNAL(projectTreeChanged(const KileProject *)), this, SIGNAL(projectTreeChanged(const KileProject *)));
}

KileProject* Manager::selectProject(const QString& caption)
{
	QStringList list;
	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		list.append((*it)->name());
	}

	KileProject *project = NULL;
	QString name;
	if (list.count() > 1) {
		KileListSelector *dlg  = new KileListSelector(list, caption, i18n("Select Project"), m_ki->parentWidget());
		if (dlg->exec())
		{
			name = list[dlg->currentItem()];
		}
		delete dlg;
	}
	else if (list.count() == 0) {
		return NULL;
	}
	else {
		name = m_projects.first()->name();
	}

	project = projectFor(name);

	return project;
}

void Manager::addToProject(const KUrl & url)
{
	KILE_DEBUG() << "===Kile::addToProject(const KUrl & url =" << url.url() << ")" << endl;

	KileProject *project = selectProject(i18n("Add to Project"));

	if (project)
		addToProject(project, url);
}

void Manager::addToProject(KileProject* project, const KUrl & url)
{
	const KUrl realurl = symlinkFreeURL(url);
	QFileInfo fi(realurl.path());

	if (project->contains(realurl))
	{
		emit printMsg(KileTool::Info,i18n("The file %1 is already member of the project %2").arg(realurl.fileName()).arg(project->name()),i18n("Add to Project"));
		return;
	}
	else if(!fi.exists() || !fi.isReadable())
	{
		emit printMsg( KileTool::Info,i18n("The file %1 can not be added because it does not exist or is not readable").arg(realurl.fileName()),i18n("Add to Project"));
		return;
	}
	
	KileProjectItem *item = new KileProjectItem(project, realurl);
	item->setOpenState(m_ki->isOpen(realurl));
	projectOpenItem(item);
	emit addToProjectView(item);
	buildProjectTree(project);
}

void Manager::removeFromProject(KileProjectItem *item)
{
	if (item && item->project()) {
		KILE_DEBUG() << "\tprojecturl = " << item->project()->url().path() << ", url = " << item->url().path() << endl;

		if (item->project()->url() == item->url()) {
			KMessageBox::error(m_ki->parentWidget(), i18n("This file is the project file, it holds all the information about your project. Therefore it is not allowed to remove this file from its project."), i18n("Cannot Remove File From Project"));
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

void Manager::projectOpenItem(KileProjectItem *item, bool openProjectItemViews)
{
	KILE_DEBUG() << "==Kile::projectOpenItem==========================" << endl;
	KILE_DEBUG() << "\titem:" << item->url().path() << endl;

	if (m_ki->isOpen(item->url())) //remove item from projectview (this file was opened before as a normal file)
		emit removeFromProjectView(item->url());

	KTextEditor::View *view = loadItem(m_ki->extensions()->determineDocumentType(item->url()), item, QString::null, openProjectItemViews);

	if ( (!item->isOpen()) && (view == 0L) && (item->getInfo()) ) //doc shouldn't be displayed, trash the doc
		trashDoc(item->getInfo());
	else if (view != 0L)
		view->setCursorPosition(KTextEditor::Cursor(item->lineNumber(), item->columnNumber()));

	//oops, doc apparently was open while the project settings wants it closed, don't trash it the doc, update openstate instead
	if ((!item->isOpen()) && (view != 0L))
		item->setOpenState(true);
}

KileProject* Manager::projectOpen(const KUrl & url, int step, int max, bool openProjectItemViews)
{
	KILE_DEBUG() << "==Kile::projectOpen==========================" << endl;
	KILE_DEBUG() << "\tfilename: " << url.fileName() << endl;

	const KUrl realurl = symlinkFreeURL(url);

	if(m_ki->projectIsOpen(realurl)) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		KMessageBox::information(m_ki->parentWidget(), i18n("The project you tried to open is already opened. If you wanted to reload the project, close the project before you re-open it."),i18n("Project Already Open"));
		return NULL;
	}

	QFileInfo fi(realurl.path());
	if(!fi.isReadable()) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}

		if (KMessageBox::warningYesNo(m_ki->parentWidget(), i18n("The project file for this project does not exists or is not readable. Remove this project from the recent projects list?"),i18n("Could Not Load Project File"))  == KMessageBox::Yes)
			emit(removeFromRecentProjects(realurl));

		return NULL;
	}

	if(!m_progressDialog) {
		createProgressDialog();
	}

	m_progressDialog->show();

	KileProject *kp = new KileProject(realurl,m_ki->extensions());
	
	if(kp->isInvalid()) {
		if(m_progressDialog) {
			m_progressDialog->hide();
		}
		delete kp;
		return NULL;
	}

	emit(addToRecentProjects(realurl));

	QList<KileProjectItem*> list = kp->items();

	int project_steps = list.count() + 1;
	m_progressDialog->progressBar()->setMaximum(project_steps * max);
	project_steps *= step;
	m_progressDialog->progressBar()->setValue(project_steps);

	// open the project files in the correct order
	QVector<KileProjectItem*> givenPositionVector(list.count(), NULL);
	QList<KileProjectItem*> notCorrectlyOrderedList;
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		KileProjectItem *item = *it;
		int order = item->order();

		if(order >= 0 && order >= list.count()) {
			order = -1;
		}
		if(!item->isOpen() || order < 0 || givenPositionVector[order] != NULL) {
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

	unsigned int counter = 0;
	for (QList<KileProjectItem*>::iterator i = orderedList.begin(); i != orderedList.end(); ++i) {
		projectOpenItem(*i, openProjectItemViews);
		m_progressDialog->progressBar()->setValue(counter + project_steps);
		kapp->processEvents();
		++counter;
	}

	kp->buildProjectTree();
	addProject(kp);

	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	// update undefined references in all project files
	updateProjectReferences(kp);

	if (step == (max - 1)) {
		m_progressDialog->hide();
	}
	
	m_ki->viewManager()->switchToTextView(kp->lastDocument());

	return kp;
}

// as all labels are gathered in the project, we can check for unsolved references
void Manager::updateProjectReferences(KileProject *project)
{
	QList<KileProjectItem*> list = project->items();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		emit(updateReferences((*it)->getInfo()));
	}
}

KileProject* Manager::projectOpen()
{
	KILE_DEBUG() << "==Kile::projectOpen==========================" << endl;
	KUrl url = KFileDialog::getOpenUrl( KileConfig::defaultProjectLocation(), i18n("*.kilepr|Kile Project Files\n*|All Files"), m_ki->parentWidget(), i18n("Open Project") );

	if (!url.isEmpty())
		return projectOpen(url);
	else
		return 0L;
}


void Manager::projectSave(KileProject *project /* = 0 */)
{
	KILE_DEBUG() << "==Kile::projectSave==========================";
	if (!project) {
		//find the project that corresponds to the active doc
		project= activeProject();
	}

	if(!project) {
		project = selectProject(i18n("Save Project"));
	}

	if(project) {
		QList<KileProjectItem*> list = project->items();
		KTextEditor::Document *doc = NULL;
		KileProjectItem *item = NULL;
		TextInfo *docinfo = NULL;

		// determine the order in which the project items are opened
		QVector<KileProjectItem*> viewPositionVector(m_ki->viewManager()->getTabCount(), NULL);
		for(QList<KileProjectItem*>::iterator i = list.begin(); i != list.end(); ++i) {
			docinfo = (*i)->getInfo();
			if(docinfo) {
				KTextEditor::View *view = m_ki->viewManager()->textView(docinfo);
				if(view) {
					int position = m_ki->viewManager()->getIndexOf(view);
					if(position >= 0 && position < viewPositionVector.size()) {
						viewPositionVector[position] = *i;
					}
				}
			}
		}
		int position = 0;
		for(int i = 0; i < viewPositionVector.size(); ++i) {
			if(viewPositionVector[i] != NULL) {
				viewPositionVector[i]->setOrder(position);
				++position;
			}
		}

		//update the open-state of the items
		for (QList<KileProjectItem*>::iterator i = list.begin(); i != list.end(); ++i) {
			item = *i;
			KILE_DEBUG() << "\tsetOpenState(" << (*i)->url().path() << ") to " << m_ki->isOpen(item->url()) << endl;
			item->setOpenState(m_ki->isOpen(item->url()));
			docinfo = item->getInfo();

			if (docinfo != 0L)
				doc = docinfo->getDoc();
			if (doc != 0L)
				storeProjectItem(item, doc);

			doc = 0L;
			docinfo = 0L;
		}

		project->save();
	}
	else
		KMessageBox::error(m_ki->parentWidget(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to save, then choose Save Project again."),i18n( "Could Determine Active Project"));
}

void Manager::projectAddFiles(const KUrl & url)
{
	KileProject *project = projectFor(url);

	if (project)
		projectAddFiles(project,url);
}

void Manager::projectAddFiles(KileProject *project,const KUrl & fileUrl)
{
	KILE_DEBUG() << "==Kile::projectAddFiles()==========================";
 	if(project == 0) {
		project = activeProject();
	}

	if(project == 0) {
		project = selectProject(i18n("Add Files to Project"));
	}

	if (project) {
		QString currentDir;
		if(fileUrl.isEmpty())
			currentDir=project->url().directory();
		else
			currentDir=fileUrl.directory();

		KILE_DEBUG() << "currentDir is " << currentDir << endl;
		KFileDialog *dlg = new KFileDialog(currentDir, i18n("*|All Files"), m_ki->parentWidget());
		dlg->setModal(true);
		KPushButton* okButton = dlg->okButton();
		okButton->setText(i18n("Add"));
		dlg->setCaption(i18n("Add Files"));
		dlg->setMode(KFile::Files | KFile::ExistingOnly);

		if(dlg->exec()) {
			KUrl::List urls = dlg->selectedUrls();
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
		KMessageBox::error(m_ki->parentWidget(), i18n("There are no projects opened. Please open the project you want to add files to, then choose Add Files again."),i18n( "Could Not Determine Active Project"));
	}
}

void Manager::toggleArchive(KileProjectItem *item)
{
	item->setArchive(!item->archive());
}

void Manager::projectOptions(const KUrl & url)
{
	KileProject *project = projectFor(url);

	if (project)
		projectOptions(project);
}

void Manager::projectOptions(KileProject *project /* = 0*/)
{
	KILE_DEBUG() << "==Kile::projectOptions==========================" << endl;
	if (project ==0 )
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Project Options For"));

	if (project)
	{
		KILE_DEBUG() << "\t" << project->name() << endl;
		KileProjectOptionsDlg *dlg = new KileProjectOptionsDlg(project, m_ki->extensions(), m_ki->parentWidget());
		dlg->exec();
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->parentWidget(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to modify, then choose Project Options again."),i18n( "Could Not Determine Active Project"));
}

bool Manager::projectCloseAll()
{
	KILE_DEBUG() << "==Kile::projectCloseAll==========================";

	for(QList<KileProject*>::iterator it = m_projects.begin(); it != m_projects.end(); ++it) {
		if(!projectClose((*it)->url())) {
			return false;
		}
	}

	return true;
}

bool Manager::projectClose(const KUrl & url)
{
	KILE_DEBUG() << "==Kile::projectClose==========================" << endl;
	KileProject *project = 0;

	if (url.isEmpty())
	{
		 project = activeProject();

		 if (project == 0 )
			project = selectProject(i18n("Close Project"));
	}
	else
	{
		project = projectFor(url);
	}

	if(project) {
		KILE_DEBUG() << "\tclosing:" << project->name();
		project->setLastDocument(m_ki->getName());

		projectSave(project);

		QList<KileProjectItem*> list = project->items();

		bool close = true;
		KTextEditor::Document *doc = NULL;
		TextInfo *docinfo = NULL;
		for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
			KileProjectItem *item = *it;

			doc = NULL;
			docinfo = item->getInfo();
			if (docinfo) {
				doc = docinfo->getDoc();
			}
			else {
				continue;
			}
			if (doc) {
				KILE_DEBUG() << "\t\tclosing item " << doc->url().path() << endl;
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
			m_projects.remove(project);
			emit removeFromProjectView(project);
			delete project;
			emit(updateModeStatus());
			return true;
		}
		else
			return false;
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->parentWidget(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to close, then choose Close Project again."),i18n( "Could Not Close Project"));

	return true;
}

void Manager::storeProjectItem(KileProjectItem *item, KTextEditor::Document *doc)
{
	KILE_DEBUG() << "===Kile::storeProjectItem==============" << endl;
	KILE_DEBUG() << "\titem = " << item << ", doc = " << doc << endl;
	item->setEncoding(doc->encoding());
	item->setHighlight(doc->highlightingMode());
	KTextEditor::View *view = static_cast<KTextEditor::View*>(doc->views().first());
	uint l = 0, c = 0;
	if (view) {
		KTextEditor::Cursor cursor = view->cursorPosition();
		l = cursor.line();
		c = cursor.column();
	}
	item->setLineNumber(l);
	item->setColumnNumber(c);

	KILE_DEBUG() << "\t" << item->encoding() << " " << item->highlight() << " should be " << doc->highlightingMode() << endl;
}

void Manager::cleanUpTempFiles(const KUrl &url, bool silent)
{
	KILE_DEBUG() << "===void Manager::cleanUpTempFiles(const KUrl " << url.path() << ", bool " << silent << ")===" << endl;
	
	if( url.isEmpty() )
		return;
	
	QStringList extlist;
	QFileInfo fi(url.path());
	const QStringList templist = KileConfig::cleanUpFileExtensions().split(" ");
	const QString fileName = fi.fileName();
	const QString dirPath = fi.absolutePath();
	const QString baseName = fi.baseName(true);
	
	for (int i=0; i < templist.count(); ++i) {
		fi.setFile( dirPath + '/' + baseName + templist[i] );
		if ( fi.exists() )
			extlist.append(templist[i]);
	}
	
	if (!silent &&  ( KileUntitled::isUntitled(fileName) || fileName.isEmpty() ) )
		return;

	if (!silent && extlist.count() > 0 )
	{
		KILE_DEBUG() << "not silent" << endl;
		KileDialog::Clean *dialog = new KileDialog::Clean(m_ki->parentWidget(), fileName, extlist);
		if ( dialog->exec() )
			extlist = dialog->getCleanlist();
		else
		{
			delete dialog;
			return;
		}

		delete dialog;
	}

	if ( extlist.count() == 0 )
		emit printMsg(KileTool::Warning, i18n("Nothing to clean for %1").arg(fileName), i18n("Clean"));
	else
	{
		for(int i = 0; i < extlist.count(); ++i) {
			QFile file( dirPath + '/' + baseName + extlist[i] );
			KILE_DEBUG() << "About to remove file = " << file.name() << endl;
			file.remove();
		}
		emit printMsg(KileTool::Info, i18n("Cleaning %1 : %2").arg(fileName).arg(extlist.join(" ")), i18n("Clean"));
	}
}

void Manager::openDroppedURLs(QDropEvent *e) {
	KUrl::List urls;
	Extensions *extensions = m_ki->extensions();
	if(K3URLDrag::decode(e, urls)) {
		for(KUrl::List::iterator i = urls.begin(); i != urls.end(); ++i) {
			KUrl url = *i;
			if(extensions->isProjectFile(url))
			{
				projectOpen(url);
			}
			else
			{
				fileOpen(url);
			}
		}
	}
}

// Show all opened projects and switch to another one, if you want

void Manager::projectShow()
{
	if ( m_projects.count() <= 1 )
		return;

	// select the new project
	KileProject *project = selectProject(i18n("Switch Project"));
	if ( !project || project==activeProject() )
		return;

	// get last opened document
	const KUrl lastdoc = project->lastDocument();
	KileProjectItem *docitem = ( !lastdoc.isEmpty() ) ? itemFor(lastdoc,project) : NULL;

	// if not, we search for the first opened tex file of this project
	// if no file is opened, we take the first tex file mentioned in the list
	KileProjectItem *first_texitem = NULL;
	if(!docitem) {
		QList<KileProjectItem*> list = project->items();
		for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
			KileProjectItem *item = *it;

			QString itempath = item->path();

			// called from KAction 'Show projects...': find the first opened 
			// LaTeX document or, if that fails, any other opened file
			QStringList extlist = (m_ki->extensions()->latexDocuments() + ' ' + m_ki->extensions()->latexPackages()).split(" ");
			for ( QStringList::Iterator it=extlist.begin(); it!=extlist.end(); ++it )
			{
				if ( itempath.indexOf( (*it), -(*it).length() ) >= 0 ) 
				{
					if  ( m_ki->isOpen(item->url()) ) 
					{
						docitem = item;
						break;
					}
					else if ( ! first_texitem )
						first_texitem = item;
				}
			}
			if ( docitem )
				break;
		}
	}

	// did we find one opened file or must we take the first entry
	if ( ! docitem ) 
	{
		if ( ! first_texitem )
			return;
		docitem = first_texitem;
	}

	// ok, we can switch to another project now
	if  ( m_ki->isOpen(docitem->url()) )
		m_ki->viewManager()->switchToTextView(docitem->url());
	else
		fileOpen( docitem->url(),docitem->encoding() );
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
	if ( item )
	{
		if  ( item->type() == KileProjectItem::ProjectFile )
			dontOpenWarning( item, i18n("Show Project Files"), i18n("project configuration file") );
		else if ( item->type() == KileProjectItem::Image )
			dontOpenWarning( item, i18n("Show Project Files"), i18n("graphics file") );
		else
		{
			// ok, we can switch to another file
			if  ( m_ki->isOpen(item->url()) )
				m_ki->viewManager()->switchToTextView(item->url());
			else
				fileOpen( item->url(),item->encoding() );
		}
	}
}

void Manager::projectOpenAllFiles()
{
	KileProject *project = selectProject(i18n("Select Project"));
	if (project != 0L)
  {
		projectOpenAllFiles(project->url());
  }
}

void Manager::projectOpenAllFiles(const KUrl & url)
{
	KileProject* project;
	KTextEditor::Document* doc = 0L;

	if(!url.isValid())
		return;
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
				filelist << item->url().path();
			}
		}
	}
	return filelist;
}

void Manager::dontOpenWarning(KileProjectItem *item, const QString &action, const QString &filetype)
{
	emit printMsg(KileTool::Info, i18n("not opened: %1 (%2)").arg(item->url().path()).arg(filetype), action);
}

KileProjectItem* Manager::selectProjectFileItem(const QString &label)
{
	// select a project
	KileProject *project = selectProject(i18n("Select Project"));
	if ( ! project )
		return 0L;

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
	KileProjectItem *item = NULL;
	KileListSelector *dlg  = new KileListSelector(filelist,i18n("Project Files"),label, m_ki->parentWidget());
	if ( dlg->exec() ) {
		if ( dlg->currentItem() >= 0 ) {
			QString name = filelist[dlg->currentItem()];
			if ( map.contains(name) )
				item = map[name];
			else
				KMessageBox::error(m_ki->parentWidget(), i18n("Could not determine the selected file."),i18n( "Project Error"));
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

	QStringList filelist, selectedfiles;
	QMap<QString,KileProjectItem *> map;

	QList<KileProjectItem*> list = project->items();
	for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
		KileProjectItem *item = *it;

		filelist << item->path();
		map[item->path()] = item;
	}

	QList<KileProjectItem*> itemsList;

	KileListSelectorMultiple *dlg  = new KileListSelectorMultiple(filelist, i18n("Project Files"), label, m_ki->parentWidget());
	if(dlg->exec()) {
		if(dlg->currentItem() >= 0) {
			selectedfiles = dlg->selected();
			for(QStringList::Iterator it = selectedfiles.begin(); it != selectedfiles.end(); ++it ){
				if(map.contains(*it)) {
					itemsList.append(map[(*it)]);
				}
				else {
					KMessageBox::error(m_ki->parentWidget(), i18n("Could not determine the selected file."), i18n( "Project Error"));
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
 	KILE_DEBUG() << "===Kile::projectAddFile==============" << endl;
	KileProject *project = activeProject();
 	if ( ! project )
		return;

	QFileInfo fi(filename);
	if ( ! fi.exists() )
	{
		if ( graphics )
			return;
		
		// called from InputDialog after a \input- or \include command:
		//  - if the chosen file has an extension: accept
		//  - if not we add the default TeX extension: accept if it exists else reject
		QString ext = fi.completeSuffix();
		if ( ! ext.isEmpty() )
			return;

		filename += m_ki->extensions()->latexDocumentDefault();
		if ( QFileInfo(filename).exists() )
			return;
	}

	//ok, we have a project and an existing file
	KILE_DEBUG() << "\tadd file: " << filename << endl;
	m_ki->viewManager()->updateStructure(true);

	KUrl url;
	url.setPath(filename);
	addToProject(project, url);
}

const KUrl Manager::symlinkFreeURL(const KUrl& url)
{
	KILE_DEBUG() << "===symlinkFreeURL==" << endl;

	if( !url.isLocalFile() )
		return url;

	QDir dir(url.directory());
	QString filename=url.path(); // if the directory does not exist we return the old url (just to be sure)

	if(dir.exists())
		filename= dir.canonicalPath() + '/' + url.fileName();
	else
		KILE_DEBUG() << "directory " << url.directory() << "does not exist" << endl;

	return KUrl::fromPathOrUrl(filename);
}

void Manager::cleanupDocumentInfoForProjectItems(KileDocument::Info *info)
{
	QList<KileProjectItem*> itemsList = itemsFor(info);
	for(QList<KileProjectItem*>::iterator it = itemsList.begin(); it != itemsList.end(); ++it) {
		(*it)->setInfo(NULL);
	}
}

void Manager::createProgressDialog()
{
	m_progressDialog = new KProgressDialog(kapp->activeWindow(), i18n("Open Project..."));
	m_progressDialog->setModal(true);
	m_progressDialog->showCancelButton(false);
	m_progressDialog->setLabelText(i18n("Scanning project files..."));
	m_progressDialog->setAutoClose(true);
	m_progressDialog->setMinimumDuration(2000);
	m_progressDialog->hide();
}

}

#include "kiledocmanager.moc"
