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

#include <qtextcodec.h>
#include <qfile.h>
#include <qdir.h>

#include <kate/document.h>
#include <kate/view.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kfile.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kpushbutton.h>
#include <kurl.h>
#include <kurldrag.h>
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
#include "kileprojectdlgs.h"
#include "kiletool.h"
#include "kiletool_enums.h"
#include "kilestdtools.h"
#include "kilelistselector.h"
#include "kiletoolmanager.h"
#include "kilekonsolewidget.h"
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
	m_ki(info)
{
	m_textInfoList.setAutoDelete(false);
	m_projects.setAutoDelete(false);
	Kate::Document::setFileChangedDialogsActivated (true);

	if ( KileConfig::defaultEncoding() == "invalid" )
		KileConfig::setDefaultEncoding(QString::fromLatin1(QTextCodec::codecForLocale()->name()));

	QWidget *par = m_ki ? m_ki->parentWidget() : 0;
	m_kpd = new KProgressDialog(par, 0, i18n("Open Project..."), QString::null, true);
	m_kpd->showCancelButton(false);
	m_kpd->setLabel(i18n("Scanning project files..."));
	m_kpd->setAutoClose(true);
	m_kpd->setMinimumDuration(2000);
	m_kpd->cancel();
}

Manager::~Manager()
{
	kdDebug() << "==KileDocument::Manager::~Manager()=========" << endl;
}

void Manager::trashDoc(TextInfo *docinfo, Kate::Document *doc /*= 0L*/ )
{
	kdDebug() << "==void Manager::trashDoc(" << docinfo->url().path() << ")=====" << endl;

	if ( m_ki->isOpen(docinfo->url()) ) return;

	if ( doc == 0L ) doc = docinfo->getDoc();
	//look for doc before we detach the docinfo
	//if we do it the other way around, docFor will always return nil
 	if ( doc == 0L ) doc = docFor(docinfo->url());

	kdDebug() << "DETACHING " << docinfo << endl;
	docinfo->detach();

	kdDebug() << "\tTRASHING " <<  doc  << endl;
	if ( doc == 0L ) return;

	kdDebug() << "just checking: docinfo->getDoc() =  " << docinfo->getDoc() << endl;
	kdDebug() << "just checking: docFor(docinfo->url()) = " << docFor(docinfo->url()) << endl;

	for ( uint i = 0; i < m_textInfoList.count(); ++i )
	{
		if ( (m_textInfoList.at(i) != docinfo) && (m_textInfoList.at(i)->getDoc() == doc) )
		{
			KMessageBox::information(0, i18n("The internal structure of Kile is corrupted (probably due to a bug in Kile). Please select Save All from the File menu and close Kile.\nThe Kile team apologizes for any inconvenience and would appreciate a bug report."));
			kdWarning() << "docinfo " << m_textInfoList.at(i) << " url " << m_textInfoList.at(i)->url().fileName() << " has a wild pointer!!!"<< endl;
		}
	}

	kdDebug() << "DELETING doc" << endl;
	delete doc;
}

// update all Info's with changed user commands
void Manager::updateInfos()
{
	for (uint i=0; i < m_textInfoList.count(); ++i)
	{
		m_textInfoList.at(i)->updateStructLevelInfo();
	}
}

Kate::Document* Manager::docFor(const KURL & url)
{
	for (uint i=0; i < m_textInfoList.count(); ++i)
	{
		if ( m_ki->similarOrEqualURL(m_textInfoList.at(i)->url(),url) )
        {
			return m_textInfoList.at(i)->getDoc();
        }
	}

	return 0L;
}

Info* Manager::getInfo() const
{
	Kate::Document *doc = m_ki->activeTextDocument();
	if ( doc != 0L )
		return textInfoFor(doc);
	else
		return 0L;
}

TextInfo* Manager::textInfoFor(const QString & path) const
{
	if( path.isEmpty() )
		return 0L;
	
	kdDebug() << "==KileInfo::textInfoFor(" << path << ")==========================" << endl;
	QPtrListIterator<TextInfo> it(m_textInfoList);
	while ( true )
	{
		if ( it.current()->url().path() == path)
			return it.current();

		if (it.atLast()) break;

		++it;
	}

	kdDebug() << "\tCOULD NOT find info for " << path << endl;
	return 0L;
}

TextInfo* Manager::textInfoForURL(const KURL& url)
{
	if( url.isEmpty() )
		return 0L;
	
	kdDebug() << "==KileInfo::textInfoFor(" << url << ")==========================" << endl;
	QPtrListIterator<TextInfo> it(m_textInfoList);
	TextInfo *info;
	while ( (info = it.current()) != 0L )
	{
		if ( info->url() == url)
		{
			return info;
		}
		++it;
	}

	kdDebug() << "\tCOULD NOT find info for " << url << endl;
	return 0L;
}

TextInfo* Manager::textInfoFor(Kate::Document* doc) const
{
	if( !doc )
		return 0L;
	
    QPtrListIterator<TextInfo> it(m_textInfoList);
    while ( true )
    {
        if ( it.current()->getDoc() == doc)
            return it.current();

        if (it.atLast()) return 0L;

        ++it;
    }

    return 0;
}

void Manager::mapItem(TextInfo *docinfo, KileProjectItem *item)
{
	item->setInfo(docinfo);
}

KileProject* Manager::projectFor(const KURL &projecturl)
{
	KileProject *project = 0;

	//find project with url = projecturl
	QPtrListIterator<KileProject> it(m_projects);
	while ( it.current() )
	{
		if ((*it)->url() == projecturl)
		{
			return *it;
		}
		++it;
	}

	return project;
}

KileProject* Manager::projectFor(const QString &name)
{
	KileProject *project = 0;

	//find project with url = projecturl
	QPtrListIterator<KileProject> it(m_projects);
	while ( it.current() )
	{
		if ((*it)->name() == name)
		{
			return *it;
		}
		++it;
	}

	return project;
}

KileProjectItem* Manager::itemFor(const KURL &url, KileProject *project /*=0L*/) const
{
	if (project == 0L)
	{
		QPtrListIterator<KileProject> it(m_projects);
		while ( it.current() )
		{
			kdDebug() << "looking in project " << (*it)->name() << endl;
			if ((*it)->contains(url))
			{
				kdDebug() << "\t\tfound!" << endl;
				return (*it)->item(url);
			}
			++it;
		}
		kdDebug() << "\t nothing found" << endl;
	}
	else
	{
		if ( project->contains(url) )
			return project->item(url);
	}

	return 0L;
}

KileProjectItem* Manager::itemFor(Info *docinfo, KileProject *project /*=0*/) const
{
	if(docinfo)
		return itemFor(docinfo->url(), project);
	else
		return 0L;
}

KileProjectItemList* Manager::itemsFor(Info *docinfo) const
{
	if ( docinfo == 0 ) return 0L;

	kdDebug() << "==KileInfo::itemsFor(" << docinfo->url().fileName() << ")============" << endl;
	KileProjectItemList *list = new KileProjectItemList();
	list->setAutoDelete(false);

	QPtrListIterator<KileProject> it(m_projects);
	while ( it.current() )
	{
		kdDebug() << "\tproject: " << (*it)->name() << endl;
		if ((*it)->contains(docinfo))
		{
			kdDebug() << "\t\tcontains" << endl;
			list->append((*it)->item(docinfo));
		}
		++it;
	}

	return list;
}

bool Manager::isProjectOpen()
{
	return ( m_projects.count() > 0 );
}

KileProject* Manager::activeProject()
{
	KileProject *curpr=0;
	Kate::Document *doc = m_ki->activeTextDocument();

	if (doc)
	{
		for (uint i=0; i < m_projects.count(); ++i)
		{
			if (m_projects.at(i)->contains(doc->url()) )
			{
				curpr = m_projects.at(i);
				break;
			}
		}
	}

	return curpr;
}

KileProjectItem* Manager::activeProjectItem()
{
	KileProject *curpr = activeProject();
	Kate::Document *doc = m_ki->activeTextDocument();
	KileProjectItem *item = 0;

	if (curpr && doc)
	{
		KileProjectItemList *list = curpr->items();

		for (uint i=0; i < list->count(); ++i)
		{
			if (list->at(i)->url() == doc->url())
			{
				item = list->at(i);
				break;
			}
		}
	}

	return item;
}

TextInfo* Manager::createTextDocumentInfo(KileDocument::Type type, const KURL & url, const KURL& baseDirectory)
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
				kdDebug() << "CREATING TextInfo for " << url.url() << endl;
				docinfo = new TextInfo(0L,m_ki->extensions());
				break;
			case LaTeX:
				kdDebug() << "CREATING LaTeXInfo for " << url.url() << endl;
				docinfo = new LaTeXInfo(0L, m_ki->extensions(), m_ki->latexCommands(), m_ki->eventFilter());
				break;
			case BibTeX:
				kdDebug() << "CREATING BibInfo for " << url.url() << endl;
				docinfo = new BibInfo(0L, m_ki->extensions(), m_ki->latexCommands());
				break;
			case Script:
				kdDebug() << "CREATING ScriptInfo for " << url.url() << endl;
				docinfo = new ScriptInfo(0L, m_ki->extensions());
				break;
		}
		docinfo->setBaseDirectory(baseDirectory);
		emit(documentInfoCreated(docinfo));
		m_textInfoList.append(docinfo);
	}

	kdDebug() << "DOCINFO: returning " << docinfo << " " << docinfo->url().fileName() << endl;
	return docinfo;
}

void Manager::recreateTextDocumentInfo(TextInfo *oldinfo)
{
	KileProjectItemList *list = itemsFor(oldinfo);
	KURL url = oldinfo->url();
	TextInfo *newinfo = createTextDocumentInfo(m_ki->extensions()->determineDocumentType(url), url, oldinfo->getBaseDirectory());

	newinfo->setDoc(oldinfo->getDoc());

	KileProjectItem *pritem = 0L;
	for ( pritem = list->first(); pritem; pritem = list->next() )
	{
		pritem->setInfo(newinfo);
	}
	removeTextDocumentInfo(oldinfo);

	emit(updateStructure(true, newinfo));
}

bool Manager::removeTextDocumentInfo(TextInfo *docinfo, bool closingproject /* = false */)
{
	kdDebug() << "==Manager::removeTextDocumentInfo(Info *docinfo)=====" << endl;
	KileProjectItemList *itms = itemsFor(docinfo);
	bool oneItem = false;
	if (itms->count() == 1)
		oneItem = true;

	if (itms->count() == 0 || ( closingproject && oneItem ))
	{
		kdDebug() << "\tremoving " << docinfo <<  " count = " << m_textInfoList.count() << endl;
		m_textInfoList.remove(docinfo);

		emit ( closingDocument ( docinfo ) );

		cleanupDocumentInfoForProjectItems(docinfo);
		delete docinfo;
		delete itms;

		return true;
	}

	kdDebug() << "\tnot removing " << docinfo << endl;
	delete itms;
	return false;
}

Kate::Document* Manager::createDocument(const QString& name, const KURL& url, TextInfo *docinfo, const QString & encoding, const QString & highlight)
{
	kdDebug() << "==Kate::Document* Manager::createDocument()===========" << endl;

	Kate::Document *doc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");
	if ( docFor(url) != 0L )
		kdWarning() << url << " already has a document!" << endl;
	else
		kdDebug() << "\tappending document " <<  doc << endl;

	//set the default encoding
	QString enc = encoding.isNull() ? KileConfig::defaultEncoding() : encoding;
	doc->setEncoding(enc);

    kdDebug() << "url is = " << docinfo->url() << endl;

	if(!url.isEmpty())
	{
		bool r = doc->openURL(url);
		// don't add scripts to the recent files
		if(r && docinfo->getType() != Script)
		{
        		emit(addToRecentFiles(url));
		}
	}
	doc->setDocName(name.isEmpty() ? url.fileName() : name);

	//handle changes of the document
	connect(doc, SIGNAL(nameChanged(Kate::Document *)), m_ki->parentWidget(), SLOT(newCaption()));
	connect(doc, SIGNAL(fileNameChanged()), m_ki->parentWidget(), SLOT(newCaption()));
	connect(doc, SIGNAL(modStateChanged(Kate::Document*)), this, SLOT(newDocumentStatus(Kate::Document*)));
	connect(doc, SIGNAL(modifiedOnDisc(Kate::Document*, bool, unsigned char)), this, SIGNAL(documentStatusChanged(Kate::Document*, bool, unsigned char)));

	docinfo->setDoc(doc);
	docinfo->setHighlightMode(highlight);

	kdDebug() << "createDocument: url " << doc->url() << " name " << doc->docName() << endl;
	kdDebug() << "createDocument: SANITY check: " << (docinfo->getDoc() == docFor(docinfo->url())) << endl;
	return doc;
}

Kate::View* Manager::loadItem(KileDocument::Type type, KileProjectItem *item, const QString & text, bool openProjectItemViews)
{
	Kate::View *view = 0L;

	kdDebug() << "==loadItem(" << item->url().path() << ")======" << endl;

	if ( item->type() != KileProjectItem::Image )
	{
		view = loadText(type, QString::null, item->url(), item->encoding(), openProjectItemViews && item->isOpen(), item->highlight(), text);
		kdDebug() << "\tloadItem: docfor = " << docFor(item->url().path()) << endl;

		TextInfo *docinfo = textInfoFor(item->url().path());
		item->setInfo(docinfo);

		kdDebug() << "\tloadItem: docinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url().path()) << endl;
		if ( docinfo->getDoc() != docFor(docinfo->url().path()) ) kdWarning() << "docinfo->getDoc() != docFor()" << endl;
	}
	else
	{
		kdDebug() << "\tloadItem: no document generated" << endl;
		TextInfo *docinfo = createTextDocumentInfo(m_ki->extensions()->determineDocumentType(item->url()), item->url());
		item->setInfo(docinfo);

		if ( docFor(item->url()) == 0L)
		{
			docinfo->detach();
			kdDebug() << "\t\t\tdetached" << endl;
		}
	}

	return view;
}

Kate::View* Manager::loadText(KileDocument::Type type, const QString& name, const KURL &url , const QString & encoding /* = QString::null */, bool create /* = true */, const QString & highlight /* = QString::null */, const QString & text /* = QString::null */, int index /* = - 1 */, const KURL& baseDirectory /* = KURL() */)
{
	kdDebug() << "==loadText(" << url.url() << ")=================" << endl;
	//if doc already opened, update the structure view and return the view
	if ( !url.isEmpty() && m_ki->isOpen(url))
		return m_ki->viewManager()->switchToTextView(url);

	TextInfo *docinfo = createTextDocumentInfo(type, url, baseDirectory);
	Kate::Document *doc = createDocument(name, url, docinfo, encoding, highlight);

	m_ki->structureWidget()->clean(docinfo);
	docinfo->updateStruct();

	if ( !text.isNull() ) doc->setText(text);

	//FIXME: use signal/slot
	if (doc && create)
		return m_ki->viewManager()->createTextView(docinfo, index);

	kdDebug() << "just after createView()" << endl;
	kdDebug() << "\tdocinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url().path()) << endl;

	return 0L;
}

//FIXME: template stuff should be in own class
Kate::View* Manager::loadTemplate(TemplateItem *sel)
{
	QString text = QString::null;

	if (sel && sel->name() != DEFAULT_EMPTY_CAPTION && sel->name() != DEFAULT_EMPTY_LATEX_CAPTION && sel->name() != DEFAULT_EMPTY_BIBTEX_CAPTION)
	{
		//create a new document to open the template in
		Kate::Document *tempdoc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");

		if (!tempdoc->openURL(KURL(sel->path())))
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

	return createDocumentWithText(text, sel->type());
}

Kate::View* Manager::createDocumentWithText(const QString& text, KileDocument::Type type /* = KileDocument::Undefined */, const QString& extension, const KURL& baseDirectory)
{
	Kate::View *view = loadText(type, KileUntitled::next() + (extension.isEmpty() ? QString::null : '.' + extension), KURL(), QString::null, true, QString::null, text, -1, baseDirectory);
	if (view)
	{
		//FIXME this shouldn't be necessary!!!
		view->getDoc()->setModified(true);
		newDocumentStatus(view->getDoc());
	}

	return view;
}

Kate::View* Manager::createNewJScript()
{
	Kate::View *view = createDocumentWithText(QString::null, Script, "js", m_ki->scriptManager()->getLocalJScriptDirectory());
	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	return view;
}

Kate::View* Manager::createNewLaTeXDocument()
{
	Kate::View *view = createDocumentWithText(QString::null, LaTeX);
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
	if (m_ki->viewManager()->currentTextView())
	{
		if (m_ki->viewManager()->currentTextView()->getDoc()->isModified() )
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

	QFileInfo fi(m_ki->viewManager()->currentTextView()->getDoc()->url().path());
	ManageTemplatesDialog mtd(&fi,i18n("Create Template From Document"));
	mtd.exec();
}

void Manager::removeTemplate()
{
	ManageTemplatesDialog mtd(i18n("Remove Template"));
	mtd.exec();
}

void Manager::fileNew()
{
	NewFileWizard *nfw = new NewFileWizard(m_ki->parentWidget());
	if (nfw->exec())
	{
		loadTemplate(nfw->getSelection());
		if ( nfw->useWizard() ) emit ( startWizard() );
		emit(updateStructure(false, 0L));
		emit(updateModeStatus());
	}
	delete nfw;
}

void Manager::fileNew(const KURL & url)
{
	//create an empty file
	QFile file(url.path());
	file.open(IO_ReadWrite);
	file.close();

	fileOpen(url, QString::null);
}

void Manager::fileOpen()
{
	//determine the starting dir for the file dialog
    QString compileName = m_ki->getCompileName();
	QString currentDir;
    if ( QFileInfo(compileName).exists() )
        currentDir = QFileInfo(compileName).dirPath(true);
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
	KEncodingFileDialog::Result result = KEncodingFileDialog::getOpenURLsAndEncoding( KileConfig::defaultEncoding(), currentDir, filter, m_ki->parentWidget(), i18n("Open Files") );

	//open them
	KURL::List urls = result.URLs;
	for (KURL::List::Iterator i=urls.begin(); i != urls.end(); ++i)
		fileOpen(*i, result.encoding);
}

void Manager::fileSelected(const KFileItem *file)
{
	fileSelected(file->url());
	m_ki->fileSelector()->dirOperator()->view()->setSelected(file,false);
}

void Manager::fileSelected(const KileProjectItem * item)
{
	fileOpen(item->url(), item->encoding());
}

void Manager::fileSelected(const KURL & url)
{
	fileOpen(url, m_ki->fileSelector()->comboEncoding()->lineEdit()->text());
}

void Manager::saveURL(const KURL & url)
{
	Kate::Document *doc = docFor(url);
	if (doc) doc->save();
}

void Manager::newDocumentStatus(Kate::Document *doc)
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
	Kate::View *view= 0L;
	QFileInfo fi;
	int saveResult = Kate::View::SAVE_ERROR;
	KURL url, backupUrl;
	
	kdDebug() << "===Kile::fileSaveAll(amAutoSaving = " <<  amAutoSaving << ",disUntitled = " << disUntitled <<")" << endl;

	for (uint i = 0; i < m_ki->viewManager()->textViews().count(); ++i)
	{
		view = m_ki->viewManager()->textView(i);

		if ( view && view->getDoc()->isModified() )
		{
			url = view->getDoc()->url();
			fi.setFile(url.path());
			
			if	( 	( !amAutoSaving && !(disUntitled && url.isEmpty() ) ) // DisregardUntitled is true and we have an untitled doc and don't autosave
					|| ( amAutoSaving && !url.isEmpty() ) //don't save untitled documents when autosaving
					|| ( !amAutoSaving && !disUntitled )	// both false, so we want to save everything
				)
			{
	
				kdDebug() << "The files _" << autosaveWarnings.join(", ") <<  "_ have autosaveWarnings" <<endl;
				
				if ( amAutoSaving)
				{
					if( !fi.isWritable() )
					{
						if ( autosaveWarnings.contains(url.path()) )
						{
							kdDebug() << "File " << url.prettyURL() << " is not writeable (again), trying next file" << endl;
							continue;
						}
						else
						{
							autosaveWarnings.append(url.path());
							kdDebug() << "File " << url.prettyURL() << " is not writeable (first time)" << endl;
						}
					}
					else
					{	
						autosaveWarnings.remove(url.path());	
					}
				}
				if (amAutoSaving && fi.size() > 0) // the size check ensures that we don't save empty files (to prevent something like #125809 in the future).
				{
					KURL backupUrl = KURL::fromPathOrURL(url.path()+ ".backup");
					
				 	// patch for secure permissions, slightly modified for kile by Thomas Braun, taken from #103331
					
    					// get the right permissions, start with safe default
					mode_t  perms = 0600;
					KIO::UDSEntry fentry;
					if (KIO::NetAccess::stat (url, fentry, kapp->mainWidget()))
					{
						kdDebug () << "stating successful: " << url.prettyURL() << endl;
						KFileItem item (fentry, url);
						perms = item.permissions();
					}

    					// first del existing file if any, than copy over the file we have
					// failure if a: the existing file could not be deleted, b: the file could not be copied
					if ( (!KIO::NetAccess::exists( backupUrl, false, kapp->mainWidget() )
					      || KIO::NetAccess::del( backupUrl, kapp->mainWidget() ) )
					      && KIO::NetAccess::file_copy( url, backupUrl, perms, true, false, kapp->mainWidget() ) )
					{
						kdDebug()<<"backing up successful ("<<url.prettyURL()<<" -> "<<backupUrl.prettyURL()<<")"<<endl;
					}
					else
					{
						kdDebug()<<"backing up failed ("<<url.prettyURL()<<" -> "<<backupUrl.prettyURL()<<")"<<endl;
						emit printMsg(KileTool::Error,i18n("The file %1 could not be saved, check the permissions and the free disk space!").arg(backupUrl.prettyURL()),i18n("Autosave"));
					}
				}
				
				kdDebug() << "trying to save: " << url.path() << endl;
				saveResult = view->save();
				fi.refresh();
			
				if(saveResult == Kate::View::SAVE_ERROR && fi.size() == 0 && !url.isEmpty()) // we probably hit bug #125809, inform the user of the possible consequences
					emit printMsg(KileTool::Error,i18n("Kile encountered problems while saving the file %1. Do you have enough free disk space left?").arg(url.url()),i18n("Saving"));
			}
		}
	}
}

void Manager::fileOpen(const KURL & url, const QString & encoding, int index)
{
	kdDebug() << "==Kile::fileOpen==========================" << endl;
	
	//don't want to receive signals from the fileselector since
	//that would allow the user to open a single file twice by double-clicking on it
	m_ki->fileSelector()->blockSignals(true);

	kdDebug() << "url is " << url.url() << endl;
	const KURL realurl = symlinkFreeURL(url);
	kdDebug() << "symlink free url is " << realurl.url() << endl;
	
	bool isopen = m_ki->isOpen(realurl);

	Kate::View *view = loadText(m_ki->extensions()->determineDocumentType(realurl), QString::null, realurl, encoding, true, QString::null, QString::null, index);
	KileProjectItem *item = itemFor(realurl);

	if(!isopen)
	{
		if(!item)
			emit addToProjectView(realurl);
		else if(view)
			view->setCursorPosition(item->lineNumber(),item->columnNumber());
	}

	emit(updateStructure(true, 0L));
	emit(updateModeStatus());
	// update undefined references in this file
	emit(updateReferences(textInfoFor(realurl.path())) );
	m_ki->fileSelector()->blockSignals(false);
}

void Manager::fileSave()
{
	Kate::View* view = m_ki->viewManager()->currentTextView();
	if(!view)
	{
		return;
	}
 	KURL url = view->getDoc()->url();
	if(url.isEmpty()) // newly created document
	{
		fileSaveAs();
		return;
	}
	else
	{
		view->save();
	}
}

void Manager::fileSaveAs()
{
	Kate::View* view = m_ki->viewManager()->currentTextView();
	if(!view)
	{
		return;
	}
	Kate::Document* doc = view->getDoc();
	Q_ASSERT(doc);
	KileDocument::TextInfo* info = textInfoFor(doc);
	Q_ASSERT(info);
	KURL startURL = info->url();
	KURL oldURL = startURL;
	if(startURL.isEmpty())
	{
		if((info->getBaseDirectory()).isEmpty())
		{
			startURL = ":KILE_LATEX_SAVE_DIR";
		}
		else
		{
			startURL = info->getBaseDirectory();
			startURL.setFileName(doc->docName());
		}
	}
	KEncodingFileDialog::Result result;
	KURL saveURL;
	while(true)
	{
		QString filter = info->getFileFilter() + "\n* |" + i18n("All Files");
		result = KEncodingFileDialog::getSaveURLAndEncoding(KileConfig::defaultEncoding(), startURL.url(), filter, m_ki->parentWidget(), i18n("Save File"));
		if(result.URLs.isEmpty() || result.URLs.first().isEmpty())
		{
			return;
		}
		saveURL = result.URLs.first();
		saveURL = Info::makeValidTeXURL(saveURL, m_ki->extensions()->isTexFile(saveURL), false); // don't check for file existence
		if(KIO::NetAccess::exists(saveURL, true, kapp->mainWidget())) // check for writing possibility
		{
			int r =  KMessageBox::warningContinueCancel(m_ki->parentWidget(), i18n("A file with the name \"%1\" exists already. Do you want to overwrite it ?").arg(saveURL.fileName()), i18n("Overwrite File ?"), KGuiItem(i18n("&Overwrite")), QString::null);
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

bool Manager::fileCloseAllOthers()
{
	Kate::View * currentview = m_ki->viewManager()->currentTextView();
	while (  m_ki->viewManager()->textViews().count() > 1 )
	{
		Kate::View *view =  m_ki->viewManager()->textViews().first();
		if ( view == currentview )
			view =  m_ki->viewManager()->textViews().next();
		if ( ! fileClose(view->getDoc()) )
			return false;
	}

	return true;
}

bool Manager::fileCloseAll()
{
	Kate::View * view = m_ki->viewManager()->currentTextView();

	//assumes one view per doc here
	while( ! m_ki->viewManager()->textViews().isEmpty() )
	{
		view = m_ki->viewManager()->textView(0);
		if (! fileClose(view->getDoc())) return false;
	}

	return true;
}

bool Manager::fileClose(const KURL & url)
{
	Kate::Document *doc = docFor(url);
	if ( doc == 0L )
		return true;
	else
		return fileClose(doc);
}

bool Manager::fileClose(Kate::Document *doc /* = 0L*/, bool closingproject /*= false*/)
{
	kdDebug() << "==Kile::fileClose==========================" << endl;

	if (doc == 0L)
		doc = m_ki->activeTextDocument();

	if (doc == 0L)
		return true;
	else
	//FIXME: remove from docinfo map, remove from dirwatch
	{
		kdDebug() << "doc->url().path()=" << doc->url().path() << endl;

		const KURL url = doc->url();

		TextInfo *docinfo= textInfoFor(doc);
		if (docinfo == 0L)
		{
			kdWarning() << "no DOCINFO for " << url.url() << endl;
			return true;
		}
		KileProjectItemList *items = itemsFor(docinfo);

		while ( items->current() )
		{
			//FIXME: refactor here
 			if (items->current() && doc) storeProjectItem(items->current(),doc);
			items->next();
		}

		delete items;

		if ( doc->closeURL() )
		{
			// docinfo may have been recreated from 'Untitled' doc to a named doc
			if ( url.isEmpty() )
				docinfo= textInfoFor(doc);
			
			if ( KileConfig::cleanUpAfterClose() )
				cleanUpTempFiles(url, true); // yes we pass here url and not docinfo->url()

			//FIXME: use signal/slot
			m_ki->viewManager()->removeView(static_cast<Kate::View*>(doc->views().first()));
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

void Manager::buildProjectTree(const KURL & url)
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
	kdDebug() << "==Kile::projectNew==========================" << endl;
	KileNewProjectDlg *dlg = new KileNewProjectDlg(m_ki->extensions(), m_ki->parentWidget());
	kdDebug()<< "\tdialog created" << endl;

	if (dlg->exec())
	{
		kdDebug()<< "\tdialog executed" << endl;
		kdDebug() << "\t" << dlg->name() << " " << dlg->location() << endl;

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
			Kate::View *view = loadTemplate(dlg->getSelection());

			//derive the URL from the base url of the project
			KURL url = project->baseURL();
			url.addPath(filename);

			TextInfo *docinfo = textInfoFor(view->getDoc());

			//save the new file
			view->getDoc()->saveAs(url);
            		emit documentStatusChanged(view->getDoc(), false, 0);

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

void Manager::addProject(const KileProject *project)
{
	kdDebug() << "==void Manager::addProject(const KileProject *project)==========" << endl;
	m_projects.append(project);
	kdDebug() << "\tnow " << m_projects.count() << " projects" << endl;
	emit addToProjectView(project);
	connect(project, SIGNAL(projectTreeChanged(const KileProject *)), this, SIGNAL(projectTreeChanged(const KileProject *)));
}

KileProject* Manager::selectProject(const QString& caption)
{
	QStringList list;
	QPtrListIterator<KileProject> it(m_projects);
	while (it.current())
	{
		list.append((*it)->name());
		++it;
	}

	KileProject *project = 0;
	QString name = QString::null;
	if (list.count() > 1)
	{
		KileListSelector *dlg  = new KileListSelector(list, caption, i18n("Select Project"), m_ki->parentWidget());
		if (dlg->exec())
		{
			name = list[dlg->currentItem()];
		}
		delete dlg;
	}
	else if (list.count() == 0)
	{
		return 0;
	}
	else
		name = m_projects.first()->name();

	project = projectFor(name);

	return project;
}

void Manager::addToProject(const KURL & url)
{
	kdDebug() << "===Kile::addToProject(const KURL & url =" << url.url() << ")" << endl;

	KileProject *project = selectProject(i18n("Add to Project"));

	if (project)
		addToProject(project, url);
}

void Manager::addToProject(KileProject* project, const KURL & url)
{
	const KURL realurl = symlinkFreeURL(url);
	QFileInfo fi(realurl.path());

	if (project->contains(realurl))
	{
		emit printMsg(KileTool::Info,i18n("The file %1 is already member of the project %2").arg(realurl.filename()).arg(project->name()),i18n("Add to Project"));
		return;
	}
	else if(!fi.exists() || !fi.isReadable())
	{
		emit printMsg( KileTool::Info,i18n("The file %1 can not be added because it does not exist or is not readable").arg(realurl.filename()),i18n("Add to Project"));
		return;
	}
	
	KileProjectItem *item = new KileProjectItem(project, realurl);
	item->setOpenState(m_ki->isOpen(realurl));
	projectOpenItem(item);
	emit addToProjectView(item);
	buildProjectTree(project);
}

void Manager::removeFromProject(const KileProjectItem *item)
{
	if (item && item->project())
	{
		kdDebug() << "\tprojecturl = " << item->project()->url().path() << ", url = " << item->url().path() << endl;

		if (item->project()->url() == item->url())
		{
			KMessageBox::error(m_ki->parentWidget(), i18n("This file is the project file, it holds all the information about your project. Therefore it is not allowed to remove this file from its project."), i18n("Cannot Remove File From Project"));
			return;
		}

		emit removeItemFromProjectView(item, m_ki->isOpen(item->url()));

		KileProject *project = item->project();
		item->project()->remove(item);

		// update undefined references in all project files
		updateProjectReferences(project);
		project->buildProjectTree();
	}
}

void Manager::projectOpenItem(KileProjectItem *item, bool openProjectItemViews)
{
	kdDebug() << "==Kile::projectOpenItem==========================" << endl;
	kdDebug() << "\titem:" << item->url().path() << endl;

	if (m_ki->isOpen(item->url())) //remove item from projectview (this file was opened before as a normal file)
		emit removeFromProjectView(item->url());

	Kate::View *view = loadItem(m_ki->extensions()->determineDocumentType(item->url()), item, QString::null, openProjectItemViews);

	if ( (!item->isOpen()) && (view == 0L) && (item->getInfo()) ) //doc shouldn't be displayed, trash the doc
		trashDoc(item->getInfo());
	else if (view != 0L)
		view->setCursorPosition(item->lineNumber(), item->columnNumber());

	//oops, doc apparently was open while the project settings wants it closed, don't trash it the doc, update openstate instead
	if ((!item->isOpen()) && (view != 0L))
		item->setOpenState(true);
}

KileProject* Manager::projectOpen(const KURL & url, int step, int max, bool openProjectItemViews)
{
	kdDebug() << "==Kile::projectOpen==========================" << endl;
	kdDebug() << "\tfilename: " << url.fileName() << endl;

	const KURL realurl = symlinkFreeURL(url);

	if (m_ki->projectIsOpen(realurl))
	{
		m_kpd->cancel();

		KMessageBox::information(m_ki->parentWidget(), i18n("The project you tried to open is already opened. If you wanted to reload the project, close the project before you re-open it."),i18n("Project Already Open"));
		return 0L;
	}

	QFileInfo fi(realurl.path());
	if ( ! fi.isReadable() )
	{
		m_kpd->cancel();

		if (KMessageBox::warningYesNo(m_ki->parentWidget(), i18n("The project file for this project does not exists or is not readable. Remove this project from the recent projects list?"),i18n("Could Not Load Project File"))  == KMessageBox::Yes)
			emit(removeFromRecentProjects(realurl));

		return 0L;
	}

	m_kpd->show();

	KileProject *kp = new KileProject(realurl,m_ki->extensions());
	
	if(kp->isInvalid())
	{
		m_kpd->cancel();
		delete kp;
		return 0L;
	}

	emit(addToRecentProjects(realurl));

	KileProjectItemList *list = kp->items();

	int project_steps = list->count() + 1;
	m_kpd->progressBar()->setTotalSteps(project_steps * max);
	project_steps *= step;
	m_kpd->progressBar()->setValue(project_steps);

	// open the project files in the correct order
	QValueVector<KileProjectItem*> givenPositionVector(list->count(), NULL);
	QValueList<KileProjectItem*> notCorrectlyOrderedList;
	for(KileProjectItem *item = list->first(); item; item = list->next())
	{
		int order = item->order();
		if(order >= 0 && static_cast<unsigned int>(order) >= list->count())
		{
			order = -1;
		}
		if(!item->isOpen() || order < 0 || givenPositionVector[order] != NULL)
		{
			notCorrectlyOrderedList.push_back(item);
		}
		else
		{
			givenPositionVector[order] = item;
		}
	}

	QValueList<KileProjectItem*> orderedList;
	for(unsigned int i = 0; i < givenPositionVector.size(); ++i)
	{
		KileProjectItem *item = givenPositionVector[i];
		if(item)
		{
			orderedList.push_back(item);
		}
	}
	for(QValueList<KileProjectItem*>::iterator i = notCorrectlyOrderedList.begin(); i != notCorrectlyOrderedList.end(); ++i)
	{
		orderedList.push_back(*i);
	}

	unsigned int counter = 0;
	for (QValueList<KileProjectItem*>::iterator i = orderedList.begin(); i != orderedList.end(); ++i)
	{
		projectOpenItem(*i, openProjectItemViews);
		m_kpd->progressBar()->setValue(counter + project_steps);
		kapp->processEvents();
		++counter;
	}

	kp->buildProjectTree();
	addProject(kp);

	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	// update undefined references in all project files
	updateProjectReferences(kp);

	if (step == (max - 1))
		m_kpd->cancel();

   	m_ki->viewManager()->switchToTextView(kp->lastDocument());

	return kp;
}

// as all labels are gathered in the project, we can check for unsolved references
void Manager::updateProjectReferences(KileProject *project)
{
	KileProjectItemList *list = project->items();
	for ( uint i=0; i < list->count(); ++i)
	{
		emit(updateReferences(list->at(i)->getInfo()));
	}
}

KileProject* Manager::projectOpen()
{
	kdDebug() << "==Kile::projectOpen==========================" << endl;
	KURL url = KFileDialog::getOpenURL( KileConfig::defaultProjectLocation(), i18n("*.kilepr|Kile Project Files\n*|All Files"), m_ki->parentWidget(), i18n("Open Project") );

	if (!url.isEmpty())
		return projectOpen(url);
	else
		return 0L;
}


void Manager::projectSave(KileProject *project /* = 0 */)
{
	kdDebug() << "==Kile::projectSave==========================" << endl;
	if (project == 0)
	{
		//find the project that corresponds to the active doc
		project= activeProject();
	}

	if (project == 0 )
		project = selectProject(i18n("Save Project"));

	if (project)
	{
		KileProjectItemList *list = project->items();
		Kate::Document *doc = NULL;
		KileProjectItem *item = NULL;
		TextInfo *docinfo = NULL;

		// determine the order in which the project items are opened
		QValueVector<KileProjectItem*> viewPositionVector(m_ki->viewManager()->getTabCount(), NULL);
		for (KileProjectItemList::iterator i = list->begin(); i != list->end(); ++i)
		{
			docinfo = (*i)->getInfo();
			if(docinfo)
			{
				Kate::View *view = m_ki->viewManager()->textView(docinfo);
				if(view)
				{
					int position = m_ki->viewManager()->getIndexOf(view);
					if(position >= 0 && static_cast<unsigned int>(position) < viewPositionVector.size())
					{
						viewPositionVector[position] = *i;
					}
				}
			}
		}
		int position = 0;
		for(unsigned int i = 0; i < viewPositionVector.size(); ++i)
		{
			if(viewPositionVector[i] != NULL)
			{
				viewPositionVector[i]->setOrder(position);
				++position;
			}
		}

		//update the open-state of the items
		for (KileProjectItemList::iterator i = list->begin(); i != list->end(); ++i)
		{
			item = *i;
			kdDebug() << "\tsetOpenState(" << (*i)->url().path() << ") to " << m_ki->isOpen(item->url()) << endl;
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

void Manager::projectAddFiles(const KURL & url)
{
	KileProject *project = projectFor(url);

	if (project)
		projectAddFiles(project,url);
}

void Manager::projectAddFiles(KileProject *project,const KURL & fileUrl)
{
	kdDebug() << "==Kile::projectAddFiles()==========================" << endl;
 	if (project == 0 )
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Add Files to Project"));

	if (project)
	{
		QString currentDir;
		if(fileUrl.isEmpty())
			currentDir=project->url().directory();
		else
			currentDir=fileUrl.directory();

		kdDebug() << "currentDir is " << currentDir << endl;
		KFileDialog *dlg = new KFileDialog(currentDir, i18n("*|All Files"),m_ki->parentWidget(), 0, true );
		KPushButton* okButton = dlg->okButton();
		okButton->setText(i18n("Add"));
		dlg->setCaption(i18n("Add Files"));
		KFile::Mode mode = static_cast<KFile::Mode>( KFile::Files | KFile::ExistingOnly);
		dlg->setMode(mode);

		if(dlg->exec())
		{
			KURL::List urls = dlg->selectedURLs();
			for (uint i=0; i < urls.count(); ++i)
			{
				addToProject(project, urls[i]);
			}
			// update undefined references in all project files
			updateProjectReferences(project);
		}
		delete dlg;

		//open them
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->parentWidget(), i18n("There are no projects opened. Please open the project you want to add files to, then choose Add Files again."),i18n( "Could Not Determine Active Project"));
}

void Manager::toggleArchive(KileProjectItem *item)
{
	item->setArchive(!item->archive());
}

void Manager::projectOptions(const KURL & url)
{
	KileProject *project = projectFor(url);

	if (project)
		projectOptions(project);
}

void Manager::projectOptions(KileProject *project /* = 0*/)
{
	kdDebug() << "==Kile::projectOptions==========================" << endl;
	if (project ==0 )
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Project Options For"));

	if (project)
	{
		kdDebug() << "\t" << project->name() << endl;
		KileProjectOptionsDlg *dlg = new KileProjectOptionsDlg(project, m_ki->extensions(), m_ki->parentWidget());
		dlg->exec();
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->parentWidget(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to modify, then choose Project Options again."),i18n( "Could Not Determine Active Project"));
}

bool Manager::projectCloseAll()
{
	kdDebug() << "==Kile::projectCloseAll==========================" << endl;
	bool close = true;

	QPtrListIterator<KileProject> it(m_projects);
	int i = m_projects.count() + 1;
	while ( it.current() && close && (i > 0))
	{
		close = close && projectClose(it.current()->url());
		--i;
	}

	return close;
}

bool Manager::projectClose(const KURL & url)
{
	kdDebug() << "==Kile::projectClose==========================" << endl;
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

 	if (project)
	{
		kdDebug() << "\tclosing:" << project->name() << endl;
        	project->setLastDocument(m_ki->getName());

		projectSave(project);

		KileProjectItemList *list = project->items();

		bool close = true;
		Kate::Document *doc = 0L;
		TextInfo *docinfo = 0L;
		for (uint i =0; i < list->count(); ++i)
		{	
			doc = 0L;
			docinfo = list->at(i)->getInfo();
			if (docinfo)
			{
				doc = docinfo->getDoc();
			}
			else
			{
				continue;
			}
			if (doc)
			{
				kdDebug() << "\t\tclosing item " << doc->url().path() << endl;
				bool r = fileClose(doc, true);
				close = close && r;
				if (!close)
					break;
			}
			else 
			{
				// we still need to delete the TextInfo object
				removeTextDocumentInfo(docinfo, true);
			}
		}

		if (close)
		{
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

void Manager::storeProjectItem(KileProjectItem *item, Kate::Document *doc)
{
	kdDebug() << "===Kile::storeProjectItem==============" << endl;
	kdDebug() << "\titem = " << item << ", doc = " << doc << endl;
	item->setEncoding(doc->encoding());
	item->setHighlight(doc->hlModeName(doc->hlMode()));
	Kate::View *view = static_cast<Kate::View*>(doc->views().first());
	uint l = 0, c = 0;
	if (view) view->cursorPosition(&l,&c);
	item->setLineNumber(l);
	item->setColumnNumber(c);

	kdDebug() << "\t" << item->encoding() << " " << item->highlight() << " should be " << doc->hlModeName(doc->hlMode()) << endl;
}

void Manager::cleanUpTempFiles(const KURL &url, bool silent)
{
	kdDebug() << "===void Manager::cleanUpTempFiles(const KURL " << url.path() << ", bool " << silent << ")===" << endl;
	
	if( url.isEmpty() )
		return;
	
	QStringList extlist;
	QFileInfo fi(url.path());
	const QStringList templist = QStringList::split(" ", KileConfig::cleanUpFileExtensions());
	const QString fileName = fi.fileName();
	const QString dirPath = fi.dirPath(true);
	const QString baseName = fi.baseName(true);
	
	for (uint i=0; i <  templist.count(); ++i)
	{
		fi.setFile( dirPath + '/' + baseName + templist[i] );
		if ( fi.exists() )
			extlist.append(templist[i]);
	}
	
	if (!silent &&  ( KileUntitled::isUntitled(fileName) || fileName.isEmpty() ) )
		return;

	if (!silent && extlist.count() > 0 )
	{
		kdDebug() << "not silent" << endl;
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
		for ( uint i = 0 ; i < extlist.count() ; ++i )
		{
			QFile file( dirPath + '/' + baseName + extlist[i] );
			kdDebug() << "About to remove file = " << file.name() << endl;
			file.remove();
		}
		emit printMsg(KileTool::Info, i18n("Cleaning %1 : %2").arg(fileName).arg(extlist.join(" ")), i18n("Clean"));
	}
}

void Manager::openDroppedURLs(QDropEvent *e) {
	KURL::List urls;
	Extensions *extensions = m_ki->extensions();
	if(KURLDrag::decode(e, urls)) {
		for(KURL::List::iterator i = urls.begin(); i != urls.end(); ++i) {
			KURL url = *i;
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
	const KURL lastdoc = project->lastDocument();
	KileProjectItem *docitem = ( !lastdoc.isEmpty() ) ? itemFor(lastdoc,project) : 0L;

	// if not, we search for the first opened tex file of this project
	// if no file is opened, we take the first tex file mentioned in the list
	KileProjectItem *first_texitem = 0L;
	if ( ! docitem ) 
	{
		KileProjectItemList *list = project->items();
		for ( KileProjectItem *item=list->first(); item; item=list->next() ) 
		{
			QString itempath = item->path();

			// called from KAction 'Show projects...': find the first opened 
			// LaTeX document or, if that fails, any other opened file
			QStringList extlist = QStringList::split(" ",m_ki->extensions()->latexDocuments() + ' ' + m_ki->extensions()->latexPackages());
			for ( QStringList::Iterator it=extlist.begin(); it!=extlist.end(); ++it )
			{
				if ( itempath.find( (*it), -(*it).length() ) >= 0 ) 
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
	KileProjectItemList* items = selectProjectFileItems( i18n("Select Files to Remove") );
	if ( items && items->count() > 0 )
		for ( KileProjectItemList::Iterator it = items->begin(); it != items->end(); ++it )
			removeFromProject(*it);
	delete items;
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

void Manager::projectOpenAllFiles(const KURL & url)
{
	KileProject* project;
	Kate::Document* doc = 0L;

	if(!url.isValid())
		return;
	project = projectFor(url);

	if(!project)
		return;


	if(m_ki->viewManager()->currentTextView())
		doc = m_ki->viewManager()->currentTextView()->getDoc();
	// we remember the actual view, so the user gets the same view back after opening

	KileProjectItemList *list = project->items();
	for ( KileProjectItem *item=list->first(); item; item = list->next() )
	{
		if  ( item->type()==KileProjectItem::ProjectFile )
			dontOpenWarning( item, i18n("Open All Project Files"), i18n("project configuration file") );
		else if  ( item->type()==KileProjectItem::Image )
			dontOpenWarning( item, i18n("Open All Project Files"), i18n("graphics file") );
		else if ( ! m_ki->isOpen(item->url()) )
			fileOpen( item->url(),item->encoding() );
	}

	if(doc) // we have a doc so switch back to original view
		m_ki->viewManager()->switchToTextView(doc->url());
}

QStringList Manager::getProjectFiles()
{
	QStringList filelist;

	KileProject *project = activeProject();
	if ( project )
	{
		KileProjectItemList *list = project->items();
		for ( KileProjectItem *item=list->first(); item; item = list->next() )
		{
			if  ( item->type()!=KileProjectItem::ProjectFile && item->type()!=KileProjectItem::Image )
				filelist <<  item->url().path();
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
	QMap<QString,KileProjectItem *> map;
	KileProjectItemList *list = project->items();
	for ( KileProjectItem *item=list->first(); item; item = list->next() ) {
		filelist << item->path();
		map[item->path()] = item;
	}

	// select one of these files
	KileProjectItem *item = 0L;
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

KileProjectItemList* Manager::selectProjectFileItems(const QString &label)
{
	KileProject *project = selectProject(i18n("Select Project"));
	if ( ! project )
		return 0L;

	QStringList filelist, selectedfiles;
	QMap<QString,KileProjectItem *> map;

	KileProjectItemList *list = project->items();
	for ( KileProjectItem *item=list->first(); item; item = list->next() ) {
		filelist << item->path();
		map[item->path()] = item;
	}

	KileProjectItemList *items = new KileProjectItemList();
	items->setAutoDelete(false);

	KileListSelectorMultiple *dlg  = new KileListSelectorMultiple(filelist,i18n("Project Files"),label, m_ki->parentWidget());
	if ( dlg->exec() ) {
		if ( dlg->currentItem() >= 0 ) {
			selectedfiles = dlg->selected();
			for ( QStringList::Iterator it = selectedfiles.begin(); it != selectedfiles.end(); ++it ){
				if ( map.contains(*it) )
					items->append( map[(*it)] );
				else
					KMessageBox::error(m_ki->parentWidget(), i18n("Could not determine the selected file."),i18n( "Project Error"));
			}
		}
	}
	delete dlg;

	return items;
}

// add a new file to the project
//  - only when there is an active project
//  - if the file doesn't already belong to it (checked by addToProject)

void Manager::projectAddFile(QString filename, bool graphics)
{
 	kdDebug() << "===Kile::projectAddFile==============" << endl;
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
		QString ext = fi.extension();
		if ( ! ext.isEmpty() )
			return;

		filename += m_ki->extensions()->latexDocumentDefault();
		if ( QFileInfo(filename).exists() )
			return;
	}

	//ok, we have a project and an existing file
	kdDebug() << "\tadd file: " << filename << endl;
	m_ki->viewManager()->updateStructure(true);

	KURL url;
	url.setPath(filename);
	addToProject(project, url);
}

const KURL Manager::symlinkFreeURL(const KURL& url)
{
	kdDebug() << "===symlinkFreeURL==" << endl;

	if( !url.isLocalFile() )
		return url;

	QDir dir(url.directory());
	QString filename=url.path(); // if the directory does not exist we return the old url (just to be sure)

	if(dir.exists())
		filename= dir.canonicalPath() + '/' + url.filename();
	else
		kdDebug() << "directory " << url.directory() << "does not exist" << endl;

	return KURL::fromPathOrURL(filename);
}

void Manager::cleanupDocumentInfoForProjectItems(KileDocument::Info *info)
{
	KileProjectItemList *itms = itemsFor(info);
	QPtrListIterator<KileProjectItem> it(*itms);
	KileProjectItem *current;
	while((current = it.current()) != 0)
	{
		++it;
		current->setInfo(0);
	}
	delete itms;
}

}

#include "kiledocmanager.moc"
