/**********************************************************************

	--- Qt Architect generated file ---

	File: legendOpData.cpp

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

#include "legendOpData.h"

#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qhbuttongroup.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbt.h>
#include <klocale.h>

legendOpData::legendOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE)
{
  QGridLayout *gbox = new QGridLayout( this, 9, 6,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QHButtonGroup* dlgedit_ButtonGroup_2;
	dlgedit_ButtonGroup_2 = new QHButtonGroup( this, "ButtonGroup_2" );
	dlgedit_ButtonGroup_2->setFrameStyle( 49 );
	dlgedit_ButtonGroup_2->setTitle( "" );
	dlgedit_ButtonGroup_2->setAlignment( 1 );
  dlgedit_ButtonGroup_2->setExclusive(TRUE);

	keyButton = new QRadioButton( dlgedit_ButtonGroup_2, "RadioButton_1" );
	keyButton->setText( i18n("&key") );
	keyButton->setAutoRepeat( FALSE );
	keyButton->setChecked( TRUE );

	noKeyButton = new QRadioButton( dlgedit_ButtonGroup_2, "RadioButton_2" );
	noKeyButton->setText( i18n("&nokey") );
	noKeyButton->setAutoRepeat( FALSE );

  gbox->addMultiCellWidget(dlgedit_ButtonGroup_2,0,0,0,2,Qt::AlignLeft);

	QVButtonGroup* dlgedit_ButtonGroup_1;
	dlgedit_ButtonGroup_1 = new QVButtonGroup( this, "ButtonGroup_1" );
	dlgedit_ButtonGroup_1->setFrameStyle( 49 );
	dlgedit_ButtonGroup_1->setTitle( i18n("Position") );
	dlgedit_ButtonGroup_1->setAlignment( 1 );

	positionLeftButton = new QCheckBox( dlgedit_ButtonGroup_1, "CheckBox_3" );
	positionLeftButton->setText( i18n("&left") );
	positionLeftButton->setAutoRepeat( FALSE );

	positionRightButton = new QCheckBox( dlgedit_ButtonGroup_1, "CheckBox_4" );
	positionRightButton->setText( i18n("&right") );
	positionRightButton->setAutoRepeat( FALSE );
	positionRightButton->setChecked( TRUE );

	positionTopButton = new QCheckBox( dlgedit_ButtonGroup_1, "CheckBox_5" );
	positionTopButton->setText( i18n("&top") );
	positionTopButton->setAutoRepeat( FALSE );
	positionTopButton->setChecked( TRUE );

	positionBottomButton = new QCheckBox( dlgedit_ButtonGroup_1, "CheckBox_6" );
	positionBottomButton->setText( i18n("&bottom") );
	positionBottomButton->setAutoRepeat( FALSE );

	positionOutsideButton = new QCheckBox( dlgedit_ButtonGroup_1, "CheckBox_7" );
	positionOutsideButton->setText( i18n("&outside") );
	positionOutsideButton->setAutoRepeat( FALSE );

	positionBelowButton = new QCheckBox( dlgedit_ButtonGroup_1, "CheckBox_8" );
	positionBelowButton->setText( i18n("belo&w") );
	positionBelowButton->setAutoRepeat( FALSE );

  gbox->addMultiCellWidget(dlgedit_ButtonGroup_1,1,7,0,0,Qt::AlignLeft);

	QLabel* dlgedit_Label_3;
	dlgedit_Label_3 = new QLabel( this, "Label_3" );
	dlgedit_Label_3->setText( i18n("Position fine tune:") );
	dlgedit_Label_3->setAlignment( 289 );
	dlgedit_Label_3->setMargin( -1 );
  gbox->addMultiCellWidget(dlgedit_Label_3,2,2,1,2,Qt::AlignLeft);

	QLabel* dlgedit_Label_4;
	dlgedit_Label_4 = new QLabel( this, "Label_4" );
	dlgedit_Label_4->setText( "X:" );
	dlgedit_Label_4->setAlignment( 289 );
	dlgedit_Label_4->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_4 , 3,1, Qt::AlignLeft );

	positionXEdit = new QLineEdit( this, "LineEdit_2" );
		positionXEdit->setText( "" );
	positionXEdit->setMaxLength( 32767 );
	positionXEdit->setEchoMode( QLineEdit::Normal );
	positionXEdit->setFrame( TRUE );
  gbox->addWidget(positionXEdit , 3,2, Qt::AlignLeft );

	QLabel* dlgedit_Label_5;
	dlgedit_Label_5 = new QLabel( this, "Label_5" );
	dlgedit_Label_5->setText( "Y:" );
	dlgedit_Label_5->setAlignment( 289 );
	dlgedit_Label_5->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_5, 5,1, Qt::AlignLeft );

	positionYEdit = new QLineEdit( this, "LineEdit_3" );
	positionYEdit->setText( "" );
	positionYEdit->setMaxLength( 32767 );
	positionYEdit->setEchoMode( QLineEdit::Normal );
	positionYEdit->setFrame( TRUE );
  gbox->addWidget(positionYEdit , 5,2, Qt::AlignLeft );

	QLabel* dlgedit_Label_6;
	dlgedit_Label_6 = new QLabel( this, "Label_6" );
	dlgedit_Label_6->setText( "Z:" );
	dlgedit_Label_6->setAlignment( 289 );
	dlgedit_Label_6->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_6 , 7,1, Qt::AlignLeft );

	positionZEdit = new QLineEdit( this, "LineEdit_4" );
	positionZEdit->setText( "" );
	positionZEdit->setMaxLength( 32767 );
	positionZEdit->setEchoMode( QLineEdit::Normal );
	positionZEdit->setFrame( TRUE );
  gbox->addWidget(positionZEdit , 7,2, Qt::AlignLeft );

	QLabel* dlgedit_Label_7;
	dlgedit_Label_7 = new QLabel( this, "Label_7" );
	dlgedit_Label_7->setText( i18n("Text justification:") );
	dlgedit_Label_7->setAlignment( 289 );
	dlgedit_Label_7->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_7 , 0,3, Qt::AlignLeft );

	textJustifyList = new QComboBox( FALSE, this, "ComboBox_1" );
	textJustifyList->insertItem( "Right" );
	textJustifyList->insertItem( "Left" );
  gbox->addWidget(textJustifyList , 0,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_8;
	dlgedit_Label_8 = new QLabel( this, "Label_8" );
	dlgedit_Label_8->setText( i18n("Reverse:") );
	dlgedit_Label_8->setAlignment( 289 );
	dlgedit_Label_8->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_8 , 1,3, Qt::AlignLeft );

	reverseList = new QComboBox( FALSE, this, "ComboBox_2" );
	reverseList->insertItem( "noreverse" );
	reverseList->insertItem( "reverse" );
  gbox->addWidget(reverseList , 1,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_9;
	dlgedit_Label_9 = new QLabel( this, "Label_9" );
	dlgedit_Label_9->setText( "Box:" );
	dlgedit_Label_9->setAlignment( 289 );
	dlgedit_Label_9->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_9 , 2,3, Qt::AlignLeft );

	boxList = new QComboBox( FALSE, this, "ComboBox_3" );
	boxList->insertItem( "nobox" );
	boxList->insertItem( "box" );
  gbox->addWidget(boxList , 2,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_10;
	dlgedit_Label_10 = new QLabel( this, "Label_10" );
	dlgedit_Label_10->setText( i18n("Box linetype:") );
	dlgedit_Label_10->setAlignment( 289 );
	dlgedit_Label_10->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_10 , 3,3, Qt::AlignLeft );

	lineTypeEdit = new QLineEdit( this, "LineEdit_5" );
	lineTypeEdit->setText( "" );
	lineTypeEdit->setMaxLength( 30 );
	lineTypeEdit->setEchoMode( QLineEdit::Normal );
	lineTypeEdit->setFrame( TRUE );
  gbox->addWidget(lineTypeEdit , 3,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_11;
	dlgedit_Label_11 = new QLabel( this, "Label_11" );
		dlgedit_Label_11->setText( i18n("Sample length:") );
	dlgedit_Label_11->setAlignment( 289 );
	dlgedit_Label_11->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_11 , 4,3, Qt::AlignLeft );

	sampleLengthEdit = new QLineEdit( this, "LineEdit_6" );
	sampleLengthEdit->setText( "4" );
	sampleLengthEdit->setMaxLength( 30 );
	sampleLengthEdit->setEchoMode( QLineEdit::Normal );
	sampleLengthEdit->setFrame( TRUE );
  gbox->addWidget(sampleLengthEdit , 4,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_12;
	dlgedit_Label_12 = new QLabel( this, "Label_12" );
	dlgedit_Label_12->setText( i18n("Spacing:") );
	dlgedit_Label_12->setAlignment( 289 );
	dlgedit_Label_12->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_12 , 5,3, Qt::AlignLeft );

	spacingEdit = new QLineEdit( this, "LineEdit_7" );
	spacingEdit->setText( "1.25" );
	spacingEdit->setMaxLength( 30 );
	spacingEdit->setEchoMode( QLineEdit::Normal );
	spacingEdit->setFrame( TRUE );
  gbox->addWidget(spacingEdit , 5,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_13;
	dlgedit_Label_13 = new QLabel( this, "Label_13" );
		dlgedit_Label_13->setText( i18n("Width increment:") );
	dlgedit_Label_13->setAlignment( 289 );
	dlgedit_Label_13->setMargin( -1 );
  gbox->addWidget( dlgedit_Label_13, 6,3, Qt::AlignLeft );

	widthIncrementEdit = new QLineEdit( this, "LineEdit_8" );
	widthIncrementEdit->setText( "" );
	widthIncrementEdit->setMaxLength( 32767 );
	widthIncrementEdit->setEchoMode( QLineEdit::Normal );
	widthIncrementEdit->setFrame( TRUE );
  gbox->addWidget(widthIncrementEdit , 6,4, Qt::AlignLeft );

	QLabel* dlgedit_Label_14;
	dlgedit_Label_14 = new QLabel( this, "Label_14" );
	dlgedit_Label_14->setText( i18n("Legend title:") );
	dlgedit_Label_14->setAlignment( 289 );
	dlgedit_Label_14->setMargin( -1 );
  gbox->addWidget( dlgedit_Label_14, 7,3, Qt::AlignLeft );

	legendTitleEdit = new QLineEdit( this, "LineEdit_9" );
	legendTitleEdit->setText( "" );
	legendTitleEdit->setMaxLength( 32767 );
	legendTitleEdit->setEchoMode( QLineEdit::Normal );
	legendTitleEdit->setFrame( TRUE );
  gbox->addWidget( legendTitleEdit, 7,4, Qt::AlignLeft );

	QPushButton* dlgedit_PushButton_1;
	dlgedit_PushButton_1 = new QPushButton( this, "PushButton_1" );
	connect( dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(setLegendOptions()) );
	dlgedit_PushButton_1->setText( i18n("&OK") );
	dlgedit_PushButton_1->setAutoRepeat( FALSE );
	dlgedit_PushButton_1->setAutoDefault( TRUE );
    dlgedit_PushButton_1->setDefault( TRUE );
    gbox->addMultiCellWidget(dlgedit_PushButton_1,8,8,0,1,Qt::AlignCenter);

	QPushButton* dlgedit_PushButton_2;
	dlgedit_PushButton_2 = new QPushButton( this, "PushButton_2" );
	connect( dlgedit_PushButton_2, SIGNAL(clicked()), SLOT(setLegendOpDefaults()) );
	dlgedit_PushButton_2->setText( i18n("&Defaults") );
	dlgedit_PushButton_2->setAutoRepeat( FALSE );
	gbox->addWidget(dlgedit_PushButton_2 , 8,2, Qt::AlignCenter );

	QPushButton* dlgedit_PushButton_3;
	dlgedit_PushButton_3 = new QPushButton( this, "PushButton_3" );
	connect( dlgedit_PushButton_3, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_PushButton_3->setText( i18n("&Cancel") );
	dlgedit_PushButton_3->setAutoRepeat( FALSE );
	gbox->addMultiCellWidget(dlgedit_PushButton_3,8,8,3,4,Qt::AlignCenter);



	resize( 440,300 );
}


legendOpData::~legendOpData()
{
}
void legendOpData::setLegendOptions()
{
}
void legendOpData::setLegendOpDefaults()
{
}



#include "legendOpData.moc"
