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
#ifndef KILESPELL2_H
#define KILESPELL2_H

#include <kdeversion.h>

#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)

#include <qobject.h>
#include <qwidget.h>

#include <kspell2/broker.h>
namespace KSpell2 { class Dialog; class BackgroundChecker; }

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

private:
	KileInfo					*m_ki;
	QWidget					*m_parent;
	int						m_spellCorrected;
	KSpell2::Broker::Ptr			m_broker;
	KSpell2::Dialog			*m_dialog;
	KSpell2::BackgroundChecker	*m_checker;
	int 						par_start, par_end, index_start, index_end;
};

#endif

#endif

