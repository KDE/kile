//
// C++ Implementation: kileviewmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpopupmenu.h>

#include <kglobal.h>
#include <kate/view.h>
#include <kate/document.h>
#include <kparts/componentfactory.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include "kileinfo.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "kileprojectview.h"
#include "kileeventfilter.h"
#include "kilestructurewidget.h"
#include "kileedit.h"

namespace KileView 
{

Manager::Manager(KileInfo *info, QObject *parent, const char *name) :
	QObject(parent, name),
	m_ki(info),
	m_activeView(0L),
	m_projectview(0L),
	m_tabs(0L)
{
}


Manager::~Manager()
{
}

void Manager::setClient(QObject *receiver, KXMLGUIClient *client)
{
	m_receiver = receiver;
	m_client = client;
}

void Manager::createTabs(QWidget *parent)
{
	m_tabs=new QTabWidget(parent);
	m_tabs->setFocusPolicy(QWidget::ClickFocus);
	m_tabs->setFocus();
	connect( m_tabs, SIGNAL( currentChanged( QWidget * ) ), m_receiver, SLOT(newCaption()) );
	connect( m_tabs, SIGNAL( currentChanged( QWidget * ) ), m_receiver, SLOT(activateView( QWidget * )) );
	connect( m_tabs, SIGNAL( currentChanged( QWidget * ) ), m_receiver, SLOT(updateModeStatus()) );
}

Kate::View* Manager::createView(Kate::Document *doc)
{
	kdDebug() << "\t"<< doc->docName() << endl;
	Kate::View *view;
	view = (Kate::View*) doc->createView (m_tabs, 0L);

	//install event filter on the view
	view->installEventFilter(m_ki->eventFilter());

	//insert the view in the tab widget
	m_tabs->addTab( view, m_ki->getShortName(doc) );
	m_tabs->showPage( view );
	m_viewList.append(view);

	connect(view, SIGNAL(viewStatusMsg(const QString&)), m_receiver, SLOT(newStatus(const QString&)));
	connect(view, SIGNAL(newStatus()), m_receiver, SLOT(newCaption()));

	connect( doc,  SIGNAL(charactersInteractivelyInserted (int,int,const QString&)), m_ki->editorExtension()->complete(),  SLOT(slotCharactersInserted(int,int,const QString&)) );
	connect( view, SIGNAL(completionDone()), m_ki->editorExtension()->complete(),  SLOT( slotCompletionDone()) );
	connect( view, SIGNAL(completionAborted()), m_ki->editorExtension()->complete(),  SLOT( slotCompletionAborted()) );
	connect( view, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)), m_ki->editorExtension()->complete(),  SLOT(slotFilterCompletion(KTextEditor::CompletionEntry*,QString *)) );

	// install a working kate part popup dialog thingy
	if (static_cast<Kate::View*>(view->qt_cast("Kate::View")))
		static_cast<Kate::View*>(view->qt_cast("Kate::View"))->installPopup((QPopupMenu*)(m_client->factory()->container("ktexteditor_popup", m_client)) );

	//activate the newly created view
// 	activateView(view, false, false);
	emit(activateView(view, false, false));

// 	newStatus();
// 	newCaption();
// 	emit(view->newStatus());

	view->setFocusPolicy(QWidget::StrongFocus);
	view->setFocus();

// 	if ( m_currentState != "Editor" ) prepareForPart("Editor");
	emit(prepareForPart("Editor"));

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

		if (views().isEmpty()) m_ki->structureWidget()->clear();
	}
}

void Manager::removeFromProjectView(const KURL & url)
{
	m_projectview->remove(url);
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

void Manager::updateStructure(bool parse /* = false */, KileDocumentInfo *docinfo /* = 0L */)
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

};

#include "kileviewmanager.moc"
