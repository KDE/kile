/**********************************************************************

	--- Qt Architect generated file ---

	File: fileLegendTitleData.cpp

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 *********************************************************************/

#include "fileLegendTitleData.h"

#include <qlabel.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>

fileLegendTitleData::fileLegendTitleData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE, 0 )
{
	QLabel* Label_1;
	Label_1 = new QLabel( this, "Label_1" );
	Label_1->setMinimumSize( 110, 20 );
	Label_1->setMaximumSize( 32767, 20 );
	Label_1->setText( i18n("File legend title:") );
	Label_1->setAlignment( 289 );
	Label_1->setMargin( -1 );

	fileLegendTitleEdit = new QLineEdit( this, "LineEdit_1" );
	fileLegendTitleEdit->setMinimumSize( 100, 20 );
	fileLegendTitleEdit->setMaximumSize( 32767, 20 );
	fileLegendTitleEdit->setText( "" );
	fileLegendTitleEdit->setMaxLength( 32767 );
	fileLegendTitleEdit->setEchoMode( QLineEdit::Normal );
	fileLegendTitleEdit->setFrame( TRUE );

    QButtonGroup* ButtonGroup_1;
    ButtonGroup_1 = new QButtonGroup( this, "ButtonGroup_1" );
    ButtonGroup_1->setMinimumSize( 200, 50 );
    ButtonGroup_1->setMaximumSize( 32767, 32767 );
    ButtonGroup_1->setFrameStyle( 49 );
    ButtonGroup_1->setTitle( "" );
    ButtonGroup_1->setAlignment( 1 );
    ButtonGroup_1->setExclusive( TRUE );

    defaultCButton = new QCheckBox(ButtonGroup_1, "CheckBox_2" );
    defaultCButton->setMinimumSize( 70, 20 );
    defaultCButton->setMaximumSize( 32767, 20 );
    defaultCButton->setText( i18n("&default") );
    defaultCButton->setAutoRepeat( FALSE );
    //defaultCButton->setAutoResize( FALSE );
    defaultCButton->setChecked(TRUE);

    noTitleCButton = new QCheckBox(ButtonGroup_1, "CheckBox_3" );
    noTitleCButton->setMinimumSize( 60, 20 );
    noTitleCButton->setMaximumSize( 32767, 20 );
    noTitleCButton->setText( i18n("&notitle") );
    noTitleCButton->setAutoRepeat( FALSE );
    //noTitleCButton->setAutoResize( FALSE );

	QPushButton* PushButton_1;
	PushButton_1 = new QPushButton( this, "PushButton_1" );
	PushButton_1->setMinimumSize( 100, 26 );
	connect( PushButton_1, SIGNAL(clicked()), SLOT(setFileLegendTitleOK()) );
	PushButton_1->setText( i18n("&OK") );
	PushButton_1->setAutoRepeat( FALSE );
	//PushButton_1->setAutoResize( FALSE );
    PushButton_1->setDefault(TRUE);
    PushButton_1->setAutoDefault(TRUE);

	QPushButton* PushButton_2;
	PushButton_2 = new QPushButton( this, "PushButton_2" );
	PushButton_2->setMinimumSize( 100, 26 );
	PushButton_2->setText( i18n("&Cancel") );
	PushButton_2->setAutoRepeat( FALSE );
	//PushButton_2->setAutoResize( FALSE );
    connect( PushButton_2, SIGNAL(clicked()), SLOT(reject()) );

	resize( 250,120 );

    // ------------------------ create layouts

    // main column
    QVBoxLayout* mainColLayout = new QVBoxLayout(this,5);

    // row for legend title edit box
    QHBoxLayout* titleRowLayout = new QHBoxLayout();

    // row for inside button group
    QHBoxLayout* insideBGRowLayout = new QHBoxLayout(ButtonGroup_1,5);

    // row for pushbuttons
    QHBoxLayout* buttonRowLayout = new QHBoxLayout();

    // ------------------------ assemble layouts and widgets
    mainColLayout->addLayout(titleRowLayout);
    titleRowLayout->addWidget(Label_1);
    titleRowLayout->addWidget(fileLegendTitleEdit);

    mainColLayout->addWidget(ButtonGroup_1);
    insideBGRowLayout->addStretch(1);
    insideBGRowLayout->addWidget(defaultCButton);
    insideBGRowLayout->addStretch(1);
    insideBGRowLayout->addWidget(noTitleCButton);
    insideBGRowLayout->addStretch(1);

    mainColLayout->addLayout(buttonRowLayout);
    buttonRowLayout->addStretch(1);
    buttonRowLayout->addWidget(PushButton_1);
    buttonRowLayout->addStretch(1);
    buttonRowLayout->addWidget(PushButton_2);
    buttonRowLayout->addStretch(1);

    mainColLayout->activate();

}


fileLegendTitleData::~fileLegendTitleData()
{
}
void fileLegendTitleData::setFileLegendTitleOK()
{
}

#include "fileLegendTitleData.moc"
