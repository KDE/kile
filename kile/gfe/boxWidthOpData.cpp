/**********************************************************************

	--- Qt Architect generated file ---

	File: boxWidthOpData.cpp

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

#include "boxWidthOpData.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>

boxWidthOpData::boxWidthOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
  QGridLayout *gbox = new QGridLayout( this, 2, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_boxWidthLabel;
	dlgedit_boxWidthLabel = new QLabel( this, "boxWidthLabel" );
	dlgedit_boxWidthLabel->setText( "Box Width:" );
	dlgedit_boxWidthLabel->setAlignment( 289 );
	dlgedit_boxWidthLabel->setMargin( -1 );
  gbox->addWidget(dlgedit_boxWidthLabel , 0,0, Qt::AlignLeft );

	boxWidthEdit = new QLineEdit( this, "boxWidthLineEdit" );
	boxWidthEdit->setGeometry( 120, 10, 100, 20 );
	boxWidthEdit->setText( "" );
	boxWidthEdit->setMaxLength( 32767 );
	boxWidthEdit->setEchoMode( QLineEdit::Normal );
	boxWidthEdit->setFrame( TRUE );
  gbox->addWidget(boxWidthEdit , 0,1, Qt::AlignLeft );

	QPushButton* dlgedit_okPushButton;
	dlgedit_okPushButton = new QPushButton( this, "okPushButton" );
	connect( dlgedit_okPushButton, SIGNAL(clicked()), SLOT(setBoxWidth()) );
	dlgedit_okPushButton->setText( "OK" );
	dlgedit_okPushButton->setAutoRepeat( FALSE );
	dlgedit_okPushButton->setAutoDefault( TRUE );
    dlgedit_okPushButton->setDefault( TRUE );
  gbox->addWidget( dlgedit_okPushButton, 1,0, Qt::AlignCenter ); 

	QPushButton* dlgedit_cancelPushButton;
	dlgedit_cancelPushButton = new QPushButton( this, "cancelPushButton" );
	connect( dlgedit_cancelPushButton, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_cancelPushButton->setText( "Cancel" );
	dlgedit_cancelPushButton->setAutoRepeat( FALSE );
  gbox->addWidget( dlgedit_cancelPushButton, 1,1, Qt::AlignCenter ); 

	resize( 230,70 );
}


boxWidthOpData::~boxWidthOpData()
{
}
void boxWidthOpData::setBoxWidth()
{
}

#include "boxWidthOpData.moc"
