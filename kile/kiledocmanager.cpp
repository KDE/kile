//
// C++ Implementation: kiledocmanager
//
// Description:
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtextcodec.h>
#include <qfile.h>

#include <kate/document.h>
#include <kate/view.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kfile.h>
#include <kencodingfiledialog.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kpushbutton.h>

#include "kileuntitled.h"
#include "templates.h"
#include "newfilewizard.h"
#include "managetemplatesdialog.h"
#include "kileinfo.h"
#include "kileproject.h"
#include "kiledocumentinfo.h"
#include "kiledocmanager.h"
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

namespace KileDocument
{

Manager::Manager(KileInfo *info, QObject *parent, const char *name) :
	QObject(parent, name),
	m_ki(info)
{
	m_infoList.setAutoDelete(false);
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

void Manager::trashDoc(Info *docinfo, Kate::Document *doc /*= 0L*/ )
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

	for ( uint i = 0; i < m_infoList.count(); ++i )
	{
		if ( (m_infoList.at(i) != docinfo) && (m_infoList.at(i)->getDoc() == doc) )
		{
			KMessageBox::information(0, i18n("The internal structure of Kile is corrupted (probably due to a bug in Kile). Please select Save All from the File menu and close Kile.\nThe Kile team apologizes for any inconvenience and would appreciate a bug report."));
			kdWarning() << "docinfo " << m_infoList.at(i) << " url " << m_infoList.at(i)->url().fileName() << " has a wild pointer!!!"<< endl;
		}
	}

	kdDebug() << "DELETING doc" << endl;
	delete doc;
}

// update all Info's with changed user commands
void Manager::updateInfos()
{
	for (uint i=0; i < m_infoList.count(); ++i)
	{
		m_infoList.at(i)->updateStructLevelInfo();
	}
}

Kate::Document* Manager::docFor(const KURL & url)
{
	for (uint i=0; i < m_infoList.count(); ++i)
	{
		if ( m_ki->similarOrEqualURL(m_infoList.at(i)->url(),url) )
        {
			return m_infoList.at(i)->getDoc();
        }
	}

	return 0L;
}

Info* Manager::getInfo() const
{
	Kate::Document *doc = m_ki->activeDocument();
	if ( doc != 0L )
		return infoFor(doc);
	else
		return 0L;
}

Info *Manager::infoFor(const QString & path) const
{
	kdDebug() << "==KileInfo::infoFor(" << path << ")==========================" << endl;
	QPtrListIterator<Info> it(m_infoList);
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

Info* Manager::infoFor(Kate::Document* doc) const
{
    QPtrListIterator<Info> it(m_infoList);
    while ( true )
    {
        if ( it.current()->getDoc() == doc)
            return it.current();

        if (it.atLast()) return 0L;

        ++it;
    }

    return 0;
}

void Manager::mapItem(Info *docinfo, KileProjectItem *item)
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
	return itemFor(docinfo->url(), project);
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
		if ((*it)->contains(docinfo->url()))
		{
			kdDebug() << "\t\tcontains" << endl;
			list->append((*it)->item(docinfo->url()));
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
	Kate::Document *doc = m_ki->activeDocument();

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
	Kate::Document *doc = m_ki->activeDocument();
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

Info* Manager::createDocumentInfo(const KURL & url)
{
	Info *docinfo = 0L;

	//see if this file belongs to an opened project
	KileProjectItem *item = itemFor(url);
	if ( item != 0L ) docinfo = item->getInfo();

	if ( docinfo == 0L )
	{
		if ( Info::isTeXFile(url) || url.isEmpty() )
		{
			kdDebug() << "CREATING TeXInfo for " << url.url() << endl;
			docinfo = new TeXInfo(0L,m_ki->latexCommands());
		}
		else if ( Info::isBibFile(url) )
		{
			kdDebug() << "CREATING BibInfo for " << url.url() << endl;
			docinfo = new BibInfo(0L,m_ki->latexCommands());
		}
		else
		{
			kdDebug() << "CREATING Info for " << url.url() << endl;
			docinfo = new Info(0L,m_ki->latexCommands());
		}
		docinfo->setURL(url);
		emit(documentInfoCreated(docinfo));
		m_infoList.append(docinfo);
	}

	kdDebug() << "DOCINFO: returning " << docinfo << " " << docinfo->url().fileName() << endl;
	return docinfo;
}

Info* Manager::recreateDocInfo(Info *oldinfo, const KURL & url)
{
	KileProjectItemList *list = itemsFor(oldinfo);
	Info *newinfo = createDocumentInfo(url);

    newinfo->setDoc(oldinfo->getDoc());

	KileProjectItem *pritem = 0L;
	for ( pritem = list->first(); pritem; pritem = list->next() )
		pritem->setInfo(newinfo);

	m_infoList.removeRef(oldinfo);
	delete oldinfo;

	m_infoList.append(newinfo);

    // parse the document
    newinfo->updateStruct();

	return newinfo;
}

bool Manager::removeDocumentInfo(Info *docinfo, bool closingproject /* = false */)
{
	kdDebug() << "==Manager::removeDocumentInfo(Info *docinfo)=====" << endl;
	KileProjectItemList *itms = itemsFor(docinfo);

	if ( itms->count() == 0 || (closingproject && itms->count() == 1))
	{
		kdDebug() << "\tremoving " << docinfo <<  " count = " << m_infoList.count() << endl;
		m_infoList.remove(docinfo);

		emit ( closingDocument ( docinfo ) );

		delete docinfo;
		delete itms;

		return true;
	}

	kdDebug() << "\tnot removing " << docinfo << endl;
	delete itms;
	return false;
}

Kate::Document* Manager::createDocument(Info *docinfo, const QString & encoding, const QString & highlight)
{
	kdDebug() << "==Kate::Document* Manager::createDocument()===========" << endl;

	Kate::Document *doc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");
	if ( docFor(docinfo->url()) != 0L )
		kdWarning() << docinfo->url().path() << " already has a document!" << endl;
	else
		kdDebug() << "\tappending document " <<  doc << endl;

	//set the default encoding
	QString enc = encoding.isNull() ? KileConfig::defaultEncoding() : encoding;
	doc->setEncoding(enc);

    kdDebug() << "url is = " << docinfo->url().url() << endl;

    doc->openURL(docinfo->url());

    if ( KileUntitled::isUntitled(docinfo->url().url()) )
    {
        doc->setDocName(docinfo->url().url());
        kdDebug() << "docName = " << doc->docName() << endl;
    }
    else
    {
        doc->setDocName(docinfo->url().path());
        emit(addToRecentFiles(docinfo->url()));
    }

	setHighlightMode(doc, highlight);

	//handle changes of the document
	connect(doc, SIGNAL(nameChanged(Kate::Document *)), docinfo, SLOT(emitNameChanged(Kate::Document *)));
	//why not connect doc->nameChanged directly to slotNameChanged ? : the function emitNameChanged
	//updates the docinfo, on which all decisions are bases in slotNameChanged
	connect(docinfo,SIGNAL(nameChanged(Kate::Document*)), this, SLOT(slotNameChanged(Kate::Document*)));
	connect(docinfo, SIGNAL(nameChanged(Kate::Document *)), m_ki->parentWidget(), SLOT(newCaption()));
	connect(doc, SIGNAL(modStateChanged(Kate::Document*)), this, SLOT(newDocumentStatus(Kate::Document*)));
	connect(doc, SIGNAL(modifiedOnDisc(Kate::Document*, bool, unsigned char)), this, SIGNAL(documentStatusChanged(Kate::Document*, bool, unsigned char)));

	docinfo->setDoc(doc);

	kdDebug() << "createDocument: SANITY check: " << (docinfo->getDoc() == docFor(docinfo->url())) << endl;
	return doc;
}

void Manager::setHighlightMode(Kate::Document * doc, const QString &highlight)
{
	kdDebug() << "==Kile::setHighlightMode(" << doc->docName() << "," << highlight << " )==================" << endl;

	int c = doc->hlModeCount();
	int nHlLaTeX = 0;
	//determine default highlighting mode (LaTeX)
	for (int i = 0; i < c; ++i)
	{
		if (doc->hlModeName(i) == "LaTeX") { nHlLaTeX = i; break; }
	}

	//don't let KatePart determine the highlighting
	if ( !highlight.isNull() )
	{
		bool found = false;
		int mode = 0;
		for (int i = 0; i < c; ++i)
		{
			if (doc->hlModeName(i) == highlight) { found = true; mode = i; }
		}
		if (found) doc->setHlMode(mode);
		else doc->setHlMode(nHlLaTeX);
	}
	else if ( doc->url().isEmpty() || KileUntitled::isUntitled(doc->docName()) )
	{
		doc->setHlMode(nHlLaTeX);
	}
}

Kate::View* Manager::loadItem(KileProjectItem *item, const QString & text)
{
	Kate::View *view = 0L;

	kdDebug() << "==loadItem(" << item->url().path() << ")======" << endl;

	if ( item->type() != KileProjectItem::Image )
	{
		view = load(item->url(), item->encoding(), item->isOpen(), item->highlight(), text);
		kdDebug() << "\tloadItem: docfor = " << docFor(item->url().path()) << endl;

		Info *docinfo = infoFor(item->url().path());
		item->setInfo(docinfo);

		kdDebug() << "\tloadItem: docinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url().path()) << endl;
		if ( docinfo->getDoc() != docFor(docinfo->url().path()) ) kdWarning() << "docinfo->getDoc() != docFor()" << endl;
	}
	else
	{
		kdDebug() << "\tloadItem: no document generated" << endl;
		Info *docinfo = createDocumentInfo(item->url());
		item->setInfo(docinfo);

		if ( docFor(item->url()) == 0L)
		{
			docinfo->detach();
			kdDebug() << "\t\t\tdetached" << endl;
		}
	}

	return view;
}

Kate::View* Manager::load(const KURL &url , const QString & encoding /* = QString::null */, bool create /* = true */, const QString & highlight /* = QString::null */, const QString & text /* = QString::null */)
{
	kdDebug() << "==load(" << url.url() << ")=================" << endl;
	//if doc already opened, update the structure view and return the view
	if ( m_ki->isOpen(url))
		return m_ki->viewManager()->switchToView(url);

	Info *docinfo = createDocumentInfo(url);
	Kate::Document *doc = createDocument(docinfo, encoding, highlight);

	m_ki->structureWidget()->clean(docinfo);
	docinfo->updateStruct();

	if ( !text.isNull() ) doc->setText(text);

	//FIXME: use signal/slot
	if (doc && create)
		return m_ki->viewManager()->createView(doc);

	kdDebug() << "just after createView()" << endl;
	kdDebug() << "\tdocinfo = " << docinfo << " doc = " << docinfo->getDoc() << " docfor = " << docFor(docinfo->url().path()) << endl;

	return 0L;
}

//FIXME: template stuff should be in own class
Kate::View* Manager::loadTemplate(TemplateItem *sel)
{
	QString text = QString::null;

	if (sel && sel->name() != DEFAULT_EMPTY_CAPTION)
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

	return createDocumentWithText(text);
}

Kate::View* Manager::createDocumentWithText(const QString & text)
{
	Kate::View *view = load(KURL::fromPathOrURL(KileUntitled::next()), QString::null, true, QString::null, text);
	if (view)
	{
		//FIXME this shouldn't be necessary!!!
		view->getDoc()->setModified(true);
		newDocumentStatus(view->getDoc());
	}

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
	if (m_ki->viewManager()->currentView())
	{
		if (m_ki->viewManager()->currentView()->getDoc()->isModified() )
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

	QFileInfo fi(m_ki->viewManager()->currentView()->getDoc()->url().path());
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

	QString filter;
	filter.append(SOURCE_EXTENSIONS);
	filter.append(" ");
	filter.append(PACKAGE_EXTENSIONS);
	filter.replace(".","*.");
	filter.append("|");
	filter.append(i18n("TeX Files"));
	filter.append("\n*|");
	filter.append(i18n("All Files"));

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

void Manager::slotNameChanged(Kate::Document * doc)
{
	kdDebug() << "==Kile::slotNameChanged==========================" << endl;

	KURL validURL = Info::makeValidTeXURL(doc->url());
	if(validURL != doc->url())
	{
		QFile oldFile(doc->url().path());
		if(doc->saveAs(validURL))
			oldFile.remove();

		m_ki->viewManager()->projectView()->add(doc->url());
	}

	Info *docinfo = infoFor(doc);

	// take special care if doc was Untitled before
	if (docinfo->oldURL().isEmpty())
	{
		kdDebug() << "\tadding URL to projectview " << doc->url().path() << endl;
		m_ki->viewManager()->projectView()->add(doc->url());

        recreateDocInfo(docinfo, doc->url());
	}

    emit(documentStatusChanged(doc, doc->isModified(), 0));
}

void Manager::newDocumentStatus(Kate::Document *doc)
{
	if (doc == 0L) return;

	//sync terminal
	m_ki->texKonsole()->sync();

	emit(documentStatusChanged(doc,  doc->isModified(), 0));

	//updatestructure if active document changed from modified to unmodified (typically after a save)
	if (!doc->isModified())
		emit(updateStructure(true, infoFor(doc)));
}

void Manager::fileSaveAll(bool amAutoSaving, bool disUntitled )
{
	Kate::View *view;
	QFileInfo fi;

	kdDebug() << "==Kile::fileSaveAll=================" << endl;
	kdDebug() << "\tautosaving = " << amAutoSaving << ",  DisRegardUntitled = " <<  disUntitled << endl;

	for (uint i = 0; i < m_ki->viewManager()->views().count(); ++i)
	{
		view = m_ki->viewManager()->view(i);

		if (view && (view->getDoc()->isModified() || amAutoSaving) )
		{
			fi.setFile(view->getDoc()->url().path());

			if ( (!amAutoSaving && !disUntitled)	// both false, so we want to save everything
				 || ( !amAutoSaving && !(disUntitled && view->getDoc()->url().isEmpty() ) ) // DisregardUntitled is true and we have an untitled doc and don't autosave
			     || ( amAutoSaving && (!view->getDoc()->url().isEmpty() ) && fi.isWritable() ) //don't save unwritable and untitled documents when autosaving
			   )
			{
				kdDebug() << "\tsaving: " << view->getDoc()->url().path() << endl;

				if (amAutoSaving)
				{
					//make a backup
					KURL url = view->getDoc()->url();
          KIO::NetAccess::file_copy (url, KURL(KURL::fromPathOrURL(url.url()+".backup")), -1, true, false, kapp->mainWidget());
				}

				view->save();
			}
		}
	}
}

void Manager::fileOpen(const KURL & url, const QString & encoding)
{
	//don't want to receive signals from the fileselector since
	//that would allow the user to open a single file twice by double-clicking on it
	m_ki->fileSelector()->blockSignals(true);

	kdDebug() << "==Kile::fileOpen==========================" << endl;
	kdDebug() << "\t" << url.url() << endl;
	bool isopen = m_ki->isOpen(url);

	load(url, encoding);

	//URL wasn't open before loading, add it to the project view
	//FIXME: use signal/slot
	if (!isopen && (itemFor(url) == 0) ) m_ki->viewManager()->projectView()->add(url);

	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	// update undefined references in this file
	emit(updateReferences(infoFor(url.path())) );
	m_ki->fileSelector()->blockSignals(false);
}

bool Manager::fileCloseAllOthers()
{
	Kate::View * currentview = m_ki->viewManager()->currentView();
	while (  m_ki->viewManager()->views().count() > 1 )
	{
		Kate::View *view =  m_ki->viewManager()->views().first();
		if ( view == currentview )
			view =  m_ki->viewManager()->views().next();
		if ( ! fileClose(view->getDoc()) )
			return false;
	}

	return true;
}

bool Manager::fileCloseAll()
{
	Kate::View * view = m_ki->viewManager()->currentView();

	//assumes one view per doc here
	while( ! m_ki->viewManager()->views().isEmpty() )
	{
		view = m_ki->viewManager()->view(0);
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
		doc = m_ki->activeDocument();

	if (doc == 0L)
		return true;
	else
	//FIXME: remove from docinfo map, remove from dirwatch
	{
		kdDebug() << "\t" << doc->url().path() << endl;
		QString fn = doc->url().fileName();

		KURL url = doc->url();

		Info *docinfo= infoFor(doc);
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
			if ( KileConfig::cleanUpAfterClose() ) cleanUpTempFiles(docinfo, true);

			//FIXME: use signal/slot
			m_ki->viewManager()->removeView(static_cast<Kate::View*>(doc->views().first()));
			//remove the decorations

			trashDoc(docinfo, doc);
            m_ki->structureWidget()->clean(docinfo);
			removeDocumentInfo(docinfo, closingproject);

			//FIXME:remove entry in projectview
			m_ki->viewManager()->removeFromProjectView(url);

            if ( m_ki->docManager()->documentInfos()->count() == 0 ) emit updateModeStatus();
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
	KileNewProjectDlg *dlg = new KileNewProjectDlg(m_ki->parentWidget());
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

			Info *docinfo = infoFor(view->getDoc());
			docinfo->setURL(url);

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
	}
}

void Manager::addProject(const KileProject *project)
{
	kdDebug() << "==void Manager::addProject(const KileProject *project)==========" << endl;
	m_projects.append(project);
	kdDebug() << "\tnow " << m_projects.count() << " projects" << endl;
	m_ki->viewManager()->projectView()->add(project);
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
	kdDebug() << "==Kile::addToProject==========================" << endl;
	kdDebug() << "\t" <<  url.fileName() << endl;

	KileProject *project = selectProject(i18n("Add to Project"));

	if (project) addToProject(project, url);
}

void Manager::addToProject(KileProject* project, const KURL & url)
{
	if (project->contains(url)) return;

	KileProjectItem *item = new KileProjectItem(project, url);
	item->setOpenState(m_ki->isOpen(url));
	projectOpenItem(item);
	m_ki->viewManager()->projectView()->add(item);
	buildProjectTree(project);
}

void Manager::removeFromProject(const KileProjectItem *item)
{
	if (item->project())
	{
		kdDebug() << "\tprojecturl = " << item->project()->url().path() << ", url = " << item->url().path() << endl;

		if (item->project()->url() == item->url())
		{
			KMessageBox::error(m_ki->parentWidget(), i18n("This file is the project file, it holds all the information about your project. Therefore it is not allowed to remove this file from its project."), i18n("Cannot Remove File From Project"));
			return;
		}

		m_ki->viewManager()->projectView()->removeItem(item, m_ki->isOpen(item->url()));

		KileProject *project = item->project();
		item->project()->remove(item);

		// update undefined references in all project files
		updateProjectReferences(project);
		project->buildProjectTree();
	}
}

void Manager::projectOpenItem(KileProjectItem *item)
{
	kdDebug() << "==Kile::projectOpenItem==========================" << endl;
	kdDebug() << "\titem:" << item->url().path() << endl;

	if (m_ki->isOpen(item->url())) //remove item from projectview (this file was opened before as a normal file)
		m_ki->viewManager()->projectView()->remove(item->url());

	Kate::View *view = loadItem(item);

	if ( (!item->isOpen()) && (view == 0L) && (item->getInfo()) ) //doc shouldn't be displayed, trash the doc
		trashDoc(item->getInfo());
	else if (view != 0L)
		view->setCursorPosition(item->lineNumber(), item->columnNumber());

	//oops, doc apparently was open while the project settings wants it closed, don't trash it the doc, update openstate instead
	if ((!item->isOpen()) && (view != 0L))
		item->setOpenState(true);
}

KileProject* Manager::projectOpen(const KURL & url, int step, int max)
{
	kdDebug() << "==Kile::projectOpen==========================" << endl;
	kdDebug() << "\tfilename: " << url.fileName() << endl;
	if (m_ki->projectIsOpen(url))
	{
		m_kpd->cancel();

		KMessageBox::information(m_ki->parentWidget(), i18n("The project you tried to open is already opened. If you wanted to reload the project, close the project before you re-open it."),i18n("Project Already Open"));
		return 0L;
	}

	QFileInfo fi(url.path());
	if ( ! fi.isReadable() )
	{
		m_kpd->cancel();

		if (KMessageBox::warningYesNo(m_ki->parentWidget(), i18n("The project file for this project does not exists or is not readable. Remove this project from the recent projects list?"),i18n("Could Not Load Project File"))  == KMessageBox::Yes)
			emit(removeFromRecentProjects(url));

		return 0L;
	}

	m_kpd->show();

	KileProject *kp = new KileProject(url);

	emit(addToRecentProjects(url));

	KileProjectItemList *list = kp->items();

	int project_steps = list->count() + 1;
	m_kpd->progressBar()->setTotalSteps(project_steps * max);
	project_steps *= step;
	m_kpd->progressBar()->setValue(project_steps);

	for ( uint i=0; i < list->count(); ++i)
	{
		projectOpenItem(list->at(i));
		m_kpd->progressBar()->setValue(i + project_steps);
		kapp->processEvents();
	}

	kp->buildProjectTree();
	addProject(kp);

	emit(updateStructure(false, 0L));
	emit(updateModeStatus());
	// update undefined references in all project files
	updateProjectReferences(kp);

	if (step == (max - 1))
		m_kpd->cancel();

    m_ki->viewManager()->switchToView(kp->lastDocument());

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
	KURL url = KFileDialog::getOpenURL( "", i18n("*.kilepr|Kile Project Files\n*|All Files"), m_ki->parentWidget(), i18n("Open Project") );

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
		Kate::Document *doc = 0L;

		KileProjectItem *item;
		Info *docinfo;
		//update the open-state of the items
		for (uint i=0; i < list->count(); ++i)
		{
			item = list->at(i);
			kdDebug() << "\tsetOpenState(" << item->url().path() << ") to " << m_ki->isOpen(item->url()) << endl;
			item->setOpenState(m_ki->isOpen(item->url()));
			docinfo = item->getInfo();

			kdDebug() << "test" << endl;
			kdDebug() << "\t\tDOCINFO = " << docinfo << ", DOC = " << docinfo->getDoc() << endl;
			if (docinfo != 0L) doc = docinfo->getDoc();
			if (doc != 0L) storeProjectItem(item, doc);

			doc = 0L;
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
		projectAddFiles(project);
}

void Manager::projectAddFiles(KileProject *project)
{
	kdDebug() << "==Kile::projectAddFiles()==========================" << endl;
 	if (project == 0 )
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Add Files to Project"));

	if (project)
	{
		//determine the starting dir for the file dialog
		QString currentDir=m_ki->fileSelector()->dirOperator()->url().path();
		QFileInfo fi;
		if (m_ki->viewManager()->currentView())
		{
			fi.setFile(m_ki->viewManager()->currentView()->getDoc()->url().path());
			if (fi.exists()) currentDir= fi.dirPath();
		}

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

bool Manager::projectArchive(const KURL & url)
{
	KileProject *project = projectFor(url);

	if (project)
		return projectArchive(project);
	else
		return false;
}

bool Manager::projectArchive(KileProject *project /* = 0*/)
{
	if (project == 0)
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Archive Project"));

	if (project)
	{
		//TODO: this should be in the KileProject class
		//QString command = project->archiveCommand();
		QString files, path;
		QPtrListIterator<KileProjectItem> it(*project->items());
		while (it.current())
		{
			if ((*it)->archive())
			{
				path = (*it)->path();
				KRun::shellQuote(path);
				files += path+" ";
			}
			++it;
		}

		KileTool::Base *tool = new KileTool::Base("Archive", m_ki->toolManager());
		tool->setSource(project->url().path());
		tool->addDict("%F", files);
		m_ki->toolManager()->run(tool);
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(m_ki->parentWidget(), i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to archive, then choose Archive again."),i18n( "Could Not Determine Active Project"));

	return true;
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
		KileProjectOptionsDlg *dlg = new KileProjectOptionsDlg(project, m_ki->parentWidget());
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

		//close the project file first, projectSave, changes this file
		Kate::Document *doc = docFor(project->url());
		if (doc)
		{
			doc->save();
			fileClose(doc);
		}

		projectSave(project);

		KileProjectItemList *list = project->items();

		bool close = true;
		Info *docinfo;
		for (uint i =0; i < list->count(); ++i)
		{
			docinfo = list->at(i)->getInfo();
			if (docinfo) doc = docinfo->getDoc();
			if (doc)
			{
				kdDebug() << "\t\tclosing item " << doc->url().path() << endl;
				bool r = fileClose(doc, true);
				close = close && r;
				if (!close) break;
			}
			else
				removeDocumentInfo(docinfo, true);

			docinfo = 0L;
			doc = 0L;
		}

		if (close)
		{
			m_projects.remove(project);
			//FIXME: use signal/slot
			m_ki->viewManager()->projectView()->remove(project);
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

void Manager::cleanUpTempFiles(Info *docinfo, bool silent)
{
	QStringList extlist;
	QStringList templist = QStringList::split(" ", KileConfig::cleanUpFileExtensions());
	QString str;
	QFileInfo file(docinfo->url().path()), fi;
	for (uint i=0; i <  templist.count(); ++i)
	{
		str = file.dirPath(true) + "/" + file.baseName(true) + templist[i];
		fi.setFile(str);
		if ( fi.exists() )
			extlist.append(templist[i]);
	}

	str = file.fileName();
	if (!silent &&  ( KileUntitled::isUntitled(str) || str.isEmpty() ) )	return;

	if (!silent && extlist.count() > 0 )
	{
		kdDebug() << "\tnot silent" << endl;
		KileDialog::Clean *dialog = new KileDialog::Clean(m_ki->parentWidget(), str, extlist);
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
	{
		m_ki->logWidget()->printMsg(KileTool::Warning, i18n("Nothing to clean for %1").arg(str), i18n("Clean"));
		return;
	}

	m_ki->logWidget()->printMsg(KileTool::Info, i18n("cleaning %1 : %2").arg(str).arg(extlist.join(" ")), i18n("Clean"));

	docinfo->cleanTempFiles(extlist);
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
	if ( ! docitem ) {
		KileProjectItemList *list = project->items();
		for ( KileProjectItem *item=list->first(); item; item=list->next() ) {
			if ( item->path().find(".tex",-4) >= 0 ) {
				if  ( m_ki->isOpen(item->url()) ) {
					docitem = item;
					break;
				} else if ( ! first_texitem )
					first_texitem = item;
			}
		}
	}

	// did we find one opened file or must we take the first entry
	if ( ! docitem ) {
		if ( ! first_texitem )
			return;
		docitem = first_texitem;
	}

	// ok, we can switch to another project now
	if  ( m_ki->isOpen(docitem->url()) )
		m_ki->viewManager()->switchToView(docitem->url());
	else
		fileOpen( docitem->url(),docitem->encoding() );
}

void Manager::projectRemoveFiles()
{
	KileProjectItem *item = selectProjectFileItem( i18n("Select File to Remove") );
	if ( item ) {
		removeFromProject(item);
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
				m_ki->viewManager()->switchToView(item->url());
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


	if(m_ki->viewManager()->currentView())
		doc = m_ki->viewManager()->currentView()->getDoc();
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
		m_ki->viewManager()->switchToView(doc->url());
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
	m_ki->logWidget()->printMsg(KileTool::Info, i18n("not opened: %1 (%2)").arg(item->url().path()).arg(filetype), action);
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

// add a new file to the project
//  - only when there is an active project
//  - if the file doesn't already belong to it (checked by addToProject)

void Manager::projectAddFile(QString filename, bool graphics)
{
 	kdDebug() << "===Kile::projectAddFile==============" << endl;
	KileProject *project = activeProject();
 	if ( ! project )
		return;

	if ( ! QFileInfo(filename).exists() )
	{
		if ( graphics )
			return;
		filename += ".tex";
		if ( ! QFileInfo(filename).exists() )
			return;
	}

	//ok, we have a project and an existing file
	kdDebug() << "\tadd file: " << filename << endl;
	m_ki->viewManager()->updateStructure(true);

	KURL url;
	url.setPath(filename);
	addToProject(project, url);
}

}

#include "kiledocmanager.moc"
