/***************************************************************************
    begin                : Tuesday Nov 15 2005
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#include <kdialog.h>
#include <klocale.h>

#include "kilestatswidget.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <kdebug.h>

KileWidgetStatistics::KileWidgetStatistics( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
	QVBoxLayout *vbox = new QVBoxLayout(parent, 5,KDialog::spacingHint() );

	// characters groupbox
	QGroupBox *chargroup = new QGroupBox( i18n("Characters"),parent); 
	chargroup->setColumnLayout(0, Qt::Vertical );
	chargroup->layout()->setSpacing( 6 );
	chargroup->layout()->setMargin( 11 );
	chargrouplayout = new QGridLayout( chargroup->layout() );
	chargrouplayout->setAlignment( Qt::AlignTop );

	m_wordCharText = new QLabel(i18n("Words and numbers:"), chargroup);
	m_commandCharText = new QLabel(i18n("LaTeX commands and environments:"), chargroup);
	m_whitespaceCharText = new QLabel(i18n("Punctuation, delimiter and whitespaces:"), chargroup);
	m_totalCharText = new QLabel(i18n("Total characters:"), chargroup);
	m_wordChar = new QLabel( chargroup, "m_wordChar" );
	m_commandChar = new QLabel( chargroup, "m_commandChar" );
	m_whitespaceChar = new QLabel( chargroup, "m_whitespaceChar" );
	m_totalChar = new QLabel( chargroup, "m_totalChar" );

	QFrame *charframe = new QFrame(chargroup);
	charframe->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	charframe->setLineWidth(1);

	chargrouplayout->addWidget( m_wordCharText, 0,0 );
	chargrouplayout->addWidget( m_commandCharText, 1,0 );
	chargrouplayout->addWidget( m_whitespaceCharText, 2,0 );
	chargrouplayout->addWidget( m_totalCharText, 4,0 );
	chargrouplayout->addWidget( m_wordChar, 0,2,Qt::AlignRight );
	chargrouplayout->addWidget( m_commandChar, 1,2,Qt::AlignRight );
	chargrouplayout->addWidget( m_whitespaceChar, 2,2,Qt::AlignRight );
	chargrouplayout->addMultiCellWidget( charframe, 3,3,1,2 );
	chargrouplayout->addWidget( m_totalChar, 4,2,Qt::AlignRight );
	chargrouplayout->setColSpacing(1,16);
	chargrouplayout->setColSpacing(3,1);
	chargrouplayout->setColStretch(3,1);

	// string groupbox
	QGroupBox *stringgroup = new QGroupBox( i18n("Strings"),parent);
	stringgroup->setColumnLayout(0, Qt::Vertical );
	stringgroup->layout()->setSpacing( 6 );
	stringgroup->layout()->setMargin( 11 );
	stringgrouplayout = new QGridLayout( stringgroup->layout() );
	stringgrouplayout->setAlignment( Qt::AlignTop );

	m_wordStringText = new QLabel(i18n("Words:"), stringgroup);
	m_commandStringText = new QLabel(i18n("LaTeX commands:"), stringgroup);
	m_environmentStringText = new QLabel(i18n("LaTeX environments:"), stringgroup);
	m_totalStringText = new QLabel(i18n("Total strings:"), stringgroup);
	m_wordString = new QLabel( stringgroup, "m_wordString" );
	m_commandString = new QLabel( stringgroup, "m_commandStringText" );
	m_environmentString = new QLabel( stringgroup, "m_environmentStringText" );
	m_totalString = new QLabel( stringgroup, "m_totalStringText" );

	QFrame *stringframe = new QFrame(stringgroup);
	stringframe->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	stringframe->setLineWidth(1);

	stringgrouplayout->addWidget( m_wordStringText, 0,0 );
	stringgrouplayout->addWidget( m_commandStringText, 1,0 );
	stringgrouplayout->addWidget( m_environmentStringText, 2,0 );
	stringgrouplayout->addWidget( m_totalStringText, 4,0 );
	stringgrouplayout->addWidget( m_wordString, 0,2,Qt::AlignRight );
	stringgrouplayout->addWidget( m_commandString, 1,2,Qt::AlignRight );
	stringgrouplayout->addWidget( m_environmentString, 2,2,Qt::AlignRight );
	stringgrouplayout->addMultiCellWidget( stringframe, 3,3,1,2 );
	stringgrouplayout->addWidget( m_totalString, 4,2,Qt::AlignRight );
	stringgrouplayout->setColSpacing(1,16);
	stringgrouplayout->setColSpacing(3,1);
	stringgrouplayout->setColStretch(3,1);

	m_commentAboutHelp = new QLabel(parent);
	m_warning =  new QLabel(parent);
 
	vbox->addWidget(chargroup);
	vbox->addWidget(stringgroup);
	vbox->addSpacing(12);
	vbox->addWidget(m_commentAboutHelp);
	vbox->addWidget(m_warning);
	vbox->addStretch(1);

	int w = m_commandCharText->sizeHint().width();
	if ( m_whitespaceCharText->sizeHint().width() > w )
		w = m_whitespaceCharText->sizeHint().width();
	stringgrouplayout->setColSpacing(0,w);

}

KileWidgetStatistics::~KileWidgetStatistics()
{
}

void KileWidgetStatistics::updateColumns()
{
	int w = m_totalChar->sizeHint().width();
	if ( m_totalString->sizeHint().width() > w )
		w = m_totalString->sizeHint().width();
	chargrouplayout->setColSpacing(2,w);
	stringgrouplayout->setColSpacing(2,w);
}


#include "kilestatswidget.moc"
