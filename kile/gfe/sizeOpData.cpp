/**********************************************************************

	--- Qt Architect generated file ---

	File: sizeOpData.cpp

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 *********************************************************************/

#include "sizeOpData.h"

#include <qlabel.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <klocale.h>

sizeOpData::sizeOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE)
{
  QGridLayout *gbox = new QGridLayout( this, 3,2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( this, "Label_1" );
	dlgedit_Label_1->setText( i18n("Horizontal size:") );
	dlgedit_Label_1->setAlignment( 289 );
	dlgedit_Label_1->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_1 , 0,0, Qt::AlignLeft );

	hSizeEdit = new QLineEdit( this, "LineEdit_1" );
	hSizeEdit->setText( "" );
	hSizeEdit->setMaxLength( 32767 );
	hSizeEdit->setEchoMode( QLineEdit::Normal );
	hSizeEdit->setFrame( TRUE );
  gbox->addWidget( hSizeEdit, 0,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_2;
	dlgedit_Label_2 = new QLabel( this, "Label_2" );
	dlgedit_Label_2->setText( i18n("Vertical size:") );
	dlgedit_Label_2->setAlignment( 289 );
	dlgedit_Label_2->setMargin( 1 );
  gbox->addWidget(dlgedit_Label_2  , 1,0, Qt::AlignLeft );

	vSizeEdit = new QLineEdit( this, "LineEdit_2" );
	vSizeEdit->setText( "" );
	vSizeEdit->setMaxLength( 32767 );
	vSizeEdit->setEchoMode( QLineEdit::Normal );
	vSizeEdit->setFrame( TRUE );
  gbox->addWidget(vSizeEdit , 1,1, Qt::AlignLeft );

	QPushButton* dlgedit_PushButton_1;
	dlgedit_PushButton_1 = new QPushButton( this, "PushButton_1" );
	connect( dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(setSize()) );
	dlgedit_PushButton_1->setText( i18n("&OK") );
	dlgedit_PushButton_1->setAutoRepeat( FALSE );
  dlgedit_PushButton_1->setDefault(TRUE);
  gbox->addWidget(dlgedit_PushButton_1 , 2,0, Qt::AlignCenter );

	QPushButton* dlgedit_PushButton_2;
	dlgedit_PushButton_2 = new QPushButton( this, "PushButton_2" );
	connect( dlgedit_PushButton_2, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_PushButton_2->setText( i18n("&Cancel") );
	dlgedit_PushButton_2->setAutoRepeat( FALSE );
  gbox->addWidget(dlgedit_PushButton_2 , 2,1, Qt::AlignCenter );

	resize( 250,60 );
}


sizeOpData::~sizeOpData()
{
}
void sizeOpData::setSize()
{
}

#include "sizeOpData.moc"
