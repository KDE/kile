/**********************************************************************

	--- Qt Architect generated file ---

	File: logScaleOpData.cpp

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

#include "logScaleOpData.h"

#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <klocale.h>

logScaleOpData::logScaleOpData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE)
{
  QGridLayout *gbox = new QGridLayout( this, 3, 2,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );
	QVButtonGroup* dlgedit_logAxisButtonGroup;
	dlgedit_logAxisButtonGroup = new QVButtonGroup( this, "logAxisButtonGroup" );
	dlgedit_logAxisButtonGroup->setFrameStyle( 49 );
	dlgedit_logAxisButtonGroup->setTitle( i18n("Log Scale Axes") );

	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( this, "Label_1" );
	dlgedit_Label_1->setText( i18n("Log scale base:") );
	dlgedit_Label_1->setAlignment( 289 );
	dlgedit_Label_1->setMargin( -1 );
  gbox->addWidget(dlgedit_Label_1,0,0, Qt::AlignLeft );

	logBaseEdit = new QLineEdit( this, "logBaseEdit" );
	logBaseEdit->setText( "10" );
	logBaseEdit->setMaxLength( 10 );
	logBaseEdit->setEchoMode( QLineEdit::Normal );
	logBaseEdit->setFrame( TRUE );
  gbox->addWidget(logBaseEdit,0,1, Qt::AlignLeft );

	logAxisX = new QCheckBox( dlgedit_logAxisButtonGroup, "logAxisCheckBoxX" );
	logAxisX->setText( "X:" );
	logAxisX->setAutoRepeat( FALSE );

	logAxisY = new QCheckBox( dlgedit_logAxisButtonGroup, "logAxisCheckBoxY" );
	logAxisY->setText( "Y:" );
	logAxisY->setAutoRepeat( FALSE );

	logAxisZ = new QCheckBox( dlgedit_logAxisButtonGroup, "logAxisCheckBoxZ" );
	logAxisZ->setText( "Z:" );
	logAxisZ->setAutoRepeat( FALSE );

	logAxisX2 = new QCheckBox( dlgedit_logAxisButtonGroup, "logAxisCheckBoxX2" );
	logAxisX2->setText( "X2:" );
	logAxisX2->setAutoRepeat( FALSE );

	logAxisY2 = new QCheckBox( dlgedit_logAxisButtonGroup, "logAxisCheckBoxY2" );
	logAxisY2->setText( "Y2:" );
	logAxisY2->setAutoRepeat( FALSE );
  gbox->addMultiCellWidget(dlgedit_logAxisButtonGroup,1,1,0,1,Qt::AlignCenter);

	QPushButton* dlgedit_PushButtonOK;
	dlgedit_PushButtonOK = new QPushButton( this, "PushButtonOK" );
	connect( dlgedit_PushButtonOK, SIGNAL(clicked()), SLOT(setLogScaleOp()) );
	dlgedit_PushButtonOK->setText( i18n("&OK") );
	dlgedit_PushButtonOK->setAutoRepeat( FALSE );
    dlgedit_PushButtonOK->setAutoDefault( TRUE );
    dlgedit_PushButtonOK->setDefault( TRUE );
    gbox->addWidget(dlgedit_PushButtonOK,2,0, Qt::AlignCenter );

	QPushButton* dlgedit_PushButtonCancel;
	dlgedit_PushButtonCancel = new QPushButton( this, "PushButtonCancel" );
	connect( dlgedit_PushButtonCancel, SIGNAL(clicked()), SLOT(reject()) );
	dlgedit_PushButtonCancel->setText( i18n("&Cancel") );
	dlgedit_PushButtonCancel->setAutoRepeat( FALSE );
  gbox->addWidget(dlgedit_PushButtonCancel,2,1, Qt::AlignCenter );

	resize( 100,210 );
}


logScaleOpData::~logScaleOpData()
{
}
void logScaleOpData::setLogScaleOp()
{
}

#include "logScaleOpData.moc"
