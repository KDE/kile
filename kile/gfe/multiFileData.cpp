/* -------------------------- multiFileData class --------------------------

   This class handles all operations related to the storage and manipulation of
   multiple files and their options from the GUI.

   *Note: This file has been converted to geometry management by hand.
   Qtarch will not regenerate this file exactly as is exists now.

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

   ------------------------------------------------------------------------*/

#include "multiFileData.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <klocale.h>

multiFileData::multiFileData
(
	QWidget* parent,
	const char* name
)
	:
	QTabDialog( parent, name, TRUE, 36864 )
{
  setCaption( i18n("Multiple Files") );

  // ----------------------- set up page one of the tab dialog ---------------

  QWidget* mainMultiFileOptions = new QWidget(this, "Main Options" );

  QLabel* FilesLabel;
  FilesLabel = new QLabel( mainMultiFileOptions, "FilesLabel" );
  FilesLabel->setMinimumSize( 80, 20 );
  FilesLabel->setMaximumSize( 32767, 20 );
  FilesLabel->setText( i18n("Files:") );
  FilesLabel->setAlignment( 289 );
  FilesLabel->setMargin( -1 );

  multiFileList = new QComboBox( FALSE, mainMultiFileOptions, "ComboBox_2" );
  multiFileList->setMinimumSize( 340, 20 );
  multiFileList->setMaximumSize( 32767, 20 );
  connect( multiFileList, SIGNAL(activated(const QString&)), this,
           SLOT(fileChanged(const QString&)) );
  multiFileList->setSizeLimit( 100 );
  //multiFileList->setAutoResize( FALSE );

  QPushButton* NewFileButton;
  NewFileButton = new QPushButton( mainMultiFileOptions, "NewFileButton" );
  NewFileButton->setMinimumSize( 100, 26 );
  connect( NewFileButton, SIGNAL(clicked()), this, SLOT(getNewFile()) );
  NewFileButton->setText( i18n("New File") );
  NewFileButton->setAutoRepeat( FALSE );
  //NewFileButton->setAutoResize( FALSE );

  QPushButton* deleteFileButton;
  deleteFileButton = new QPushButton( mainMultiFileOptions, "deleteFileButton" );
  deleteFileButton->setMinimumSize( 100, 26 );
  connect( deleteFileButton, SIGNAL(clicked()), this, SLOT(deleteFile()) );
  deleteFileButton->setText( i18n("Delete File") );
  deleteFileButton->setAutoRepeat( FALSE );
  //deleteFileButton->setAutoResize( FALSE );

  fileStyleList = new QComboBox( FALSE, mainMultiFileOptions, "ComboBox_4" );
  fileStyleList->setMinimumSize( 120, 20 );
  fileStyleList->setMaximumSize( 32767, 20 );
  fileStyleList->setAutoResize( FALSE );
  fileStyleList->insertItem( "points" );
  fileStyleList->insertItem( "lines" );
  fileStyleList->insertItem( "linespoints" );
  fileStyleList->insertItem( "impulses" );
  fileStyleList->insertItem( "dots" );
  fileStyleList->insertItem( "steps" );
  fileStyleList->insertItem( "fsteps" );
  fileStyleList->insertItem( "histeps" );
  fileStyleList->insertItem( "errorbars" );
  fileStyleList->insertItem( "xerrorbars" );
  fileStyleList->insertItem( "yerrorbars" );
  fileStyleList->insertItem( "xyerrorbars" );
  fileStyleList->insertItem( "boxes" );
  fileStyleList->insertItem( "boxerrorbars" );
  fileStyleList->insertItem( "boxxyerrorbars" );
  fileStyleList->insertItem( "financebars" );
  fileStyleList->insertItem( "candlesticks" );

  QLabel* styleLabel;
  styleLabel = new QLabel( mainMultiFileOptions, "styleLabel" );
  styleLabel->setMinimumSize( 80, 20 );
  styleLabel->setMaximumSize( 32767, 20 );
  styleLabel->setText( i18n("Style:") );
  styleLabel->setAlignment( 289 );
  styleLabel->setMargin( -1 );

  QButtonGroup* legendButtonGroup;
  legendButtonGroup = new QButtonGroup(mainMultiFileOptions, "legendButtonGroup" );
  legendButtonGroup->setMinimumSize( 250, 120 );
  legendButtonGroup->setMaximumSize( 32767, 32767 );
  legendButtonGroup->setFrameStyle( 49 );
  legendButtonGroup->setTitle( i18n("Legend Options") );
  legendButtonGroup->setAlignment( AlignCenter );
  legendButtonGroup->setExclusive( TRUE );

  QLabel* legendTitleLabel;
  legendTitleLabel = new QLabel(legendButtonGroup, "legendTitleLabel" );
  legendTitleLabel->setMinimumSize( 110, 20 );
  legendTitleLabel->setMaximumSize( 32767, 20 );
  legendTitleLabel->setText( i18n("File legend title:") );
  legendTitleLabel->setAlignment( 289 );
  legendTitleLabel->setMargin( -1 );

  legendTitleEdit = new QLineEdit(legendButtonGroup, "LineEdit_7" );
  legendTitleEdit->setMinimumSize( 280, 20 );
  legendTitleEdit->setMaximumSize( 32767, 20 );
  legendTitleEdit->setText( "" );
  legendTitleEdit->setMaxLength( 32767 );
  legendTitleEdit->setEchoMode( QLineEdit::Normal );
  legendTitleEdit->setFrame( TRUE );

  legendTitleDefaultButton = new QCheckBox(legendButtonGroup , "CheckBox_4" );
  legendTitleDefaultButton->setMinimumSize( 70, 20 );
  legendTitleDefaultButton->setMaximumSize( 32767, 20 );
  legendTitleDefaultButton->setText( "&default" );
  legendTitleDefaultButton->setAutoRepeat( FALSE );
  //legendTitleDefaultButton->setAutoResize( FALSE );
  legendTitleDefaultButton->setChecked( TRUE );

  legendTitlenotitleButton = new QCheckBox(legendButtonGroup, "CheckBox_5" );
  legendTitlenotitleButton->setMinimumSize( 60, 20 );
  legendTitlenotitleButton->setMaximumSize( 32767, 20 );
  legendTitlenotitleButton->setText( i18n("&notitle") );
  legendTitlenotitleButton->setAutoRepeat( FALSE );
  //legendTitlenotitleButton->setAutoResize( FALSE );

  // ------------------------ create layouts

  // main column layout for whole dialog
  QVBoxLayout* mainColLayout = new QVBoxLayout(mainMultiFileOptions, 5, -1, "mainCol");

  // top row for file dropbox
  QHBoxLayout* fileComboRowLayout = new QHBoxLayout(-1, "fileComboRow");

  // row for file manipulation pushbuttons
  QHBoxLayout* fileManipRowLayout = new QHBoxLayout(-1, "fileManipRow");

  // row for style options
  QHBoxLayout* styleRowLayout = new QHBoxLayout(-1, "styleRow");

  // column for inside button group
  QVBoxLayout* legendBGInsideColLayout = new QVBoxLayout(legendButtonGroup, 10, -1,
                                                         "legendBGInsideCol");

  // row for legend title inside button group
  QHBoxLayout* legendTitleRowLayout = new QHBoxLayout(-1, "legendTitleRow");

  // row for legend options pushbuttons
  QHBoxLayout* legendOptionsRowLayout = new QHBoxLayout(-1, "legendOptionsRow");

  // ----------- assemble layouts and widgets

  mainColLayout->addLayout(fileComboRowLayout ,0);
  fileComboRowLayout->addWidget(FilesLabel,0);
  fileComboRowLayout->addWidget(multiFileList,0);

  mainColLayout->addLayout(fileManipRowLayout ,0);
  fileManipRowLayout->addSpacing(85);
  fileManipRowLayout->addWidget(NewFileButton, 1);
  fileManipRowLayout->addStretch(1);
  fileManipRowLayout->addWidget(deleteFileButton, 1);
  fileManipRowLayout->addStretch(1);

  mainColLayout->addLayout(styleRowLayout ,0);
  styleRowLayout->addWidget(styleLabel ,0);
  styleRowLayout->addWidget(fileStyleList ,0);
  styleRowLayout->addStretch(1);

  mainColLayout->addWidget(legendButtonGroup ,0);
  legendBGInsideColLayout->addLayout(legendTitleRowLayout,0);
  legendBGInsideColLayout->addLayout(legendOptionsRowLayout,0);
  legendTitleRowLayout->addWidget(legendTitleLabel ,0);
  legendTitleRowLayout->addWidget(legendTitleEdit ,0);
  legendOptionsRowLayout->addStretch(1);
  legendOptionsRowLayout->addWidget(legendTitleDefaultButton ,0);
  legendOptionsRowLayout->addStretch(1);
  legendOptionsRowLayout->addWidget(legendTitlenotitleButton ,0);
  legendOptionsRowLayout->addStretch(1);

  addTab(mainMultiFileOptions, i18n("&Main") );
  mainColLayout->activate();

  // ----------------------- set up page two of the tab dialog ---------------

  QWidget* multiFileOptions = new QWidget(this, "File Options");

  QButtonGroup* interpButtonGroup;
  interpButtonGroup = new QButtonGroup(multiFileOptions, "interpButtonGroup" );
  interpButtonGroup->setMinimumSize( 460, 60 );
  interpButtonGroup->setMaximumSize( 32767, 32767 );
  interpButtonGroup->setFrameStyle( 49 );
  interpButtonGroup->setTitle( i18n("Interpolation && Approximation") );

  QButtonGroup* colFormatButtonGroup;
  colFormatButtonGroup = new QButtonGroup(multiFileOptions, "colFormatButtonGroup" );
  colFormatButtonGroup->setMinimumSize( 460, 150 );
  colFormatButtonGroup->setMaximumSize( 32767, 32767 );
  colFormatButtonGroup->setFrameStyle( 49 );
  colFormatButtonGroup->setTitle( i18n("Columns && Format") );

  QButtonGroup* samplingButtonGroup;
  samplingButtonGroup = new QButtonGroup( multiFileOptions, "samplingButtonGroup" );
  samplingButtonGroup->setMinimumSize( 460, 110 );
  samplingButtonGroup->setMaximumSize( 32767, 32767 );
  samplingButtonGroup->setFrameStyle( 49 );
  samplingButtonGroup->setTitle( i18n("Periodic Sampling") );

  QButtonGroup* dataSetButtonGroup;
  dataSetButtonGroup = new QButtonGroup( multiFileOptions, "dataSetButtonGroup" );
  dataSetButtonGroup->setMinimumSize( 460, 60 );
  dataSetButtonGroup->setMaximumSize( 32767, 32767 );
  dataSetButtonGroup->setFrameStyle( 49 );
  dataSetButtonGroup->setTitle( i18n("Data Set Selection") );

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

  dataSetStartEdit = new QLineEdit(dataSetButtonGroup, "dataSetStartLineEdit" );
  dataSetStartEdit->setMinimumSize( 30, 20 );
  dataSetStartEdit->setMaximumSize( 32767, 20 );
  dataSetStartEdit->setText( "" );
  dataSetStartEdit->setMaxLength( 32767 );
  dataSetStartEdit->setEchoMode( QLineEdit::Normal );
  dataSetStartEdit->setFrame( TRUE );

  dataSetEndEdit = new QLineEdit(dataSetButtonGroup, "dataSetEndLineEdit" );
  dataSetEndEdit->setMinimumSize( 30, 20 );
  dataSetEndEdit->setMaximumSize( 32767, 20 );
  dataSetEndEdit->setText( "" );
  dataSetEndEdit->setMaxLength( 32767 );
  dataSetEndEdit->setEchoMode( QLineEdit::Normal );
  dataSetEndEdit->setFrame( TRUE );

  QLabel* dataSetStartLabel;
  dataSetStartLabel = new QLabel(dataSetButtonGroup, "dataSetStartLabel" );
  dataSetStartLabel->setMinimumSize( 40, 20 );
  dataSetStartLabel->setMaximumSize( 32767, 20 );
  dataSetStartLabel->setText( i18n("Start:") );
  dataSetStartLabel->setAlignment( 289 );
  dataSetStartLabel->setMargin( -1 );

  QLabel* dataSetEndLabel;
  dataSetEndLabel = new QLabel(dataSetButtonGroup, "dataSetEndLabel" );
  dataSetEndLabel->setMinimumSize( 40, 20 );
  dataSetEndLabel->setMaximumSize( 32767, 20 );
  dataSetEndLabel->setText( i18n("End:") );
  dataSetEndLabel->setAlignment( 289 );
  dataSetEndLabel->setMargin( -1 );

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

  // create layouts

  // main column layout
  QVBoxLayout* mainColFileOptionsLayout = new QVBoxLayout(multiFileOptions,5);

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

  // assemble layouts and widgets
  mainColFileOptionsLayout->addWidget(dataSetButtonGroup,1);
  insideDataSetBGRowLayout->addStretch(1);
  insideDataSetBGRowLayout->addWidget(dataSetStartLabel,1);
  insideDataSetBGRowLayout->addWidget(dataSetStartEdit,1);
  insideDataSetBGRowLayout->addStretch(1);
  insideDataSetBGRowLayout->addWidget(dataSetEndLabel,1);
  insideDataSetBGRowLayout->addWidget(dataSetEndEdit,1);
  insideDataSetBGRowLayout->addStretch(1);
  insideDataSetBGRowLayout->addWidget(dataSetIncLabel,1);
  insideDataSetBGRowLayout->addWidget(dataSetIncEdit,1);
  insideDataSetBGRowLayout->addStretch(1);

  mainColFileOptionsLayout->addWidget(samplingButtonGroup,1);
  insidePeriodicBGTopColLayout->addLayout(insidePeriodicBGTopRowLayout,1);
  insidePeriodicBGTopRowLayout->addStretch(1);
  insidePeriodicBGTopRowLayout->addWidget(pointIncLabel,1);
  insidePeriodicBGTopRowLayout->addWidget(pointIncEdit,1);
  insidePeriodicBGTopRowLayout->addStretch(1);
  insidePeriodicBGTopRowLayout->addWidget(lineIncLabel,1);
  insidePeriodicBGTopRowLayout->addWidget(lineIncEdit,1);
  insidePeriodicBGTopRowLayout->addStretch(1);
  insidePeriodicBGTopColLayout->addLayout(insidePeriodicBGBottomRowLayout,1);
  insidePeriodicBGBottomRowLayout->addWidget(startPointLabel,1);
  insidePeriodicBGBottomRowLayout->addWidget(startPointEdit,1);
  insidePeriodicBGBottomRowLayout->addStretch(1);
  insidePeriodicBGBottomRowLayout->addWidget(startLineLabel,1);
  insidePeriodicBGBottomRowLayout->addWidget(startLineEdit,1);
  insidePeriodicBGBottomRowLayout->addStretch(1);
  insidePeriodicBGBottomRowLayout->addWidget(endPointLabel,1);
  insidePeriodicBGBottomRowLayout->addWidget(endPointEdit,1);
  insidePeriodicBGBottomRowLayout->addStretch(1);
  insidePeriodicBGBottomRowLayout->addWidget(endLineLabel,1);
  insidePeriodicBGBottomRowLayout->addWidget(endLineEdit,1);
  insidePeriodicBGBottomRowLayout->addStretch(1);

  mainColFileOptionsLayout->addWidget(colFormatButtonGroup,1);
  colFormatBGColLayout->addLayout(colFormatBGTopRowLayout,1);
  colFormatBGTopRowLayout->addStretch(1);
  colFormatBGTopRowLayout->addWidget(xColLabel,1);
  colFormatBGTopRowLayout->addWidget(xColumnEdit,1);
  colFormatBGTopRowLayout->addStretch(1);
  colFormatBGTopRowLayout->addWidget(yColLabel,1);
  colFormatBGTopRowLayout->addWidget(yColumnEdit,1);
  colFormatBGTopRowLayout->addStretch(1);
  colFormatBGTopRowLayout->addWidget(zColLabel,1);
  colFormatBGTopRowLayout->addWidget(zColumnEdit,1);
  colFormatBGColLayout->addLayout(colFormatBGMiddleRowLayout,1);
  colFormatBGMiddleRowLayout->addWidget(formatLabel,1);
  colFormatBGMiddleRowLayout->addWidget(formatEdit,1);
  colFormatBGColLayout->addLayout(colFormatBGBottomRowLayout,1);
  colFormatBGBottomRowLayout->addWidget(rawFormatLabel,1);
  colFormatBGBottomRowLayout->addWidget(rawFormatEdit,1);

  mainColFileOptionsLayout->addWidget(interpButtonGroup,1);
  insideInterpBGRowLayout->addWidget(interpLabel,1);
  insideInterpBGRowLayout->addWidget(interpList,1);
  insideInterpBGRowLayout->addStretch(1);

  mainColFileOptionsLayout->activate();
  addTab(multiFileOptions, i18n("Mo&difiers") );

  // ----------------------- set up page three of the tab dialog ---------------

  QWidget* multiFileFiltering = new QWidget(this, "Filtering Options" );

  QButtonGroup* QuoteButtonGroup;
  QuoteButtonGroup = new QButtonGroup( multiFileFiltering, "QuoteButtonGroup" );
  QuoteButtonGroup->setMinimumSize( 380, 60 );
  QuoteButtonGroup->setMaximumSize( 32767, 32767 );
  QuoteButtonGroup->setFrameStyle( 49 );
  QuoteButtonGroup->setTitle( i18n("Filter Command Quoting") );
  QuoteButtonGroup->setExclusive( TRUE );

  QLabel* filterCmdLabel;
  filterCmdLabel = new QLabel( multiFileFiltering, "filterCmdLabel" );
  filterCmdLabel->setMinimumSize( 100, 20 );
  filterCmdLabel->setMaximumSize( 32767, 20 );
  filterCmdLabel->setText( i18n("Filter command:") );
  filterCmdLabel->setAlignment( 289 );
  filterCmdLabel->setMargin( -1 );

  filterEdit = new QLineEdit( multiFileFiltering, "FilterLineEdit" );
  filterEdit->setMinimumSize( 280, 20 );
  filterEdit->setMaximumSize( 32767, 20 );
  filterEdit->setText( "" );
  filterEdit->setMaxLength( 32767 );
  filterEdit->setEchoMode( QLineEdit::Normal );
  filterEdit->setFrame( TRUE );

  doubleQuoteRB = new QRadioButton(QuoteButtonGroup,"doubleQuoteRadioButton" );
  doubleQuoteRB->setMinimumSize( 110, 20 );
  doubleQuoteRB->setMaximumSize( 32767, 20 );
  doubleQuoteRB->setText( i18n("Double duotes") );
  doubleQuoteRB->setAutoRepeat( FALSE );
  //doubleQuoteRB->setAutoResize( FALSE );
  doubleQuoteRB->setChecked( TRUE );

  singleQuoteRB = new QRadioButton(QuoteButtonGroup,"SingleQuoteRadioButton" );
  singleQuoteRB->setMinimumSize( 110, 20 );
  singleQuoteRB->setMaximumSize( 32767, 20 );
  singleQuoteRB->setText( i18n("Single quotes") );
  singleQuoteRB->setAutoRepeat( FALSE );
  //singleQuoteRB->setAutoResize( FALSE );

  QPushButton* InsertCurrentPushButton;
  InsertCurrentPushButton = new QPushButton( multiFileFiltering, "InsertCurrentPushButton" );
  InsertCurrentPushButton->setMinimumSize( 150, 26 );
  connect( InsertCurrentPushButton, SIGNAL(clicked()), SLOT(insertCurrentFilename()) );
  InsertCurrentPushButton->setText( i18n("Insert &Current Filename") );
  InsertCurrentPushButton->setAutoRepeat( FALSE );
  //InsertCurrentPushButton->setAutoResize( FALSE );

  QPushButton* insertNewButton;
  insertNewButton = new QPushButton( multiFileFiltering, "insertNewButton" );
  insertNewButton->setMinimumSize( 150, 26 );
  connect( insertNewButton, SIGNAL(clicked()), SLOT(insertNewFilename()) );
  insertNewButton->setText( i18n("Insert &New Filename...") );
  insertNewButton->setAutoRepeat( FALSE );
  //insertNewButton->setAutoResize( FALSE );

  // ------------------------ create layouts

  // main column layout for whole dialog
  QVBoxLayout* filterMainColLayout = new QVBoxLayout(multiFileFiltering,
                                                     5, -1, "mainCol");

  // row for filter command
  QHBoxLayout* filterCmdRowLayout = new QHBoxLayout(-1, "filterCmdRow");

  // row inside button group
  QHBoxLayout* quoteBoxRowLayout = new QHBoxLayout(QuoteButtonGroup,
                                                   4, -1, "quoteBoxRow");

  // row for filename buttons
  QHBoxLayout* filenamePBRowLayout = new QHBoxLayout(-1, "filenamePBRow");

  // ----------- assemble layouts and widgets

  filterMainColLayout->addLayout(filterCmdRowLayout,0);
  filterCmdRowLayout->addWidget(filterCmdLabel,0);
  filterCmdRowLayout->addWidget(filterEdit,0);

  filterMainColLayout->addWidget(QuoteButtonGroup,0);
  quoteBoxRowLayout->addStretch(1);
  quoteBoxRowLayout->addWidget(doubleQuoteRB,0);
  quoteBoxRowLayout->addStretch(1);
  quoteBoxRowLayout->addWidget(singleQuoteRB,0);
  quoteBoxRowLayout->addStretch(1);

  filterMainColLayout->addLayout(filenamePBRowLayout,0);
  filenamePBRowLayout->addStretch(1);
  filenamePBRowLayout->addWidget(InsertCurrentPushButton,0);
  filenamePBRowLayout->addStretch(1);
  filenamePBRowLayout->addWidget(insertNewButton,0);
  filenamePBRowLayout->addStretch(1);

  filterMainColLayout->activate();
  addTab(multiFileFiltering, i18n("&Filtering") );

  setApplyButton(i18n("&Apply"));
  setOKButton(i18n("&Close"));
  connect(this, SIGNAL(applyButtonPressed()), SLOT(apply()) );
resize(500,500);

}


multiFileData::~multiFileData()
{
}

void multiFileData::fileChanged(const QString&)
{
}

void multiFileData::getNewFile()
{
}

void multiFileData::deleteFile()
{
}

void multiFileData::apply()
{
}

void multiFileData::insertCurrentFilename()
{
}

void multiFileData::insertNewFilename()
{
}

#include "multiFileData.moc"
