//
// C++ Implementation: kileviewmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//         Michel Ludwig <michel.ludwig@kdemail.net>, (C) 2006, 2007

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kileviewmanager.h"

#include <q3popupmenu.h>
#include <qtimer.h> //for QTimer::singleShot trick
#include <qpixmap.h>
#include <qclipboard.h>
//Added by qt3to4:
#include <QDragMoveEvent>
#include <QDropEvent>

#include <kdeversion.h>
#include <kglobal.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kparts/componentfactory.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <klocale.h>
#include <ktexteditor/editinterfaceext.h>
#include <kapplication.h>
#include <kurldrag.h>

#include "editorkeysequencemanager.h"
#include "kileinfo.h"
#include "kileconstants.h"
#include "kiledocmanager.h"
#include "kileextensions.h"
#include "kileprojectview.h"
#include "kileeventfilter.h"
#include "kilestructurewidget.h"
#include "kileedit.h"
#include "plaintolatexconverter.h"
#include "previewwidget.h"
#include "quickpreview.h"

namespace KileView 
{

Manager::Manager(KileInfo *info, QObject *parent, const char *name) :
	QObject(parent, name),
	m_ki(info),
	m_activeTextView(0L),
// 	m_projectview(0L),
	m_tabs(0L),
	m_widgetStack(0L),
	m_emptyDropWidget(0L)
{
}


Manager::~Manager()
{
}

void Manager::setClient(QObject *receiver, KXMLGUIClient *client)
{
	m_receiver = receiver;
	m_client = client;
	if(NULL == m_client->actionCollection()->action("popup_pasteaslatex"))
		new KAction(i18n("Paste as LaTe&X"), 0, this,
			SLOT(pasteAsLaTeX()), m_client->actionCollection(), "popup_pasteaslatex");
	if(NULL == m_client->actionCollection()->action("popup_converttolatex"))
		new KAction(i18n("Convert Selection to &LaTeX"), 0, this,
			SLOT(convertSelectionToLaTeX()), m_client->actionCollection(), "popup_converttolatex");
	if(NULL == m_client->actionCollection()->action("popup_quickpreview"))
		new KAction(i18n("&QuickPreview Selection"), 0, this,
			SLOT(quickPreviewPopup()), m_client->actionCollection(), "popup_quickpreview");
}

void Manager::createTabs(QWidget *parent)
{
	m_widgetStack = new Q3WidgetStack(parent);
	m_emptyDropWidget = new DropWidget(parent);
	m_widgetStack->addWidget(m_emptyDropWidget);
	connect(m_emptyDropWidget, SIGNAL(testCanDecode(const QDragMoveEvent *,  bool &)), this, SLOT(testCanDecodeURLs(const QDragMoveEvent *, bool &)));
	connect(m_emptyDropWidget, SIGNAL(receivedDropEvent(QDropEvent *)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent *)));
	m_tabs = new KTabWidget(parent);
	m_widgetStack->addWidget(m_tabs);
	m_tabs->setFocusPolicy(QWidget::ClickFocus);
	m_tabs->setTabReorderingEnabled(true);
	m_tabs->setHoverCloseButton(true);
	m_tabs->setHoverCloseButtonDelayed(true);
	m_tabs->setFocus();
	connect( m_tabs, SIGNAL( currentChanged( QWidget * ) ), m_receiver, SLOT(newCaption()) );
	connect( m_tabs, SIGNAL( currentChanged( QWidget * ) ), m_receiver, SLOT(activateView( QWidget * )) );
	connect( m_tabs, SIGNAL( currentChanged( QWidget * ) ), m_receiver, SLOT(updateModeStatus()) );
	connect( m_tabs, SIGNAL( closeRequest(QWidget *) ), this, SLOT(closeWidget(QWidget *)));
	connect( m_tabs, SIGNAL( testCanDecode( const QDragMoveEvent *,  bool & ) ), this, SLOT(testCanDecodeURLs( const QDragMoveEvent *, bool & )) );
	connect( m_tabs, SIGNAL( receivedDropEvent( QDropEvent * ) ), m_ki->docManager(), SLOT(openDroppedURLs( QDropEvent * )) );
	connect( m_tabs, SIGNAL( receivedDropEvent( QWidget*, QDropEvent * ) ), this, SLOT(replaceLoadedURL( QWidget *, QDropEvent * )) );
	m_widgetStack->raiseWidget(m_emptyDropWidget); // there are no tabs, so show the DropWidget
}

void Manager::closeWidget(QWidget *widget)
{
	if (widget->inherits( "KTextEditor::View" ))
	{
		KTextEditor::View *view = static_cast<KTextEditor::View*>(widget);
		m_ki->docManager()->fileClose(view->getDoc());
	}
}

KTextEditor::View* Manager::createTextView(KileDocument::TextInfo *info, int index)
{
	KTextEditor::Document *doc = info->getDoc();
	KTextEditor::View *view = static_cast<KTextEditor::View*>(info->createView (m_tabs, 0L));

	//install a key sequence recorder on the view
	view->focusProxy()->installEventFilter(new KileEditorKeySequence::Recorder(view, m_ki->editorKeySequenceManager()));

	// in the case of simple text documents, we mimic the behaviour of LaTeX documents
	if(info->getType() == KileDocument::Text)
	{
		view->focusProxy()->installEventFilter(m_ki->eventFilter());
	}

	//insert the view in the tab widget
	m_tabs->insertTab( view, m_ki->getShortName(doc), index );
	#if KDE_VERSION >= KDE_MAKE_VERSION(3,4,0)
		m_tabs->setTabToolTip(view, doc->url().pathOrUrl() );
	#else
		m_tabs->setTabToolTip(view, doc->url().prettyUrl() );
	#endif
	
	m_tabs->showPage( view );
	m_textViewList.insert((index < 0 || (uint)index >= m_textViewList.count()) ? m_textViewList.count() : index, view);

	connect(view, SIGNAL(viewStatusMsg(const QString&)), m_receiver, SLOT(newStatus(const QString&)));
	connect(view, SIGNAL(newStatus()), m_receiver, SLOT(newCaption()));
	connect(view, SIGNAL(dropEventPass(QDropEvent *)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent *)));
	connect(info, SIGNAL(urlChanged(KileDocument::Info*, const KUrl&)), this, SLOT(urlChanged(KileDocument::Info*, const KUrl&)));

	connect( doc,  SIGNAL(charactersInteractivelyInserted (int,int,const QString&)), m_ki->editorExtension()->complete(),  SLOT(slotCharactersInserted(int,int,const QString&)) );
	connect( view, SIGNAL(completionDone(KTextEditor::CompletionEntry)), m_ki->editorExtension()->complete(),  SLOT( slotCompletionDone(KTextEditor::CompletionEntry)) );
	connect( view, SIGNAL(completionAborted()), m_ki->editorExtension()->complete(),  SLOT( slotCompletionAborted()) );
	connect( view, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)), m_ki->editorExtension()->complete(),  SLOT(slotFilterCompletion(KTextEditor::CompletionEntry*,QString *)) );

	// install a working kate part popup dialog thingy
	Q3PopupMenu *viewPopupMenu = (Q3PopupMenu*)(m_client->factory()->container("ktexteditor_popup", m_client));
	if((NULL != view) && (NULL != viewPopupMenu))
		view->installPopup(viewPopupMenu);
	if(NULL != viewPopupMenu)
		connect(viewPopupMenu, SIGNAL(aboutToShow()), this, SLOT(onKatePopupMenuRequest()));

	//activate the newly created view
	emit(activateView(view, false));
	QTimer::singleShot(0, m_receiver, SLOT(newCaption())); //make sure the caption gets updated
	
	reflectDocumentStatus(view->getDoc(), false, 0);

	view->setFocusPolicy(QWidget::StrongFocus);
	view->setFocus();

	emit(prepareForPart("Editor"));
	unplugKatePartMenu(view);

	// use Kile's save and save-as functions instead of Katepart's
	KAction *action = view->actionCollection()->action(KStandardAction::stdName(KStandardAction::Save)); 
	if ( action ) 
	{
		KILE_DEBUG() << "   reconnect action 'file_save'..." << endl;
		action->disconnect(SIGNAL(activated()));
		connect(action, SIGNAL(activated()), m_ki->docManager(), SLOT(fileSave()));
	}
	action = view->actionCollection()->action(KStandardAction::stdName(KStandardAction::SaveAs));
	if ( action ) 
	{
		KILE_DEBUG() << "   reconnect action 'file_save_as'..." << endl;
		action->disconnect(SIGNAL(activated()));
		connect(action, SIGNAL(activated()), m_ki->docManager(), SLOT(fileSaveAs()));
	}
	m_widgetStack->raiseWidget(m_tabs); // there is at least one tab, so show the KTabWidget now

	return view;
}

void Manager::removeView(KTextEditor::View *view)
{
	if (view)
	{
		m_client->factory()->removeClient(view);

		m_tabs->removePage(view);
		m_textViewList.remove(view);
		delete view;
		
		QTimer::singleShot(0, m_receiver, SLOT(newCaption())); //make sure the caption gets updated
		if (textViews().isEmpty()) {
			m_ki->structureWidget()->clear();
			m_widgetStack->raiseWidget(m_emptyDropWidget); // there are no tabs left, so show
			                                               // the DropWidget
		}
	}
}

KTextEditor::View *Manager::currentTextView() const
{
	if ( m_tabs->currentPage() &&
		m_tabs->currentPage()->inherits( "KTextEditor::View" ) )
	{
		return (KTextEditor::View*) m_tabs->currentPage();
	}

	return 0;
}

KTextEditor::View* Manager::textView(KileDocument::TextInfo *info)
{
	KTextEditor::Document *doc = info->getDoc();
	if(!doc)
	{
		return NULL;
	}
	for(KTextEditor::View *view = m_textViewList.first(); view; view = m_textViewList.next())
	{
		if(view->getDoc() == doc)
		{
			return view;
		}
	}
	return NULL;
}

int Manager::getIndexOf(KTextEditor::View* view) const
{
	return m_tabs->indexOf(view);
}

unsigned int Manager::getTabCount() const {
	return m_tabs->count();
}

KTextEditor::View* Manager::switchToTextView(const KUrl & url, bool requestFocus)
{
	KTextEditor::View *view = 0L;
	KTextEditor::Document *doc = m_ki->docManager()->docFor(url);

	if (doc)
	{
		view = static_cast<KTextEditor::View*>(doc->views().first());
		if(view)
		{
			m_tabs->showPage(view);
			if(requestFocus)
				view->setFocus();
		}
	}
	return view;
}

void Manager::updateStructure(bool parse /* = false */, KileDocument::Info *docinfo /* = 0L */)
{
	if (docinfo == 0L)
		docinfo = m_ki->docManager()->getInfo();

	if (docinfo)
		m_ki->structureWidget()->update(docinfo, parse);

	KTextEditor::View *view = currentTextView();
	if (view) {view->setFocus();}

	if ( textViews().count() == 0 )
		m_ki->structureWidget()->clear();
}

void Manager::gotoNextView()
{
	if ( m_tabs->count() < 2 )
		return;

	int cPage = m_tabs->currentPageIndex() + 1;
	if ( cPage >= m_tabs->count() )
		m_tabs->setCurrentPage( 0 );
	else
		m_tabs->setCurrentPage( cPage );
}

void Manager::gotoPrevView()
{
	if ( m_tabs->count() < 2 )
		return;

	int cPage = m_tabs->currentPageIndex() - 1;
	if ( cPage < 0 )
		m_tabs->setCurrentPage( m_tabs->count() - 1 );
	else
		m_tabs->setCurrentPage( cPage );
}

void Manager::reflectDocumentStatus(KTextEditor::Document *doc, bool isModified, unsigned char reason)
{
	QPixmap icon;
	if ( reason == 0 && isModified ) //nothing
		icon = SmallIcon("filesave");
	else if ( reason == 1 || reason == 2 ) //dirty file
		icon = SmallIcon("revert");
	else if ( reason == 3 ) //file deleted
		icon = SmallIcon("stop");
	else if ( m_ki->extensions()->isScriptFile(doc->url()) )
		icon = SmallIcon("js");
	else
		icon = KMimeType::pixmapForURL (doc->url(), 0, KIcon::Small);

	changeTab(doc->views().first(), icon, m_ki->getShortName(doc));
}

/**
 * Adds/removes the "Convert to LaTeX" entry in Kate's popup menu according to the selection.
 */
void Manager::onKatePopupMenuRequest(void)
{
	KTextEditor::View *view = currentTextView();
	if(NULL == view)
		return;

	Q3PopupMenu *viewPopupMenu = (Q3PopupMenu*)(m_client->factory()->container("ktexteditor_popup", m_client));
	if(NULL == viewPopupMenu)
		return;

	// Setting up the "QuickPreview selection" entry
	KAction *quickPreviewAction = m_client->actionCollection()->action("popup_quickpreview");
	if(NULL != quickPreviewAction) {
		if(!quickPreviewAction->isPlugged())
			quickPreviewAction->plug(viewPopupMenu);

		quickPreviewAction->setEnabled( view->getDoc()->hasSelection() || 
		                                m_ki->editorExtension()->hasMathgroup(view) ||
		                                m_ki->editorExtension()->hasEnvironment(view)
		                              );
	}

	// Setting up the "Convert to LaTeX" entry
	KAction *latexCvtAction = m_client->actionCollection()->action("popup_converttolatex");
	if(NULL != latexCvtAction) {
		if(!latexCvtAction->isPlugged())
			latexCvtAction->plug(viewPopupMenu);

		latexCvtAction->setEnabled(view->getDoc()->hasSelection());
	}

	// Setting up the "Paste as LaTeX" entry
	KAction *pasteAsLaTeXAction = m_client->actionCollection()->action("popup_pasteaslatex");
	if((NULL != pasteAsLaTeXAction)) {
		if(!pasteAsLaTeXAction->isPlugged())
			pasteAsLaTeXAction->plug(viewPopupMenu);

		QClipboard *clip = KApplication::clipboard();
		if(NULL != clip)
			pasteAsLaTeXAction->setEnabled(!clip->text().isNull());
	}
}

void Manager::convertSelectionToLaTeX(void)
{
	KTextEditor::View *view = currentTextView();

	if(NULL == view)
		return;

	KTextEditor::Document *doc = view->getDoc();

	if(NULL == doc)
		return;

	// Getting the selection
	uint selStartLine = doc->selStartLine(), selStartCol = doc->selStartCol();
	uint selEndLine = doc->selEndLine(), selEndCol = doc->selEndCol();

	/* Variable to "restore" the selection after replacement: if {} was selected,
	   we increase the selection of two characters */
	uint newSelEndCol;

	PlainToLaTeXConverter cvt;

	// "Notifying" the editor that what we're about to do must be seen as ONE operation
	KTextEditor::EditInterfaceExt *editInterfaceExt = KTextEditor::editInterfaceExt(doc);
	if(NULL != editInterfaceExt)
		editInterfaceExt->editBegin();

	// Processing the first line
	int firstLineLength;
	if(selStartLine != selEndLine)
		firstLineLength = doc->lineLength(selStartLine);
	else
		firstLineLength = selEndCol;
	QString firstLine = doc->text(selStartLine, selStartCol, selStartLine, firstLineLength);
	QString firstLineCvt = cvt.ConvertToLaTeX(firstLine);
	doc->removeText(selStartLine, selStartCol, selStartLine, firstLineLength);
	doc->insertText(selStartLine, selStartCol, firstLineCvt);
	newSelEndCol = selStartCol + firstLineCvt.length();

	// Processing the intermediate lines
	for(uint nLine = selStartLine + 1 ; nLine < selEndLine ; ++nLine) {
		QString line = doc->textLine(nLine);
		QString newLine = cvt.ConvertToLaTeX(line);
		doc->removeLine(nLine);
		doc->insertLine(nLine, newLine);
	}

	// Processing the final line
	if(selStartLine != selEndLine) {
		QString lastLine = doc->text(selEndLine, 0, selEndLine, selEndCol);
		QString lastLineCvt = cvt.ConvertToLaTeX(lastLine);
		doc->removeText(selEndLine, 0, selEndLine, selEndCol);
		doc->insertText(selEndLine, 0, lastLineCvt);
		newSelEndCol = lastLineCvt.length();
	}

	// End of the "atomic edit operation"
	if(NULL != editInterfaceExt)
		editInterfaceExt->editEnd();

	doc->setSelection(selStartLine, selStartCol, selEndLine, newSelEndCol);
}

/**
 * Pastes the clipboard's contents as LaTeX (ie. % -> \%, etc.).
 */
void Manager::pasteAsLaTeX(void)
{
	KTextEditor::View *view = currentTextView();

	if(NULL == view)
		return;

	KTextEditor::Document *doc = view->getDoc();

	if(NULL == doc)
		return;

	// Getting a proper text insertion point BEFORE the atomic editing operation
	uint cursorLine, cursorCol;
	if(doc->hasSelection()) {
		cursorLine = doc->selStartLine();
		cursorCol = doc->selStartCol();
	} else {
		view->cursorPositionReal(&cursorLine, &cursorCol);
	}

	// "Notifying" the editor that what we're about to do must be seen as ONE operation
	KTextEditor::EditInterfaceExt *editInterfaceExt = KTextEditor::editInterfaceExt(doc);
	if(NULL != editInterfaceExt)
		editInterfaceExt->editBegin();

	// If there is a selection, one must remove it
	if(doc->hasSelection())
		doc->removeSelectedText();

	PlainToLaTeXConverter cvt;
	QString toPaste = cvt.ConvertToLaTeX(KApplication::clipboard()->text());
	doc->insertText(cursorLine, cursorCol, toPaste);

	// End of the "atomic edit operation"
	if(NULL != editInterfaceExt)
		editInterfaceExt->editEnd();
}

void Manager::quickPreviewPopup()
{
	KTextEditor::View *view = currentTextView();
	if( ! view )
		return;

	KTextEditor::Document *doc = view->getDoc();
	if ( doc )
	{
		if ( doc->hasSelection() )
			emit( startQuickPreview(KileTool::qpSelection) );
		else if ( m_ki->editorExtension()->hasMathgroup(view) )
			emit( startQuickPreview(KileTool::qpMathgroup) );
		else if ( m_ki->editorExtension()->hasEnvironment(view) )
			emit( startQuickPreview(KileTool::qpEnvironment) );
	}
}

void Manager::testCanDecodeURLs(const QDragMoveEvent *e, bool &accept)
{
	accept = K3URLDrag::canDecode(e); // only accept URL drops
}

void Manager::replaceLoadedURL(QWidget *w, QDropEvent *e)
{
	KUrl::List urls;
	if(!K3URLDrag::decode(e, urls)) {
		return;
	}
	int index = m_tabs->indexOf(w);
	KileDocument::Extensions *extensions = m_ki->extensions();
	bool hasReplacedTab = false;
	for(KUrl::List::iterator i = urls.begin(); i != urls.end(); ++i) {
		KUrl url = *i;
		if(extensions->isProjectFile(url)) {
			m_ki->docManager()->projectOpen(url);
		}
		else if(!hasReplacedTab) {
			closeWidget(w);
			m_ki->docManager()->fileOpen(url, QString::null, index);
			hasReplacedTab = true;
		}
		else {
			m_ki->docManager()->fileOpen(url);
		}
	}
}

void Manager::urlChanged(KileDocument::Info* info, const KUrl& /*url*/)
{
	KileDocument::TextInfo *textInfo = dynamic_cast<KileDocument::TextInfo*>(info);
	if(textInfo)
	{
		KTextEditor::View *view = textView(textInfo);
		if(!view)
		{
			return;
		}
		setTabLabel(view, m_ki->getShortName(textInfo->getDoc()));
	}
}

DropWidget::DropWidget(QWidget * parent, const char * name, Qt::WFlags f) : QWidget(parent, name, f)
{
	setAcceptDrops(true);
}

DropWidget::~DropWidget()
{
}

void DropWidget::dragMoveEvent(QDragMoveEvent *e)
{
	bool b;
	emit testCanDecode(e, b);
	e->accept(b);
}

void DropWidget::dropEvent(QDropEvent *e)
{
	emit receivedDropEvent(e);
}

// remove entries from KatePart menu: 
//  - menu entry to config Kate, because there is
//    already one call to this configuration dialog from Kile
//  - goto line, because we put it into a submenu

void Manager::unplugKatePartMenu(KTextEditor::View* view)
{
	if ( view ) 
	{
		QStringList actionlist;
		actionlist << "set_confdlg" << "go_goto_line";      // action names from katepartui.rc

		for ( uint i=0; i < actionlist.count(); ++i )
		{
			KAction *action = view->actionCollection()->action( actionlist[i].ascii() ); 
			if ( action ) 
			{
				action->unplugAll();
//				action->setShortcut(KShortcut());
			}
		}
	}
}

}

#include "kileviewmanager.moc"
