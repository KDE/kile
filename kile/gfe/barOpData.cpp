/**********************************************************************

	--- Qt Architect generated file ---

	File: barOpData.cpp

   This file is part of Xgfe: X Windows GUI front end to Gnuplot
   Copyright (C) 1998 David Ishee

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 *********************************************************************/

#include "barOpData.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <klocale.h>

barOpData::barOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
  QGridLayout *gbox = new QGridLayout( this, 3, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_barLabel;
	dlgedit_barLabel = new QLabel( this, "barLabel" );
	dlgedit_barLabel->setText( i18n("Bar size:") );
	dlgedit_barLabel->setAlignment( 289 );
	dlgedit_barLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_barLabel , 0,0, Qt::AlignLeft );

	barSizeEdit = new QLineEdit( this, "barSizeLineEdit" );
	barSizeEdit->setText( "" );
	barSizeEdit->setMaxLength( 32767 );
	barSizeEdit->setEchoMode( QLineEdit::Normal );
	barSizeEdit->setFrame( TRUE );
  gbox->addWidget(barSizeEdit , 0,1, Qt::AlignLeft );

	QLabel* dlgedit_synLabel;
	dlgedit_synLabel = new QLabel( this, "synLabel" );
	dlgedit_synLabel->setText( i18n("Synonym:") );
	dlgedit_synLabel->setAlignment( 289 );
	dlgedit_synLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_synLabel , 1,0, Qt::AlignLeft );

	synList = new QComboBox( FALSE, this, "synComboBox" );
	synList->insertItem( "small" );
	synList->insertItem( "large" );
  gbox->addWidget(synList , 1,1, Qt::AlignLeft );

	QPushButton* dlgedit_okPushButton;
	dlgedit_okPushButton = new QPushButton( this, "okPushButton" );
	connect( dlgedit_okPushButton, SIGNAL(clicked()), SLOT(setBarOption()) );
	dlgedit_okPushButton->setText( i18n("&OK") );
	dlgedit_okPushButton->setAutoRepeat( FALSE );
    dlgedit_okPushButton->setAutoDefault( TRUE );
    dlgedit_okPushButton->setDefault( TRUE );
    gbox->addWidget( dlgedit_okPushButton, 2,0, Qt::AlignCenter );

	QPushButton* dlgedit_cancelPushButton;
	dlgedit_cancelPushButton = new QPushButton( this, "cancelPushButton" );
	connect( dlgedit_cancelPushButton, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_cancelPushButton->setText( i18n("&Cancel") );
	dlgedit_cancelPushButton->setAutoRepeat( FALSE );
  gbox->addWidget(dlgedit_cancelPushButton , 2,1, Qt::AlignCenter );

	resize( 230,110 );
}


barOpData::~barOpData()
{
}
void barOpData::setBarOption()
{
}

#include "barOpData.moc"
