/**********************************************************************

	--- Qt Architect generated file ---

	File: rawGnuData.cpp

    Note*: This file has been modified by hand for geometry management.

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

#include "rawGnuData.h"

#include <qpushbt.h>
#include <qlayout.h>
#include <klocale.h>

rawGnuData::rawGnuData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE)
{
	rawCommand = new KTextEdit( this, "MultiLineEdit_1" );
  rawCommand->setTextFormat(Qt::PlainText);
	rawCommand->insertAt( "",0,0 );
  rawCommand->setFocus();

	QPushButton* PushButton_1;
	PushButton_1 = new QPushButton( this, "PushButton_1" );
	connect( PushButton_1, SIGNAL(clicked()), SLOT(accept()) );
	PushButton_1->setText( i18n("&OK") );
	PushButton_1->setAutoRepeat( FALSE );
    PushButton_1->setDefault(TRUE);

	QPushButton* PushButton_2;
	PushButton_2 = new QPushButton( this, "PushButton_2" );
	connect( PushButton_2, SIGNAL(clicked()), SLOT(reject()) );
	PushButton_2->setText( i18n("&Cancel") );
	PushButton_2->setAutoRepeat( FALSE );

    resize(300,160);

    // create layouts

    // main column layout
    QVBoxLayout* mainColLayout = new QVBoxLayout(this,5);

    // row layout for pushbuttons
    QHBoxLayout* pbRowLayout = new QHBoxLayout();

    // assemble layouts
    mainColLayout->addWidget(rawCommand);

    mainColLayout->addLayout(pbRowLayout);
    pbRowLayout->addStretch(1);
    pbRowLayout->addWidget(PushButton_1);
    pbRowLayout->addStretch(1);
    pbRowLayout->addWidget(PushButton_2);
    pbRowLayout->addStretch(1);

    mainColLayout->activate();
}


rawGnuData::~rawGnuData()
{
}


#include "rawGnuData.moc"
