/***************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ABBREVIATIONINPUTDIALOG_H
#define ABBREVIATIONINPUTDIALOG_H

#include <QString>
#include <QLabel>
#include <q3popupmenu.h>

#include <klineedit.h>
#include <k3listbox.h>
#include <k3listview.h>
#include <KDialog>

#include "widgets/abbreviationview.h"

//////////////////// add/edit dialog for abbreviations ////////////////////

namespace KileDialog {

class AbbreviationInputDialog : public KDialog
{
   Q_OBJECT

public:
	AbbreviationInputDialog(KileWidget::AbbreviationView *listview, K3ListViewItem *item, int mode, const char *name=0);
	~AbbreviationInputDialog();

	void abbreviation(QString &abbrev, QString &expansion);

private:
	KileWidget::AbbreviationView *m_listview;
	K3ListViewItem *m_abbrevItem;
	KLineEdit *m_leAbbrev;
	KLineEdit *m_leExpansion;

	int m_mode;
	QString m_abbrev, m_expansion;

public Q_SLOTS:
	void slotOk();

private Q_SLOTS:
	void slotTextChanged(const QString &text);

};

}

#endif
