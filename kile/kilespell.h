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
#include <kspell.h>

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
	void spell_started ( KSpell *);
	void spell_done(const QString&);
	void spell_finished();
	void corrected (const QString & originalword, const QString & newword, unsigned int pos);
	void misspelling (const QString & originalword, const QStringList & suggestions,unsigned int pos);

private:
	KileInfo					*m_ki;
	QWidget					*m_parent;
	int						m_spellCorrected;
	KSpell 					*kspell;
	int 						ks_corrected;
	int 						par_start, par_end, index_start, index_end;
	QString 					spell_text;

};

#endif


