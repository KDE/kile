/**********************************************************************

	--- Qt Architect generated file ---

	File: curveFitData.cpp

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 *********************************************************************/

#include "curveFitData.h"

#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qlayout.h>
#include <klocale.h>

curveFitData::curveFitData
(
	QWidget* parent,
	const char* name
)
	:
	QTabDialog( parent, name, TRUE, 0 )
{

  // -------------------- setup page one of tab dialog --------------------

  QWidget* mainCurveFit = new QWidget(this, "Main Options");

	QButtonGroup* specVarButtonGroup;
	specVarButtonGroup = new QButtonGroup( mainCurveFit, "specVarButtonGroup" );
	specVarButtonGroup->setMinimumSize( 380, 60 );
	specVarButtonGroup->setMaximumSize( 32767, 32767 );
	specVarButtonGroup->setFrameStyle( 49 );
	specVarButtonGroup->setTitle( i18n("Special Variables") );
	specVarButtonGroup->setAlignment( 1 );

	QButtonGroup* paramButtonGroup;
	paramButtonGroup = new QButtonGroup( mainCurveFit, "paramButtonGroup" );
	paramButtonGroup->setMinimumSize( 380, 80 );
	paramButtonGroup->setMaximumSize( 32767, 32767 );
	paramButtonGroup->setFrameStyle( 49 );
	paramButtonGroup->setTitle( i18n("Parameters") );
	paramButtonGroup->setAlignment( 1 );

	QButtonGroup* yRangeButtonGroup;
	yRangeButtonGroup = new QButtonGroup( mainCurveFit, "yRangeButtonGroup" );
	yRangeButtonGroup->setMinimumSize( 380, 60 );
	yRangeButtonGroup->setMaximumSize( 32767, 32767 );
	yRangeButtonGroup->setFrameStyle( 49 );
	yRangeButtonGroup->setTitle( i18n("Y range:") );
	yRangeButtonGroup->setAlignment( 1 );

	QButtonGroup* xRangeButtonGroup;
	xRangeButtonGroup = new QButtonGroup( mainCurveFit, "xRangeButtonGroup" );
	xRangeButtonGroup->setMinimumSize( 380, 60 );
	xRangeButtonGroup->setMaximumSize( 32767, 32767 );
	xRangeButtonGroup->setFrameStyle( 49 );
	xRangeButtonGroup->setTitle( i18n("X range:") );
	xRangeButtonGroup->setAlignment( 1 );

	QLabel* FunctionLabel;
	FunctionLabel = new QLabel( mainCurveFit, "FunctionLabel" );
	FunctionLabel->setMinimumSize( 60, 20 );
	FunctionLabel->setMaximumSize( 32767, 20 );
	FunctionLabel->setText( i18n("Function:") );
	FunctionLabel->setAlignment( 289 );
	FunctionLabel->setMargin( -1 );

	functionNameEdit = new QLineEdit( mainCurveFit, "functionNameLineEdit" );
	functionNameEdit->setMinimumSize( 50, 20 );
	functionNameEdit->setMaximumSize( 32767, 20 );
	functionNameEdit->setText( "" );
	functionNameEdit->setMaxLength( 32767 );
	functionNameEdit->setEchoMode( QLineEdit::Normal );
	functionNameEdit->setFrame( TRUE );

    QLabel* equalLabel;
	equalLabel = new QLabel( mainCurveFit, "equalLabel" );
	equalLabel->setMinimumSize( 20, 20 );
	equalLabel->setMaximumSize( 32767, 20 );
	equalLabel->setText( "=" );
	equalLabel->setAlignment( 289 );
	equalLabel->setMargin( -1 );

	functionValueEdit = new QLineEdit( mainCurveFit, "functionValueLineEdit" );
	functionValueEdit->setMinimumSize( 80, 20 );
	functionValueEdit->setMaximumSize( 32767, 20 );
	functionValueEdit->setText( "" );
	functionValueEdit->setMaxLength( 32767 );
	functionValueEdit->setEchoMode( QLineEdit::Normal );
	functionValueEdit->setFrame( TRUE );

	QLabel* dataFileLabel;
	dataFileLabel = new QLabel( mainCurveFit, "dataFileLabel" );
	dataFileLabel->setMinimumSize( 60, 20 );
	dataFileLabel->setMaximumSize( 32767, 20 );
	dataFileLabel->setText( i18n("Data file:") );
	dataFileLabel->setAlignment( 289 );
	dataFileLabel->setMargin( -1 );

	dataFileEdit = new QLineEdit( mainCurveFit, "dataFileLineEdit" );
	dataFileEdit->setMinimumSize( 80, 20 );
	dataFileEdit->setMaximumSize( 32767, 20 );
	dataFileEdit->setText( "" );
	dataFileEdit->setMaxLength( 32767 );
	dataFileEdit->setEchoMode( QLineEdit::Normal );
	dataFileEdit->setFrame( TRUE );

	QPushButton* getDatafilePushButton;
	getDatafilePushButton = new QPushButton(mainCurveFit , "getDatafilePushButton" );
	getDatafilePushButton->setMinimumSize( 40, 26 );
	connect(getDatafilePushButton , SIGNAL(clicked()), SLOT(dataFileOpen()) );
	getDatafilePushButton->setText( i18n("&Get") );
	getDatafilePushButton->setAutoRepeat( FALSE );
	getDatafilePushButton->setAutoResize( FALSE );

	QLabel* varXRangeNameLabel;
	varXRangeNameLabel = new QLabel(xRangeButtonGroup , "varXRangeNameLabel" );
	varXRangeNameLabel->setMinimumSize( 90, 20 );
	varXRangeNameLabel->setMaximumSize( 32767, 20 );
	varXRangeNameLabel->setText( i18n("Variable name:") );
	varXRangeNameLabel->setAlignment( 289 );
	varXRangeNameLabel->setMargin( -1 );

	varXRangeNameEdit = new QLineEdit( xRangeButtonGroup, "varXRangeNameLineEdit" );
	varXRangeNameEdit->setMinimumSize( 40, 20 );
	varXRangeNameEdit->setMaximumSize( 32767, 20 );
	varXRangeNameEdit->setText( "" );
	varXRangeNameEdit->setMaxLength( 32767 );
	varXRangeNameEdit->setEchoMode( QLineEdit::Normal );
	varXRangeNameEdit->setFrame( TRUE );

	QLabel* varXRangeMinLabel;
	varXRangeMinLabel = new QLabel( xRangeButtonGroup, "varXRangeMinLabel" );
	varXRangeMinLabel->setMinimumSize( 30, 20 );
	varXRangeMinLabel->setMaximumSize( 32767, 20 );
	varXRangeMinLabel->setText( i18n("Min:") );
	varXRangeMinLabel->setAlignment( 289 );
	varXRangeMinLabel->setMargin( -1 );

	varXRangeMinEdit = new QLineEdit( xRangeButtonGroup, "varXRangeMinLineEdit" );
	varXRangeMinEdit->setMinimumSize( 40, 20 );
	varXRangeMinEdit->setMaximumSize( 32767, 20 );
	varXRangeMinEdit->setText( "" );
	varXRangeMinEdit->setMaxLength( 32767 );
	varXRangeMinEdit->setEchoMode( QLineEdit::Normal );
	varXRangeMinEdit->setFrame( TRUE );

	QLabel* varXRangeMaxLabel;
	varXRangeMaxLabel = new QLabel( xRangeButtonGroup, "varXRangeMaxLabel" );
	varXRangeMaxLabel->setMinimumSize( 30, 20 );
	varXRangeMaxLabel->setMaximumSize( 32767, 20 );
	varXRangeMaxLabel->setText( i18n("Max:") );
	varXRangeMaxLabel->setAlignment( 289 );
	varXRangeMaxLabel->setMargin( -1 );

	varXRangeMaxEdit = new QLineEdit( xRangeButtonGroup, "varXRangeMaxLineEdit" );
	varXRangeMaxEdit->setMinimumSize( 40, 20 );
	varXRangeMaxEdit->setMaximumSize( 32767, 20 );
	varXRangeMaxEdit->setText( "" );
	varXRangeMaxEdit->setMaxLength( 32767 );
	varXRangeMaxEdit->setEchoMode( QLineEdit::Normal );
	varXRangeMaxEdit->setFrame( TRUE );

	QLabel* varYRangeNameLabel;
	varYRangeNameLabel = new QLabel( yRangeButtonGroup, "varYRangeNameLabel" );
	varYRangeNameLabel->setMinimumSize( 90, 20 );
	varYRangeNameLabel->setMaximumSize( 32767, 20 );
	varYRangeNameLabel->setText( i18n("Variable name:") );
	varYRangeNameLabel->setAlignment( 289 );
	varYRangeNameLabel->setMargin( -1 );

	varYRangeNameEdit = new QLineEdit( yRangeButtonGroup, "varYRangeNameLineEdit" );
	varYRangeNameEdit->setMinimumSize( 40, 20 );
	varYRangeNameEdit->setMaximumSize( 32767, 20 );
	varYRangeNameEdit->setText( "" );
	varYRangeNameEdit->setMaxLength( 32767 );
	varYRangeNameEdit->setEchoMode( QLineEdit::Normal );
	varYRangeNameEdit->setFrame( TRUE );

	QLabel* varYRangeMinLabel;
	varYRangeMinLabel = new QLabel( yRangeButtonGroup, "varYRangeMinLabel" );
	varYRangeMinLabel->setMinimumSize( 30, 20 );
	varYRangeMinLabel->setMaximumSize( 32767, 20 );
	varYRangeMinLabel->setText( i18n("Min:") );
	varYRangeMinLabel->setAlignment( 289 );
	varYRangeMinLabel->setMargin( -1 );

	varYRangeMinEdit = new QLineEdit(yRangeButtonGroup , "varYRangeMinLineEdit" );
	varYRangeMinEdit->setMinimumSize( 40, 20 );
	varYRangeMinEdit->setMaximumSize( 32767, 20 );
	varYRangeMinEdit->setText( "" );
	varYRangeMinEdit->setMaxLength( 32767 );
	varYRangeMinEdit->setEchoMode( QLineEdit::Normal );
	varYRangeMinEdit->setFrame( TRUE );

	QLabel* varYRangeMaxLabel;
	varYRangeMaxLabel = new QLabel( yRangeButtonGroup, "varYRangeMaxLabel" );
	varYRangeMaxLabel->setMinimumSize( 30, 20 );
	varYRangeMaxLabel->setMaximumSize( 32767, 20 );
	varYRangeMaxLabel->setText( i18n("Max:") );
	varYRangeMaxLabel->setAlignment( 289 );
	varYRangeMaxLabel->setMargin( -1 );

	varYRangeMaxEdit = new QLineEdit( yRangeButtonGroup, "varYRangeMaxLineEdit" );
	varYRangeMaxEdit->setMinimumSize( 40, 20 );
	varYRangeMaxEdit->setMaximumSize( 32767, 20 );
	varYRangeMaxEdit->setText( "" );
	varYRangeMaxEdit->setMaxLength( 32767 );
	varYRangeMaxEdit->setEchoMode( QLineEdit::Normal );
	varYRangeMaxEdit->setFrame( TRUE );

	paramFileRB = new QRadioButton( paramButtonGroup, "paramFileRadioButton" );
	paramFileRB->setMinimumSize( 160, 20 );
	paramFileRB->setMaximumSize( 32767, 20 );
	paramFileRB->setText( i18n("&Parameter file:") );
	paramFileRB->setAutoRepeat( FALSE );
	paramFileRB->setAutoResize( FALSE );

	paramFileEdit = new QLineEdit( paramButtonGroup, "paramFileLineEdit" );
	paramFileEdit->setMinimumSize( 80, 20 );
	paramFileEdit->setMaximumSize( 32767, 20 );
	paramFileEdit->setText( "" );
	paramFileEdit->setMaxLength( 32767 );
	paramFileEdit->setEchoMode( QLineEdit::Normal );
	paramFileEdit->setFrame( TRUE );

    QPushButton* getParamFilePushButton;
	getParamFilePushButton = new QPushButton(paramButtonGroup, "getParamFilePushButton" );
	getParamFilePushButton->setMinimumSize( 40, 26 );
	connect( getParamFilePushButton, SIGNAL(clicked()), SLOT(paramFileOpen()) );
	getParamFilePushButton->setText( i18n("G&et") );
	getParamFilePushButton->setAutoRepeat( FALSE );
	getParamFilePushButton->setAutoResize(TRUE );

	paramCSLRB = new QRadioButton( paramButtonGroup, "paramCSLRadioButton" );
	paramCSLRB->setMinimumSize( 160, 20 );
	paramCSLRB->setMaximumSize( 32767, 20 );
	paramCSLRB->setText( i18n("Co&mma seperated list:") );
	paramCSLRB->setAutoRepeat( FALSE );
	paramCSLRB->setAutoResize( FALSE );

    paramCSLRB->setChecked(TRUE);

	paramCSLEdit = new QLineEdit( paramButtonGroup, "paramCSLLineEdit" );
	paramCSLEdit->setMinimumSize( 80, 20 );
	paramCSLEdit->setMaximumSize( 32767, 20 );
	paramCSLEdit->setText( "" );
	paramCSLEdit->setMaxLength( 32767 );
	paramCSLEdit->setEchoMode( QLineEdit::Normal );
	paramCSLEdit->setFrame( TRUE );

	QLabel* fitLimitLabel;
	fitLimitLabel = new QLabel(specVarButtonGroup, "fitLimitLabel" );
	fitLimitLabel->setMinimumSize( 70, 20 );
	fitLimitLabel->setMaximumSize( 32767, 20 );
	fitLimitLabel->setText( i18n("Fit limit:") );
	fitLimitLabel->setAlignment( 289 );
	fitLimitLabel->setMargin( -1 );

	fitLimitEdit = new QLineEdit(specVarButtonGroup , "fitLimitLineEdit" );
	fitLimitEdit->setMinimumSize( 70, 20 );
	fitLimitEdit->setMaximumSize( 32767, 20 );
	fitLimitEdit->setText( "" );
	fitLimitEdit->setMaxLength( 32767 );
	fitLimitEdit->setEchoMode( QLineEdit::Normal );
	fitLimitEdit->setFrame( TRUE );

	QLabel* fitMaxIterLabel;
	fitMaxIterLabel = new QLabel(specVarButtonGroup , "fitMaxIterLabel" );
	fitMaxIterLabel->setMinimumSize( 110, 20 );
	fitMaxIterLabel->setMaximumSize( 32767, 20 );
	fitMaxIterLabel->setText( i18n("Fit max iterations:") );
	fitMaxIterLabel->setAlignment( 289 );
	fitMaxIterLabel->setMargin( -1 );

	fitMaxIterEdit = new QLineEdit(specVarButtonGroup , "fitMaxIterLineEdit" );
	fitMaxIterEdit->setMinimumSize( 50, 20 );
	fitMaxIterEdit->setMaximumSize( 32767, 20 );
	fitMaxIterEdit->setText( "" );
	fitMaxIterEdit->setMaxLength( 32767 );
	fitMaxIterEdit->setEchoMode( QLineEdit::Normal );
	fitMaxIterEdit->setFrame( TRUE );

	QPushButton* fitPushButton;
	fitPushButton = new QPushButton(mainCurveFit , "getDatafilePushButton" );
	fitPushButton->setMinimumSize( 100, 26 );
	connect(fitPushButton , SIGNAL(clicked()), SLOT(doFit()) );
	fitPushButton->setText( i18n("&Fit") );
	fitPushButton->setAutoRepeat( FALSE );
	fitPushButton->setAutoResize( FALSE );

    // create layouts

    // main column layout
    QVBoxLayout* mainColLayout = new QVBoxLayout(mainCurveFit, 5);

    // row layout for function edit
    QHBoxLayout* functionRowLayout = new QHBoxLayout();

    // row layout for data file edit
    QHBoxLayout* datafileRowLayout = new QHBoxLayout();

    // row layout for inside x-range button group
    QHBoxLayout* xRangeRowLayout = new QHBoxLayout(xRangeButtonGroup,10);

    // row layout for inside y-range button group
    QHBoxLayout* yRangeRowLayout = new QHBoxLayout(yRangeButtonGroup,10);

    // parameter button group layouts
    QVBoxLayout* paramColLayout = new QVBoxLayout(paramButtonGroup,10);
    QHBoxLayout* paramTopRowLayout = new QHBoxLayout();
    QHBoxLayout* paramBottomRowLayout = new QHBoxLayout();

    // special variables button group and inside layouts
    QVBoxLayout* svColLayout = new QVBoxLayout(specVarButtonGroup,10);
    QHBoxLayout* svTopRowLayout = new QHBoxLayout();

    // fit pushbutton
    QHBoxLayout* bottomRowLayout = new QHBoxLayout();

    // assemble layouts and widgets

    mainColLayout->addLayout(functionRowLayout);
    functionRowLayout->addWidget(FunctionLabel);
    functionRowLayout->addWidget(functionNameEdit);
    functionRowLayout->addWidget(equalLabel);
    functionRowLayout->addWidget(functionValueEdit);

    mainColLayout->addLayout(datafileRowLayout);
    datafileRowLayout->addWidget(dataFileLabel);
    datafileRowLayout->addWidget(dataFileEdit);
    datafileRowLayout->addWidget(getDatafilePushButton);

    mainColLayout->addWidget(xRangeButtonGroup);
    xRangeRowLayout->addWidget(varXRangeNameLabel);
    xRangeRowLayout->addWidget(varXRangeNameEdit);
    //xRangeRowLayout->addStretch(1);
    xRangeRowLayout->addWidget(varXRangeMinLabel);
    xRangeRowLayout->addWidget(varXRangeMinEdit);
    //xRangeRowLayout->addStretch(1);
    xRangeRowLayout->addWidget(varXRangeMaxLabel);
    xRangeRowLayout->addWidget(varXRangeMaxEdit);

    mainColLayout->addWidget(yRangeButtonGroup);
    yRangeRowLayout->addWidget(varYRangeNameLabel);
    yRangeRowLayout->addWidget(varYRangeNameEdit);
    //yRangeRowLayout->addStretch(1);
    yRangeRowLayout->addWidget(varYRangeMinLabel);
    yRangeRowLayout->addWidget(varYRangeMinEdit);
    //yRangeRowLayout->addStretch(1);
    yRangeRowLayout->addWidget(varYRangeMaxLabel);
    yRangeRowLayout->addWidget(varYRangeMaxEdit);

    mainColLayout->addWidget(paramButtonGroup);
    paramColLayout->addLayout(paramTopRowLayout);
    paramColLayout->addLayout(paramBottomRowLayout);
    paramTopRowLayout->addWidget(paramFileRB);
    paramTopRowLayout->addWidget(paramFileEdit);
    paramTopRowLayout->addWidget(getParamFilePushButton);
    paramBottomRowLayout->addWidget(paramCSLRB);
    paramBottomRowLayout->addWidget(paramCSLEdit);

    mainColLayout->addWidget(specVarButtonGroup);
    svColLayout->addLayout(svTopRowLayout);
    svTopRowLayout->addWidget(fitLimitLabel);
    svTopRowLayout->addWidget(fitLimitEdit);
    //svTopRowLayout->addStretch(1);
    svTopRowLayout->addWidget(fitMaxIterLabel);
    svTopRowLayout->addWidget(fitMaxIterEdit);

    mainColLayout->addLayout(bottomRowLayout);
    bottomRowLayout->addStretch(1);
    bottomRowLayout->addWidget(fitPushButton);
    bottomRowLayout->addStretch(1);

    mainColLayout->activate();

    addTab(mainCurveFit, i18n("&Main"));

  // -------------------- setup page two of tab dialog --------------------

    QWidget* datafileMods = new QWidget(this, "File Modifiers");

	QButtonGroup* dataSetButtonGroup;
	dataSetButtonGroup = new QButtonGroup( datafileMods, "dataSetButtonGroup" );
	dataSetButtonGroup->setMinimumSize( 460, 60 );
	dataSetButtonGroup->setMaximumSize( 32767, 32767 );
	dataSetButtonGroup->setFrameStyle( 49 );
	dataSetButtonGroup->setTitle( i18n("Data Set Selection") );

	QButtonGroup* samplingButtonGroup;
	samplingButtonGroup = new QButtonGroup( datafileMods, "samplingButtonGroup" );
	samplingButtonGroup->setMinimumSize( 460, 110 );
	samplingButtonGroup->setMaximumSize( 32767, 32767 );
	samplingButtonGroup->setFrameStyle( 49 );
	samplingButtonGroup->setTitle( i18n("Periodic Sampling") );

	QButtonGroup* colFormatButtonGroup;
	colFormatButtonGroup = new QButtonGroup( datafileMods, "colFormatButtonGroup" );
	colFormatButtonGroup->setMinimumSize( 460, 150 );
	colFormatButtonGroup->setMaximumSize( 32767, 32767 );
	colFormatButtonGroup->setFrameStyle( 49 );
	colFormatButtonGroup->setTitle( i18n("Columns && Format") );

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
	formatEdit->setMinimumSize( 50, 20 );
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
	rawFormatEdit->setMinimumSize( 50, 20 );
	rawFormatEdit->setMaximumSize( 32767, 20 );
	rawFormatEdit->setText( "" );
	rawFormatEdit->setMaxLength( 32767 );
	rawFormatEdit->setEchoMode( QLineEdit::Normal );
	rawFormatEdit->setFrame( TRUE );

    // main column layout
    QVBoxLayout* mainColModLayout = new QVBoxLayout(datafileMods,5);

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

    // assemble layouts and widgets
    mainColModLayout->addWidget(dataSetButtonGroup,1);
    //insideDataSetBGRowLayout->addStretch(1);
    insideDataSetBGRowLayout->addWidget(dataSetStartLabel,1);
    insideDataSetBGRowLayout->addWidget(dataSetStartEdit,1);
    //insideDataSetBGRowLayout->addStretch(1);
    insideDataSetBGRowLayout->addWidget(dataSetEndLabel,1);
    insideDataSetBGRowLayout->addWidget(dataSetEndEdit,1);
    //insideDataSetBGRowLayout->addStretch(1);
    insideDataSetBGRowLayout->addWidget(dataSetIncLabel,1);
    insideDataSetBGRowLayout->addWidget(dataSetIncEdit,1);
    //insideDataSetBGRowLayout->addStretch(1);

    mainColModLayout->addWidget(samplingButtonGroup,1);
    insidePeriodicBGTopColLayout->addLayout(insidePeriodicBGTopRowLayout,1);
    //insidePeriodicBGTopRowLayout->addStretch(1);
    insidePeriodicBGTopRowLayout->addWidget(pointIncLabel,1);
    insidePeriodicBGTopRowLayout->addWidget(pointIncEdit,1);
    //insidePeriodicBGTopRowLayout->addStretch(1);
    insidePeriodicBGTopRowLayout->addWidget(lineIncLabel,1);
    insidePeriodicBGTopRowLayout->addWidget(lineIncEdit,1);
    //insidePeriodicBGTopRowLayout->addStretch(1);
    insidePeriodicBGTopColLayout->addLayout(insidePeriodicBGBottomRowLayout,1);
    insidePeriodicBGBottomRowLayout->addWidget(startPointLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(startPointEdit,1);
    //insidePeriodicBGBottomRowLayout->addStretch(1);
    insidePeriodicBGBottomRowLayout->addWidget(startLineLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(startLineEdit,1);
    //insidePeriodicBGBottomRowLayout->addStretch(1);
    insidePeriodicBGBottomRowLayout->addWidget(endPointLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(endPointEdit,1);
    //insidePeriodicBGBottomRowLayout->addStretch(1);
    insidePeriodicBGBottomRowLayout->addWidget(endLineLabel,1);
    insidePeriodicBGBottomRowLayout->addWidget(endLineEdit,1);
    //insidePeriodicBGBottomRowLayout->addStretch(1);

    mainColModLayout->addWidget(colFormatButtonGroup,1);
    colFormatBGColLayout->addLayout(colFormatBGTopRowLayout,1);
    //colFormatBGTopRowLayout->addStretch(1);
    colFormatBGTopRowLayout->addWidget(xColLabel,1);
    colFormatBGTopRowLayout->addWidget(xColumnEdit,1);
    //colFormatBGTopRowLayout->addStretch(1);
    colFormatBGTopRowLayout->addWidget(yColLabel,1);
    colFormatBGTopRowLayout->addWidget(yColumnEdit,1);
    //colFormatBGTopRowLayout->addStretch(1);
    colFormatBGTopRowLayout->addWidget(zColLabel,1);
    colFormatBGTopRowLayout->addWidget(zColumnEdit,1);
    colFormatBGColLayout->addLayout(colFormatBGMiddleRowLayout,1);
    colFormatBGMiddleRowLayout->addWidget(formatLabel,1);
    colFormatBGMiddleRowLayout->addWidget(formatEdit,1);
    colFormatBGColLayout->addLayout(colFormatBGBottomRowLayout,1);
    colFormatBGBottomRowLayout->addWidget(rawFormatLabel,1);
    colFormatBGBottomRowLayout->addWidget(rawFormatEdit,1);

    mainColModLayout->activate();
    addTab(datafileMods, i18n("&Datafile Modifiers"));

    setOKButton(i18n("&Close"));
    resize(500,470);
}


curveFitData::~curveFitData()
{
}
void curveFitData::doFit()
{
}
void curveFitData::dataFileOpen()
{
}
void curveFitData::paramFileOpen()
{
}

#include "curveFitData.moc"
