/**********************************************************************

	--- Qt Architect generated file ---

	File: ticsLevelData.cpp

    Xgfe: X Windows GUI front end to Gnuplot
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 *********************************************************************/

#include "ticsLevelData.h"

#include <qlabel.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <klocale.h>

ticsLevelData::ticsLevelData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
  QGridLayout *gbox = new QGridLayout( this, 2,2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_ticsLevelLabel;
	dlgedit_ticsLevelLabel = new QLabel( this, "ticsLevelLabel" );
	dlgedit_ticsLevelLabel->setText( i18n("Tics level:") );
	dlgedit_ticsLevelLabel->setAlignment( 289 );
	dlgedit_ticsLevelLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_ticsLevelLabel , 0,0, Qt::AlignLeft );

	ticsLevelEdit = new QLineEdit( this, "ticsLevelLineEdit" );
	ticsLevelEdit->setText( "" );
	ticsLevelEdit->setMaxLength( 32767 );
	ticsLevelEdit->setEchoMode( QLineEdit::Normal );
	ticsLevelEdit->setFrame( TRUE );
  gbox->addWidget( ticsLevelEdit, 0,1, Qt::AlignLeft );

	QPushButton* dlgedit_OKPushButton;
	dlgedit_OKPushButton = new QPushButton( this, "OKPushButton" );
	connect( dlgedit_OKPushButton, SIGNAL(clicked()), SLOT(setTicsLevel()) );
	dlgedit_OKPushButton->setText( i18n("&OK") );
	dlgedit_OKPushButton->setAutoRepeat( FALSE );
  dlgedit_OKPushButton->setDefault( TRUE );
	dlgedit_OKPushButton->setAutoDefault( TRUE );
  gbox->addWidget(dlgedit_OKPushButton , 1,0, Qt::AlignCenter );

	QPushButton* dlgedit_cancelPushButton;
	dlgedit_cancelPushButton = new QPushButton( this, "cancelPushButton" );
	connect( dlgedit_cancelPushButton, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_cancelPushButton->setText( i18n("&Cancel") );
	dlgedit_cancelPushButton->setAutoRepeat( FALSE );
  gbox->addWidget( dlgedit_cancelPushButton, 1,1, Qt::AlignCenter );

	resize( 200,50 );
}


ticsLevelData::~ticsLevelData()
{
}
void ticsLevelData::setTicsLevel()
{
}

#include "ticsLevelData.moc"
