/***************************************************************************
                          qplotdialog.cpp  -  description
                             -------------------
    begin                : Sat Aug 25 2001
    copyright            : (C) 2001 by Pascal Brachet
    email                :
    adaptation of the Xgfe program: X Windows GUI front end to Gnuplot
    Copyright (C) 1998 David Ishee
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qplotdialog.h"
#include <qlabel.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlayout.h>
#include <kfiledialog.h>
#include <klocale.h>

Qplotdialog::Qplotdialog(QWidget *parent, const char *name )
: QMainWindow( parent, name, WDestructiveClose )

{
  gnuInt = new gnuInterface;

  // setup gnuplot interface

  gnuRC = gnuInt->openGnuplot();

  if (gnuRC == NULL) // trouble opening gnuplot
  {
    QMessageBox::critical(0, "Kile",
                          i18n("Could not open pipe to Gnuplot!\n"
                          "Application will now exit"));
  }
  // end setup of gnuplot

  QWidget *page = new QWidget(this);
  setCentralWidget( page );

  QGridLayout *gbox = new QGridLayout( page, 19, 6,5,5,"");
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  gbox->addColSpacing( 0, fontMetrics().lineSpacing() );

  GfeLabel = new QLabel( page, "GfeLabel" );
  GfeLabel->setAlignment( AlignCenter );
  GfeLabel->setText( i18n("<b>Gnuplot Front End for Kile</b>") );
  GfeLabel->setMinimumWidth(425);
  gbox->addMultiCellWidget(GfeLabel,0,0,0,5,Qt::AlignCenter);

	filenameCB = new QCheckBox( page, "CheckBox_1" );
	filenameCB->setText( i18n("Data file:") );
  gbox->addWidget(filenameCB , 1, 0 );

  filenameEdit = new QLineEdit( page, "LineEdit_23" );
	filenameEdit->setText( i18n("none") );
	filenameEdit->setMaxLength( 32767 );
  gbox->addMultiCellWidget(filenameEdit,1,1,1,3,Qt::AlignLeft);

	QPushButton* PushButton_6;
	PushButton_6 = new QPushButton( page, "PushButton_6" );
  connect( PushButton_6, SIGNAL(clicked()), this, SLOT(dataFileOpen()) );
	PushButton_6->setText( i18n("Open...") );
  gbox->addMultiCellWidget(PushButton_6,1,1,4,5,Qt::AlignLeft);

	multiFileCheckbox = new QCheckBox( page, "CheckBox_3" );
	multiFileCheckbox->setText( i18n("Multiple data files") );
  gbox->addMultiCellWidget(multiFileCheckbox,2,2,0,3,Qt::AlignLeft);

	QPushButton* PushButton_4;
	PushButton_4 = new QPushButton( page, "PushButton_4" );
  connect( PushButton_4, SIGNAL(clicked()), this, SLOT(getMultiFile()) );
	PushButton_4->setText( i18n("Define...") );
  gbox->addMultiCellWidget(PushButton_4,2,2,4,5,Qt::AlignLeft);

	functionCB = new QCheckBox( page, "CheckBox_2" );
	functionCB->setText( i18n("Function") );
  gbox->addWidget(functionCB , 3, 0 );


	functionEdit = new QLineEdit( page, "LineEdit_1" );
	functionEdit->setText( "" );
	functionEdit->setMaxLength( 32767 );
  gbox->addMultiCellWidget(functionEdit,3,3,1,5,Qt::AlignLeft);

	multiFuncCheckbox = new QCheckBox( page, "CheckBox_4" );
	multiFuncCheckbox->setText( i18n("Multiple functions") );
  gbox->addMultiCellWidget(multiFuncCheckbox,4,4,0,3,Qt::AlignLeft);


	QPushButton* PushButton_5;
	PushButton_5 = new QPushButton( page, "PushButton_5" );
  connect( PushButton_5, SIGNAL(clicked()), this, SLOT(getMultiFunction()) );
	PushButton_5->setText( i18n("Define...") );
  gbox->addMultiCellWidget(PushButton_5,4,4,4,5,Qt::AlignLeft);


    Line1 = new QFrame( page, "Line1" );
    Line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    Line1->setMinimumWidth(425);
    gbox->addMultiCellWidget(Line1,5,5,0,5,Qt::AlignCenter);


  QLabel* Label_4;
	Label_4 = new QLabel( page, "Label_4" );
	Label_4->setText( i18n("Variable X:") );
  gbox->addWidget(Label_4 , 6, 0 );



	varX = new QLineEdit( page, "LineEdit_2" );
	varX->setText( "" );
	varX->setMaxLength( 32767 );
  gbox->addWidget( varX, 6, 1 );



	QLabel* Label_5;
	Label_5 = new QLabel( page, "Label_5" );
	Label_5->setText( i18n("Start:") );
  gbox->addWidget( Label_5, 6, 2,Qt::AlignRight );


	xStart = new QLineEdit( page, "LineEdit_3" );
	xStart->setText( "" );
	xStart->setMaxLength( 32767 );
  gbox->addWidget( xStart, 6, 3 );



	QLabel* Label_6;
	Label_6 = new QLabel( page, "Label_6" );
	Label_6->setText( i18n("End:") );
  gbox->addWidget( Label_6, 6, 4,Qt::AlignRight );


	xEnd = new QLineEdit( page, "LineEdit_4" );
	xEnd->setText( "" );
	xEnd->setMaxLength( 32767 );
  gbox->addWidget( xEnd, 6, 5 );



	QLabel* Label_10;
	Label_10 = new QLabel( page, "Label_10" );
	Label_10->setText( i18n("Variable Y:") );
  gbox->addWidget( Label_10, 7, 0 );



	varY = new QLineEdit( page, "LineEdit_8" );
	varY->setText( "" );
	varY->setMaxLength( 32767 );
  gbox->addWidget( varY, 7, 1 );



	QLabel* Label_12;
	Label_12 = new QLabel( page, "Label_12" );
	Label_12->setText( i18n("Start:") );
  gbox->addWidget( Label_12, 7, 2 ,Qt::AlignRight);



	yStart = new QLineEdit( page, "LineEdit_9" );
	yStart->setText( "" );
	yStart->setMaxLength( 32767 );
  gbox->addWidget( yStart, 7, 3 );



	QLabel* Label_13;
	Label_13 = new QLabel( page, "Label_13" );
	Label_13->setText( i18n("End:") );
  gbox->addWidget( Label_13, 7, 4,Qt::AlignRight );



	yEnd = new QLineEdit( page, "LineEdit_10" );
	yEnd->setText( "" );
	yEnd->setMaxLength( 32767 );
  gbox->addWidget( yEnd, 7, 5 );


	QLabel* Label_14;
	Label_14 = new QLabel( page, "Label_14" );
	Label_14->setText( i18n("Variable Z:") );
  gbox->addWidget( Label_14, 8, 0 );



	QLabel* Label_15;
	Label_15 = new QLabel( page, "Label_15" );
	Label_15->setText( i18n("Start:") );
  gbox->addWidget( Label_15, 8, 2,Qt::AlignRight );



	zStart = new QLineEdit( page, "LineEdit_12" );
	zStart->setText( "" );
	zStart->setMaxLength( 32767 );
  gbox->addWidget( zStart, 8, 3 );



	QLabel* Label_16;
	Label_16 = new QLabel( page, "Label_16" );
	Label_16->setText( i18n("End:") );
  gbox->addWidget( Label_16, 8, 4,Qt::AlignRight );



	zEnd = new QLineEdit( page, "LineEdit_13" );
	zEnd->setText( "" );
	zEnd->setMaxLength( 32767 );
  gbox->addWidget( zEnd, 8, 5 );


    Line2 = new QFrame( page, "Line2" );
    Line2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    Line2->setMinimumWidth(425);
    gbox->addMultiCellWidget(Line2,9,9,0,5,Qt::AlignCenter);




    QLabel* Label_22;
	Label_22 = new QLabel( page, "Label_20" );
	Label_22->setText( i18n("X offset:") );
  gbox->addWidget( Label_22, 10, 4 );


    QLabel* Label_23;
	Label_23 = new QLabel( page, "Label_23" );
	Label_23->setText( i18n("Y offset:") );
  gbox->addWidget( Label_23, 10, 5 );



	QLabel* Label_7;
	Label_7 = new QLabel( page, "Label_7" );
	Label_7->setText( i18n("X label:") );
  gbox->addWidget( Label_7, 11, 0 );


	xLabel = new QLineEdit( page, "LineEdit_7" );
	xLabel->setText( "" );
	xLabel->setMaxLength( 32767 );
  gbox->addMultiCellWidget(xLabel,11,11,1,3,Qt::AlignLeft);


    XLabelOffset_X = new QLineEdit( page, "LineEdit_15" );
	XLabelOffset_X->setText( "" );
	XLabelOffset_X->setMaxLength( 32767 );
  gbox->addWidget(XLabelOffset_X , 11, 4 );



	XLabelOffset_Y = new QLineEdit( page, "LineEdit_16" );
	XLabelOffset_Y->setText( "" );
	XLabelOffset_Y->setMaxLength( 32767 );
  gbox->addWidget(XLabelOffset_Y , 11, 5 );



	QLabel* Label_8;
	Label_8 = new QLabel( page, "Label_8" );
	Label_8->setText( i18n("Y label:") );
  gbox->addWidget( Label_8, 12, 0 );



	yLabel = new QLineEdit( page, "LineEdit_6" );
	yLabel->setText( "" );
	yLabel->setMaxLength( 32767 );
  gbox->addMultiCellWidget(yLabel,12,12,1,3,Qt::AlignLeft);



	YLabelOffset_X = new QLineEdit( page, "LineEdit_17" );
	YLabelOffset_X->setText( "" );
	YLabelOffset_X->setMaxLength( 32767 );
  gbox->addWidget(YLabelOffset_X , 12, 4 );


	YLabelOffset_Y = new QLineEdit( page, "LineEdit_18" );
	YLabelOffset_Y->setText( "" );
	YLabelOffset_Y->setMaxLength( 32767 );
  gbox->addWidget(YLabelOffset_Y , 12, 5 );


	QLabel* Label_9;
	Label_9 = new QLabel( page, "Label_9" );
	Label_9->setText( i18n("Z label:") );
  gbox->addWidget( Label_9, 13, 0 );



  zLabel = new QLineEdit( page, "LineEdit_5" );
	zLabel->setText( "" );
	zLabel->setMaxLength( 32767 );
  gbox->addMultiCellWidget(zLabel,13,13,1,3,Qt::AlignLeft);


	ZLabelOffset_X = new QLineEdit( page, "LineEdit_19" );
	ZLabelOffset_X->setText( "" );
	ZLabelOffset_X->setMaxLength( 32767 );
  gbox->addWidget(ZLabelOffset_X , 13, 4 );



	ZLabelOffset_Y = new QLineEdit( page, "LineEdit_20" );
	ZLabelOffset_Y->setText( "" );
	ZLabelOffset_Y->setMaxLength( 32767 );
  gbox->addWidget(ZLabelOffset_Y , 13, 5 );



    QLabel* Label_21;
	Label_21 = new QLabel( page, "Label_21" );
	Label_21->setText( i18n("Title:") );
  gbox->addWidget( Label_21, 14, 0 );


	titleLabel = new QLineEdit( page, "LineEdit_14" );
	titleLabel->setText( "" );
	titleLabel->setMaxLength( 32767 );
  gbox->addMultiCellWidget(titleLabel,14,14,1,3,Qt::AlignLeft);


    titleOffset_X = new QLineEdit( page, "LineEdit_21" );
	titleOffset_X->setText( "" );
	titleOffset_X->setMaxLength( 32767 );
  gbox->addWidget(titleOffset_X , 14, 4 );


	titleOffset_Y = new QLineEdit( page, "LineEdit_22" );
	titleOffset_Y->setText( "" );
	titleOffset_Y->setMaxLength( 32767 );
  gbox->addWidget(titleOffset_Y , 14, 5 );

  QLabel* Label_19;
  Label_19 = new QLabel( page, "Label_19" );
  Label_19->setText( i18n("Terminal:") );
  gbox->addWidget(Label_19 , 15, 0 );


  termLabel = new QLabel( page, "Label_20" );
  termLabel->setText( "x11" );
  gbox->addWidget(termLabel , 15, 1 );


  QLabel* Label_17;
  Label_17 = new QLabel( page, "Label_17" );
  Label_17->setText( i18n("Output:") );
  gbox->addWidget(Label_17 , 15, 3,Qt::AlignRight );

  outputLabel = new QLineEdit( page, "Label_18" );
  outputLabel->setReadOnly(true);
  outputLabel->setText( "stdout" );
  gbox->addMultiCellWidget(outputLabel,15,15,4,5,Qt::AlignLeft);

	QPushButton* PushButton_7;
	PushButton_7 = new QPushButton( page, "PushButton_7" );
	PushButton_7->setText( i18n("Output File...") );
  connect( PushButton_7, SIGNAL(clicked()), this, SLOT(getOutput()) );
  gbox->addMultiCellWidget(PushButton_7,16,16,0,1,Qt::AlignCenter);

 	QPushButton* PushButton_8;
	PushButton_8 = new QPushButton( page, "PushButton_8" );
	PushButton_8->setText( i18n("Reset Output") );
  connect( PushButton_8, SIGNAL(clicked()), this, SLOT(resetOutput()) );
  gbox->addMultiCellWidget(PushButton_8,16,16,4,5,Qt::AlignCenter);


    Line3 = new QFrame( page, "Line3" );
    Line3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    Line3->setMinimumWidth(425);
    gbox->addMultiCellWidget(Line3,17,17,0,5,Qt::AlignCenter);


	QPushButton* PushButton_1;
	PushButton_1 = new QPushButton( page, "PushButton_1" );
	PushButton_1->setText( i18n("Plot...") );
    PushButton_1->setDefault(TRUE);
    connect( PushButton_1, SIGNAL(clicked()), this, SLOT(plot()) );
    gbox->addMultiCellWidget(PushButton_1,18,18,0,1,Qt::AlignCenter);

	QPushButton* PushButton_2;
	PushButton_2 = new QPushButton( page, "PushButton_2" );
	PushButton_2->setText( i18n("Replot...") );
    connect(PushButton_2, SIGNAL(clicked()), this, SLOT(replot()));
    gbox->addMultiCellWidget(PushButton_2,18,18,2,3,Qt::AlignCenter);


	QPushButton* PushButton_3;
	PushButton_3 = new QPushButton( page, "PushButton_3" );
    connect( PushButton_3, SIGNAL(clicked()), this, SLOT(xgfeQuit()) );
	PushButton_3->setText( i18n("Quit") );
  gbox->addMultiCellWidget(PushButton_3,18,18,4,5,Qt::AlignCenter);

// create menubar

    // file menu
    file = new QPopupMenu;
    file->insertItem(i18n("Save Gnuplot..."), this, SLOT(save()));
    file->insertItem(i18n("Load Gnuplot..."), this, SLOT(load()));
    file->insertSeparator();
    file->insertItem(i18n("Save Xgfe..."), this, SLOT(saveXgfe()));
    file->insertItem(i18n("Load Xgfe..."), this, SLOT(loadXgfe()));
    file->insertSeparator();
    file->insertItem(i18n("Quit"), this, SLOT(xgfeQuit()));

    // file plotting style popup menu
    fileStyle = new QPopupMenu;
    file_p_id = fileStyle->insertItem(i18n("Points"), this, SLOT(setFilePoints()));
    file_l_id = fileStyle->insertItem(i18n("Lines"), this, SLOT(setFileLines()));
    file_lp_id = fileStyle->insertItem(i18n("Linespoints"), this, SLOT(setFileLinesPoints()));
    file_i_id = fileStyle->insertItem(i18n("Impulses"), this, SLOT(setFileImpulses()));
    file_d_id = fileStyle->insertItem(i18n("Dots"), this, SLOT(setFileDots()));
    file_s_id = fileStyle->insertItem(i18n("Steps"), this, SLOT(setFileSteps()));
    file_fs_id = fileStyle->insertItem(i18n("Fsteps"), this, SLOT(setFileFsteps()));
    file_hs_id = fileStyle->insertItem(i18n("Histeps"), this, SLOT(setFileHisteps()));
    file_eb_id = fileStyle->insertItem(i18n("Errorbars"), this, SLOT(setFileErrorbars()));
    file_xeb_id = fileStyle->insertItem(i18n("Xerrorbars"), this, SLOT(setFileXerrorbars()));
    file_yeb_id = fileStyle->insertItem(i18n("Yerrorbars"), this, SLOT(setFileYerrorbars()));
    file_xyeb_id = fileStyle->insertItem(i18n("Xyerrorbars"), this, SLOT(setFileXyerrorbars()));
    file_b_id = fileStyle->insertItem(i18n("Boxes"), this, SLOT(setFileBoxes()));
    file_be_id = fileStyle->insertItem(i18n("Boxerrorbars"), this, SLOT(setFileBoxerrorbars()));
    file_bxye_id = fileStyle->insertItem(i18n("Boxxyerrorbars"), this, SLOT(setFileBoxxyerrorbars()));
    file_fin_id = fileStyle->insertItem(i18n("Financebars"), this, SLOT(setFileFinancebars()));
    file_cs_id = fileStyle->insertItem(i18n("Candlesticks"), this, SLOT(setFileCandlesticks()));
    fileStyle->setCheckable(TRUE);
    fileStyle->setItemChecked(file_p_id, TRUE);

    // function plotting style popup menu
    funcStyle = new QPopupMenu;
    func_p_id = funcStyle->insertItem(i18n("Points"), this, SLOT(setFuncPoints()));
    func_l_id = funcStyle->insertItem(i18n("Lines"), this, SLOT(setFuncLines()));
    func_lp_id = funcStyle->insertItem(i18n("Linespoints"), this, SLOT(setFuncLinesPoints()));
    func_i_id = funcStyle->insertItem(i18n("Impulses"), this, SLOT(setFuncImpulses()));
    func_d_id = funcStyle->insertItem(i18n("Dots"), this, SLOT(setFuncDots()));
    func_s_id = funcStyle->insertItem(i18n("Steps"), this, SLOT(setFuncSteps()));
    func_eb_id = funcStyle->insertItem(i18n("Errorbars"), this, SLOT(setFuncErrorbars()));
    func_b_id = funcStyle->insertItem(i18n("Boxes"), this, SLOT(setFuncBoxes()));
    funcStyle->setCheckable(TRUE);
    funcStyle->setItemChecked(func_l_id, TRUE);

    // file plotting type menu
    filePlotType = new QPopupMenu();
    file2d_id = filePlotType->insertItem(i18n("2D"), this, SLOT(setFilePlotType2d()));
    file3d_id = filePlotType->insertItem(i18n("3D"), this, SLOT(setFilePlotType3d()));
    filePlotType->setCheckable(TRUE);
    filePlotType->setItemChecked(file2d_id, TRUE);

    // function plotting type menu
    funcPlotType = new QPopupMenu();
    func2d_id = funcPlotType->insertItem(i18n("2D"), this, SLOT(setFuncPlotType2d()));
    func3d_id = funcPlotType->insertItem(i18n("3D"), this, SLOT(setFuncPlotType3d()));
    funcPlotType->setCheckable(TRUE);
    funcPlotType->setItemChecked(func2d_id, TRUE);


    // terminal menu
    terminals = new QPopupMenu;
    bfig_t_id = terminals->insertItem("bfig", this, SLOT(setTermBfig()));
    corel_t_id = terminals->insertItem("corel", this, SLOT(setTermCorel()));
    dxf_t_id = terminals->insertItem("dxf", this, SLOT(setTermDxf()));
    eepic_t_id = terminals->insertItem("eepic", this, SLOT(setTermEepic()));
    emtex_t_id = terminals->insertItem("emtex", this, SLOT(setTermEmtex()));
    fig_t_id = terminals->insertItem("fig", this, SLOT(setTermFig()));
    latex_t_id = terminals->insertItem("latex", this, SLOT(setTermLatex()));
    pbm_t_id = terminals->insertItem("pbm", this, SLOT(setTermPbm()));
    ps_t_id = terminals->insertItem("postscript", this, SLOT(setTermPostscript()));
    pslatex_t_id = terminals->insertItem("pslatex", this, SLOT(setTermPslatex()));
    pstricks_t_id = terminals->insertItem("pstricks", this, SLOT(setTermPstricks()));
    table_t_id = terminals->insertItem("table", this, SLOT(setTermTable()));
    texdraw_t_id = terminals->insertItem("texdraw", this, SLOT(setTermTexdraw()));
    tgif_t_id = terminals->insertItem("tgif", this, SLOT(setTermTgif()));
    tpic_t_id = terminals->insertItem("tpic", this, SLOT(setTermTpic()));
    x11_t_id = terminals->insertItem("x11", this, SLOT(setTermX11()));
    terminals->setCheckable(TRUE);
    terminals->setItemChecked(x11_t_id, TRUE);

    output = ""; // initialize output to empty (stdout)

    // datafile menu
    datafileOpMenu = new QPopupMenu;
    datafileOpMenu->insertItem(i18n("Type (2D/3D)"), filePlotType);
    datafileOpMenu->insertItem(i18n("Style"), fileStyle);
    datafileOpMenu->insertItem(i18n("Legend Title"),this, SLOT(setFileLegendTitle()));
    datafileOpMenu->insertItem(i18n("Modifiers"), this, SLOT(getFileOptions()) );
    datafileOpMenu->insertItem(i18n("Filtering"), this, SLOT(setFileFilter()));

    // functions menu
    funcOpMenu = new QPopupMenu;
    funcOpMenu->insertItem(i18n("Type (2D/3D)"),funcPlotType);
    funcOpMenu->insertItem(i18n("Style"),funcStyle );
    funcOpMenu->insertItem(i18n("Legend Title"), this, SLOT(setFuncLegendTitle()));

    d3Menu = new QPopupMenu;
    d3Menu->insertItem(i18n("Rotation"), this, SLOT(getRotation()));
    d3Menu->insertItem(i18n("Tics Level"), this, SLOT(getTicsLevel()));
    d3HiddenLine_id = d3Menu->insertItem(i18n("Hidden Line Removal"), this,SLOT(set3dHiddenLine()));
    d3Menu->insertItem(i18n("Isolines"), this, SLOT(setIsolines()));
    d3Menu->setCheckable(TRUE);

    // option menu
    options = new QPopupMenu;
    options->insertItem(i18n("Plot Size"), this, SLOT(setPlotSize()));
    options->insertItem(i18n("Reset Size"), this, SLOT(resetSize()));
    options->insertItem(i18n("Legend"), this, SLOT(getLegendOps()));
    options->insertItem(i18n("Log Scale"), this, SLOT(setLogScaleOptions()));
    options->insertItem(i18n("Bar Size"), this, SLOT(setBarOptions()));
    options->insertItem(i18n("Reset Bar Size"), this, SLOT(resetBarOptions()));
    options->insertItem(i18n("Box Width"), this, SLOT(setBoxWidthOption()));
    options->insertItem(i18n("Reset Box Width"), this, SLOT(resetBoxWidthOption()));
    options->insertItem(i18n("Tics"), this, SLOT(setTicsOptions()));
    options->insertItem(i18n("Curve Fitting"), this, SLOT(getCurveFit()));

    // help menu
    help = new QPopupMenu;
    help->insertItem(i18n("About"), this, SLOT(showAbout()));

    menuBar()->insertItem(i18n("File"), file);
    menuBar()->insertItem(i18n("Terminal"), terminals);
    menuBar()->insertItem(i18n("Datafile"), datafileOpMenu);
    menuBar()->insertItem(i18n("Function"), funcOpMenu);
    menuBar()->insertItem(i18n("3DPlots"), d3Menu);
    menuBar()->insertItem(i18n("Options"), options);
    menuBar()->insertItem(i18n("About"),help);

    this->resize(400,500);




}
Qplotdialog::~Qplotdialog(){
}
void Qplotdialog::plot()
{
}

void Qplotdialog::replot()
{
}

void Qplotdialog::dataFileOpen()
{
}

void Qplotdialog::save()
{
}

void Qplotdialog::load()
{
}

void Qplotdialog::saveXgfe()
{
}

void Qplotdialog::loadXgfe()
{
}

void Qplotdialog::xgfeQuit()
{
}

void Qplotdialog::setFilePoints()
{
}

void Qplotdialog::setFileLines()
{
}

void Qplotdialog::setFileLinesPoints()
{
}

void Qplotdialog::setFileImpulses()
{
}

void Qplotdialog::setFileDots()
{
}

void Qplotdialog::setFileSteps()
{
}

void Qplotdialog::setFileFsteps()
{
}

void Qplotdialog::setFileHisteps()
{
}

void Qplotdialog::setFileErrorbars()
{
}

void Qplotdialog::setFileXerrorbars()
{
}

void Qplotdialog::setFileYerrorbars()
{
}

void Qplotdialog::setFileXyerrorbars()
{
}

void Qplotdialog::setFileBoxes()
{
}

void Qplotdialog::setFileBoxerrorbars()
{
}

void Qplotdialog::setFileBoxxyerrorbars()
{
}

void Qplotdialog::setFileFinancebars()
{
}

void Qplotdialog::setFileCandlesticks()
{
}

void Qplotdialog::setFuncPoints()
{
}

void Qplotdialog::setFuncLines()
{
}

void Qplotdialog::setFuncLinesPoints()
{
}

void Qplotdialog::setFuncImpulses()
{
}

void Qplotdialog::setFuncDots()
{
}

void Qplotdialog::setFuncSteps()
{
}

void Qplotdialog::setFuncErrorbars()
{
}

void Qplotdialog::setFuncBoxes()
{
}

void Qplotdialog::setFilePlotType2d()
{
}

void Qplotdialog::setFilePlotType3d()
{
}

void Qplotdialog::setFuncPlotType2d()
{
}

void Qplotdialog::setFuncPlotType3d()
{
}

void Qplotdialog::setTermBfig()
{
}

void Qplotdialog::setTermCorel()
{
}

void Qplotdialog::setTermDxf()
{
}

void Qplotdialog::setTermEepic()
{
}

void Qplotdialog::setTermEmtex()
{
}

void Qplotdialog::setTermFig()
{
}

void Qplotdialog::setTermLatex()
{
}

void Qplotdialog::setTermPbm()
{
}

void Qplotdialog::setTermPostscript()
{
}

void Qplotdialog::setTermPslatex()
{
}

void Qplotdialog::setTermPstricks()
{
}

void Qplotdialog::setTermTable()
{
}

void Qplotdialog::setTermTexdraw()
{
}

void Qplotdialog::setTermTgif()
{
}

void Qplotdialog::setTermTpic()
{
}

void Qplotdialog::setTermX11()
{
}

void Qplotdialog::getOutput()
{
  QString temp;
  QString f = KFileDialog::getSaveFileName( QDir::currentDirPath(),"", this,i18n("Output File") );
  if (!f.isEmpty())
  {
    temp = f;
    gnuInt->setOutput(temp);
    outputLabel->setText(f);
  }
}

void Qplotdialog::resetOutput()
{
  gnuInt->setOutput(""); // reset output to empty (stdout)
  outputLabel->setText("stdout");
}

void Qplotdialog::setPlotSize()
{
}

void Qplotdialog::resetSize()
{
  gnuInt->setHorizSize("");
  gnuInt->setVertSize("");
}

void Qplotdialog::showAbout()
{
  QMessageBox::information(this, i18n("About"),i18n("Gnuplot Front End for Kile\n"
                           "Special version of the Xgfe program created by David Ishee"));

}


void Qplotdialog::getFileOptions()
{
}

void Qplotdialog::getLegendOps()
{
}

void Qplotdialog::getMultiFile()
{
}

void Qplotdialog::getMultiFunction()
{
}

void Qplotdialog::setFileLegendTitle()
{
}

void Qplotdialog::setFuncLegendTitle()
{
}

void Qplotdialog::setLogScaleOptions()
{
}

void Qplotdialog::setFileFilter()
{
}

void Qplotdialog::setBarOptions()
{
}

void Qplotdialog::resetBarOptions()
{
}

void Qplotdialog::getCurveFit()
{
}

void Qplotdialog::setBoxWidthOption()
{
}

void Qplotdialog::resetBoxWidthOption()
{
}

void Qplotdialog::setTicsOptions()
{
}

void Qplotdialog::getRotation()
{
}


void Qplotdialog::getTicsLevel()
{
}

void Qplotdialog::set3dHiddenLine()
{
}

void Qplotdialog::setIsolines()
{
}

#include "qplotdialog.moc"
