/**********************************************************************

	--- Qt Architect generated file ---

	File: funcLegendTitleData.cpp

    Note* This file has been modified by hand for geometry management

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

#include "funcLegendTitleData.h"

#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qlayout.h>

funcLegendTitleData::funcLegendTitleData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE, 0 )
{
	QButtonGroup* ButtonGroup_2;
	ButtonGroup_2 = new QButtonGroup( this, "ButtonGroup_2" );
	ButtonGroup_2->setMinimumSize( 210, 50 );
	ButtonGroup_2->setMaximumSize( 32767, 32767 );
	ButtonGroup_2->setFrameStyle( 49 );
	ButtonGroup_2->setTitle( "" );
	ButtonGroup_2->setAlignment( 1 );
	ButtonGroup_2->setExclusive( TRUE );

	QLabel* Label_3;
	Label_3 = new QLabel( this, "Label_3" );
	Label_3->setMinimumSize( 140, 20 );
	Label_3->setMaximumSize( 32767, 20 );
	Label_3->setText( "Function Legend Title:" );
	Label_3->setAlignment( 289 );
	Label_3->setMargin( -1 );

	funcLegendTitleEdit = new QLineEdit( this, "LineEdit_3" );
	funcLegendTitleEdit->setMinimumSize( 150, 20 );
	funcLegendTitleEdit->setMaximumSize( 32767, 20 );
	funcLegendTitleEdit->setText( "" );
	funcLegendTitleEdit->setMaxLength( 32767 );
	funcLegendTitleEdit->setEchoMode( QLineEdit::Normal );
	funcLegendTitleEdit->setFrame( TRUE );

	defaultCButton = new QCheckBox(ButtonGroup_2, "CheckBox_4" );
	defaultCButton->setMinimumSize( 70, 20 );
	defaultCButton->setMaximumSize( 32767, 20 );
	defaultCButton->setText( "&default" );
	defaultCButton->setAutoRepeat( FALSE );
	defaultCButton->setAutoResize( FALSE );
	defaultCButton->setChecked( TRUE );

	notitleCButton = new QCheckBox(ButtonGroup_2, "CheckBox_5" );
	notitleCButton->setMinimumSize( 60, 20 );
	notitleCButton->setMaximumSize( 32767, 20 );
	notitleCButton->setText( "&notitle" );
	notitleCButton->setAutoRepeat( FALSE );
	notitleCButton->setAutoResize( FALSE );

	QPushButton* PushButton_5;
	PushButton_5 = new QPushButton( this, "PushButton_5" );
	PushButton_5->setMinimumSize( 100, 26 );
	connect( PushButton_5, SIGNAL(clicked()), SLOT(setFuncLegendTitleOK()) );
	PushButton_5->setText( "OK" );
	PushButton_5->setAutoRepeat( FALSE );
	PushButton_5->setAutoResize( FALSE );
    PushButton_5->setDefault(TRUE);
    PushButton_5->setAutoDefault(TRUE);

	QPushButton* PushButton_6;
	PushButton_6 = new QPushButton( this, "PushButton_6" );
	PushButton_6->setMinimumSize( 100, 26 );
	connect( PushButton_6, SIGNAL(clicked()), SLOT(reject()) );
	PushButton_6->setText( "&Cancel" );
	PushButton_6->setAutoRepeat( FALSE );
	PushButton_6->setAutoResize( FALSE );

	resize( 320,120 );

    // ------------------------ create layouts

    // main column
    QVBoxLayout* mainColLayout = new QVBoxLayout(this,5);

    // row for legend title edit box
    QHBoxLayout* titleRowLayout = new QHBoxLayout();
    
    // row for inside button group
    QHBoxLayout* insideBGRowLayout = new QHBoxLayout(ButtonGroup_2,5);
    
    // row for pushbuttons
    QHBoxLayout* buttonRowLayout = new QHBoxLayout();
    
    // ------------------------ assemble layouts and widgets
    mainColLayout->addLayout(titleRowLayout);
    titleRowLayout->addWidget(Label_3);
    titleRowLayout->addWidget(funcLegendTitleEdit);

    mainColLayout->addWidget(ButtonGroup_2);
    insideBGRowLayout->addStretch(1);
    insideBGRowLayout->addWidget(defaultCButton);
    insideBGRowLayout->addStretch(1);
    insideBGRowLayout->addWidget(notitleCButton);
    insideBGRowLayout->addStretch(1);

    mainColLayout->addLayout(buttonRowLayout);
    buttonRowLayout->addStretch(1);
    buttonRowLayout->addWidget(PushButton_5);
    buttonRowLayout->addStretch(1);
    buttonRowLayout->addWidget(PushButton_6);
    buttonRowLayout->addStretch(1);

    mainColLayout->activate();
}


funcLegendTitleData::~funcLegendTitleData()
{
}
void funcLegendTitleData::setFuncLegendTitleOK()
{
}

#include "funcLegendTitleData.moc"
