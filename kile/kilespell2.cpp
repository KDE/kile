//
// C++ Implementation: kilespell
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

#include "kilespell2.h"

#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)

#include <kspell2/dialog.h>
#include <kspell2/backgroundchecker.h>
#include <kspell2/broker.h>
#include "texfilter.h"
using namespace KSpell2;

#include <kconfig.h>
#include <kate/document.h>
#include <kate/view.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kileviewmanager.h"

KileSpell::KileSpell(QWidget *parent, KileInfo *info, const char *name) :
	QObject(parent, name),
	m_ki(info),
	m_parent(parent)
{
	m_broker = Broker::openBroker( KSharedConfig::openConfig( "kilerc" ) );
	m_checker = new BackgroundChecker( m_broker, this );
	TeXFilter *filter = new TeXFilter();
	filter->setSettings( m_broker->settings() );
	m_checker->setFilter( filter );
	m_dialog = 0;
}


KileSpell::~KileSpell()
{
}

void KileSpell::spellcheck()
{
	kdDebug() <<"==KileSpell::spellcheck()==============" << endl;

	if ( !m_ki->viewManager()->currentView() ) return;

	if ( !m_dialog )
	{
		m_dialog = new Dialog( m_checker, m_parent, "spelling dialog" );
		connect (m_dialog, SIGNAL(done(const QString&)), SLOT(slotDone(const QString&)));
		connect (m_dialog, SIGNAL(misspelling(const QString&, int)), SLOT(slotMisspelling(const QString&, int)));
		connect (m_dialog, SIGNAL(replace(const QString&, int, const QString&)),
						 SLOT(slotCorrected(const QString&, int, const QString&)));
	}

	Kate::View *view = m_ki->viewManager()->currentView();

	m_spellCorrected = 0;

	if ( view->getDoc()->hasSelection() )
	{
		m_dialog->setBuffer(view->getDoc()->selection());
		par_start = view->getDoc()->selStartLine();
		par_end =  view->getDoc()->selEndLine();
		index_start =  view->getDoc()->selStartCol();
		index_end =  view->getDoc()->selEndCol();
	}
	else
	{
		m_dialog->setBuffer(view->getDoc()->text());
		par_start=0;
		par_end=view->getDoc()->numLines()-1;
		index_start=0;
		index_end=view->getDoc()->textLine(par_end).length();
	}

	m_dialog->show();
}

void KileSpell::slotMisspelling (const QString & originalword, int pos)
{
	Kate::View *view = m_ki->viewManager()->currentView();
	if ( view == 0L ) return;

	int l=par_start;
	int cnt=0;
	int col=0;
	int p=pos+index_start;

	while ((cnt+view->getDoc()->lineLength(l)<=p) && (l < par_end))
	{
		cnt+=view->getDoc()->lineLength(l)+1;
		++l;
	}

	col=p-cnt;
	view->setCursorPosition(l,col);
	view->getDoc()->setSelection( l,col,l,col+originalword.length());
}


void KileSpell::slotCorrected (const QString & originalword, int pos, const QString & newword)
{
	Kate::View *view = m_ki->viewManager()->currentView();
	if ( view == 0L ) return;

	int l=par_start;
	int cnt=0;
	int col=0;
	int p=pos+index_start;

	if( newword != originalword )
	{
		while ((cnt+view->getDoc()->lineLength(l)<=p) && (l < par_end))
		{
			cnt+=view->getDoc()->lineLength(l)+1;
			++l;
		}

		col=p-cnt;
		view->setCursorPosition(l,col);
		view->getDoc()->setSelection( l,col,l,col+originalword.length());
		view->getDoc()->removeSelectedText();
		view->getDoc()->insertText( l,col,newword );
		view->getDoc()->setModified( true );
	}

	view->getDoc()->clearSelection();
	++m_spellCorrected;
}

void KileSpell::slotDone(const QString& /*newtext*/)
{
	m_ki->viewManager()->currentView()->getDoc()->clearSelection();
	KMessageBox::information(m_parent, i18n("Corrected 1 word.", "Corrected %n words.", m_spellCorrected),
                             i18n("Spell Checking Done"));
}

#include "kilespell2.moc"

#endif
