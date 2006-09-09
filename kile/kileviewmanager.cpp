//
// C++ Implementation: kileviewmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//         Michel Ludwig <michel.ludwig@kdemail.net>, (C) 2006

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpopupmenu.h>
#include <qtimer.h> //for QTimer::singleShot trick
#include <qpixmap.h>
#include <qclipboard.h>

#include <kdeversion.h>
#include <kglobal.h>
#include <kate/view.h>
#include <kate/document.h>
#include <kparts/componentfactory.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <klocale.h>
#include <ktexteditor/editinterfaceext.h>
#include <kapplication.h>
#include <kurldrag.h>

#include "kileinfo.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
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
	m_activeView(0L),
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
	m_widgetStack = new QWidgetStack(parent);
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
	if (widget->inherits( "Kate::View" ))
	{
		Kate::View *view = static_cast<Kate::View*>(widget);
		m_ki->docManager()->fileClose(view->getDoc());
	}
}

Kate::View* Manager::createView(Kate::Document *doc, int index)
{
	Kate::View *view = (Kate::View*) doc->createView (m_tabs, 0L);

	//install event filter on the view
	view->focusProxy()->installEventFilter(m_ki->eventFilter());

	//insert the view in the tab widget
	m_tabs->insertTab( view, m_ki->getShortName(doc), index );
	m_tabs->setTabToolTip(view, doc->url().url() );
	m_tabs->showPage( view );
	m_viewList.insert((index < 0 || (uint)index >= m_viewList.count()) ? m_viewList.count() : index, view);

	connect(view, SIGNAL(viewStatusMsg(const QString&)), m_receiver, SLOT(newStatus(const QString&)));
	connect(view, SIGNAL(newStatus()), m_receiver, SLOT(newCaption()));
	connect(view, SIGNAL(dropEventPass(QDropEvent *)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent *)));

	connect( doc,  SIGNAL(charactersInteractivelyInserted (int,int,const QString&)), m_ki->editorExtension()->complete(),  SLOT(slotCharactersInserted(int,int,const QString&)) );
	connect( view, SIGNAL(completionDone(KTextEditor::CompletionEntry)), m_ki->editorExtension()->complete(),  SLOT( slotCompletionDone(KTextEditor::CompletionEntry)) );
	connect( view, SIGNAL(completionAborted()), m_ki->editorExtension()->complete(),  SLOT( slotCompletionAborted()) );
	connect( view, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)), m_ki->editorExtension()->complete(),  SLOT(slotFilterCompletion(KTextEditor::CompletionEntry*,QString *)) );

	KAction *spa = view->actionCollection()->action( "tools_spelling" );
	if ( spa )
	{
		kdDebug() << "RECONNECTING SPELLCHECKER" << endl;
		disconnect(spa , 0 , 0 , 0);
		connect( spa, SIGNAL(activated()), this, SIGNAL(startSpellCheck()) );
	}

	// install a working kate part popup dialog thingy
	Kate::View *kateView = static_cast<Kate::View*>(view->qt_cast("Kate::View"));
	QPopupMenu *viewPopupMenu = (QPopupMenu*)(m_client->factory()->container("ktexteditor_popup", m_client));
	if((NULL != kateView) && (NULL != viewPopupMenu))
		kateView->installPopup(viewPopupMenu);
	if(NULL != viewPopupMenu)
		connect(viewPopupMenu, SIGNAL(aboutToShow()), this, SLOT(onKatePopupMenuRequest()));

	//activate the newly created view
	emit(activateView(view, false));
	QTimer::singleShot(0, m_receiver, SLOT(newCaption())); //make sure the caption gets updated
	
	reflectDocumentStatus(view->getDoc(), false, 0);

	view->setFocusPolicy(QWidget::StrongFocus);
	view->setFocus();

	emit(prepareForPart("Editor"));

	// remove 'configure editor' dialog of Kate
	// this will be called directly from Kile
	KAction *action = view->actionCollection()->action("set_confdlg"); 
	if ( action ) 
	{
		kdDebug() << "   unplug action 'set_confdlg'..." << endl;
		action->unplugAll();
	}

	m_widgetStack->raiseWidget(m_tabs); // there is at least one tab, so show the KTabWidget now

	return view;
}

void Manager::removeView(Kate::View *view)
{
	if (view)
	{
		m_client->factory()->removeClient(view);

		m_tabs->removePage(view);
		m_viewList.remove(view);
		delete view;
		
		QTimer::singleShot(0, m_receiver, SLOT(newCaption())); //make sure the caption gets updated
		if (views().isEmpty()) {
			m_ki->structureWidget()->clear();
			m_widgetStack->raiseWidget(m_emptyDropWidget); // there are no tabs left, so show
			                                               // the DropWidget
		}
	}
}

Kate::View *Manager::currentView() const
{
	if ( m_tabs->currentPage() &&
		m_tabs->currentPage()->inherits( "Kate::View" ) )
	{
		return (Kate::View*) m_tabs->currentPage();
	}

	return 0;
}

Kate::View* Manager::switchToView(const KURL & url)
{
	Kate::View *view = 0L;
	Kate::Document *doc = m_ki->docManager()->docFor(url);

	if (doc)
	{
		view = static_cast<Kate::View*>(doc->views().first());
		m_tabs->showPage(view);
	}

	return view;
}

void Manager::updateStructure(bool parse /* = false */, KileDocument::Info *docinfo /* = 0L */)
{
	if (docinfo == 0L)
		docinfo = m_ki->docManager()->getInfo();

	if (docinfo)
		m_ki->structureWidget()->update(docinfo, parse);

	Kate::View *view = currentView();
	if (view) {view->setFocus();}

	if ( views().count() == 0 )
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

void Manager::reflectDocumentStatus(Kate::Document *doc, bool isModified, unsigned char reason)
{
	QPixmap icon;
	if ( reason == 0 && isModified ) //nothing
		icon = SmallIcon("filesave");
	else if ( reason == 1 || reason == 2 ) //dirty file
		icon = SmallIcon("revert");
	else if ( reason == 3 ) //file deleted
		icon = SmallIcon("stop");
	else
		icon = KMimeType::pixmapForURL (doc->url(), 0, KIcon::Small);
	
	changeTab(doc->views().first(), icon, m_ki->getShortName(doc));
}

/**
 * Adds/removes the "Convert to LaTeX" entry in Kate's popup menu according to the selection.
 */
void Manager::onKatePopupMenuRequest(void)
{
	Kate::View *view = currentView();
	if(NULL == view)
		return;

	QPopupMenu *viewPopupMenu = (QPopupMenu*)(m_client->factory()->container("ktexteditor_popup", m_client));
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
	Kate::View *view = currentView();

	if(NULL == view)
		return;

	Kate::Document *doc = view->getDoc();

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
	Kate::View *view = currentView();

	if(NULL == view)
		return;

	Kate::Document *doc = view->getDoc();

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
	Kate::View *view = currentView();
	if( ! view )
		return;

	Kate::Document *doc = view->getDoc();
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
	accept = KURLDrag::canDecode(e); // only accept URL drops
}

void Manager::replaceLoadedURL(QWidget *w, QDropEvent *e)
{
	KURL::List urls;
	if(!KURLDrag::decode(e, urls)) {
		return;
	}
	int index = m_tabs->indexOf(w);
	closeWidget(w);
	for(KURL::List::iterator i = urls.begin(); i != urls.end(); ++i) {
		if(i == urls.begin()) {
			m_ki->docManager()->fileOpen(*i, QString::null, index);
		}
		else {
			m_ki->docManager()->fileOpen(*i);
		}
	}
}

DropWidget::DropWidget(QWidget * parent, const char * name, WFlags f) : QWidget(parent, name, f)
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

}

#include "kileviewmanager.moc"
