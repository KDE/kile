/**********************************************************************

	--- Qt Architect generated file ---

	File: latexEmtexOpData.cpp

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

#include "latexEmtexOpData.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <klocale.h>

latexEmtexOpData::latexEmtexOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
  QGridLayout *gbox = new QGridLayout( this, 4, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( this, "Label_1" );
	dlgedit_Label_1->setText( i18n("Font:") );
	dlgedit_Label_1->setAlignment( 289 );
	dlgedit_Label_1->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_1 , 0,0, Qt::AlignLeft );

	FontList = new QComboBox( FALSE, this, "ComboBox_1" );
	FontList->insertItem( "roman" );
	FontList->insertItem( "courier" );
  gbox->addWidget(FontList , 0,1, Qt::AlignLeft );


	QLabel* dlgedit_Label_2;
	dlgedit_Label_2 = new QLabel( this, "Label_2" );
	dlgedit_Label_2->setText( i18n("Size:") );
	dlgedit_Label_2->setAlignment( 289 );
	dlgedit_Label_2->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_2 , 1,0, Qt::AlignLeft );

	FontSizeList = new QComboBox( FALSE, this, "ComboBox_2" );
	FontSizeList->insertItem( "10" );
	FontSizeList->insertItem( "11" );
	FontSizeList->insertItem( "12" );
  gbox->addWidget(FontSizeList , 1,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_3;
	dlgedit_Label_3 = new QLabel( this, "Label_3" );
	dlgedit_Label_3->setText( i18n("Other:") );
	dlgedit_Label_3->setAlignment( 289 );
	dlgedit_Label_3->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_3 , 2,0, Qt::AlignLeft );

	FontSizeOther = new QLineEdit( this, "LineEdit_1" );
	FontSizeOther->setText( "" );
	FontSizeOther->setMaxLength( 32767 );
	FontSizeOther->setEchoMode( QLineEdit::Normal );
	FontSizeOther->setFrame( TRUE );
  gbox->addWidget(FontSizeOther , 2,1, Qt::AlignLeft );

	QPushButton* dlgedit_PushButton_1;
	dlgedit_PushButton_1 = new QPushButton( this, "PushButton_1" );
	dlgedit_PushButton_1->setGeometry( 80, 50, 60, 26 );
	connect( dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(setTerm()) );
	dlgedit_PushButton_1->setText( i18n("&OK") );
	dlgedit_PushButton_1->setAutoRepeat( FALSE );
    dlgedit_PushButton_1->setDefault(TRUE);
    gbox->addWidget(dlgedit_PushButton_1 , 3,0, Qt::AlignCenter );

	QPushButton* dlgedit_PushButton_2;
	dlgedit_PushButton_2 = new QPushButton( this, "PushButton_2" );
	dlgedit_PushButton_2->setGeometry( 180, 50, 60, 26 );
	connect( dlgedit_PushButton_2, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_PushButton_2->setText( i18n("&Cancel") );
	dlgedit_PushButton_2->setAutoRepeat( FALSE );
  gbox->addWidget( dlgedit_PushButton_2, 3,1, Qt::AlignCenter );

	resize( 100,100 );
}


latexEmtexOpData::~latexEmtexOpData()
{
}

void latexEmtexOpData::setTerm()
{
}

#include "latexEmtexOpData.moc"
