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

#include <QClipboard>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLayout>
#include <QPixmap>
#include <QTimer> //for QTimer::singleShot trick

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KGlobal>
#include <KIconLoader>
#include <kio/global.h>
#include <KLocale>
#include <KMimeType>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include "editorkeysequencemanager.h"
#include "kileinfo.h"
#include "kileconstants.h"
#include "kiledocmanager.h"
#include "kileextensions.h"
#include "widgets/projectview.h"
#include "widgets/structurewidget.h"
#include "editorextension.h"
#include "plaintolatexconverter.h"
#include "widgets/previewwidget.h"
#include "quickpreview.h"

namespace KileView 
{

Manager::Manager(KileInfo *info, QObject *parent, const char *name) :
	QObject(parent, name),
	m_ki(info),
	m_activeTextView(NULL),
// 	m_projectview(NULL),
	m_tabs(NULL),
	m_widgetStack(NULL),
	m_emptyDropWidget(NULL)
{
}


Manager::~Manager()
{
}

void Manager::setClient(QObject *receiver, KXMLGUIClient *client)
{
	m_receiver = receiver;
	m_client = client;
	if(NULL == m_client->actionCollection()->action("popup_pasteaslatex")) {
		KAction *action = new KAction(i18n("Paste as LaTe&X"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(pasteAsLaTeX()));
	}
	if(NULL == m_client->actionCollection()->action("popup_converttolatex")) {
		KAction *action = new KAction(i18n("Convert Selection to &LaTeX"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(convertSelectionToLaTeX()));
	}
	if(NULL == m_client->actionCollection()->action("popup_quickpreview")) {
		KAction *action = new KAction(i18n("&QuickPreview Selection"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(quickPreviewPopup()));
	}
}

QWidget* Manager::createTabs(QWidget *parent)
{
	m_widgetStack = new QStackedWidget(parent);
	m_emptyDropWidget = new DropWidget(m_widgetStack);
	m_widgetStack->addWidget(m_emptyDropWidget);
	connect(m_emptyDropWidget, SIGNAL(testCanDecode(const QDragEnterEvent*, bool&)), this, SLOT(testCanDecodeURLs(const QDragEnterEvent*, bool&)));
	connect(m_emptyDropWidget, SIGNAL(receivedDropEvent(QDropEvent*)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent*)));
	m_tabs = new KTabWidget(parent);
	m_widgetStack->addWidget(m_tabs);
	m_tabs->setFocusPolicy(Qt::ClickFocus);
	m_tabs->setTabReorderingEnabled(true);
	m_tabs->setHoverCloseButton(true);
	m_tabs->setHoverCloseButtonDelayed(true);
	m_tabs->setFocus();
	connect(m_tabs, SIGNAL(currentChanged(QWidget*)), m_receiver, SLOT(newCaption()));
	connect(m_tabs, SIGNAL(currentChanged(QWidget*)), m_receiver, SLOT(activateView(QWidget*)));
	connect(m_tabs, SIGNAL(currentChanged(QWidget*)), m_receiver, SLOT(updateModeStatus()));
	connect(m_tabs, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeWidget(QWidget*)));
	connect(m_tabs, SIGNAL(testCanDecode(const QDragMoveEvent*, bool&)), this, SLOT(testCanDecodeURLs(const QDragMoveEvent*, bool&)));
	connect(m_tabs, SIGNAL(receivedDropEvent(QDropEvent*)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent*)));
	connect(m_tabs, SIGNAL(receivedDropEvent(QWidget*, QDropEvent*)), this, SLOT(replaceLoadedURL(QWidget*, QDropEvent*)));
	m_widgetStack->setCurrentWidget(m_emptyDropWidget); // there are no tabs, so show the DropWidget

	return m_widgetStack;
}

void Manager::closeWidget(QWidget *widget)
{
	if (widget->inherits( "KTextEditor::View" ))
	{
		KTextEditor::View *view = static_cast<KTextEditor::View*>(widget);
		m_ki->docManager()->fileClose(view->document());
	}
}

KTextEditor::View* Manager::createTextView(KileDocument::TextInfo *info, int index)
{
	KTextEditor::Document *doc = info->getDoc();
	KTextEditor::View *view = static_cast<KTextEditor::View*>(info->createView (m_tabs, NULL));

	//install a key sequence recorder on the view
	view->focusProxy()->installEventFilter(new KileEditorKeySequence::Recorder(view, m_ki->editorKeySequenceManager()));

	// in the case of simple text documents, we mimic the behaviour of LaTeX documents
	if(info->getType() == KileDocument::Text) {
// 		view->focusProxy()->installEventFilter(m_ki->eventFilter());
	}

	//insert the view in the tab widget
	m_tabs->insertTab(index, view, m_ki->getShortName(doc));
	m_tabs->setTabToolTip(m_tabs->indexOf(view), doc->url().pathOrUrl());
	
	m_tabs->showPage(view);
	m_textViewList.insert((index < 0 || index >= m_textViewList.count()) ? m_textViewList.count() : index, view);

	connect(view, SIGNAL(viewStatusMsg(const QString&)), m_receiver, SLOT(newStatus(const QString&)));
	connect(view, SIGNAL(newStatus()), m_receiver, SLOT(newCaption()));
	connect(view, SIGNAL(dropEventPass(QDropEvent *)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent *)));
	connect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(updateTabTexts(KTextEditor::Document*)));
	connect(doc, SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SLOT(updateTabTexts(KTextEditor::Document*)));

	connect( view, SIGNAL(completionDone(KTextEditor::CompletionEntry)), m_ki->editorExtension()->complete(),  SLOT( slotCompletionDone(KTextEditor::CompletionEntry)) );
	connect( view, SIGNAL(completionAborted()), m_ki->editorExtension()->complete(),  SLOT( slotCompletionAborted()) );
	connect( view, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)), m_ki->editorExtension()->complete(),  SLOT(slotFilterCompletion(KTextEditor::CompletionEntry*,QString *)) );

	// code completion
	KTextEditor::CodeCompletionInterface *completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(completionInterface) {
		connect(view, SIGNAL(textInserted(KTextEditor::View*, const KTextEditor::Cursor&, const QString&)), m_ki->editorExtension()->complete(), SLOT(textInsertedInView(KTextEditor::View*, const KTextEditor::Cursor&, const QString&)));
		completionInterface->setAutomaticInvocationEnabled(false);
	}

	// install a working text editor part popup dialog thingy
	QMenu *viewPopupMenu = qobject_cast<QMenu*>(m_client->factory()->container("ktexteditor_popup", m_client));

	if(view && viewPopupMenu) {
		view->setContextMenu(viewPopupMenu);
	}

	if(viewPopupMenu) {
		connect(viewPopupMenu, SIGNAL(aboutToShow()), this, SLOT(onTextEditorPopupMenuRequest()));
	}

	//activate the newly created view
	emit(activateView(view, false));
	QTimer::singleShot(0, m_receiver, SLOT(newCaption())); //make sure the caption gets updated
	
	reflectDocumentStatus(view->document(), false, 0);

	view->setFocusPolicy(Qt::StrongFocus);
	view->setFocus();

	emit(prepareForPart("Editor"));
	unplugTextEditorPartMenu(view);

	// use Kile's save and save-as functions instead of the text editor's
	QAction *action = view->actionCollection()->action(KStandardAction::stdName(KStandardAction::Save)); 
	if(action) {
		KILE_DEBUG() << "   reconnect action 'file_save'...";
		action->disconnect(SIGNAL(triggered(bool)));
		connect(action, SIGNAL(triggered()), m_ki->docManager(), SLOT(fileSave()));
	}
	action = view->actionCollection()->action(KStandardAction::stdName(KStandardAction::SaveAs));
	if(action) {
		KILE_DEBUG() << "   reconnect action 'file_save_as'...";
		action->disconnect(SIGNAL(triggered(bool)));
		connect(action, SIGNAL(triggered()), m_ki->docManager(), SLOT(fileSaveAs()));
	}
	updateTabTexts(doc);
	m_widgetStack->setCurrentWidget(m_tabs); // there is at least one tab, so show the KTabWidget now

	return view;
}

void Manager::removeView(KTextEditor::View *view)
{
	if (view) {
		m_client->factory()->removeClient(view);

		m_tabs->removePage(view);
		m_textViewList.remove(view);
		delete view;
		
		QTimer::singleShot(0, m_receiver, SLOT(newCaption())); //make sure the caption gets updated
		if (textViews().isEmpty()) {
			m_ki->structureWidget()->clear();
			m_widgetStack->setCurrentWidget(m_emptyDropWidget); // there are no tabs left, so show
			                                                    // the DropWidget
		}
	}
}

KTextEditor::View *Manager::currentTextView() const
{
	KTextEditor::View *view = qobject_cast<KTextEditor::View*>(m_tabs->currentWidget());

	return view;
}

KTextEditor::View* Manager::textView(KileDocument::TextInfo *info)
{
	KTextEditor::Document *doc = info->getDoc();
	if(!doc) {
		return NULL;
	}
	for(QList<KTextEditor::View*>::iterator i =  m_textViewList.begin(); i != m_textViewList.end(); ++i) {
		KTextEditor::View *view = *i;

		if(view->document() == doc) {
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
	KTextEditor::View *view = NULL;
	KTextEditor::Document *doc = m_ki->docManager()->docFor(url);

	if (doc) {
		if (doc->views().count() > 0) {
			view = static_cast<KTextEditor::View*>(doc->views().first());
			if(view) {
				m_tabs->showPage(view);
				if(requestFocus)
					view->setFocus();
			}
		}
	}
	return view;
}


void Manager::setTabLabel(QWidget *view, const QString& name)
{
	m_tabs->setTabText(m_tabs->indexOf(view), name);
}

void Manager::setTabIcon(QWidget *view, const QPixmap& icon)
{
	m_tabs->setTabIcon(m_tabs->indexOf(view), QIcon(icon));
}

void Manager::updateStructure(bool parse /* = false */, KileDocument::Info *docinfo /* = NULL */)
{
	if (!docinfo) {
		docinfo = m_ki->docManager()->getInfo();
	}

	if(docinfo) {
		m_ki->structureWidget()->update(docinfo, parse);
	}

	KTextEditor::View *view = currentTextView();
	if(view) {
		view->setFocus();
	}

	if(textViews().count() == 0) {
		m_ki->structureWidget()->clear();
	}
}

void Manager::gotoNextView()
{
	if(m_tabs->count() < 2) {
		return;
	}

	int cPage = m_tabs->currentIndex() + 1;
	if(cPage >= m_tabs->count()) {
		m_tabs->setCurrentIndex(0);
	}
	else {
		m_tabs->setCurrentIndex(cPage);
	}
}

void Manager::gotoPrevView()
{
	if(m_tabs->count() < 2) {
		return;
	}

	int cPage = m_tabs->currentIndex() - 1;
	if(cPage < 0) {
		m_tabs->setCurrentIndex(m_tabs->count() - 1);
	}
	else {
		m_tabs->setCurrentIndex(cPage);
	}
}

void Manager::reflectDocumentStatus(KTextEditor::Document *doc, bool isModified, unsigned char reason)
{
	QPixmap icon;
	if(reason == 0 && isModified) { //nothing
		icon = SmallIcon("filesave");
	}
	else if(reason == 1 || reason == 2) { //dirty file
		icon = SmallIcon("revert");
	}
	else if(reason == 3) { //file deleted
		icon = SmallIcon("process-stop");
	}
	else if(m_ki->extensions()->isScriptFile(doc->url())) {
		icon = SmallIcon("js");
	}
	else {
		icon = KIO::pixmapForUrl(doc->url(), 0, KIconLoader::Small);
	}

	const QList<KTextEditor::View*> &viewsList = doc->views();
	for(QList<KTextEditor::View*>::const_iterator i = viewsList.begin(); i != viewsList.end(); ++i) {
		setTabIcon(*i, icon);
	}
}

/**
 * Adds/removes the "Convert to LaTeX" entry in Kate's popup menu according to the selection.
 */
void Manager::onTextEditorPopupMenuRequest(void)
{
	KTextEditor::View *view = currentTextView();
	if(!view) {
		return;
	}

	QMenu *viewPopupMenu = qobject_cast<QMenu*>(m_client->factory()->container("ktexteditor_popup", m_client));
	if(!viewPopupMenu) {
		return;
	}

	// Setting up the "QuickPreview selection" entry
	QAction *quickPreviewAction = m_client->actionCollection()->action("popup_quickpreview");
	if(NULL != quickPreviewAction) {
		if(!quickPreviewAction->associatedWidgets().isEmpty()) {
			viewPopupMenu->addAction(quickPreviewAction);
		}

		quickPreviewAction->setEnabled( view->selection() ||
		                                m_ki->editorExtension()->hasMathgroup(view) ||
		                                m_ki->editorExtension()->hasEnvironment(view)
		                              );
	}

	// Setting up the "Convert to LaTeX" entry
	QAction *latexCvtAction = m_client->actionCollection()->action("popup_converttolatex");
	if(NULL != latexCvtAction) {
		if(!latexCvtAction->associatedWidgets().isEmpty()) {
			viewPopupMenu->addAction(latexCvtAction);
		}

		latexCvtAction->setEnabled(view->selection());
	}

	// Setting up the "Paste as LaTeX" entry
	QAction *pasteAsLaTeXAction = m_client->actionCollection()->action("popup_pasteaslatex");
	if((NULL != pasteAsLaTeXAction)) {
		if(!pasteAsLaTeXAction->associatedWidgets().isEmpty()) {
			viewPopupMenu->addAction(pasteAsLaTeXAction);
		}

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

	KTextEditor::Document *doc = view->document();

	if(NULL == doc)
		return;

	// Getting the selection
	KTextEditor::Range range = view->selectionRange();
	uint selStartLine = range.start().line(), selStartCol = range.start().column();
	uint selEndLine = range.end().line(), selEndCol = range.start().column();

	/* Variable to "restore" the selection after replacement: if {} was selected,
	   we increase the selection of two characters */
	uint newSelEndCol;

	PlainToLaTeXConverter cvt;

	// "Notifying" the editor that what we're about to do must be seen as ONE operation
	doc->startEditing();

	// Processing the first line
	int firstLineLength;
	if(selStartLine != selEndLine)
		firstLineLength = doc->lineLength(selStartLine);
	else
		firstLineLength = selEndCol;
	QString firstLine = doc->text(KTextEditor::Range(selStartLine, selStartCol, selStartLine, firstLineLength));
	QString firstLineCvt = cvt.ConvertToLaTeX(firstLine);
	doc->removeText(KTextEditor::Range(selStartLine, selStartCol, selStartLine, firstLineLength));
	doc->insertText(KTextEditor::Cursor(selStartLine, selStartCol), firstLineCvt);
	newSelEndCol = selStartCol + firstLineCvt.length();

	// Processing the intermediate lines
	for(uint nLine = selStartLine + 1 ; nLine < selEndLine ; ++nLine) {
		QString line = doc->line(nLine);
		QString newLine = cvt.ConvertToLaTeX(line);
		doc->removeLine(nLine);
		doc->insertLine(nLine, newLine);
	}

	// Processing the final line
	if(selStartLine != selEndLine) {
		QString lastLine = doc->text(KTextEditor::Range(selEndLine, 0, selEndLine, selEndCol));
		QString lastLineCvt = cvt.ConvertToLaTeX(lastLine);
		doc->removeText(KTextEditor::Range(selEndLine, 0, selEndLine, selEndCol));
		doc->insertText(KTextEditor::Cursor(selEndLine, 0), lastLineCvt);
		newSelEndCol = lastLineCvt.length();
	}

	// End of the "atomic edit operation"
	doc->endEditing();

	view->setSelection(KTextEditor::Range(selStartLine, selStartCol, selEndLine, newSelEndCol));
}

/**
 * Pastes the clipboard's contents as LaTeX (ie. % -> \%, etc.).
 */
void Manager::pasteAsLaTeX(void)
{
	KTextEditor::View *view = currentTextView();

	if(!view) {
		return;
	}

	KTextEditor::Document *doc = view->document();

	if(!doc) {
		return;
	}

	// Getting a proper text insertion point BEFORE the atomic editing operation
	uint cursorLine, cursorCol;
	if(view->selection()) {
		KTextEditor::Range range = view->selectionRange();
		cursorLine = range.start().line();
		cursorCol = range.start().column();
	} else {
		KTextEditor::Cursor cursor = view->cursorPosition();
		cursorLine = cursor.line();
		cursorCol = cursor.column();
	}

	// "Notifying" the editor that what we're about to do must be seen as ONE operation
	doc->startEditing();

	// If there is a selection, one must remove it
	if(view->selection()) {
		doc->removeText(view->selectionRange());
	}

	PlainToLaTeXConverter cvt;
	QString toPaste = cvt.ConvertToLaTeX(KApplication::clipboard()->text());
	doc->insertText(KTextEditor::Cursor(cursorLine, cursorCol), toPaste);

	// End of the "atomic edit operation"
	doc->endEditing();
}

void Manager::quickPreviewPopup()
{
	KTextEditor::View *view = currentTextView();
	if(!view) {
		return;
	}

	if(view->selection()) {
		emit(startQuickPreview(KileTool::qpSelection));
	}
	else if(m_ki->editorExtension()->hasMathgroup(view)) {
		emit(startQuickPreview(KileTool::qpMathgroup));
	}
	else if(m_ki->editorExtension()->hasEnvironment(view)) {
		emit(startQuickPreview(KileTool::qpEnvironment));
	}
}

void Manager::testCanDecodeURLs(const QDragEnterEvent *e, bool &accept)
{
	accept = e->mimeData()->hasUrls(); // only accept URL drops
}

void Manager::testCanDecodeURLs(const QDragMoveEvent *e, bool &accept)
{
	accept = e->mimeData()->hasUrls(); // only accept URL drops
}

void Manager::replaceLoadedURL(QWidget *w, QDropEvent *e)
{
	KUrl::List urls = KUrl::List::fromMimeData(e->mimeData());
	if (urls.isEmpty()) {
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
			m_ki->docManager()->fileOpen(url, QString(), index);
			hasReplacedTab = true;
		}
		else {
			m_ki->docManager()->fileOpen(url);
		}
	}
}

void Manager::updateTabTexts(KTextEditor::Document* changedDoc)
{
	const QList<KTextEditor::View*> &viewsList = changedDoc->views();
	for(QList<KTextEditor::View*>::const_iterator i = viewsList.begin(); i != viewsList.end(); ++i) {
		QString documentName = changedDoc->documentName();
		if(documentName.isEmpty()) {
			documentName = i18n("Untitled");
		}
		setTabLabel(*i, documentName);
	}
}

DropWidget::DropWidget(QWidget * parent, const char * name, Qt::WFlags f) : QWidget(parent, name, f)
{
	setAcceptDrops(true);
}

DropWidget::~DropWidget()
{
}

void DropWidget::dragEnterEvent(QDragEnterEvent *e)
{
	bool b;
	emit testCanDecode(e, b);
	if(b) {
		e->acceptProposedAction();
	}
}

void DropWidget::dropEvent(QDropEvent *e)
{
	emit receivedDropEvent(e);
}

// remove entries from KatePart menu: 
//  - menu entry to config Kate, because there is
//    already one call to this configuration dialog from Kile
//  - goto line, because we put it into a submenu

#ifdef __GNUC__
#warning Check whether this function is actually needed / allowed!
#endif
void Manager::unplugTextEditorPartMenu(KTextEditor::View* view)
{
	if(view) {
		QStringList actionlist;
		actionlist << "set_confdlg" << "go_goto_line";      // action names from katepartui.rc

		for(int i = 0; i < actionlist.count(); ++i) {
			QAction *action = view->actionCollection()->action(actionlist[i].ascii());
			if(action) {
//FIXME: should be removed for KDE4
//				action->setShortcut(KShortcut());
				foreach(QWidget *w, action->associatedWidgets()) {
					w->removeAction(action);
				}
			}
		}
	}
}


void Manager::installEventFilter(KTextEditor::View *view, QObject *eventFilter)
{
	QWidget *focusProxy = view->focusProxy();
	if(focusProxy) {
		focusProxy->installEventFilter(eventFilter);
	}
	else {
		view->installEventFilter(eventFilter);
	}
}

void Manager::removeEventFilter(KTextEditor::View *view, QObject *eventFilter)
{
	QWidget *focusProxy = view->focusProxy();
	if(focusProxy) {
		focusProxy->removeEventFilter(eventFilter);
	}
	else {
		view->removeEventFilter(eventFilter);
	}
}

}

#include "kileviewmanager.moc"
