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
#include "kilespell.h"

#include <kspell.h>
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
	kspell = 0;
}


KileSpell::~KileSpell()
{
}

void KileSpell::spellcheck()
{
	kdDebug() <<"==KileSpell::spellcheck()==============" << endl;

	if ( !m_ki->viewManager()->currentView() ) return;

	if ( kspell )
	{
		kdDebug() << "kspell wasn't deleted before!" << endl;
		delete kspell;
	}


	kspell = new KSpell(m_parent, i18n("Spellcheck"), this,SLOT( spell_started(KSpell *)), 0, true, false, KSpell::TeX);

	ks_corrected=0;
	connect (kspell, SIGNAL ( death()),this, SLOT ( spell_finished( )));
// 	connect (kspell, SIGNAL (progress (unsigned int)),this, SLOT (spell_progress (unsigned int)));
	connect (kspell, SIGNAL (misspelling (const QString & , const QStringList & , unsigned int )),this, SLOT (misspelling (const QString & , const QStringList & , unsigned int )));
	connect (kspell, SIGNAL (corrected (const QString & , const QString & , unsigned int )),this, SLOT (corrected (const QString & , const QString & , unsigned int )));
	connect (kspell, SIGNAL (done(const QString&)), this, SLOT (spell_done(const QString&)));
}

void KileSpell::spell_started( KSpell *)
{
	kspell->setProgressResolution(2);
	Kate::View *view = m_ki->viewManager()->currentView();

	if ( view->getDoc()->hasSelection() )
	{
		kspell->check(view->getDoc()->selection());
		par_start = view->getDoc()->selStartLine();
		par_end =  view->getDoc()->selEndLine();
		index_start =  view->getDoc()->selStartCol();
		index_end =  view->getDoc()->selEndCol();
	}
	else
	{
		kspell->check(view->getDoc()->text());
		par_start=0;
		par_end=view->getDoc()->numLines()-1;
		index_start=0;
		index_end=view->getDoc()->textLine(par_end).length();
	}
}

void KileSpell::spell_done(const QString& /*newtext*/)
{
	m_ki->viewManager()->currentView()->getDoc()->clearSelection();
	kspell->cleanUp();
	KMessageBox::information(m_parent, i18n("Corrected %1 words.").arg(ks_corrected), i18n("Spell Checking Done"));
}

void KileSpell::spell_finished( )
{
	KSpell::spellStatus status = kspell->status();

	delete kspell;
	kspell = 0;
	if (status == KSpell::Error)
		KMessageBox::sorry(m_parent, i18n("I(A)Spell could not be started."));
	else if (status == KSpell::Crashed)
	{
		m_ki->viewManager()->currentView()->getDoc()->clearSelection();
		KMessageBox::sorry(m_parent, i18n("I(A)Spell seems to have crashed."));
	}
}

void KileSpell::misspelling (const QString & originalword, const QStringList & /*suggestions*/,unsigned int pos)
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
		l++;
	}

	col=p-cnt;
	view->setCursorPosition(l,col);
	view->getDoc()->setSelection( l,col,l,col+originalword.length());
}


void KileSpell::corrected (const QString & originalword, const QString & newword, unsigned int pos)
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
			l++;
		}

		col=p-cnt;
		view->setCursorPosition(l,col);
		view->getDoc()->setSelection( l,col,l,col+originalword.length());
		view->getDoc()->removeSelectedText();
		view->getDoc()->insertText( l,col,newword );
		view->getDoc()->setModified( TRUE );
	}

	view->getDoc()->clearSelection();
	ks_corrected++;
}

#include "kilespell.moc"
