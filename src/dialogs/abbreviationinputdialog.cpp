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

#include "dialogs/abbreviationinputdialog.h"

#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "kiledebug.h"

#include <QTextStream>

#include <q3header.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qfile.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>

namespace KileDialog {

//////////////////// add/edit abbreviation ////////////////////

AbbreviationInputDialog::AbbreviationInputDialog(KileWidget::AbbreviationView *listview, K3ListViewItem *item, int mode, const char *name )
	: KDialog(listview), m_listview(listview), m_abbrevItem(item), m_mode(mode)
{
	setCaption(i18n("Add Abbreviation"));
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);
	setObjectName(name);

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	Q3VBoxLayout *vl = new Q3VBoxLayout(page, 0, spacingHint());

	if ( m_mode == KileWidget::AbbreviationView::ALVedit )
	{
		setCaption( i18n("Edit Abbreviation") );
		m_abbrev = m_abbrevItem->text(KileWidget::AbbreviationView::ALVabbrev);
		m_expansion = m_abbrevItem->text(KileWidget::AbbreviationView::ALVexpansion);
	}
	
	QLabel *abbrev = new QLabel(i18n("&Abbreviation:"),page);
	QLabel *expansion = new QLabel(i18n("&Expanded Text:"),page);
	m_leAbbrev = new KLineEdit(m_abbrev,page);
	m_leExpansion = new KLineEdit(m_expansion,page);

	vl->addWidget(abbrev);
	vl->addWidget(m_leAbbrev);
	vl->addWidget(expansion);
	vl->addWidget(m_leExpansion);
	vl->addSpacing(8);

	abbrev->setBuddy(m_leAbbrev);
	expansion->setBuddy(m_leExpansion);

	QRegExp reg("[a-zA-Z0-9]+");
	QRegExpValidator *abbrevValidator = new QRegExpValidator(reg,this);
	m_leAbbrev->setValidator(abbrevValidator);

	connect(m_leAbbrev,SIGNAL(textChanged(const QString &)),
	        this,SLOT(slotTextChanged(const QString &)));
	connect(m_leExpansion,SIGNAL(textChanged(const QString &)),
	        this,SLOT(slotTextChanged(const QString &)));

	slotTextChanged(QString::null);
	m_leAbbrev->setFocus();
	page->setMinimumWidth(350);
}

AbbreviationInputDialog::~AbbreviationInputDialog()
{
}

void AbbreviationInputDialog::abbreviation(QString &abbrev, QString &expansion)
{
	abbrev = m_leAbbrev->text(); 
	expansion = m_leExpansion->text().trimmed();
}

void AbbreviationInputDialog::slotTextChanged(const QString &)
{
	bool state = ( m_mode == KileWidget::AbbreviationView::ALVadd )
	           ? ! m_listview->findAbbreviation( m_leAbbrev->text() ) : true;
 	state = state && !m_leAbbrev->text().isEmpty() && !m_leExpansion->text().isEmpty();

	enableButton(Ok,state);
}

void AbbreviationInputDialog::slotOk()
{
	QString abbrev = m_leAbbrev->text();
	QString expansion = m_leExpansion->text().trimmed();

	if ( abbrev.isEmpty() || expansion.isEmpty() )
	{
		KMessageBox::error( this, i18n("Empty strings are not allowed.") );
		return;
	}

	if ( abbrev!=m_abbrev || expansion!=m_expansion )
		accept();
	else
		reject();
}

}

#include "abbreviationinputdialog.moc"
