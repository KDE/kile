//
// C++ Interface: kilespell
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
#ifndef KILESPELL_H
#define KILESPELL_H

#include <qobject.h>
#include <qwidget.h>

#include <kdeversion.h>

#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)
#include <kspell2/broker.h>
namespace KSpell2 {
	class Dialog;
	class BackgroundChecker;
}
class KSpell;
#else
#include <kspell.h>
#endif

#include "kileinfo.h"

/**
@author Jeroen Wijnhout
*/
class KileSpell : public QObject
{
	Q_OBJECT

public:
	KileSpell(QWidget *parent, KileInfo *info = 0, const char *name = 0);

	~KileSpell();

public slots:
	void spellcheck();

private slots:
	void slotDone (const QString&);
	void slotCorrected (const QString & originalword, int start, const QString & newword);
	void slotMisspelling (const QString & originalword, int pos);
	void spell_started ( KSpell *);
	void spell_progress (unsigned int percent);
	void spell_done(const QString&);
	void spell_finished();
	void corrected (const QString & originalword, const QString & newword, unsigned int pos);
	void misspelling (const QString & originalword, const QStringList & suggestions,unsigned int pos);

private:
	KileInfo					*m_ki;
	QWidget					*m_parent;
	int						m_spellCorrected;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)
	KSpell2::Broker::Ptr			m_broker;
	KSpell2::Dialog			*m_dialog;
	KSpell2::BackgroundChecker	*m_checker;
	int 						par_start, par_end, index_start, index_end;
#else
	KSpell 					*kspell;
	int 						ks_corrected;
	int 						par_start, par_end, index_start, index_end;
	QString 					spell_text;
#endif
};

#endif
