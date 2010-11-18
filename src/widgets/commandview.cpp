/****************************************************************************************
    begin                : June 12 2009
    copyright            : (C) 2009 dani
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "commandview.h"

#include <KLocale>
#include <KTextEditor/View>

#include "kileconfig.h"
#include "kileviewmanager.h"
#include "kiledebug.h"


namespace KileWidget {

//-------------------- CommandView --------------------

CommandView::CommandView(QWidget *parent, const QString &title, const char *name)
		: QListWidget(parent)
{
	setObjectName(name);
	setViewMode(ListMode);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setSortingEnabled(true);
	setDragDropMode(NoDragDrop);

	m_title = title;
	connect(this, SIGNAL(itemActivated(QListWidgetItem*)), parent, SLOT(slotItemActivated(QListWidgetItem*)));
	KILE_DEBUG() << "connect view: " << m_title;
}

CommandView::~CommandView()
{
	KILE_DEBUG() << "disconnect view: " << m_title;
}

//-------------------- CommandViewToolBox --------------------

CommandViewToolBox::CommandViewToolBox(KileInfo *ki, QWidget *parent, const char *name)
		: QToolBox(parent), m_ki(ki)
{
	setObjectName(name);
	m_viewmap = new QMap<QString,CommandView *>;
	m_activeMaps = 0;
	
	// we need a completion model for some auxiliary functions
	m_latexCompletionModel = new KileCodeCompletion::LaTeXCompletionModel(this,
	                                                                      m_ki->codeCompletionManager(),
	                                                                      m_ki->editorExtension());
}

CommandViewToolBox::~CommandViewToolBox()
{
	delete m_viewmap;
}

void CommandViewToolBox::readCommandViewFiles()
{
	clearItems();
  
	KileCodeCompletion::Manager *manager = m_ki->codeCompletionManager();
	QStringList files = KileConfig::completeTex();	
	int maxFiles = ( KileConfig::showCwlCommands() ) ? KileConfig::maxCwlFiles() : 0;	

	for(int i = 0; i<files.count() && i<maxFiles; ++i) {
		// check, if the wordlist has to be read
		QString cwlfile = manager->validCwlFile(files[i]);
		if( !cwlfile.isEmpty() ) {
			// read wordlist from cwl file
			QStringList wordlist = manager->readCWLFile("tex/" + cwlfile + ".cwl");

			// Add new commandView with entries
			if ( wordlist.size() > 0 ) {
				CommandView *view = new CommandView(this,cwlfile);
				addItem(view,cwlfile);
				m_viewmap->insert(cwlfile,view);
				for(QStringList::Iterator it = wordlist.begin(); it != wordlist.end(); ++it) 
					view->addItem(*it);
			}
		}
	}
	m_activeMaps = m_viewmap->size();
	
	if ( m_activeMaps == 0 ) {
		KILE_DEBUG() << "no completion files or view not visible";
		QString title = i18n("LaTeX commands");
		CommandView *view = new CommandView(this,title);
		addItem(view,title);
		m_viewmap->insert(title,view);
		view->addItem(i18n("No wordlists chosen"));
		view->setDisabled(true);
	}
}	

void CommandViewToolBox::clearItems()
{
	// iterate through the map and delete all CommandViews
	QMap<QString,CommandView*>::iterator it;
	for( it=m_viewmap->begin(); it!=m_viewmap->end(); ++it ) {
		if(it.value())
			delete it.value();
	}
	m_viewmap->clear();
	m_activeMaps = 0;
}

void CommandViewToolBox::slotItemActivated(QListWidgetItem *item)
{
	KTextEditor::View *view = m_ki->viewManager()->currentTextView();	
	if (view && m_activeMaps > 0 ) {
		KTextEditor::Cursor cursor = view->cursorPosition();
		
		//insert text
		int xpos,ypos;  
		QString text = m_latexCompletionModel->filterLatexCommand(item->text(),ypos,xpos);
		if ( !text.isEmpty() ) {
			emit( sendText(text) );
		
			// place cursor
			if(KileConfig::completeCursor() && (xpos > 0 || ypos > 0) ) {
				view->setCursorPosition(KTextEditor::Cursor(cursor.line() + (ypos >= 0 ? ypos : 0),
				                                            cursor.column() + (xpos >= 0 ? xpos : 0)));
			}
		}
	}
}

}

#include "commandview.moc"
