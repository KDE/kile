/**********************************************************************

	--- Qt Architect generated file ---

	File: isoLinesOpData.cpp

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

#include "isoLinesOpData.h"

#include <qlabel.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <klocale.h>

isoLinesOpData::isoLinesOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE )
{
	QLabel* isoULabel;
	isoULabel = new QLabel( this, "isoULabel" );
	isoULabel->setText( i18n("U isolines:") );
	isoULabel->setAlignment( 289 );
	isoULabel->setMargin( -1 );

	isoUEdit = new QLineEdit( this, "isoULineEdit" );
	isoUEdit->setText( "" );
	isoUEdit->setMaxLength( 32767 );
	isoUEdit->setEchoMode( QLineEdit::Normal );
	isoUEdit->setFrame( TRUE );

	QLabel* isoVLabel;
	isoVLabel = new QLabel( this, "isoVLabel" );
	isoVLabel->setText( i18n("V isolines:") );
	isoVLabel->setAlignment( 289 );
	isoVLabel->setMargin( -1 );

	isoVEdit = new QLineEdit( this, "isoVLineEdit" );
	isoVEdit->setText( "" );
	isoVEdit->setMaxLength( 32767 );
	isoVEdit->setEchoMode( QLineEdit::Normal );
	isoVEdit->setFrame( TRUE );

	QPushButton* OKPushButton;
	OKPushButton = new QPushButton( this, "OKPushButton" );
	connect( OKPushButton, SIGNAL(clicked()), SLOT(setIsolinesOp()) );
	OKPushButton->setText( i18n("&OK") );
	OKPushButton->setAutoRepeat( FALSE );
	OKPushButton->setAutoDefault( TRUE );

	QPushButton* defaultsPushButton;
	defaultsPushButton = new QPushButton( this, "defaultsPushButton" );
	connect( defaultsPushButton, SIGNAL(clicked()), SLOT(setIsolineDefaults()) );
	defaultsPushButton->setText( i18n("&Defaults") );
	defaultsPushButton->setAutoRepeat( FALSE );

	QPushButton* cancelPushButton;
	cancelPushButton = new QPushButton( this, "cancelPushButton" );
	connect( cancelPushButton, SIGNAL(clicked()), SLOT(reject()) );
	cancelPushButton->setText( i18n("&Cancel") );
	cancelPushButton->setAutoRepeat( FALSE );



    // ------------------------ create layouts

    // main column layout for whole dialog
    QVBoxLayout* mainCol = new QVBoxLayout(this, 5, -1, "mainCol");

    // horizontal row for labels and edit boxes
    QHBoxLayout* editRowLayout = new QHBoxLayout(-1,"editRowLayout");

    // add layout to main column layout
    mainCol->addLayout(editRowLayout,0);

    // add buttons to row layout
    editRowLayout->addWidget(isoULabel,0);
    editRowLayout->addWidget(isoUEdit,0);
    editRowLayout->addWidget(isoVLabel,0);
    editRowLayout->addWidget(isoVEdit,0);

    // create row layout for pushbuttons
    QHBoxLayout* buttonRowLayout = new QHBoxLayout(-1,"buttonRowLayout");

    // add to main column layout
    mainCol->addLayout(buttonRowLayout,0);

    // add pushbuttons to row layout
    buttonRowLayout->addWidget(OKPushButton,0);
    buttonRowLayout->addStretch(1);
    buttonRowLayout->addWidget(defaultsPushButton,0);
    buttonRowLayout->addStretch(1);
    buttonRowLayout->addWidget(cancelPushButton,0);

	resize( 340,90 );
    mainCol->activate();
}


isoLinesOpData::~isoLinesOpData()
{
}
void isoLinesOpData::setIsolinesOp()
{
}
void isoLinesOpData::setIsolineDefaults()
{
}

#include "isoLinesOpData.moc"
