/**********************************************************************

	--- Qt Architect generated file ---

	File: fileFilterData.cpp

    Note* This file has been modified by hand for geometry management.

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

#include "fileFilterData.h"

#include <qlabel.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>

fileFilterData::fileFilterData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
	QHButtonGroup* QuoteButtonGroup;
	QuoteButtonGroup = new QHButtonGroup( this, "QuoteButtonGroup" );
	QuoteButtonGroup->setFrameStyle( 49 );
	QuoteButtonGroup->setTitle( i18n("Filter Command Quoting") );
	QuoteButtonGroup->setExclusive( TRUE );

	QLabel* Label_1;
	Label_1 = new QLabel(this , "Label_1" );
	Label_1->setText( i18n("Filter command:") );
	Label_1->setAlignment( 289 );
	Label_1->setMargin( -1 );

	filterEdit = new QLineEdit( this, "FilterLineEdit" );
	filterEdit->setText( "" );
	filterEdit->setMaxLength( 32767 );
	filterEdit->setEchoMode( QLineEdit::Normal );
	filterEdit->setFrame( TRUE );

	singleQuoteRB = new QRadioButton(QuoteButtonGroup,"SingleQuoteRadioButton" );
	singleQuoteRB->setText( i18n("&Single quotes") );
	singleQuoteRB->setAutoRepeat( FALSE );

	doubleQuoteRB = new QRadioButton(QuoteButtonGroup,"doubleQuoteRadioButton" );
	doubleQuoteRB->setText( i18n("&Double quotes") );
	doubleQuoteRB->setAutoRepeat( FALSE );
	doubleQuoteRB->setChecked( TRUE );

	QPushButton* InsertCurrentPushButton;
	InsertCurrentPushButton = new QPushButton( this, "InsertCurrentPushButton" );
	connect( InsertCurrentPushButton, SIGNAL(clicked()), SLOT(insertCurrentFilename()) );
	InsertCurrentPushButton->setText( i18n("Insert C&urrent Filename") );
	InsertCurrentPushButton->setAutoRepeat( FALSE );

	QPushButton* PushButton_2;
	PushButton_2 = new QPushButton( this, "PushButton_2" );
	connect( PushButton_2, SIGNAL(clicked()), SLOT(insertNewFilename()) );
	PushButton_2->setText( i18n("Insert &New Filename") );
	PushButton_2->setAutoRepeat( FALSE );

	QPushButton* PushButton_3;
	PushButton_3 = new QPushButton( this, "PushButton_3" );
	connect( PushButton_3, SIGNAL(clicked()), SLOT(setFilter()) );
	PushButton_3->setText( i18n("&OK") );
	PushButton_3->setAutoRepeat( FALSE );
  PushButton_3->setDefault(TRUE);
  PushButton_3->setAutoDefault(TRUE);

	QPushButton* PushButton_4;
	PushButton_4 = new QPushButton( this, "PushButton_4" );
	connect( PushButton_4, SIGNAL(clicked()), SLOT(reject()) );
	PushButton_4->setText( i18n("&Cancel") );
	PushButton_4->setAutoRepeat( FALSE );

    // ------------------------ create layouts

    // main column layout for whole dialog
    QVBoxLayout* mainColLayout = new QVBoxLayout(this, 5, -1, "mainCol");

    // row for filter command
    QHBoxLayout* filterCmdRowLayout = new QHBoxLayout(-1, "filterCmdRow");

    // row inside button group
//    QHBoxLayout* quoteBoxRowLayout = new QHBoxLayout(QuoteButtonGroup, 4, -1, "quoteBoxRow");

    // row for filename buttons
    QHBoxLayout* filenamePBRowLayout = new QHBoxLayout(-1, "filenamePBRow");

    // bottom row for OK/Cancel buttons
    QHBoxLayout* bottomRowLayout = new QHBoxLayout(-1, "bottomRow");

    // ----------- assemble layouts and widgets

    mainColLayout->addLayout(filterCmdRowLayout,0);
    filterCmdRowLayout->addWidget(Label_1,0);
    filterCmdRowLayout->addWidget(filterEdit,0);

    mainColLayout->addWidget(QuoteButtonGroup,0);
//    quoteBoxRowLayout->addStretch(1);
//    quoteBoxRowLayout->addWidget(doubleQuoteRB,0);
//    quoteBoxRowLayout->addStretch(1);
//    quoteBoxRowLayout->addWidget(singleQuoteRB,0);
//    quoteBoxRowLayout->addStretch(1);

    mainColLayout->addLayout(filenamePBRowLayout,0);
    filenamePBRowLayout->addStretch(1);
    filenamePBRowLayout->addWidget(InsertCurrentPushButton,0);
    filenamePBRowLayout->addStretch(1);
    filenamePBRowLayout->addWidget(PushButton_2,0);
    filenamePBRowLayout->addStretch(1);

    mainColLayout->addLayout(bottomRowLayout,0);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(PushButton_3,0);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(PushButton_4,0);
    bottomRowLayout->addStretch(1);
    
    resize(370,150);
    mainColLayout->activate();
}


fileFilterData::~fileFilterData()
{
}
void fileFilterData::insertCurrentFilename()
{
}
void fileFilterData::insertNewFilename()
{
}
void fileFilterData::setFilter()
{
}

#include "fileFilterData.moc"
