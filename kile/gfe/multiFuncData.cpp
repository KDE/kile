/* -------------------------- multiFuncData class --------------------------

   This class handles all operations related to the storage and manipulation of
   multiple functions and their options from the GUI.

   Note* This file has been modified by hand for geometry management

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   ------------------------------------------------------------------------*/

#include "multiFuncData.h"

#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <klocale.h>

multiFuncData::multiFuncData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE, 45056 )
{
	QLabel* Label_7;
	Label_7 = new QLabel( this, "Label_7" );
	Label_7->setMinimumSize( 90, 20 );
	Label_7->setMaximumSize( 32767, 20 );
	Label_7->setText( i18n("Functions:") );
	Label_7->setAlignment( 289 );
	Label_7->setMargin( -1 );

	multiFuncList = new QComboBox( FALSE, this, "ComboBox_3" );
	multiFuncList->setMinimumSize( 340, 20 );
	multiFuncList->setMaximumSize( 32767, 20 );
	connect( multiFuncList, SIGNAL(activated(const QString&)), SLOT(funcChanged(const QString&)) );
	multiFuncList->setSizeLimit( 100 );
	//multiFuncList->setAutoResize( FALSE );

	QLabel* Label_16;
	Label_16 = new QLabel( this, "Label_16" );
	Label_16->setMinimumSize( 90, 20 );
	Label_16->setMaximumSize( 32767, 20 );
	Label_16->setText( i18n("Edit function:") );
	Label_16->setAlignment( 289 );
	Label_16->setMargin( -1 );

	functionEdit = new QLineEdit( this, "LineEdit_1" );
	functionEdit->setMinimumSize( 340, 20 );
	functionEdit->setMaximumSize( 32767, 20 );
	functionEdit->setText( "" );
	functionEdit->setMaxLength( 32767 );
	functionEdit->setEchoMode( QLineEdit::Normal );
	functionEdit->setFrame( TRUE );

	QLabel* Label_15;
	Label_15 = new QLabel( this, "Label_15" );
	Label_15->setMinimumSize( 90, 20 );
	Label_15->setMaximumSize( 32767, 20 );
	Label_15->setText( i18n("Style:") );
	Label_15->setAlignment( 289 );
	Label_15->setMargin( -1 );

	funcStyleList = new QComboBox( FALSE, this, "ComboBox_6" );
	funcStyleList->setMinimumSize( 100, 20 );
	funcStyleList->setMaximumSize( 32767, 20 );
	funcStyleList->setSizeLimit( 8 );
	//funcStyleList->setAutoResize( FALSE );
	funcStyleList->insertItem( "points" );
	funcStyleList->insertItem( "lines" );
	funcStyleList->insertItem( "linespoints" );
	funcStyleList->insertItem( "impulses" );
	funcStyleList->insertItem( "dots" );
	funcStyleList->insertItem( "steps" );
	funcStyleList->insertItem( "errorbars" );
	funcStyleList->insertItem( "boxes" );
    funcStyleList->setCurrentItem(1);

    QButtonGroup* ButtonGroup_3;
    ButtonGroup_3 = new QButtonGroup( this, "ButtonGroup_3" );
    ButtonGroup_3->setMinimumSize( 430, 90 );
    ButtonGroup_3->setMaximumSize( 32767, 32767 );
    ButtonGroup_3->setFrameStyle( 49 );
    ButtonGroup_3->setTitle( "" );
    ButtonGroup_3->setAlignment( 1 );
    ButtonGroup_3->setExclusive( TRUE );

    QLabel* Label_17;
    Label_17 = new QLabel(ButtonGroup_3, "Label_17" );
    Label_17->setMinimumSize( 140, 20 );
    Label_17->setMaximumSize( 32767, 20 );
    Label_17->setText( i18n("Function legend title:") );
    Label_17->setAlignment( 289 );
    Label_17->setMargin( -1 );

    legendTitleEdit = new QLineEdit(ButtonGroup_3, "LineEdit_9" );
    legendTitleEdit->setMinimumSize( 270, 20 );
    legendTitleEdit->setMaximumSize( 32767, 20 );
    legendTitleEdit->setText( "" );
    legendTitleEdit->setMaxLength( 32767 );
    legendTitleEdit->setEchoMode( QLineEdit::Normal );
    legendTitleEdit->setFrame( TRUE );

    legendTitleDefaultButton = new QCheckBox(ButtonGroup_3, "CheckBox_6" );
    legendTitleDefaultButton->setMinimumSize( 70, 20 );
    legendTitleDefaultButton->setMaximumSize( 32767, 20 );
    legendTitleDefaultButton->setText( "&default" );
    legendTitleDefaultButton->setAutoRepeat( FALSE );
    //legendTitleDefaultButton->setAutoResize( FALSE );
    legendTitleDefaultButton->setChecked( TRUE );

    legendTitlenotitleButton = new QCheckBox(ButtonGroup_3, "CheckBox_7" );
    legendTitlenotitleButton->setMinimumSize( 60, 20 );
    legendTitlenotitleButton->setMaximumSize( 32767, 20 );
    legendTitlenotitleButton->setText( "&notitle" );
    legendTitlenotitleButton->setAutoRepeat( FALSE );
    //legendTitlenotitleButton->setAutoResize( FALSE );

	QPushButton* PushButton_6;
	PushButton_6 = new QPushButton( this, "PushButton_6" );
	PushButton_6->setMinimumSize( 100, 26 );
	connect( PushButton_6, SIGNAL(clicked()), SLOT(insertNewFunction()) );
	PushButton_6->setText( i18n("&Add Function") );
	PushButton_6->setAutoRepeat( FALSE );
	//PushButton_6->setAutoResize( FALSE );
	PushButton_6->setDefault( TRUE );

	QPushButton* PushButton_7;
	PushButton_7 = new QPushButton( this, "PushButton_7" );
	PushButton_7->setMinimumSize( 100, 26 );
	connect( PushButton_7, SIGNAL(clicked()), SLOT(deleteFunction()) );
	PushButton_7->setText( i18n("Delete &Function") );
	PushButton_7->setAutoRepeat( FALSE );
	//PushButton_7->setAutoResize( FALSE );

	QPushButton* PushButton_12;
	PushButton_12 = new QPushButton( this, "PushButton_12" );
	PushButton_12->setMinimumSize( 100, 26 );
	connect( PushButton_12, SIGNAL(clicked()), SLOT(setFuncOptions()) );
	PushButton_12->setText( i18n("&Modify Options") );
	PushButton_12->setAutoRepeat( FALSE );
	//PushButton_12->setAutoResize( FALSE );

	QPushButton* PushButton_13;
	PushButton_13 = new QPushButton( this, "PushButton_13" );
	PushButton_13->setMinimumSize( 100, 26 );
	connect( PushButton_13, SIGNAL(clicked()), SLOT(closeMultiFunc()) );
	PushButton_13->setText( i18n("&Close") );
	PushButton_13->setAutoRepeat( FALSE );
	//PushButton_13->setAutoResize( FALSE );

    // ------------------------ create layouts

    // main column layout for whole dialog
    QVBoxLayout* mainColLayout = new QVBoxLayout(this,5);

    // row for functions list box
    QHBoxLayout* funcListRowLayout = new QHBoxLayout();

    // row for edit function
    QHBoxLayout* editFuncRowLayout = new QHBoxLayout();

    // row for style
    QHBoxLayout* styleRowLayout = new QHBoxLayout();

    // column for inside button group
    QVBoxLayout* insideBGColLayout = new QVBoxLayout(ButtonGroup_3, 5);

    // row for function legend title
    QHBoxLayout* funcLegendTitleRowLayout = new QHBoxLayout();

    // row for check buttons
    QHBoxLayout* cbRowLayout = new QHBoxLayout();

    // bottom row for push buttons
    QHBoxLayout* bottomRowLayout = new QHBoxLayout();

    // ------------------------ assemble layouts and widgets

    mainColLayout->addLayout(funcListRowLayout,1);
    funcListRowLayout->addWidget(Label_7,0);
    funcListRowLayout->addWidget(multiFuncList,0);

    mainColLayout->addLayout(editFuncRowLayout,1);
    editFuncRowLayout->addWidget(Label_16,0);
    editFuncRowLayout->addWidget(functionEdit,0);

    mainColLayout->addLayout(styleRowLayout,1);
    styleRowLayout->addWidget(Label_15,0);
    styleRowLayout->addWidget(funcStyleList,0);
    styleRowLayout->addStretch(1);

    mainColLayout->addWidget(ButtonGroup_3,0);
    insideBGColLayout->addLayout(funcLegendTitleRowLayout,1);
    funcLegendTitleRowLayout->addWidget(Label_17,0);
    funcLegendTitleRowLayout->addWidget(legendTitleEdit,0);
    insideBGColLayout->addLayout(cbRowLayout,1);
    cbRowLayout->addStretch(1);
    cbRowLayout->addWidget(legendTitleDefaultButton,0);
    cbRowLayout->addStretch(1);
    cbRowLayout->addWidget(legendTitlenotitleButton,0);
    cbRowLayout->addStretch(1);

    mainColLayout->addLayout(bottomRowLayout,1);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(PushButton_6,0);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(PushButton_7,0);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(PushButton_12,0);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(PushButton_13,0);
    bottomRowLayout->addStretch(1);

    resize(445,200);
    mainColLayout->activate();
}


multiFuncData::~multiFuncData()
{
}
void multiFuncData::funcChanged(const QString&)
{
}
void multiFuncData::insertNewFunction()
{
}
void multiFuncData::deleteFunction()
{
}
void multiFuncData::setFuncOptions()
{
}
void multiFuncData::closeMultiFunc()
{
}

#include "multiFuncData.moc"
