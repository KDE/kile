/**********************************************************************

	--- Qt Architect generated file ---

	File: pbmOpData.cpp

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

#include "pbmOpData.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <klocale.h>

pbmOpData::pbmOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
  QGridLayout *gbox = new QGridLayout( this, 3,4,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( this, "Label_1" );
	dlgedit_Label_1->setText( i18n("Font size:") );
	dlgedit_Label_1->setAlignment( 289 );
	dlgedit_Label_1->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_1 , 0,0, Qt::AlignLeft );

	fontSize = new QComboBox( FALSE, this, "ComboBox_1" );
	fontSize->setSizeLimit( 3 );
	fontSize->insertItem( "small" );
	fontSize->insertItem( "medium" );
	fontSize->insertItem( "large" );
  gbox->addWidget(fontSize , 0,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_2;
	dlgedit_Label_2 = new QLabel( this, "Label_2" );
	dlgedit_Label_2->setText( i18n("Color mode:") );
	dlgedit_Label_2->setAlignment( 289 );
	dlgedit_Label_2->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_2 , 1,0, Qt::AlignLeft );

	colorMode = new QComboBox( FALSE, this, "ComboBox_2" );
	colorMode->insertItem( "monochrome" );
	colorMode->insertItem( "gray" );
	colorMode->insertItem( "color" );
  gbox->addWidget(colorMode , 1,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_4;
	dlgedit_Label_4 = new QLabel( this, "Label_4" );
	dlgedit_Label_4->setText( i18n("Horizontal size:") );
	dlgedit_Label_4->setAlignment( 289 );
	dlgedit_Label_4->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_4 , 0,2, Qt::AlignLeft );

	horizSize = new QLineEdit( this, "LineEdit_1" );
	horizSize->setText( "1" );
	horizSize->setMaxLength( 32767 );
	horizSize->setEchoMode( QLineEdit::Normal );
	horizSize->setFrame( TRUE );
  gbox->addWidget(horizSize , 0,3, Qt::AlignLeft );

	QLabel* dlgedit_Label_5;
	dlgedit_Label_5 = new QLabel( this, "Label_5" );
	dlgedit_Label_5->setText( i18n("Vertical size:") );
	dlgedit_Label_5->setAlignment( 289 );
	dlgedit_Label_5->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_5 , 1,2, Qt::AlignLeft );

	vertSize = new QLineEdit( this, "LineEdit_2" );
	vertSize->setText( "1" );
	vertSize->setMaxLength( 32767 );
	vertSize->setEchoMode( QLineEdit::Normal );
	vertSize->setFrame( TRUE );
  gbox->addWidget(vertSize , 1,3, Qt::AlignLeft );

	QPushButton* dlgedit_PushButton_1;
	dlgedit_PushButton_1 = new QPushButton( this, "PushButton_1" );
	connect( dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(setTerm()) );
	dlgedit_PushButton_1->setText( i18n("&OK") );
	dlgedit_PushButton_1->setAutoRepeat( FALSE );
  dlgedit_PushButton_1->setDefault(TRUE);
  gbox->addMultiCellWidget(dlgedit_PushButton_1,2,2,0,1,Qt::AlignCenter);

	QPushButton* dlgedit_PushButton_2;
	dlgedit_PushButton_2 = new QPushButton( this, "PushButton_2" );
	connect( dlgedit_PushButton_2, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_PushButton_2->setText( i18n("&Cancel") );
	dlgedit_PushButton_2->setAutoRepeat( FALSE );
  gbox->addMultiCellWidget(dlgedit_PushButton_2,2,2,2,3,Qt::AlignCenter);

	resize( 340,100 );
}


pbmOpData::~pbmOpData()
{
}
void pbmOpData::setTerm()
{
}

#include "pbmOpData.moc"
