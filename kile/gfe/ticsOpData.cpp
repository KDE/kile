/**********************************************************************

	--- Qt Architect generated file ---

	File: ticsOpData.cpp

    This file has been converted to geometry management and tab dialog by
    hand. Dialog file will not re-create this exactly.

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

#include "ticsOpData.h"

#include <qlabel.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qlayout.h>
#include <klocale.h>

ticsOpData::ticsOpData
(
	QWidget* parent,
	const char* name
)
	:
	QTabDialog( parent, name, TRUE, 0 )
{

  // ------------------- page one of tab dialog: x tics -----------------

  QWidget* xticsTab = new QWidget(this, "XTics Tab");

  QButtonGroup* xticsActiveButtonGroup;
  xticsActiveButtonGroup = new QButtonGroup( xticsTab, "xticsActiveButtonGroup" );
  xticsActiveButtonGroup->setMinimumSize( 500, 60 );
  xticsActiveButtonGroup->setMaximumSize( 32767, 32767 );
  xticsActiveButtonGroup->setFrameStyle( 49 );
  xticsActiveButtonGroup->setTitle( i18n("X Tics Activation") );
  xticsActiveButtonGroup->setExclusive( TRUE );

  xticsOnRButton = new QRadioButton( xticsActiveButtonGroup, "xticsOnRadioButton" );
  xticsOnRButton->setMinimumSize( 90, 20 );
  xticsOnRButton->setMaximumSize( 32767, 20 );
  xticsOnRButton->setText( i18n("X Tics on") );
  xticsOnRButton->setAutoRepeat( FALSE );
  //xticsOnRButton->setAutoResize( FALSE );
  xticsOnRButton->setChecked( TRUE );

  xticsOffRButton = new QRadioButton( xticsActiveButtonGroup, "xticsOffRadioButton" );
  xticsOffRButton->setMinimumSize( 100, 20 );
  xticsOffRButton->setMaximumSize( 32767, 20 );
  xticsOffRButton->setText( i18n("X Tics off") );
  xticsOffRButton->setAutoRepeat( FALSE );
  //xticsOffRButton->setAutoResize( FALSE );

  QLabel* xticsLocationLabel;
  xticsLocationLabel = new QLabel( xticsTab, "xticsLocationLabel" );
  xticsLocationLabel->setMinimumSize( 60, 20 );
  xticsLocationLabel->setMaximumSize( 32767, 20 );
  xticsLocationLabel->setText( i18n("Location:") );
  xticsLocationLabel->setAlignment( 289 );
  xticsLocationLabel->setMargin( -1 );

  xticsLocationCBox = new QComboBox( FALSE, xticsTab, "ComboBox_1" );
  xticsLocationCBox->setMinimumSize( 100, 20 );
  xticsLocationCBox->setMaximumSize( 32767, 20 );
  xticsLocationCBox->setSizeLimit( 2 );
  //xticsLocationCBox->setAutoResize( FALSE );
  xticsLocationCBox->insertItem( "border" );
  xticsLocationCBox->insertItem( "axis" );

  QLabel* xticsMirrorLabel;
  xticsMirrorLabel = new QLabel( xticsTab, "xticsMirrorLabel" );
  xticsMirrorLabel->setMinimumSize( 60, 20 );
  xticsMirrorLabel->setMaximumSize( 32767, 20 );
  xticsMirrorLabel->setText( i18n("Mirroring:") );
  xticsMirrorLabel->setAlignment( 289 );
  xticsMirrorLabel->setMargin( -1 );

  xticsMirrorCBox = new QComboBox( FALSE, xticsTab, "xticsMirrorComboBox" );
  xticsMirrorCBox->setMinimumSize( 100, 20 );
  xticsMirrorCBox->setMaximumSize( 32767, 20 );
  xticsMirrorCBox->setSizeLimit( 2 );
  //xticsMirrorCBox->setAutoResize( FALSE );
  xticsMirrorCBox->insertItem( "mirror" );
  xticsMirrorCBox->insertItem( "nomirror" );

  QLabel* xticsRotationLabel;
  xticsRotationLabel = new QLabel( xticsTab, "xticsRotationLabel" );
  xticsRotationLabel->setMinimumSize( 60, 20 );
  xticsRotationLabel->setMaximumSize( 32767, 20 );
  xticsRotationLabel->setText( i18n("Rotation:") );
  xticsRotationLabel->setAlignment( 289 );
  xticsRotationLabel->setMargin( -1 );

  xticsRotationCBox = new QComboBox( FALSE, xticsTab, "xticsRotationComboBox" );
  xticsRotationCBox->setMinimumSize( 100, 20 );
  xticsRotationCBox->setMaximumSize( 32767, 20 );
  xticsRotationCBox->setSizeLimit( 2 );
  //xticsRotationCBox->setAutoResize( FALSE );
  xticsRotationCBox->insertItem( "norotate" );
  xticsRotationCBox->insertItem( "rotate" );

  QButtonGroup* xticsPosButtonGroup;
  xticsPosButtonGroup = new QButtonGroup( xticsTab, "xticsPosButtonGroup" );
  xticsPosButtonGroup->setMinimumSize( 500, 110 );
  xticsPosButtonGroup->setMaximumSize( 32767, 32767 );
  xticsPosButtonGroup->setFrameStyle( 49 );
  xticsPosButtonGroup->setTitle( i18n("X Tics Position") );
  xticsPosButtonGroup->setExclusive( TRUE );

  xticsSIERadioButton = new QRadioButton(xticsPosButtonGroup , "xticsSIERadioButton" );
  xticsSIERadioButton->setMinimumSize( 150, 20 );
  xticsSIERadioButton->setMaximumSize( 32767, 20 );
  xticsSIERadioButton->setText( i18n("Start/Inc/End") );
  xticsSIERadioButton->setAutoRepeat( FALSE );
  //xticsSIERadioButton->setAutoResize( FALSE );
  xticsSIERadioButton->setChecked( TRUE );

  QLabel* xticsStartPosLabel;
  xticsStartPosLabel = new QLabel( xticsPosButtonGroup, "xticsStartPosLabel" );
  xticsStartPosLabel->setMinimumSize( 40, 20 );
  xticsStartPosLabel->setMaximumSize( 32767, 20 );
  xticsStartPosLabel->setText( i18n("Start:") );
  xticsStartPosLabel->setAlignment( 289 );
  xticsStartPosLabel->setMargin( -1 );

  xticsStartPosEdit = new QLineEdit( xticsPosButtonGroup, "xticsStartPosLineEdit" );
  xticsStartPosEdit->setMinimumSize( 30, 20 );
  xticsStartPosEdit->setMaximumSize( 32767, 20 );
  xticsStartPosEdit->setText( "" );
  xticsStartPosEdit->setMaxLength( 32767 );
  xticsStartPosEdit->setEchoMode( QLineEdit::Normal );
  xticsStartPosEdit->setFrame( TRUE );

  QLabel* xticsIncPosLabel;
  xticsIncPosLabel = new QLabel( xticsPosButtonGroup, "xticsIncPosLabel" );
  xticsIncPosLabel->setMinimumSize( 65, 20 );
  xticsIncPosLabel->setMaximumSize( 32767, 20 );
  xticsIncPosLabel->setText( i18n("Increment:") );
  xticsIncPosLabel->setAlignment( 289 );
  xticsIncPosLabel->setMargin( -1 );

  xticsIncPosEdit = new QLineEdit( xticsPosButtonGroup, "xticsIncPosLineEdit" );
  xticsIncPosEdit->setMinimumSize( 30, 20 );
  xticsIncPosEdit->setMaximumSize( 32767, 20 );
  xticsIncPosEdit->setText( "" );
  xticsIncPosEdit->setMaxLength( 32767 );
  xticsIncPosEdit->setEchoMode( QLineEdit::Normal );
  xticsIncPosEdit->setFrame( TRUE );

  QLabel* xticsEndPosLabel;
  xticsEndPosLabel = new QLabel( xticsPosButtonGroup, "xticsEndPosLabel" );
  xticsEndPosLabel->setMinimumSize( 35, 20 );
  xticsEndPosLabel->setMaximumSize( 32767, 20 );
  xticsEndPosLabel->setText( i18n("End:") );
  xticsEndPosLabel->setAlignment( 289 );
  xticsEndPosLabel->setMargin( -1 );

  xticsEndPosEdit = new QLineEdit( xticsPosButtonGroup, "xticsEndPosLineEdit" );
  xticsEndPosEdit->setMinimumSize( 30, 20 );
  xticsEndPosEdit->setMaximumSize( 32767, 20 );
  xticsEndPosEdit->setText( "" );
  xticsEndPosEdit->setMaxLength( 32767 );
  xticsEndPosEdit->setEchoMode( QLineEdit::Normal );
  xticsEndPosEdit->setFrame( TRUE );

  xticsLabelPosRButton = new QRadioButton( xticsPosButtonGroup, "xticsLabelPosRadioButton" );
  xticsLabelPosRButton->setMinimumSize( 150, 20 );
  xticsLabelPosRButton->setMaximumSize( 32767, 20 );
  xticsLabelPosRButton->setText( i18n("Labels/Positions") );
  xticsLabelPosRButton->setAutoRepeat( FALSE );
  //xticsLabelPosRButton->setAutoResize( FALSE );

  xticsLabelsPosEdit = new QLineEdit( xticsPosButtonGroup, "xticsLabelsPosLineEdit" );
  xticsLabelsPosEdit->setMinimumSize( 330, 20 );
  xticsLabelsPosEdit->setMaximumSize( 32767, 20 );
  xticsLabelsPosEdit->setText( "" );
  xticsLabelsPosEdit->setMaxLength( 32767 );
  xticsLabelsPosEdit->setEchoMode( QLineEdit::Normal );
  xticsLabelsPosEdit->setFrame( TRUE );

  // create and assemble layouts for xtics dialog

  // main col layout
  QVBoxLayout* xticsMainColLayout = new QVBoxLayout(xticsTab,5);

  // activate button group
  xticsMainColLayout->addWidget(xticsActiveButtonGroup);
  QHBoxLayout* xticsActiveRowLayout = new QHBoxLayout(xticsActiveButtonGroup,5);

  xticsActiveRowLayout->addStretch(1);
  xticsActiveRowLayout->addWidget(xticsOnRButton);
  xticsActiveRowLayout->addStretch(1);
  xticsActiveRowLayout->addWidget(xticsOffRButton);
  xticsActiveRowLayout->addStretch(1);

  // combo box row
  QHBoxLayout* xticsComboRowLayout = new QHBoxLayout();
  xticsMainColLayout->addLayout(xticsComboRowLayout);

  xticsComboRowLayout->addWidget(xticsLocationLabel);
  xticsComboRowLayout->addWidget(xticsLocationCBox);
  //xticsComboRowLayout->addStretch(1);
  xticsComboRowLayout->addWidget(xticsMirrorLabel);
  xticsComboRowLayout->addWidget(xticsMirrorCBox);
  //xticsComboRowLayout->addStretch(1);
  xticsComboRowLayout->addWidget(xticsRotationLabel);
  xticsComboRowLayout->addWidget(xticsRotationCBox);

  // position button group
  xticsMainColLayout->addWidget(xticsPosButtonGroup);

  QVBoxLayout* xticsPosColLayout = new QVBoxLayout(xticsPosButtonGroup,5);

  QHBoxLayout* xticsTopPosRowLayout = new QHBoxLayout();
  xticsPosColLayout->addLayout(xticsTopPosRowLayout);

  xticsTopPosRowLayout->addWidget(xticsSIERadioButton);
  xticsTopPosRowLayout->addWidget(xticsStartPosLabel);
  xticsTopPosRowLayout->addWidget(xticsStartPosEdit);
  //xticsTopPosRowLayout->addStretch(1);
  xticsTopPosRowLayout->addWidget(xticsIncPosLabel);
  xticsTopPosRowLayout->addWidget(xticsIncPosEdit);
  //xticsTopPosRowLayout->addStretch(1);
  xticsTopPosRowLayout->addWidget(xticsEndPosLabel);
  xticsTopPosRowLayout->addWidget(xticsEndPosEdit);

  QHBoxLayout* xticsBottomPosRowLayout = new QHBoxLayout();
  xticsPosColLayout->addLayout(xticsBottomPosRowLayout);

  xticsBottomPosRowLayout->addWidget(xticsLabelPosRButton);
  xticsBottomPosRowLayout->addWidget(xticsLabelsPosEdit);

  xticsMainColLayout->activate();

  // insert tab
  addTab(xticsTab, i18n("&X Tics"));

  // --------------------- setup page two of dialog; ytics ----------------

  QWidget* yticsTab = new QWidget(this, "Ytics Tab");

  QButtonGroup* yticsActiveButtonGroup;
  yticsActiveButtonGroup = new QButtonGroup( yticsTab, "yticsActiveButtonGroup" );
  yticsActiveButtonGroup->setMinimumSize( 500, 60 );
  yticsActiveButtonGroup->setMaximumSize( 32767, 32767 );
  yticsActiveButtonGroup->setFrameStyle( 49 );
  yticsActiveButtonGroup->setTitle( i18n("Y Tics Activation") );
  yticsActiveButtonGroup->setExclusive( TRUE );

  yticsOnRButton = new QRadioButton( yticsActiveButtonGroup, "yticsOnRadioButton" );
  yticsOnRButton->setMinimumSize( 90, 20 );
  yticsOnRButton->setMaximumSize( 32767, 20 );
  yticsOnRButton->setText( i18n("Y Tics on") );
  yticsOnRButton->setAutoRepeat( FALSE );
  //yticsOnRButton->setAutoResize( FALSE );
  yticsOnRButton->setChecked( TRUE );

  yticsOffRButton = new QRadioButton( yticsActiveButtonGroup, "yticsOffRadioButton" );
  yticsOffRButton->setMinimumSize( 100, 20 );
  yticsOffRButton->setMaximumSize( 32767, 20 );
  yticsOffRButton->setText( i18n("Y Tics off") );
  yticsOffRButton->setAutoRepeat( FALSE );
  //yticsOffRButton->setAutoResize( FALSE );

  QLabel* yticsLocationLabel;
  yticsLocationLabel = new QLabel( yticsTab, "yticsLocationLabel" );
  yticsLocationLabel->setMinimumSize( 60, 20 );
  yticsLocationLabel->setMaximumSize( 32767, 20 );
  yticsLocationLabel->setText( i18n("Location:") );
  yticsLocationLabel->setAlignment( 289 );
  yticsLocationLabel->setMargin( -1 );

  yticsLocationCBox = new QComboBox( FALSE, yticsTab, "ComboBox_1" );
  yticsLocationCBox->setMinimumSize( 100, 20 );
  yticsLocationCBox->setMaximumSize( 32767, 20 );
  yticsLocationCBox->setSizeLimit( 2 );
  //yticsLocationCBox->setAutoResize( FALSE );
  yticsLocationCBox->insertItem( "border" );
  yticsLocationCBox->insertItem( "axis" );

  QLabel* yticsMirrorLabel;
  yticsMirrorLabel = new QLabel( yticsTab, "yticsMirrorLabel" );
  yticsMirrorLabel->setMinimumSize( 60, 20 );
  yticsMirrorLabel->setMaximumSize( 32767, 20 );
  yticsMirrorLabel->setText( i18n("Mirroring:") );
  yticsMirrorLabel->setAlignment( 289 );
  yticsMirrorLabel->setMargin( -1 );

  yticsMirrorCBox = new QComboBox( FALSE, yticsTab, "yticsMirrorComboBox" );
  yticsMirrorCBox->setMinimumSize( 100, 20 );
  yticsMirrorCBox->setMaximumSize( 32767, 20 );
  yticsMirrorCBox->setSizeLimit( 2 );
  //yticsMirrorCBox->setAutoResize( FALSE );
  yticsMirrorCBox->insertItem( "mirror" );
  yticsMirrorCBox->insertItem( "nomirror" );

  QLabel* yticsRotationLabel;
  yticsRotationLabel = new QLabel( yticsTab, "yticsRotationLabel" );
  yticsRotationLabel->setMinimumSize( 60, 20 );
  yticsRotationLabel->setMaximumSize( 32767, 20 );
  yticsRotationLabel->setText( i18n("Rotation:") );
  yticsRotationLabel->setAlignment( 289 );
  yticsRotationLabel->setMargin( -1 );

  yticsRotationCBox = new QComboBox( FALSE, yticsTab, "yticsRotationComboBox" );
  yticsRotationCBox->setMinimumSize( 100, 20 );
  yticsRotationCBox->setMaximumSize( 32767, 20 );
  yticsRotationCBox->setSizeLimit( 2 );
  //yticsRotationCBox->setAutoResize( FALSE );
  yticsRotationCBox->insertItem( "norotate" );
  yticsRotationCBox->insertItem( "rotate" );

  QButtonGroup* yticsPosButtonGroup;
  yticsPosButtonGroup = new QButtonGroup( yticsTab, "yticsPosButtonGroup" );
  yticsPosButtonGroup->setMinimumSize( 500, 110 );
  yticsPosButtonGroup->setMaximumSize( 32767, 32767 );
  yticsPosButtonGroup->setFrameStyle( 49 );
  yticsPosButtonGroup->setTitle( i18n("Y Tics Position") );
  yticsPosButtonGroup->setExclusive( TRUE );

  yticsSIERadioButton = new QRadioButton(yticsPosButtonGroup , "yticsSIERadioButton" );
  yticsSIERadioButton->setMinimumSize( 150, 20 );
  yticsSIERadioButton->setMaximumSize( 32767, 20 );
  yticsSIERadioButton->setText( i18n("Start/Inc/End") );
  yticsSIERadioButton->setAutoRepeat( FALSE );
  //yticsSIERadioButton->setAutoResize( FALSE );
  yticsSIERadioButton->setChecked( TRUE );

  QLabel* yticsStartPosLabel;
  yticsStartPosLabel = new QLabel( yticsPosButtonGroup, "yticsStartPosLabel" );
  yticsStartPosLabel->setMinimumSize( 40, 20 );
  yticsStartPosLabel->setMaximumSize( 32767, 20 );
  yticsStartPosLabel->setText( i18n("Start:") );
  yticsStartPosLabel->setAlignment( 289 );
  yticsStartPosLabel->setMargin( -1 );

  yticsStartPosEdit = new QLineEdit( yticsPosButtonGroup, "yticsStartPosLineEdit" );
  yticsStartPosEdit->setMinimumSize( 30, 20 );
  yticsStartPosEdit->setMaximumSize( 32767, 20 );
  yticsStartPosEdit->setText( "" );
  yticsStartPosEdit->setMaxLength( 32767 );
  yticsStartPosEdit->setEchoMode( QLineEdit::Normal );
  yticsStartPosEdit->setFrame( TRUE );

  QLabel* yticsIncPosLabel;
  yticsIncPosLabel = new QLabel( yticsPosButtonGroup, "yticsIncPosLabel" );
  yticsIncPosLabel->setMinimumSize( 65, 20 );
  yticsIncPosLabel->setMaximumSize( 32767, 20 );
  yticsIncPosLabel->setText( i18n("Increment:") );
  yticsIncPosLabel->setAlignment( 289 );
  yticsIncPosLabel->setMargin( -1 );

  yticsIncPosEdit = new QLineEdit( yticsPosButtonGroup, "yticsIncPosLineEdit" );
  yticsIncPosEdit->setMinimumSize( 30, 20 );
  yticsIncPosEdit->setMaximumSize( 32767, 20 );
  yticsIncPosEdit->setText( "" );
  yticsIncPosEdit->setMaxLength( 32767 );
  yticsIncPosEdit->setEchoMode( QLineEdit::Normal );
  yticsIncPosEdit->setFrame( TRUE );

  QLabel* yticsEndPosLabel;
  yticsEndPosLabel = new QLabel( yticsPosButtonGroup, "yticsEndPosLabel" );
  yticsEndPosLabel->setMinimumSize( 35, 20 );
  yticsEndPosLabel->setMaximumSize( 32767, 20 );
  yticsEndPosLabel->setText( i18n("End:") );
  yticsEndPosLabel->setAlignment( 289 );
  yticsEndPosLabel->setMargin( -1 );

  yticsEndPosEdit = new QLineEdit( yticsPosButtonGroup, "yticsEndPosLineEdit" );
  yticsEndPosEdit->setMinimumSize( 30, 20 );
  yticsEndPosEdit->setMaximumSize( 32767, 20 );
  yticsEndPosEdit->setText( "" );
  yticsEndPosEdit->setMaxLength( 32767 );
  yticsEndPosEdit->setEchoMode( QLineEdit::Normal );
  yticsEndPosEdit->setFrame( TRUE );

  yticsLabelPosRButton = new QRadioButton( yticsPosButtonGroup, "yticsLabelPosRadioButton" );
  yticsLabelPosRButton->setMinimumSize( 150, 20 );
  yticsLabelPosRButton->setMaximumSize( 32767, 20 );
  yticsLabelPosRButton->setText( i18n("Labels/Positions") );
  yticsLabelPosRButton->setAutoRepeat( FALSE );
  //yticsLabelPosRButton->setAutoResize( FALSE );

  yticsLabelsPosEdit = new QLineEdit( yticsPosButtonGroup, "yticsLabelsPosLineEdit" );
  yticsLabelsPosEdit->setMinimumSize( 330, 20 );
  yticsLabelsPosEdit->setMaximumSize( 32767, 20 );
  yticsLabelsPosEdit->setText( "" );
  yticsLabelsPosEdit->setMaxLength( 32767 );
  yticsLabelsPosEdit->setEchoMode( QLineEdit::Normal );
  yticsLabelsPosEdit->setFrame( TRUE );

  // create and assemble layouts for ytics dialog

  // main col layout
  QVBoxLayout* yticsMainColLayout = new QVBoxLayout(yticsTab,5);

  // activate button group
  yticsMainColLayout->addWidget(yticsActiveButtonGroup);
  QHBoxLayout* yticsActiveRowLayout = new QHBoxLayout(yticsActiveButtonGroup,5);

  yticsActiveRowLayout->addStretch(1);
  yticsActiveRowLayout->addWidget(yticsOnRButton);
  yticsActiveRowLayout->addStretch(1);
  yticsActiveRowLayout->addWidget(yticsOffRButton);
  yticsActiveRowLayout->addStretch(1);

  // combo box row
  QHBoxLayout* yticsComboRowLayout = new QHBoxLayout();
  yticsMainColLayout->addLayout(yticsComboRowLayout);

  yticsComboRowLayout->addWidget(yticsLocationLabel);
  yticsComboRowLayout->addWidget(yticsLocationCBox);
  //yticsComboRowLayout->addStretch(1);
  yticsComboRowLayout->addWidget(yticsMirrorLabel);
  yticsComboRowLayout->addWidget(yticsMirrorCBox);
  //yticsComboRowLayout->addStretch(1);
  yticsComboRowLayout->addWidget(yticsRotationLabel);
  yticsComboRowLayout->addWidget(yticsRotationCBox);

  // position button group
  yticsMainColLayout->addWidget(yticsPosButtonGroup);

  QVBoxLayout* yticsPosColLayout = new QVBoxLayout(yticsPosButtonGroup,5);

  QHBoxLayout* yticsTopPosRowLayout = new QHBoxLayout();
  yticsPosColLayout->addLayout(yticsTopPosRowLayout);

  yticsTopPosRowLayout->addWidget(yticsSIERadioButton);
  yticsTopPosRowLayout->addWidget(yticsStartPosLabel);
  yticsTopPosRowLayout->addWidget(yticsStartPosEdit);
  //yticsTopPosRowLayout->addStretch(1);
  yticsTopPosRowLayout->addWidget(yticsIncPosLabel);
  yticsTopPosRowLayout->addWidget(yticsIncPosEdit);
  //yticsTopPosRowLayout->addStretch(1);
  yticsTopPosRowLayout->addWidget(yticsEndPosLabel);
  yticsTopPosRowLayout->addWidget(yticsEndPosEdit);

  QHBoxLayout* yticsBottomPosRowLayout = new QHBoxLayout();
  yticsPosColLayout->addLayout(yticsBottomPosRowLayout);

  yticsBottomPosRowLayout->addWidget(yticsLabelPosRButton);
  yticsBottomPosRowLayout->addWidget(yticsLabelsPosEdit);

  yticsMainColLayout->activate();

  // insert tab
  addTab(yticsTab, i18n("&Y Tics") );

  // ------------------- page three of tab dialog: z tics -----------------

  QWidget* zticsTab = new QWidget(this, "Ztics Tab");

  QButtonGroup* zticsActiveButtonGroup;
  zticsActiveButtonGroup = new QButtonGroup( zticsTab, "zticsActiveButtonGroup" );
  zticsActiveButtonGroup->setMinimumSize( 500, 60 );
  zticsActiveButtonGroup->setMaximumSize( 32767, 32767 );
  zticsActiveButtonGroup->setFrameStyle( 49 );
  zticsActiveButtonGroup->setTitle( i18n("Z Tics Activation") );
  zticsActiveButtonGroup->setExclusive( TRUE );

  zticsOnRButton = new QRadioButton( zticsActiveButtonGroup, "zticsOnRadioButton" );
  zticsOnRButton->setMinimumSize( 90, 20 );
  zticsOnRButton->setMaximumSize( 32767, 20 );
  zticsOnRButton->setText( i18n("Z Tics on") );
  zticsOnRButton->setAutoRepeat( FALSE );
  //zticsOnRButton->setAutoResize( FALSE );
  zticsOnRButton->setChecked( TRUE );

  zticsOffRButton = new QRadioButton( zticsActiveButtonGroup, "zticsOffRadioButton" );
  zticsOffRButton->setMinimumSize( 100, 20 );
  zticsOffRButton->setMaximumSize( 32767, 20 );
  zticsOffRButton->setText( i18n("Z Tics off") );
  zticsOffRButton->setAutoRepeat( FALSE );
  //zticsOffRButton->setAutoResize( FALSE );

//  QLabel* zticsLocationLabel;
//  zticsLocationLabel = new QLabel( zticsTab, "zticsLocationLabel" );
//  zticsLocationLabel->setMinimumSize( 60, 20 );
//  zticsLocationLabel->setMaximumSize( 60, 20 );
//  zticsLocationLabel->setText( "Location:" );
//  zticsLocationLabel->setAlignment( 289 );
//  zticsLocationLabel->setMargin( -1 );
//
//  zticsLocationCBox = new QComboBox( FALSE, zticsTab, "ComboBox_1" );
//  zticsLocationCBox->setMinimumSize( 100, 20 );
//  zticsLocationCBox->setMaximumSize( 100, 20 );
//  zticsLocationCBox->setSizeLimit( 2 );
//  zticsLocationCBox->setAutoResize( FALSE );
//  zticsLocationCBox->insertItem( "border" );
//  zticsLocationCBox->insertItem( "axis" );

  QLabel* zticsMirrorLabel;
  zticsMirrorLabel = new QLabel( zticsTab, "zticsMirrorLabel" );
  zticsMirrorLabel->setMinimumSize( 60, 20 );
  zticsMirrorLabel->setMaximumSize( 32767, 20 );
  zticsMirrorLabel->setText( i18n("Mirroring:") );
  zticsMirrorLabel->setAlignment( 289 );
  zticsMirrorLabel->setMargin( -1 );

  zticsMirrorCBox = new QComboBox( FALSE, zticsTab, "zticsMirrorComboBox" );
  zticsMirrorCBox->setMinimumSize( 100, 20 );
  zticsMirrorCBox->setMaximumSize( 32767, 20 );
  zticsMirrorCBox->setSizeLimit( 2 );
  //zticsMirrorCBox->setAutoResize( FALSE );
  zticsMirrorCBox->insertItem( "mirror" );
  zticsMirrorCBox->insertItem( "nomirror" );

  QLabel* zticsRotationLabel;
  zticsRotationLabel = new QLabel( zticsTab, "zticsRotationLabel" );
  zticsRotationLabel->setMinimumSize( 60, 20 );
  zticsRotationLabel->setMaximumSize( 32767, 20 );
  zticsRotationLabel->setText( i18n("Rotation:") );
  zticsRotationLabel->setAlignment( 289 );
  zticsRotationLabel->setMargin( -1 );

  zticsRotationCBox = new QComboBox( FALSE, zticsTab, "zticsRotationComboBox" );
  zticsRotationCBox->setMinimumSize( 100, 20 );
  zticsRotationCBox->setMaximumSize( 32767, 20 );
  zticsRotationCBox->setSizeLimit( 2 );
  //zticsRotationCBox->setAutoResize( FALSE );
  zticsRotationCBox->insertItem( "norotate" );
  zticsRotationCBox->insertItem( "rotate" );

  QButtonGroup* zticsPosButtonGroup;
  zticsPosButtonGroup = new QButtonGroup( zticsTab, "zticsPosButtonGroup" );
  zticsPosButtonGroup->setMinimumSize( 500, 110 );
  zticsPosButtonGroup->setMaximumSize( 32767, 32767 );
  zticsPosButtonGroup->setFrameStyle( 49 );
  zticsPosButtonGroup->setTitle( i18n("Z Tics Position") );
  zticsPosButtonGroup->setExclusive( TRUE );

  zticsSIERadioButton = new QRadioButton(zticsPosButtonGroup , "zticsSIERadioButton" );
  zticsSIERadioButton->setMinimumSize( 150, 20 );
  zticsSIERadioButton->setMaximumSize( 32767, 20 );
  zticsSIERadioButton->setText( i18n("Start/Inc/End") );
  zticsSIERadioButton->setAutoRepeat( FALSE );
  //zticsSIERadioButton->setAutoResize( FALSE );
  zticsSIERadioButton->setChecked( TRUE );

  QLabel* zticsStartPosLabel;
  zticsStartPosLabel = new QLabel( zticsPosButtonGroup, "zticsStartPosLabel" );
  zticsStartPosLabel->setMinimumSize( 40, 20 );
  zticsStartPosLabel->setMaximumSize( 32767, 20 );
  zticsStartPosLabel->setText( i18n("Start:") );
  zticsStartPosLabel->setAlignment( 289 );
  zticsStartPosLabel->setMargin( -1 );

  zticsStartPosEdit = new QLineEdit( zticsPosButtonGroup, "zticsStartPosLineEdit" );
  zticsStartPosEdit->setMinimumSize( 30, 20 );
  zticsStartPosEdit->setMaximumSize( 32767, 20 );
  zticsStartPosEdit->setText( "" );
  zticsStartPosEdit->setMaxLength( 32767 );
  zticsStartPosEdit->setEchoMode( QLineEdit::Normal );
  zticsStartPosEdit->setFrame( TRUE );

  QLabel* zticsIncPosLabel;
  zticsIncPosLabel = new QLabel( zticsPosButtonGroup, "zticsIncPosLabel" );
  zticsIncPosLabel->setMinimumSize( 65, 20 );
  zticsIncPosLabel->setMaximumSize( 32767, 20 );
  zticsIncPosLabel->setText( i18n("Increment:") );
  zticsIncPosLabel->setAlignment( 289 );
  zticsIncPosLabel->setMargin( -1 );

  zticsIncPosEdit = new QLineEdit( zticsPosButtonGroup, "zticsIncPosLineEdit" );
  zticsIncPosEdit->setMinimumSize( 30, 20 );
  zticsIncPosEdit->setMaximumSize( 32767, 20 );
  zticsIncPosEdit->setText( "" );
  zticsIncPosEdit->setMaxLength( 32767 );
  zticsIncPosEdit->setEchoMode( QLineEdit::Normal );
  zticsIncPosEdit->setFrame( TRUE );

  QLabel* zticsEndPosLabel;
  zticsEndPosLabel = new QLabel( zticsPosButtonGroup, "zticsEndPosLabel" );
  zticsEndPosLabel->setMinimumSize( 35, 20 );
  zticsEndPosLabel->setMaximumSize( 32767, 20 );
  zticsEndPosLabel->setText( i18n("End:") );
  zticsEndPosLabel->setAlignment( 289 );
  zticsEndPosLabel->setMargin( -1 );

  zticsEndPosEdit = new QLineEdit( zticsPosButtonGroup, "zticsEndPosLineEdit" );
  zticsEndPosEdit->setMinimumSize( 30, 20 );
  zticsEndPosEdit->setMaximumSize( 32767, 20 );
  zticsEndPosEdit->setText( "" );
  zticsEndPosEdit->setMaxLength( 32767 );
  zticsEndPosEdit->setEchoMode( QLineEdit::Normal );
  zticsEndPosEdit->setFrame( TRUE );

  zticsLabelPosRButton = new QRadioButton( zticsPosButtonGroup, "zticsLabelPosRadioButton" );
  zticsLabelPosRButton->setMinimumSize( 150, 20 );
  zticsLabelPosRButton->setMaximumSize( 32767, 20 );
  zticsLabelPosRButton->setText( i18n("Labels/Positions") );
  zticsLabelPosRButton->setAutoRepeat( FALSE );
  //zticsLabelPosRButton->setAutoResize( FALSE );

  zticsLabelsPosEdit = new QLineEdit( zticsPosButtonGroup, "zticsLabelsPosLineEdit" );
  zticsLabelsPosEdit->setMinimumSize( 330, 20 );
  zticsLabelsPosEdit->setMaximumSize( 32767, 20 );
  zticsLabelsPosEdit->setText( "" );
  zticsLabelsPosEdit->setMaxLength( 32767 );
  zticsLabelsPosEdit->setEchoMode( QLineEdit::Normal );
  zticsLabelsPosEdit->setFrame( TRUE );

  // create and assemble layouts for ztics dialog

  // main col layout
  QVBoxLayout* zticsMainColLayout = new QVBoxLayout(zticsTab,5);

  // activate button group
  zticsMainColLayout->addWidget(zticsActiveButtonGroup);
  QHBoxLayout* zticsActiveRowLayout = new QHBoxLayout(zticsActiveButtonGroup,5);

  zticsActiveRowLayout->addStretch(1);
  zticsActiveRowLayout->addWidget(zticsOnRButton);
  zticsActiveRowLayout->addStretch(1);
  zticsActiveRowLayout->addWidget(zticsOffRButton);
  zticsActiveRowLayout->addStretch(1);

  // combo box row
  QHBoxLayout* zticsComboRowLayout = new QHBoxLayout();
  zticsMainColLayout->addLayout(zticsComboRowLayout);

//  zticsComboRowLayout->addWidget(zticsLocationLabel);
//  zticsComboRowLayout->addWidget(zticsLocationCBox);
//  zticsComboRowLayout->addStretch(1);
  zticsComboRowLayout->addWidget(zticsMirrorLabel);
  zticsComboRowLayout->addWidget(zticsMirrorCBox);
  //zticsComboRowLayout->addStretch(1);
  zticsComboRowLayout->addWidget(zticsRotationLabel);
  zticsComboRowLayout->addWidget(zticsRotationCBox);
  //zticsComboRowLayout->addStretch(1);

  // position button group
  zticsMainColLayout->addWidget(zticsPosButtonGroup);

  QVBoxLayout* zticsPosColLayout = new QVBoxLayout(zticsPosButtonGroup,5);

  QHBoxLayout* zticsTopPosRowLayout = new QHBoxLayout();
  zticsPosColLayout->addLayout(zticsTopPosRowLayout);

  zticsTopPosRowLayout->addWidget(zticsSIERadioButton);
  zticsTopPosRowLayout->addWidget(zticsStartPosLabel);
  zticsTopPosRowLayout->addWidget(zticsStartPosEdit);
  //zticsTopPosRowLayout->addStretch(1);
  zticsTopPosRowLayout->addWidget(zticsIncPosLabel);
  zticsTopPosRowLayout->addWidget(zticsIncPosEdit);
  //zticsTopPosRowLayout->addStretch(1);
  zticsTopPosRowLayout->addWidget(zticsEndPosLabel);
  zticsTopPosRowLayout->addWidget(zticsEndPosEdit);

  QHBoxLayout* zticsBottomPosRowLayout = new QHBoxLayout();
  zticsPosColLayout->addLayout(zticsBottomPosRowLayout);

  zticsBottomPosRowLayout->addWidget(zticsLabelPosRButton);
  zticsBottomPosRowLayout->addWidget(zticsLabelsPosEdit);

  zticsMainColLayout->activate();

  // insert tab
  addTab(zticsTab, i18n("&Z Tics") );

  // ------------------- page four of tab dialog: x2 tics -----------------

  QWidget* x2ticsTab = new QWidget(this, "X2tics Tab");

  QButtonGroup* x2ticsActiveButtonGroup;
  x2ticsActiveButtonGroup = new QButtonGroup( x2ticsTab, "x2ticsActiveButtonGroup" );
  x2ticsActiveButtonGroup->setMinimumSize( 500, 60 );
  x2ticsActiveButtonGroup->setMaximumSize( 32767, 32767 );
  x2ticsActiveButtonGroup->setFrameStyle( 49 );
  x2ticsActiveButtonGroup->setTitle( i18n("X2 Tics Activation") );
  x2ticsActiveButtonGroup->setExclusive( TRUE );

  x2ticsOnRButton = new QRadioButton( x2ticsActiveButtonGroup, "x2ticsOnRadioButton" );
  x2ticsOnRButton->setMinimumSize( 90, 20 );
  x2ticsOnRButton->setMaximumSize( 32767, 20 );
  x2ticsOnRButton->setText( i18n("X2 Tics on") );
  x2ticsOnRButton->setAutoRepeat( FALSE );
  //x2ticsOnRButton->setAutoResize( FALSE );
  x2ticsOnRButton->setChecked( TRUE );

  x2ticsOffRButton = new QRadioButton( x2ticsActiveButtonGroup, "x2ticsOffRadioButton" );
  x2ticsOffRButton->setMinimumSize( 100, 20 );
  x2ticsOffRButton->setMaximumSize( 32767, 20 );
  x2ticsOffRButton->setText( i18n("X2 Tics off") );
  x2ticsOffRButton->setAutoRepeat( FALSE );
  //x2ticsOffRButton->setAutoResize( FALSE );

  QLabel* x2ticsLocationLabel;
  x2ticsLocationLabel = new QLabel( x2ticsTab, "x2ticsLocationLabel" );
  x2ticsLocationLabel->setMinimumSize( 60, 20 );
  x2ticsLocationLabel->setMaximumSize( 32767, 20 );
  x2ticsLocationLabel->setText( i18n("Location:") );
  x2ticsLocationLabel->setAlignment( 289 );
  x2ticsLocationLabel->setMargin( -1 );

  x2ticsLocationCBox = new QComboBox( FALSE, x2ticsTab, "ComboBox_1" );
  x2ticsLocationCBox->setMinimumSize( 100, 20 );
  x2ticsLocationCBox->setMaximumSize( 32767, 20 );
  x2ticsLocationCBox->setSizeLimit( 2 );
  //x2ticsLocationCBox->setAutoResize( FALSE );
  x2ticsLocationCBox->insertItem( "border" );
  x2ticsLocationCBox->insertItem( "axis" );

  QLabel* x2ticsMirrorLabel;
  x2ticsMirrorLabel = new QLabel( x2ticsTab, "x2ticsMirrorLabel" );
  x2ticsMirrorLabel->setMinimumSize( 60, 20 );
  x2ticsMirrorLabel->setMaximumSize( 32767, 20 );
  x2ticsMirrorLabel->setText( i18n("Mirroring:") );
  x2ticsMirrorLabel->setAlignment( 289 );
  x2ticsMirrorLabel->setMargin( -1 );

  x2ticsMirrorCBox = new QComboBox( FALSE, x2ticsTab, "x2ticsMirrorComboBox" );
  x2ticsMirrorCBox->setMinimumSize( 100, 20 );
  x2ticsMirrorCBox->setMaximumSize( 32767, 20 );
  x2ticsMirrorCBox->setSizeLimit( 2 );
  //x2ticsMirrorCBox->setAutoResize( FALSE );
  x2ticsMirrorCBox->insertItem( "mirror" );
  x2ticsMirrorCBox->insertItem( "nomirror" );

  QLabel* x2ticsRotationLabel;
  x2ticsRotationLabel = new QLabel( x2ticsTab, "x2ticsRotationLabel" );
  x2ticsRotationLabel->setMinimumSize( 60, 20 );
  x2ticsRotationLabel->setMaximumSize( 32767, 20 );
  x2ticsRotationLabel->setText( i18n("Rotation:") );
  x2ticsRotationLabel->setAlignment( 289 );
  x2ticsRotationLabel->setMargin( -1 );

  x2ticsRotationCBox = new QComboBox( FALSE, x2ticsTab, "x2ticsRotationComboBox" );
  x2ticsRotationCBox->setMinimumSize( 100, 20 );
  x2ticsRotationCBox->setMaximumSize( 32767, 20 );
  x2ticsRotationCBox->setSizeLimit( 2 );
  //x2ticsRotationCBox->setAutoResize( FALSE );
  x2ticsRotationCBox->insertItem( "norotate" );
  x2ticsRotationCBox->insertItem( "rotate" );

  QButtonGroup* x2ticsPosButtonGroup;
  x2ticsPosButtonGroup = new QButtonGroup( x2ticsTab, "x2ticsPosButtonGroup" );
  x2ticsPosButtonGroup->setMinimumSize( 500, 110 );
  x2ticsPosButtonGroup->setMaximumSize( 32767, 32767 );
  x2ticsPosButtonGroup->setFrameStyle( 49 );
  x2ticsPosButtonGroup->setTitle( i18n("X2 Tics Position") );
  x2ticsPosButtonGroup->setExclusive( TRUE );

  x2ticsSIERadioButton = new QRadioButton(x2ticsPosButtonGroup , "x2ticsSIERadioButton" );
  x2ticsSIERadioButton->setMinimumSize( 150, 20 );
  x2ticsSIERadioButton->setMaximumSize( 32767, 20 );
  x2ticsSIERadioButton->setText( i18n("Start/Inc/End") );
  x2ticsSIERadioButton->setAutoRepeat( FALSE );
  //x2ticsSIERadioButton->setAutoResize( FALSE );
  x2ticsSIERadioButton->setChecked( TRUE );

  QLabel* x2ticsStartPosLabel;
  x2ticsStartPosLabel = new QLabel( x2ticsPosButtonGroup, "x2ticsStartPosLabel" );
  x2ticsStartPosLabel->setMinimumSize( 40, 20 );
  x2ticsStartPosLabel->setMaximumSize( 32767, 20 );
  x2ticsStartPosLabel->setText( i18n("Start:") );
  x2ticsStartPosLabel->setAlignment( 289 );
  x2ticsStartPosLabel->setMargin( -1 );

  x2ticsStartPosEdit = new QLineEdit( x2ticsPosButtonGroup, "x2ticsStartPosLineEdit" );
  x2ticsStartPosEdit->setMinimumSize( 30, 20 );
  x2ticsStartPosEdit->setMaximumSize( 32767, 20 );
  x2ticsStartPosEdit->setText( "" );
  x2ticsStartPosEdit->setMaxLength( 32767 );
  x2ticsStartPosEdit->setEchoMode( QLineEdit::Normal );
  x2ticsStartPosEdit->setFrame( TRUE );

  QLabel* x2ticsIncPosLabel;
  x2ticsIncPosLabel = new QLabel( x2ticsPosButtonGroup, "x2ticsIncPosLabel" );
  x2ticsIncPosLabel->setMinimumSize( 65, 20 );
  x2ticsIncPosLabel->setMaximumSize( 32767, 20 );
  x2ticsIncPosLabel->setText( i18n("Increment:") );
  x2ticsIncPosLabel->setAlignment( 289 );
  x2ticsIncPosLabel->setMargin( -1 );

  x2ticsIncPosEdit = new QLineEdit( x2ticsPosButtonGroup, "x2ticsIncPosLineEdit" );
  x2ticsIncPosEdit->setMinimumSize( 30, 20 );
  x2ticsIncPosEdit->setMaximumSize( 32767, 20 );
  x2ticsIncPosEdit->setText( "" );
  x2ticsIncPosEdit->setMaxLength( 32767 );
  x2ticsIncPosEdit->setEchoMode( QLineEdit::Normal );
  x2ticsIncPosEdit->setFrame( TRUE );

  QLabel* x2ticsEndPosLabel;
  x2ticsEndPosLabel = new QLabel( x2ticsPosButtonGroup, "x2ticsEndPosLabel" );
  x2ticsEndPosLabel->setMinimumSize( 35, 20 );
  x2ticsEndPosLabel->setMaximumSize( 32767, 20 );
  x2ticsEndPosLabel->setText( i18n("End:") );
  x2ticsEndPosLabel->setAlignment( 289 );
  x2ticsEndPosLabel->setMargin( -1 );

  x2ticsEndPosEdit = new QLineEdit( x2ticsPosButtonGroup, "x2ticsEndPosLineEdit" );
  x2ticsEndPosEdit->setMinimumSize( 30, 20 );
  x2ticsEndPosEdit->setMaximumSize( 32767, 20 );
  x2ticsEndPosEdit->setText( "" );
  x2ticsEndPosEdit->setMaxLength( 32767 );
  x2ticsEndPosEdit->setEchoMode( QLineEdit::Normal );
  x2ticsEndPosEdit->setFrame( TRUE );

  x2ticsLabelPosRButton = new QRadioButton( x2ticsPosButtonGroup, "x2ticsLabelPosRadioButton" );
  x2ticsLabelPosRButton->setMinimumSize( 150, 20 );
  x2ticsLabelPosRButton->setMaximumSize( 32767, 20 );
  x2ticsLabelPosRButton->setText( i18n("Labels/Positions") );
  x2ticsLabelPosRButton->setAutoRepeat( FALSE );
  //x2ticsLabelPosRButton->setAutoResize( FALSE );

  x2ticsLabelsPosEdit = new QLineEdit( x2ticsPosButtonGroup, "x2ticsLabelsPosLineEdit" );
  x2ticsLabelsPosEdit->setMinimumSize( 330, 20 );
  x2ticsLabelsPosEdit->setMaximumSize( 32767, 20 );
  x2ticsLabelsPosEdit->setText( "" );
  x2ticsLabelsPosEdit->setMaxLength( 32767 );
  x2ticsLabelsPosEdit->setEchoMode( QLineEdit::Normal );
  x2ticsLabelsPosEdit->setFrame( TRUE );

  // create and assemble layouts for x2tics dialog

  // main col layout
  QVBoxLayout* x2ticsMainColLayout = new QVBoxLayout(x2ticsTab,5);

  // activate button group
  x2ticsMainColLayout->addWidget(x2ticsActiveButtonGroup);
  QHBoxLayout* x2ticsActiveRowLayout = new QHBoxLayout(x2ticsActiveButtonGroup,5);

  x2ticsActiveRowLayout->addStretch(1);
  x2ticsActiveRowLayout->addWidget(x2ticsOnRButton);
  x2ticsActiveRowLayout->addStretch(1);
  x2ticsActiveRowLayout->addWidget(x2ticsOffRButton);
  x2ticsActiveRowLayout->addStretch(1);

  // combo box row
  QHBoxLayout* x2ticsComboRowLayout = new QHBoxLayout();
  x2ticsMainColLayout->addLayout(x2ticsComboRowLayout);

  x2ticsComboRowLayout->addWidget(x2ticsLocationLabel);
  x2ticsComboRowLayout->addWidget(x2ticsLocationCBox);
  //x2ticsComboRowLayout->addStretch(1);
  x2ticsComboRowLayout->addWidget(x2ticsMirrorLabel);
  x2ticsComboRowLayout->addWidget(x2ticsMirrorCBox);
  //x2ticsComboRowLayout->addStretch(1);
  x2ticsComboRowLayout->addWidget(x2ticsRotationLabel);
  x2ticsComboRowLayout->addWidget(x2ticsRotationCBox);

  // position button group
  x2ticsMainColLayout->addWidget(x2ticsPosButtonGroup);

  QVBoxLayout* x2ticsPosColLayout = new QVBoxLayout(x2ticsPosButtonGroup,5);

  QHBoxLayout* x2ticsTopPosRowLayout = new QHBoxLayout();
  x2ticsPosColLayout->addLayout(x2ticsTopPosRowLayout);

  x2ticsTopPosRowLayout->addWidget(x2ticsSIERadioButton);
  x2ticsTopPosRowLayout->addWidget(x2ticsStartPosLabel);
  x2ticsTopPosRowLayout->addWidget(x2ticsStartPosEdit);
  //x2ticsTopPosRowLayout->addStretch(1);
  x2ticsTopPosRowLayout->addWidget(x2ticsIncPosLabel);
  x2ticsTopPosRowLayout->addWidget(x2ticsIncPosEdit);
  //x2ticsTopPosRowLayout->addStretch(1);
  x2ticsTopPosRowLayout->addWidget(x2ticsEndPosLabel);
  x2ticsTopPosRowLayout->addWidget(x2ticsEndPosEdit);

  QHBoxLayout* x2ticsBottomPosRowLayout = new QHBoxLayout();
  x2ticsPosColLayout->addLayout(x2ticsBottomPosRowLayout);

  x2ticsBottomPosRowLayout->addWidget(x2ticsLabelPosRButton);
  x2ticsBottomPosRowLayout->addWidget(x2ticsLabelsPosEdit);

  x2ticsMainColLayout->activate();

  // insert tab
  addTab(x2ticsTab, i18n("X2 &Tics") );

  // ------------------- page five of tab dialog: y2tics  -----------------

  QWidget* y2ticsTab = new QWidget(this, "Y2TICS Tab");

  QButtonGroup* y2ticsActiveButtonGroup;
  y2ticsActiveButtonGroup = new QButtonGroup( y2ticsTab, "y2ticsActiveButtonGroup" );
  y2ticsActiveButtonGroup->setMinimumSize( 500, 60 );
  y2ticsActiveButtonGroup->setMaximumSize( 32767, 32767 );
  y2ticsActiveButtonGroup->setFrameStyle( 49 );
  y2ticsActiveButtonGroup->setTitle( i18n("Y2 Tics Activation") );
  y2ticsActiveButtonGroup->setExclusive( TRUE );

  y2ticsOnRButton = new QRadioButton( y2ticsActiveButtonGroup, "y2ticsOnRadioButton" );
  y2ticsOnRButton->setMinimumSize( 90, 20 );
  y2ticsOnRButton->setMaximumSize( 32767, 20 );
  y2ticsOnRButton->setText( i18n("Y2 Tics on") );
  y2ticsOnRButton->setAutoRepeat( FALSE );
  //y2ticsOnRButton->setAutoResize( FALSE );
  y2ticsOnRButton->setChecked( TRUE );

  y2ticsOffRButton = new QRadioButton( y2ticsActiveButtonGroup, "y2ticsOffRadioButton" );
  y2ticsOffRButton->setMinimumSize( 100, 20 );
  y2ticsOffRButton->setMaximumSize( 32767, 20 );
  y2ticsOffRButton->setText( i18n("Y2 Tics off") );
  y2ticsOffRButton->setAutoRepeat( FALSE );
  //y2ticsOffRButton->setAutoResize( FALSE );

  QLabel* y2ticsLocationLabel;
  y2ticsLocationLabel = new QLabel( y2ticsTab, "y2ticsLocationLabel" );
  y2ticsLocationLabel->setMinimumSize( 60, 20 );
  y2ticsLocationLabel->setMaximumSize( 32767, 20 );
  y2ticsLocationLabel->setText( i18n("Location:") );
  y2ticsLocationLabel->setAlignment( 289 );
  y2ticsLocationLabel->setMargin( -1 );

  y2ticsLocationCBox = new QComboBox( FALSE, y2ticsTab, "ComboBox_1" );
  y2ticsLocationCBox->setMinimumSize( 100, 20 );
  y2ticsLocationCBox->setMaximumSize( 32767, 20 );
  y2ticsLocationCBox->setSizeLimit( 2 );
  //y2ticsLocationCBox->setAutoResize( FALSE );
  y2ticsLocationCBox->insertItem( "border" );
  y2ticsLocationCBox->insertItem( "axis" );

  QLabel* y2ticsMirrorLabel;
  y2ticsMirrorLabel = new QLabel( y2ticsTab, "y2ticsMirrorLabel" );
  y2ticsMirrorLabel->setMinimumSize( 60, 20 );
  y2ticsMirrorLabel->setMaximumSize( 32767, 20 );
  y2ticsMirrorLabel->setText( i18n("Mirroring:") );
  y2ticsMirrorLabel->setAlignment( 289 );
  y2ticsMirrorLabel->setMargin( -1 );

  y2ticsMirrorCBox = new QComboBox( FALSE, y2ticsTab, "y2ticsMirrorComboBox" );
  y2ticsMirrorCBox->setMinimumSize( 100, 20 );
  y2ticsMirrorCBox->setMaximumSize( 32767, 20 );
  y2ticsMirrorCBox->setSizeLimit( 2 );
  //y2ticsMirrorCBox->setAutoResize( FALSE );
  y2ticsMirrorCBox->insertItem( "mirror" );
  y2ticsMirrorCBox->insertItem( "nomirror" );

  QLabel* y2ticsRotationLabel;
  y2ticsRotationLabel = new QLabel( y2ticsTab, "y2ticsRotationLabel" );
  y2ticsRotationLabel->setMinimumSize( 60, 20 );
  y2ticsRotationLabel->setMaximumSize( 32767, 20 );
  y2ticsRotationLabel->setText( i18n("Rotation:") );
  y2ticsRotationLabel->setAlignment( 289 );
  y2ticsRotationLabel->setMargin( -1 );

  y2ticsRotationCBox = new QComboBox( FALSE, y2ticsTab, "y2ticsRotationComboBox" );
  y2ticsRotationCBox->setMinimumSize( 100, 20 );
  y2ticsRotationCBox->setMaximumSize( 32767, 20 );
  y2ticsRotationCBox->setSizeLimit( 2 );
  //y2ticsRotationCBox->setAutoResize( FALSE );
  y2ticsRotationCBox->insertItem( "norotate" );
  y2ticsRotationCBox->insertItem( "rotate" );

  QButtonGroup* y2ticsPosButtonGroup;
  y2ticsPosButtonGroup = new QButtonGroup( y2ticsTab, "y2ticsPosButtonGroup" );
  y2ticsPosButtonGroup->setMinimumSize( 500, 110 );
  y2ticsPosButtonGroup->setMaximumSize( 32767, 32767 );
  y2ticsPosButtonGroup->setFrameStyle( 49 );
  y2ticsPosButtonGroup->setTitle( i18n("Y2 Tics Position") );
  y2ticsPosButtonGroup->setExclusive( TRUE );

  y2ticsSIERadioButton = new QRadioButton(y2ticsPosButtonGroup , "y2ticsSIERadioButton" );
  y2ticsSIERadioButton->setMinimumSize( 150, 20 );
  y2ticsSIERadioButton->setMaximumSize( 32767, 20 );
  y2ticsSIERadioButton->setText( i18n("Start/Inc/End") );
  y2ticsSIERadioButton->setAutoRepeat( FALSE );
  //y2ticsSIERadioButton->setAutoResize( FALSE );
  y2ticsSIERadioButton->setChecked( TRUE );

  QLabel* y2ticsStartPosLabel;
  y2ticsStartPosLabel = new QLabel( y2ticsPosButtonGroup, "y2ticsStartPosLabel" );
  y2ticsStartPosLabel->setMinimumSize( 40, 20 );
  y2ticsStartPosLabel->setMaximumSize( 32767, 20 );
  y2ticsStartPosLabel->setText( i18n("Start:") );
  y2ticsStartPosLabel->setAlignment( 289 );
  y2ticsStartPosLabel->setMargin( -1 );

  y2ticsStartPosEdit = new QLineEdit( y2ticsPosButtonGroup, "y2ticsStartPosLineEdit" );
  y2ticsStartPosEdit->setMinimumSize( 30, 20 );
  y2ticsStartPosEdit->setMaximumSize( 32767, 20 );
  y2ticsStartPosEdit->setText( "" );
  y2ticsStartPosEdit->setMaxLength( 32767 );
  y2ticsStartPosEdit->setEchoMode( QLineEdit::Normal );
  y2ticsStartPosEdit->setFrame( TRUE );

  QLabel* y2ticsIncPosLabel;
  y2ticsIncPosLabel = new QLabel( y2ticsPosButtonGroup, "y2ticsIncPosLabel" );
  y2ticsIncPosLabel->setMinimumSize( 65, 20 );
  y2ticsIncPosLabel->setMaximumSize( 32767, 20 );
  y2ticsIncPosLabel->setText( i18n("Increment:") );
  y2ticsIncPosLabel->setAlignment( 289 );
  y2ticsIncPosLabel->setMargin( -1 );

  y2ticsIncPosEdit = new QLineEdit( y2ticsPosButtonGroup, "y2ticsIncPosLineEdit" );
  y2ticsIncPosEdit->setMinimumSize( 30, 20 );
  y2ticsIncPosEdit->setMaximumSize( 32767, 20 );
  y2ticsIncPosEdit->setText( "" );
  y2ticsIncPosEdit->setMaxLength( 32767 );
  y2ticsIncPosEdit->setEchoMode( QLineEdit::Normal );
  y2ticsIncPosEdit->setFrame( TRUE );

  QLabel* y2ticsEndPosLabel;
  y2ticsEndPosLabel = new QLabel( y2ticsPosButtonGroup, "y2ticsEndPosLabel" );
  y2ticsEndPosLabel->setMinimumSize( 35, 20 );
  y2ticsEndPosLabel->setMaximumSize( 32767, 20 );
  y2ticsEndPosLabel->setText( i18n("End:") );
  y2ticsEndPosLabel->setAlignment( 289 );
  y2ticsEndPosLabel->setMargin( -1 );

  y2ticsEndPosEdit = new QLineEdit( y2ticsPosButtonGroup, "y2ticsEndPosLineEdit" );
  y2ticsEndPosEdit->setMinimumSize( 30, 20 );
  y2ticsEndPosEdit->setMaximumSize( 32767, 20 );
  y2ticsEndPosEdit->setText( "" );
  y2ticsEndPosEdit->setMaxLength( 32767 );
  y2ticsEndPosEdit->setEchoMode( QLineEdit::Normal );
  y2ticsEndPosEdit->setFrame( TRUE );

  y2ticsLabelPosRButton = new QRadioButton( y2ticsPosButtonGroup, "y2ticsLabelPosRadioButton" );
  y2ticsLabelPosRButton->setMinimumSize( 150, 20 );
  y2ticsLabelPosRButton->setMaximumSize( 32767, 20 );
  y2ticsLabelPosRButton->setText( i18n("Labels/Positions") );
  y2ticsLabelPosRButton->setAutoRepeat( FALSE );
  //y2ticsLabelPosRButton->setAutoResize( FALSE );

  y2ticsLabelsPosEdit = new QLineEdit( y2ticsPosButtonGroup, "y2ticsLabelsPosLineEdit" );
  y2ticsLabelsPosEdit->setMinimumSize( 330, 20 );
  y2ticsLabelsPosEdit->setMaximumSize( 32767, 20 );
  y2ticsLabelsPosEdit->setText( "" );
  y2ticsLabelsPosEdit->setMaxLength( 32767 );
  y2ticsLabelsPosEdit->setEchoMode( QLineEdit::Normal );
  y2ticsLabelsPosEdit->setFrame( TRUE );

  // create and assemble layouts for y2tics dialog

  // main col layout
  QVBoxLayout* y2ticsMainColLayout = new QVBoxLayout(y2ticsTab,5);

  // activate button group
  y2ticsMainColLayout->addWidget(y2ticsActiveButtonGroup);
  QHBoxLayout* y2ticsActiveRowLayout = new QHBoxLayout(y2ticsActiveButtonGroup,5);

  y2ticsActiveRowLayout->addStretch(1);
  y2ticsActiveRowLayout->addWidget(y2ticsOnRButton);
  y2ticsActiveRowLayout->addStretch(1);
  y2ticsActiveRowLayout->addWidget(y2ticsOffRButton);
  y2ticsActiveRowLayout->addStretch(1);

  // combo box row
  QHBoxLayout* y2ticsComboRowLayout = new QHBoxLayout();
  y2ticsMainColLayout->addLayout(y2ticsComboRowLayout);

  y2ticsComboRowLayout->addWidget(y2ticsLocationLabel);
  y2ticsComboRowLayout->addWidget(y2ticsLocationCBox);
  //y2ticsComboRowLayout->addStretch(1);
  y2ticsComboRowLayout->addWidget(y2ticsMirrorLabel);
  y2ticsComboRowLayout->addWidget(y2ticsMirrorCBox);
  //y2ticsComboRowLayout->addStretch(1);
  y2ticsComboRowLayout->addWidget(y2ticsRotationLabel);
  y2ticsComboRowLayout->addWidget(y2ticsRotationCBox);

  // position button group
  y2ticsMainColLayout->addWidget(y2ticsPosButtonGroup);

  QVBoxLayout* y2ticsPosColLayout = new QVBoxLayout(y2ticsPosButtonGroup,5);

  QHBoxLayout* y2ticsTopPosRowLayout = new QHBoxLayout();
  y2ticsPosColLayout->addLayout(y2ticsTopPosRowLayout);

  y2ticsTopPosRowLayout->addWidget(y2ticsSIERadioButton);
  y2ticsTopPosRowLayout->addWidget(y2ticsStartPosLabel);
  y2ticsTopPosRowLayout->addWidget(y2ticsStartPosEdit);
  //y2ticsTopPosRowLayout->addStretch(1);
  y2ticsTopPosRowLayout->addWidget(y2ticsIncPosLabel);
  y2ticsTopPosRowLayout->addWidget(y2ticsIncPosEdit);
  //y2ticsTopPosRowLayout->addStretch(1);
  y2ticsTopPosRowLayout->addWidget(y2ticsEndPosLabel);
  y2ticsTopPosRowLayout->addWidget(y2ticsEndPosEdit);

  QHBoxLayout* y2ticsBottomPosRowLayout = new QHBoxLayout();
  y2ticsPosColLayout->addLayout(y2ticsBottomPosRowLayout);

  y2ticsBottomPosRowLayout->addWidget(y2ticsLabelPosRButton);
  y2ticsBottomPosRowLayout->addWidget(y2ticsLabelsPosEdit);

  y2ticsMainColLayout->activate();

  // insert tab
  addTab(y2ticsTab, i18n("Y2 T&ics") );


  // add buttons
  setOKButton("&OK");
  setCancelButton("&Cancel");

  connect(this, SIGNAL(applyButtonPressed()), SLOT(setTicsOptions()));

  resize( 520,240 );
}


ticsOpData::~ticsOpData()
{
}

void ticsOpData::setTicsOptions()
{
}

#include "ticsOpData.moc"
