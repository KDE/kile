/**********************************************************************

	--- Qt Architect generated file ---

	File: psOptData.cpp

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

#include "psOptData.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <klocale.h>

psOptData::psOptData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE)
{
  QGridLayout *gbox = new QGridLayout( this, 5,4,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( this, "Label_1" );
	dlgedit_Label_1->setText( i18n("Mode:") );
	dlgedit_Label_1->setAlignment( 289 );
	dlgedit_Label_1->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_1 , 0,0, Qt::AlignLeft );

	modeList = new QComboBox( FALSE, this, "ComboBox_1" );
	modeList->insertItem( "landscape" );
	modeList->insertItem( "portrait" );
	modeList->insertItem( "eps" );
  gbox->addWidget(modeList , 0,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_2;
	dlgedit_Label_2 = new QLabel( this, "Label_2" );
	dlgedit_Label_2->setText( i18n("Color:") );
	dlgedit_Label_2->setAlignment( 289 );
	dlgedit_Label_2->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_2 , 1,0, Qt::AlignLeft );

	colorList = new QComboBox( FALSE, this, "ComboBox_2" );
	colorList->insertItem( "monochrome" );
	colorList->insertItem( "color" );
  gbox->addWidget(colorList , 1,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_3;
	dlgedit_Label_3 = new QLabel( this, "Label_3" );
	dlgedit_Label_3->setText( i18n("Dashed:") );
	dlgedit_Label_3->setAlignment( 289 );
	dlgedit_Label_3->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_3 , 2,0, Qt::AlignLeft );

	dashedList = new QComboBox( FALSE, this, "ComboBox_3" );
	dashedList->insertItem( "dashed" );
	dashedList->insertItem( "solid" );
  gbox->addWidget(dashedList , 2,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_4;
	dlgedit_Label_4 = new QLabel( this, "Label_4" );
	dlgedit_Label_4->setText( i18n("Font name:") );
	dlgedit_Label_4->setAlignment( 289 );
	dlgedit_Label_4->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_4 , 0,2, Qt::AlignLeft );

	fontNameEdit = new QLineEdit( this, "LineEdit_1" );
	fontNameEdit->setText( "Helvetica" );
	fontNameEdit->setMaxLength( 32767 );
	fontNameEdit->setEchoMode( QLineEdit::Normal );
	fontNameEdit->setFrame( TRUE );
  gbox->addWidget(fontNameEdit , 0,3, Qt::AlignLeft );

	QLabel* dlgedit_Label_5;
	dlgedit_Label_5 = new QLabel( this, "Label_5" );
	dlgedit_Label_5->setText( i18n("Font size:") );
	dlgedit_Label_5->setAlignment( 289 );
	dlgedit_Label_5->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_5 , 1,2, Qt::AlignLeft );

	fontSizeEdit = new QLineEdit( this, "LineEdit_2" );
	fontSizeEdit->setText( "14" );
	fontSizeEdit->setMaxLength( 32767 );
	fontSizeEdit->setEchoMode( QLineEdit::Normal );
	fontSizeEdit->setFrame( TRUE );
  gbox->addWidget(fontSizeEdit , 1,3, Qt::AlignLeft );

	QLabel* dlgedit_Label_8;
	dlgedit_Label_8 = new QLabel( this, "Label_8" );
	dlgedit_Label_8->setText( i18n("Enhanced:") );
	dlgedit_Label_8->setAlignment( 289 );
	dlgedit_Label_8->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_8 , 2,2, Qt::AlignLeft );

	enhancedList = new QComboBox( FALSE, this, "ComboBox_4" );
	enhancedList->insertItem( "noenhanced" );
	enhancedList->insertItem( "enhanced" );
  gbox->addWidget(enhancedList , 2,3, Qt::AlignLeft );

	QLabel* dlgedit_Label_6;
	dlgedit_Label_6 = new QLabel( this, "Label_6" );
	dlgedit_Label_6->setText( i18n("Horizontal size:") );
	dlgedit_Label_6->setAlignment( 289 );
	dlgedit_Label_6->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_6 , 3,0, Qt::AlignLeft );

	horizSize = new QLineEdit( this, "LineEdit_3" );
	horizSize->setText( "1" );
	horizSize->setMaxLength( 32767 );
	horizSize->setEchoMode( QLineEdit::Normal );
	horizSize->setFrame( TRUE );
  gbox->addWidget(horizSize , 3,1, Qt::AlignLeft );

	QLabel* dlgedit_Label_7;
	dlgedit_Label_7 = new QLabel( this, "Label_7" );
	dlgedit_Label_7->setText( i18n("Vertical size:") );
	dlgedit_Label_7->setAlignment( 289 );
	dlgedit_Label_7->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_7 , 3,2, Qt::AlignLeft );

	vertSize = new QLineEdit( this, "LineEdit_4" );
	vertSize->setText( "1" );
	vertSize->setMaxLength( 32767 );
	vertSize->setEchoMode( QLineEdit::Normal );
	vertSize->setFrame( TRUE );
  gbox->addWidget(vertSize, 3,3, Qt::AlignLeft );

	QPushButton* dlgedit_PushButton_1;
	dlgedit_PushButton_1 = new QPushButton( this, "PushButton_1" );
	connect( dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(setTerm()) );
	dlgedit_PushButton_1->setText( i18n("&OK") );
	dlgedit_PushButton_1->setAutoRepeat( FALSE );
  dlgedit_PushButton_1->setDefault(TRUE);
  gbox->addMultiCellWidget(dlgedit_PushButton_1,4,4,0,1,Qt::AlignCenter);

	QPushButton* dlgedit_PushButton_2;
	dlgedit_PushButton_2 = new QPushButton( this, "PushButton_2" );
	connect( dlgedit_PushButton_2, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_PushButton_2->setText( i18n("&Cancel") );
	dlgedit_PushButton_2->setAutoRepeat( FALSE );
  gbox->addMultiCellWidget(dlgedit_PushButton_2,4,4,2,3,Qt::AlignCenter);

	resize( 340,150 );
}


psOptData::~psOptData()
{
}
void psOptData::setTerm()
{
}

#include "psOptData.moc"
