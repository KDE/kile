/**********************************************************************

	--- Qt Architect generated file ---

	File: fileOptionsData.cpp

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

 *********************************************************************/

#include "fileOptionsData.h"

#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <klocale.h>

fileOptionsData::fileOptionsData
(
	QWidget* parent,
	const char* name
)
	:
	QDialog( parent, name, TRUE, 36864 )
{

	QButtonGroup* dataSetButtonGroup;
	dataSetButtonGroup = new QButtonGroup( this, "dataSetButtonGroup" );
	dataSetButtonGroup->setMinimumSize( 460, 60 );
	dataSetButtonGroup->setMaximumSize( 32767, 32767 );
	dataSetButtonGroup->setFrameStyle( 49 );
	dataSetButtonGroup->setTitle( i18n("Data Set Selection") );

	QButtonGroup* samplingButtonGroup;
	samplingButtonGroup = new QButtonGroup( this, "samplingButtonGroup" );
	samplingButtonGroup->setMinimumSize( 460, 110 );
	samplingButtonGroup->setMaximumSize( 32767, 32767 );
	samplingButtonGroup->setFrameStyle( 49 );
	samplingButtonGroup->setTitle(i18n( "Periodic Sampling") );

	QButtonGroup* colFormatButtonGroup;
	colFormatButtonGroup = new QButtonGroup( this, "colFormatButtonGroup" );
	colFormatButtonGroup->setMinimumSize( 460, 150 );
	colFormatButtonGroup->setMaximumSize( 32767, 32767 );
	colFormatButtonGroup->setFrameStyle( 49 );
	colFormatButtonGroup->setTitle( i18n("Columns && Format") );

	QButtonGroup* interpButtonGroup;
	interpButtonGroup = new QButtonGroup( this, "interpButtonGroup" );
	interpButtonGroup->setMinimumSize( 460, 60 );
	interpButtonGroup->setMaximumSize( 32767, 32767 );
	interpButtonGroup->setFrameStyle( 49 );
	interpButtonGroup->setTitle( i18n("Interpolation && Approximation") );

	QLabel* dataSetStartLabel;
	dataSetStartLabel = new QLabel(dataSetButtonGroup, "dataSetStartLabel" );
	dataSetStartLabel->setMinimumSize( 40, 20 );
	dataSetStartLabel->setMaximumSize( 32767, 20 );
	dataSetStartLabel->setText( i18n("Start:") );
	dataSetStartLabel->setAlignment( 289 );
	dataSetStartLabel->setMargin( -1 );

	dataSetStartEdit = new QLineEdit(dataSetButtonGroup, "dataSetStartLineEdit" );
	dataSetStartEdit->setMinimumSize( 30, 20 );
	dataSetStartEdit->setMaximumSize( 32767, 20 );
	dataSetStartEdit->setText( "" );
	dataSetStartEdit->setMaxLength( 32767 );
	dataSetStartEdit->setEchoMode( QLineEdit::Normal );
	dataSetStartEdit->setFrame( TRUE );

	QLabel* dataSetEndLabel;
	dataSetEndLabel = new QLabel(dataSetButtonGroup, "dataSetEndLabel" );
	dataSetEndLabel->setMinimumSize( 40, 20 );
	dataSetEndLabel->setMaximumSize( 32767, 20 );
	dataSetEndLabel->setText( i18n("End:") );
	dataSetEndLabel->setAlignment( 289 );
	dataSetEndLabel->setMargin( -1 );

	dataSetEndEdit = new QLineEdit(dataSetButtonGroup, "dataSetEndLineEdit" );
	dataSetEndEdit->setMinimumSize( 30, 20 );
	dataSetEndEdit->setMaximumSize( 32767, 20 );
	dataSetEndEdit->setText( "" );
	dataSetEndEdit->setMaxLength( 32767 );
	dataSetEndEdit->setEchoMode( QLineEdit::Normal );
	dataSetEndEdit->setFrame( TRUE );

	QLabel* dataSetIncLabel;
	dataSetIncLabel = new QLabel(dataSetButtonGroup, "dataSetIncLabel" );
	dataSetIncLabel->setMinimumSize( 70, 20 );
	dataSetIncLabel->setMaximumSize( 32767, 20 );
	dataSetIncLabel->setText( i18n("Increment:") );
	dataSetIncLabel->setAlignment( 289 );
	dataSetIncLabel->setMargin( -1 );

	dataSetIncEdit = new QLineEdit(dataSetButtonGroup, "dataSetIncLineEdit" );
	dataSetIncEdit->setMinimumSize( 30, 20 );
	dataSetIncEdit->setMaximumSize( 32767, 20 );
	dataSetIncEdit->setText( "" );
	dataSetIncEdit->setMaxLength( 32767 );
	dataSetIncEdit->setEchoMode( QLineEdit::Normal );
	dataSetIncEdit->setFrame( TRUE );

	QLabel* pointIncLabel;
	pointIncLabel = new QLabel(samplingButtonGroup, "pointIncLabel" );
	pointIncLabel->setMinimumSize( 100, 20 );
	pointIncLabel->setMaximumSize( 32767, 20 );
	pointIncLabel->setText( i18n("Point increment:") );
	pointIncLabel->setAlignment( 289 );
	pointIncLabel->setMargin( -1 );

	pointIncEdit = new QLineEdit(samplingButtonGroup, "pointIncLineEdit" );
	pointIncEdit->setMinimumSize( 30, 20 );
	pointIncEdit->setMaximumSize( 32767, 20 );
	pointIncEdit->setText( "" );
	pointIncEdit->setMaxLength( 32767 );
	pointIncEdit->setEchoMode( QLineEdit::Normal );
	pointIncEdit->setFrame( TRUE );

	QLabel* lineIncLabel;
	lineIncLabel = new QLabel(samplingButtonGroup, "lineIncLabel" );
	lineIncLabel->setMinimumSize( 100, 20 );
	lineIncLabel->setMaximumSize( 32767, 20 );
	lineIncLabel->setText( i18n("Line increment:") );
	lineIncLabel->setAlignment( 289 );
	lineIncLabel->setMargin( -1 );

	lineIncEdit = new QLineEdit(samplingButtonGroup, "lineIncLineEdit" );
	lineIncEdit->setMinimumSize( 30, 20 );
	lineIncEdit->setMaximumSize( 32767, 20 );
	lineIncEdit->setText( "" );
	lineIncEdit->setMaxLength( 32767 );
	lineIncEdit->setEchoMode( QLineEdit::Normal );
	lineIncEdit->setFrame( TRUE );

	QLabel* startPointLabel;
	startPointLabel = new QLabel(samplingButtonGroup, "startPointLabel" );
	startPointLabel->setMinimumSize( 70, 20 );
	startPointLabel->setMaximumSize( 32767, 20 );
	startPointLabel->setText( i18n("Start point:") );
	startPointLabel->setAlignment( 289 );
	startPointLabel->setMargin( -1 );

	startPointEdit = new QLineEdit(samplingButtonGroup, "startPointLineEdit" );
	startPointEdit->setMinimumSize( 30, 20 );
	startPointEdit->setMaximumSize( 32767, 20 );
	startPointEdit->setText( "" );
	startPointEdit->setMaxLength( 32767 );
	startPointEdit->setEchoMode( QLineEdit::Normal );
	startPointEdit->setFrame( TRUE );

	QLabel* startLineLabel;
	startLineLabel = new QLabel(samplingButtonGroup, "startLineLabel" );
	startLineLabel->setMinimumSize( 70, 20 );
	startLineLabel->setMaximumSize( 32767, 20 );
	startLineLabel->setText( i18n("Start line:") );
	startLineLabel->setAlignment( 289 );
	startLineLabel->setMargin( -1 );

	startLineEdit = new QLineEdit(samplingButtonGroup, "startLineLineEdit" );
	startLineEdit->setMinimumSize( 30, 20 );
	startLineEdit->setMaximumSize( 32767, 20 );
	startLineEdit->setText( "" );
	startLineEdit->setMaxLength( 32767 );
	startLineEdit->setEchoMode( QLineEdit::Normal );
	startLineEdit->setFrame( TRUE );

	QLabel* endPointLabel;
	endPointLabel = new QLabel(samplingButtonGroup, "endPointLabel" );
	endPointLabel->setMinimumSize( 70, 20 );
	endPointLabel->setMaximumSize( 32767, 20 );
	endPointLabel->setText( i18n("End point:") );
	endPointLabel->setAlignment( 289 );
	endPointLabel->setMargin( -1 );

	endPointEdit = new QLineEdit(samplingButtonGroup, "endPointLineEdit" );
	endPointEdit->setMinimumSize( 30, 20 );
	endPointEdit->setMaximumSize( 32767, 20 );
	endPointEdit->setText( "" );
	endPointEdit->setMaxLength( 32767 );
	endPointEdit->setEchoMode( QLineEdit::Normal );
	endPointEdit->setFrame( TRUE );

	QLabel* endLineLabel;
	endLineLabel = new QLabel(samplingButtonGroup, "endLineLabel" );
	endLineLabel->setMinimumSize( 60, 20 );
	endLineLabel->setMaximumSize( 32767, 20 );
	endLineLabel->setText( i18n("End line:") );
	endLineLabel->setAlignment( 289 );
	endLineLabel->setMargin( -1 );

	endLineEdit = new QLineEdit(samplingButtonGroup, "endLineLineEdit" );
	endLineEdit->setMinimumSize( 30, 20 );
	endLineEdit->setMaximumSize( 32767, 20 );
	endLineEdit->setText( "" );
	endLineEdit->setMaxLength( 32767 );
	endLineEdit->setEchoMode( QLineEdit::Normal );
	endLineEdit->setFrame( TRUE );

	QLabel* xColLabel;
	xColLabel = new QLabel(colFormatButtonGroup , "xColLabel" );
	xColLabel->setMinimumSize( 70, 20 );
	xColLabel->setMaximumSize( 32767, 20 );
	xColLabel->setText( i18n("X column:") );
	xColLabel->setAlignment( 289 );
	xColLabel->setMargin( -1 );

	xColumnEdit = new QLineEdit(colFormatButtonGroup, "xColLineEdit" );
	xColumnEdit->setMinimumSize( 30, 20 );
	xColumnEdit->setMaximumSize( 32767, 20 );
	xColumnEdit->setText( "" );
	xColumnEdit->setMaxLength( 32767 );
	xColumnEdit->setEchoMode( QLineEdit::Normal );
	xColumnEdit->setFrame( TRUE );

	QLabel* yColLabel;
	yColLabel = new QLabel(colFormatButtonGroup, "yColLabel" );
	yColLabel->setMinimumSize( 70, 20 );
	yColLabel->setMaximumSize( 32767, 20 );
	yColLabel->setText( i18n("Y column:") );
	yColLabel->setAlignment( 289 );
	yColLabel->setMargin( -1 );

	yColumnEdit = new QLineEdit(colFormatButtonGroup, "yColLineEdit" );
	yColumnEdit->setMinimumSize( 30, 20 );
	yColumnEdit->setMaximumSize( 32767, 20 );
	yColumnEdit->setText( "" );
	yColumnEdit->setMaxLength( 32767 );
	yColumnEdit->setEchoMode( QLineEdit::Normal );
	yColumnEdit->setFrame( TRUE );

	QLabel* zColLabel;
	zColLabel = new QLabel(colFormatButtonGroup, "zColLabel" );
	zColLabel->setMinimumSize( 70, 20 );
	zColLabel->setMaximumSize( 32767, 20 );
	zColLabel->setText( i18n("Z column:") );
	zColLabel->setAlignment( 289 );
	zColLabel->setMargin( -1 );

	zColumnEdit = new QLineEdit(colFormatButtonGroup, "zColLineEdit" );
	zColumnEdit->setMinimumSize( 30, 20 );
	zColumnEdit->setMaximumSize( 32767, 20 );
	zColumnEdit->setText( "" );
	zColumnEdit->setMaxLength( 32767 );
	zColumnEdit->setEchoMode( QLineEdit::Normal );
	zColumnEdit->setFrame( TRUE );

	QLabel* formatLabel;
	formatLabel = new QLabel(colFormatButtonGroup, "formatLabel" );
	formatLabel->setMinimumSize( 80, 20 );
	formatLabel->setMaximumSize( 32767, 20 );
	formatLabel->setText( i18n("Format:") );
	formatLabel->setAlignment( 289 );
	formatLabel->setMargin( -1 );

	formatEdit = new QLineEdit(colFormatButtonGroup, "formatLineEdit" );
	formatEdit->setMinimumSize( 340, 20 );
	formatEdit->setMaximumSize( 32767, 20 );
	formatEdit->setText( "" );
	formatEdit->setMaxLength( 32767 );
	formatEdit->setEchoMode( QLineEdit::Normal );
	formatEdit->setFrame( TRUE );

	QLabel* rawFormatLabel;
	rawFormatLabel = new QLabel(colFormatButtonGroup, "rawFormatLabel" );
	rawFormatLabel->setMinimumSize( 80, 20 );
	rawFormatLabel->setMaximumSize( 32767, 20 );
	rawFormatLabel->setText( i18n("Raw format:") );
	rawFormatLabel->setAlignment( 289 );
	rawFormatLabel->setMargin( -1 );

	rawFormatEdit = new QLineEdit(colFormatButtonGroup, "rawFormatLineEdit" );
	rawFormatEdit->setMinimumSize( 340, 20 );
	rawFormatEdit->setMaximumSize( 32767, 20 );
	rawFormatEdit->setText( "" );
	rawFormatEdit->setMaxLength( 32767 );
	rawFormatEdit->setEchoMode( QLineEdit::Normal );
	rawFormatEdit->setFrame( TRUE );

	QLabel* interpLabel;
	interpLabel = new QLabel(interpButtonGroup, "interpLabel" );
	interpLabel->setMinimumSize( 100, 20 );
	interpLabel->setMaximumSize( 32767, 20 );
	interpLabel->setText( i18n("Smoothing:") );
	interpLabel->setAlignment( 289 );
	interpLabel->setMargin( -1 );

	interpList = new QComboBox( FALSE, interpButtonGroup, "interpComboBox" );
	interpList->setMinimumSize( 100, 20 );
	interpList->setMaximumSize( 32767, 20 );
	interpList->setSizeLimit( 6 );
	interpList->setAutoResize( FALSE );
    interpList->insertItem( "none" );
	interpList->insertItem( "unique" );
	interpList->insertItem( "csplines" );
	interpList->insertItem( "acsplines" );
	interpList->insertItem( "bezier" );
	interpList->insertItem( "sbezier" );

	QPushButton* OKPushButton;
	OKPushButton = new QPushButton( this, "OKPushButton" );
	OKPushButton->setMinimumSize( 100, 26 );
	connect( OKPushButton, SIGNAL(clicked()), SLOT(setFormat()) );
	OKPushButton->setText( i18n("&OK") );
	OKPushButton->setAutoRepeat( FALSE );
	OKPushButton->setAutoResize( FALSE );
	OKPushButton->setAutoDefault( TRUE );
    OKPushButton->setDefault( TRUE );

	QPushButton* cancelPushButton;
	cancelPushButton = new QPushButton( this, "cancelPushButton" );
	cancelPushButton->setMinimumSize( 100, 26 );
	connect( cancelPushButton, SIGNAL(clicked()), SLOT(reject()) );
	cancelPushButton->setText( i18n("&Cancel") );
	cancelPushButton->setAutoRepeat( FALSE );
	cancelPushButton->setAutoResize( FALSE );

    // create layouts

    // main column layout
    QVBoxLayout* mainColLayout = new QVBoxLayout(this,5);
    
    
    // row layout for inside data set button group
    QHBoxLayout* insideDataSetBGRowLayout = new QHBoxLayout(dataSetButtonGroup,5);
    
    // column layout for inside periodic sampling button group
    QVBoxLayout* insidePeriodicBGTopColLayout = new QVBoxLayout(samplingButtonGroup,5);
    
    // top row layout for inside periodic sampling button group
    QHBoxLayout* insidePeriodicBGTopRowLayout = new QHBoxLayout(-1);
    
    // bottom row layout for inside periodic sampling button group
    QHBoxLayout* insidePeriodicBGBottomRowLayout = new QHBoxLayout(-1);
    
    // column layout for inside columns and formats button group
    QVBoxLayout* colFormatBGColLayout = new QVBoxLayout(colFormatButtonGroup,5);

    // top row layout for inside columns and formats button group
    QHBoxLayout* colFormatBGTopRowLayout = new QHBoxLayout(-1);
    
    // middle row layout for inside columns and formats button group
    QHBoxLayout* colFormatBGMiddleRowLayout = new QHBoxLayout(-1);

    // bottom row layout for inside columns and formats button group
    QHBoxLayout* colFormatBGBottomRowLayout = new QHBoxLayout(-1);

    // row layout for inside interpolation button group
    QHBoxLayout* insideInterpBGRowLayout = new QHBoxLayout(interpButtonGroup,5);

    // row layout for push buttons
    QHBoxLayout* pushButtonRowLayout = new QHBoxLayout(-1);

    // assemble layouts and widgets
    mainColLayout->addWidget(dataSetButtonGroup,1);
//    insideDataSetBGRowLayout->addStretch(1);
    insideDataSetBGRowLayout->addWidget(dataSetStartLabel,1);
    insideDataSetBGRowLayout->addWidget(dataSetStartEdit,1);
//    insideDataSetBGRowLayout->addStretch(1);
    insideDataSetBGRowLayout->addWidget(dataSetEndLabel,1);
    insideDataSetBGRowLayout->addWidget(dataSetEndEdit,1);
//    insideDataSetBGRowLayout->addStretch(1);
    insideDataSetBGRowLayout->addWidget(dataSetIncLabel,1);
    insideDataSetBGRowLayout->addWidget(dataSetIncEdit,1);
//    insideDataSetBGRowLayout->addStretch(1);

    mainColLayout->addWidget(samplingButtonGroup,1);
    insidePeriodicBGTopColLayout->addLayout(insidePeriodicBGTopRowLayout,1);
//    insidePeriodicBGTopRowLayout->addStretch(1);
    insidePeriodicBGTopRowLayout->addWidget(pointIncLabel,1);
    insidePeriodicBGTopRowLayout->addWidget(pointIncEdit,1);
//    insidePeriodicBGTopRowLayout->addStretch(1);
    insidePeriodicBGTopRowLayout->addWidget(lineIncLabel,1);
    insidePeriodicBGTopRowLayout->addWidget(lineIncEdit,1);
//    insidePeriodicBGTopRowLayout->addStretch(1);
    insidePeriodicBGTopColLayout->addLayout(insidePeriodicBGBottomRowLayout,1);
    insidePeriodicBGBottomRowLayout->addWidget(startPointLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(startPointEdit,1);
//    insidePeriodicBGBottomRowLayout->addStretch(1);
    insidePeriodicBGBottomRowLayout->addWidget(startLineLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(startLineEdit,1);
//    insidePeriodicBGBottomRowLayout->addStretch(1);
    insidePeriodicBGBottomRowLayout->addWidget(endPointLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(endPointEdit,1);
//    insidePeriodicBGBottomRowLayout->addStretch(1);
    insidePeriodicBGBottomRowLayout->addWidget(endLineLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(endLineEdit,1);
//    insidePeriodicBGBottomRowLayout->addStretch(1);

    mainColLayout->addWidget(colFormatButtonGroup,1);
    colFormatBGColLayout->addLayout(colFormatBGTopRowLayout,1);
//    colFormatBGTopRowLayout->addStretch(1);
    colFormatBGTopRowLayout->addWidget(xColLabel,1);
    colFormatBGTopRowLayout->addWidget(xColumnEdit,1);
//    colFormatBGTopRowLayout->addStretch(1);
    colFormatBGTopRowLayout->addWidget(yColLabel,1);
    colFormatBGTopRowLayout->addWidget(yColumnEdit,1);
//    colFormatBGTopRowLayout->addStretch(1);
    colFormatBGTopRowLayout->addWidget(zColLabel,1);
    colFormatBGTopRowLayout->addWidget(zColumnEdit,1);
    colFormatBGColLayout->addLayout(colFormatBGMiddleRowLayout,1);
    colFormatBGMiddleRowLayout->addWidget(formatLabel,1);
    colFormatBGMiddleRowLayout->addWidget(formatEdit,1);
    colFormatBGColLayout->addLayout(colFormatBGBottomRowLayout,1);
    colFormatBGBottomRowLayout->addWidget(rawFormatLabel,1);
    colFormatBGBottomRowLayout->addWidget(rawFormatEdit,1);

    mainColLayout->addWidget(interpButtonGroup,1);
    insideInterpBGRowLayout->addWidget(interpLabel,1);
    insideInterpBGRowLayout->addWidget(interpList,1);
    insideInterpBGRowLayout->addStretch(1);

    mainColLayout->addLayout(pushButtonRowLayout);
    pushButtonRowLayout->addStretch(1);
    pushButtonRowLayout->addWidget(OKPushButton,1);
    pushButtonRowLayout->addStretch(1);
    pushButtonRowLayout->addWidget(cancelPushButton,1);
    pushButtonRowLayout->addStretch(1);

	resize( 480,450 );

    mainColLayout->activate();
}


fileOptionsData::~fileOptionsData()
{
}
void fileOptionsData::setFormat()
{
}

#include "fileOptionsData.moc"
