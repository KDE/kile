/***************************************************************************
                          qplotmaker.cpp  -  description
                             -------------------
    begin                : Sat Aug 25 13:16:55 CEST 2001
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kfiledialog.h>
#include "qplotmaker.h"

Qplotmaker::Qplotmaker
(
	QWidget* parent,
	const char* name
)
	:
	Qplotdialog( parent, name )
{
  setCaption( "Gnuplot Front End" );
}


Qplotmaker::~Qplotmaker()
{

}

void Qplotmaker::plot()
{
  gnuInt->setReplotFlag(0); // set replot flag to false

  // figure out if plotting 2d or 3d

  // look at files
  if (filePlotType->isItemChecked(file2d_id) == TRUE)
    gnuInt->setFilePlotType("plot");
  else if (filePlotType->isItemChecked(file3d_id) == TRUE)
    gnuInt->setFilePlotType("splot");

  // look at function
  if (funcPlotType->isItemChecked(func2d_id) == TRUE)
    gnuInt->setFuncPlotType("plot");
  else if (funcPlotType->isItemChecked(func3d_id) == TRUE)
    gnuInt->setFuncPlotType("splot");

  // save filename
  gnuInt->setPlotFilename(filenameEdit->text());

  // save function
  gnuInt->setPlotFunction(functionEdit->text());

  // save x and y variable names
  gnuInt->setXVariableName(varX->text());
  gnuInt->setYVariableName(varY->text());

  // save x,y,z ranges
  gnuInt->setXRangeStart(xStart->text());
  gnuInt->setXRangeEnd(xEnd->text());
  gnuInt->setYRangeStart(yStart->text());
  gnuInt->setYRangeEnd(yEnd->text());
  gnuInt->setZRangeStart(zStart->text());
  gnuInt->setZRangeEnd(zEnd->text());

  // set x,y,z axis labels
  gnuInt->setXlabel(xLabel->text());
  gnuInt->setXOffset_X(XLabelOffset_X->text());
  gnuInt->setXOffset_Y(XLabelOffset_Y->text());

  gnuInt->setYlabel(yLabel->text());
  gnuInt->setYOffset_X(YLabelOffset_X->text());
  gnuInt->setYOffset_Y(YLabelOffset_Y->text());

  gnuInt->setZlabel(zLabel->text());
  gnuInt->setZOffset_X(ZLabelOffset_X->text());
  gnuInt->setZOffset_Y(ZLabelOffset_Y->text());

  // save title
  gnuInt->setTitle(titleLabel->text());
  gnuInt->setTitleOffset_X(titleOffset_X->text());
  gnuInt->setTitleOffset_Y(titleOffset_Y->text());

  // filename option is checked
  if (filenameCB->isChecked() == TRUE)
    gnuInt->setPlotFileFlag(1);
  else
    gnuInt->setPlotFileFlag(0);

  // multiple filename option is checked
  if (multiFileCheckbox->isChecked() == TRUE)
    gnuInt->setPlotMultipleFileFlag(1);
  else
    gnuInt->setPlotMultipleFileFlag(0);

  // function option is checked
   if (functionCB->isChecked() == TRUE)
     gnuInt->setPlotFuncFlag(1);
   else
     gnuInt->setPlotFuncFlag(0);

  // multiple function option is checked
   if (multiFuncCheckbox->isChecked() == TRUE)
     gnuInt->setPlotMultipleFuncFlag(1);
   else
     gnuInt->setPlotMultipleFuncFlag(0);

   // plot
  rawGnu* rawInput = new rawGnu(this);
  rawInput->show();
  rawInput->showCommands(gnuInt->getCommands());
  if ( rawInput->exec() )
    {
    for(int i = 0; i < rawInput->rawCommand->paragraphs(); i++)
      {
      gnuInt->doCommand(rawInput->rawCommand->text(i)) ;
      }
    }
    delete (rawInput);
}

void Qplotmaker::replot()
{
  gnuInt->setReplotFlag(1); // set replot flag to true

  // save filename
  gnuInt->setPlotFilename(filenameEdit->text());

  // save function
  gnuInt->setPlotFunction(functionEdit->text());

  // don't set ranges because they can't be reset in a replot

  // set x,y,z axis labels
  gnuInt->setXlabel(xLabel->text());
  gnuInt->setXOffset_X(XLabelOffset_X->text());
  gnuInt->setXOffset_Y(XLabelOffset_Y->text());

  gnuInt->setYlabel(yLabel->text());
  gnuInt->setYOffset_X(YLabelOffset_X->text());
  gnuInt->setYOffset_Y(YLabelOffset_Y->text());

  gnuInt->setZlabel(zLabel->text());
  gnuInt->setZOffset_X(ZLabelOffset_X->text());
  gnuInt->setZOffset_Y(ZLabelOffset_Y->text());

  // save title
  gnuInt->setTitle(titleLabel->text());
  gnuInt->setTitleOffset_X(titleOffset_X->text());
  gnuInt->setTitleOffset_Y(titleOffset_Y->text());

  if (filenameCB->isChecked() == TRUE) // filename option is checked
  {
    gnuInt->setPlotFileFlag(1);
    gnuInt->setFilePlotType("replot");
  }
  else
    gnuInt->setPlotFileFlag(0);

  if (functionCB->isChecked() == TRUE) // function option is checked
  {
    gnuInt->setPlotFuncFlag(1);
    gnuInt->setFuncPlotType("replot");
  }
  else
    gnuInt->setPlotFuncFlag(0);

  // plot
  rawGnu* rawInput = new rawGnu(this);
  rawInput->show();
  rawInput->showCommands(gnuInt->getCommands());
  if ( rawInput->exec() )
    {
    for(int i = 0; i < rawInput->rawCommand->paragraphs(); i++)
      {
      gnuInt->doCommand(rawInput->rawCommand->text(i)) ;
      }
    }
  delete (rawInput);
}

void Qplotmaker::setFilePlotType2d()
{
  // clear check marks
  filePlotType->setItemChecked(file2d_id, FALSE);
  filePlotType->setItemChecked(file3d_id, FALSE);

  // set appropriate check mark
  filePlotType->setItemChecked(file2d_id, TRUE);

}

void Qplotmaker::setFilePlotType3d()
{
  // clear check marks
  filePlotType->setItemChecked(file2d_id, FALSE);
  filePlotType->setItemChecked(file3d_id, FALSE);

  // set appropriate check mark
  filePlotType->setItemChecked(file3d_id, TRUE);

}

void Qplotmaker::setFuncPlotType2d()
{
  // clear check marks
  funcPlotType->setItemChecked(func2d_id, FALSE);
  funcPlotType->setItemChecked(func3d_id, FALSE);

  // set appropriate check mark
  funcPlotType->setItemChecked(func2d_id, TRUE);
}

void Qplotmaker::setFuncPlotType3d()
{
  // clear check marks
  funcPlotType->setItemChecked(func2d_id, FALSE);
  funcPlotType->setItemChecked(func3d_id, FALSE);

  // set appropriate check mark
  funcPlotType->setItemChecked(func3d_id, TRUE);
}

void Qplotmaker::dataFileOpen()
{
  QString temp;
QString filename = KFileDialog::getOpenFileName(QDir::currentDirPath(), "", this,"Open File" );
  if ( !filename.isNull() )
  {
    filenameEdit->setText(filename); // set label to filename
    filenameCB->setChecked(TRUE);     // set filename checkbox
    temp = filename;
    gnuInt->setPlotFilename(temp);
  }

}

void Qplotmaker::save()
{
  QString saveFile;
  QString temp;

saveFile = KFileDialog::getSaveFileName(QDir::currentDirPath() ,"", this,"Save File" );
  if (!saveFile.isEmpty())
  {
    temp = saveFile;
    gnuInt->setGnuFileSave(temp);
    gnuInt->doSave();
  }
}

void Qplotmaker::load()
{
  QString loadFile;
  QString temp;

loadFile= KFileDialog::getOpenFileName(QDir::currentDirPath(), "", this,"Open File" );
  if (!loadFile.isEmpty())
  {
    temp = loadFile;
    gnuInt->setGnuFileLoad(temp);
    gnuInt->doLoad();
  }
}

void Qplotmaker::saveXgfe()
{
  QString saveFile;
  QString temp;

saveFile = KFileDialog::getSaveFileName(QDir::currentDirPath() ,"", this,"Save File" );
  if (!saveFile.isEmpty())
  {
    temp = saveFile;
    gnuInt->savePlotData(temp);
  }
}

void Qplotmaker::loadXgfe()
{
  QString loadFile;
  QString temp;

loadFile = KFileDialog::getOpenFileName(QDir::currentDirPath(), "", this,"Open File" );

  if (!loadFile.isEmpty())
  {
    temp = loadFile;
    gnuInt->loadPlotData(temp);

    // get options and set GUI

    // checkboxes
    int plotFileFlag = gnuInt->getPlotFileFlag();
    int plotMultipleFileFlag = gnuInt->getPlotMultipleFileFlag();
    int plotFuncFlag = gnuInt->getPlotFuncFlag();
    int plotMultipleFuncFlag = gnuInt->getPlotMultipleFuncFlag();

    if (plotFileFlag == 1)
      filenameCB->setChecked(TRUE);
    else
      filenameCB->setChecked(FALSE);

    if (plotMultipleFileFlag == 1)
      multiFileCheckbox->setChecked(TRUE);
    else
      multiFileCheckbox->setChecked(FALSE);

    if (plotFuncFlag == 1)
      functionCB->setChecked(TRUE);
    else
      functionCB->setChecked(FALSE);

    if (plotMultipleFuncFlag == 1)
      multiFuncCheckbox->setChecked(TRUE);
    else
      multiFuncCheckbox->setChecked(FALSE);

    QString tempval;
    tempval = "";

    // filename edit box
    tempval = gnuInt->getPlotFilename();
    filenameEdit->setText(tempval);

    // function edit box
    tempval = gnuInt->getPlotFunction();
    functionEdit->setText(tempval);

    // x,y,z variable names and start/end values
    tempval = gnuInt->getXVariableName();
    varX->setText(tempval);

    tempval = gnuInt->getXRangeStart();
    xStart->setText(tempval);

    tempval = gnuInt->getXRangeEnd();
    xEnd->setText(tempval);

    tempval = gnuInt->getYVariableName();
    varY->setText(tempval);

    tempval = gnuInt->getYRangeStart();
    yStart->setText(tempval);

    tempval = gnuInt->getYRangeEnd();
    yEnd->setText(tempval);

    tempval = gnuInt->getZRangeStart();
    zStart->setText(tempval);

    tempval = gnuInt->getZRangeEnd();
    zEnd->setText(tempval);

    // x,y,z labels and offsets
    tempval = gnuInt->getXlabel();
    xLabel->setText(tempval);

    tempval = gnuInt->getXOffset_X();
    XLabelOffset_X->setText(tempval);

    tempval = gnuInt->getXOffset_Y();
    XLabelOffset_Y->setText(tempval);

    tempval = gnuInt->getYlabel();
    yLabel->setText(tempval);

    tempval = gnuInt->getYOffset_X();
    YLabelOffset_X->setText(tempval);

    tempval = gnuInt->getYOffset_Y();
    YLabelOffset_Y->setText(tempval);

    tempval = gnuInt->getZlabel();
    zLabel->setText(tempval);

    tempval = gnuInt->getZOffset_X();
    ZLabelOffset_X->setText(tempval);

    tempval = gnuInt->getZOffset_Y();
    ZLabelOffset_Y->setText(tempval);

    // title and offsets
    tempval = gnuInt->getTitle();
    titleLabel->setText(tempval);

    tempval = gnuInt->getTitleOffset_X();
    titleOffset_X->setText(tempval);

    tempval = gnuInt->getTitleOffset_Y();
    titleOffset_Y->setText(tempval);

    // output
    tempval = gnuInt->getOutput();
    outputLabel->setText(tempval);

    // terminal
    tempval = gnuInt->getTerminal();

    if (tempval == "bfig")
    {
      clearTermChecks();
      terminals->setItemChecked(bfig_t_id, TRUE);
      termLabel->setText("bfig");
    }
    else if (tempval == "corel")
    {
      clearTermChecks();
      terminals->setItemChecked(corel_t_id, TRUE);
      termLabel->setText("corel");
    }
    else if (tempval == "dxf")
    {
      clearTermChecks();
      terminals->setItemChecked(dxf_t_id, TRUE);
      termLabel->setText("dxf");
    }
    else if (tempval == "eepic")
    {
      clearTermChecks();
      terminals->setItemChecked(eepic_t_id, TRUE);
      termLabel->setText("eepic");
    }
    else if (tempval == "emtex")
    {
      clearTermChecks();
      terminals->setItemChecked(emtex_t_id, TRUE);
      termLabel->setText("emtex");
    }
    else if (tempval == "fig")
    {
      clearTermChecks();
      terminals->setItemChecked(fig_t_id, TRUE);
      termLabel->setText("fig");
    }
    else if (tempval == "latex")
    {
      clearTermChecks();
      terminals->setItemChecked(latex_t_id, TRUE);
      termLabel->setText("latex");
    }
    else if (tempval == "pbm")
    {
      clearTermChecks();
      terminals->setItemChecked(pbm_t_id, TRUE);
      termLabel->setText("pbm");
    }
    else if (tempval == "postscript")
    {
      clearTermChecks();
      terminals->setItemChecked(ps_t_id, TRUE);
      termLabel->setText("postscript");
    }
    else if (tempval == "pslatex")
    {
      clearTermChecks();
      terminals->setItemChecked(pslatex_t_id, TRUE);
      termLabel->setText("pslatex");
    }
    else if (tempval == "pstricks")
    {
      clearTermChecks();
      terminals->setItemChecked(pstricks_t_id, TRUE);
      termLabel->setText("pstricks");
    }
    else if (tempval == "table")
    {
      clearTermChecks();
      terminals->setItemChecked(table_t_id, TRUE);
      termLabel->setText("table");
    }
    else if (tempval == "texdraw")
    {
      clearTermChecks();
      terminals->setItemChecked(texdraw_t_id, TRUE);
      termLabel->setText("texdraw");
    }
    else if (tempval == "tgif")
    {
      clearTermChecks();
      terminals->setItemChecked(tgif_t_id, TRUE);
      termLabel->setText("tgif");
    }
    else if (tempval == "tpic")
    {
      clearTermChecks();
      terminals->setItemChecked(tpic_t_id, TRUE);
      termLabel->setText("tpic");
    }
    else if (tempval == "x11")
    {
      clearTermChecks();
      terminals->setItemChecked(x11_t_id, TRUE);
      termLabel->setText("x11");
    }

    // file plot type (2d/3d)
    tempval = gnuInt->getFilePlotType();

    if (tempval == "plot")
    {
      filePlotType->setItemChecked(file2d_id,TRUE);
      filePlotType->setItemChecked(file3d_id,FALSE);
    }
    else if (tempval == "splot")
    {
      filePlotType->setItemChecked(file2d_id,FALSE);
      filePlotType->setItemChecked(file3d_id,TRUE);
    }

    // function plot type  (2d/3d)
    tempval = gnuInt->getFuncPlotType();

    if (tempval == "plot")
    {
      funcPlotType->setItemChecked(func2d_id,TRUE);
      funcPlotType->setItemChecked(func3d_id,FALSE);
    }
    else if (tempval == "splot")
    {
      funcPlotType->setItemChecked(func2d_id,FALSE);
      funcPlotType->setItemChecked(func3d_id,TRUE);
    }

    // file style
    tempval = gnuInt->getFileStyleType();

    if (tempval == "points")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_p_id,TRUE);
    }
    else if (tempval == "lines")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_l_id, TRUE);
    }
    else if (tempval == "linespoints")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_lp_id, TRUE);
    }
    else if (tempval == "impulses")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_i_id, TRUE);
    }
    else if (tempval == "dots")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_d_id, TRUE);
    }
    else if (tempval == "steps")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_s_id, TRUE);
    }
    else if (tempval == "fsteps")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_fs_id, TRUE);
    }
    else if (tempval == "histeps")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_hs_id, TRUE);
    }
    else if (tempval == "errorbars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_eb_id, TRUE);
    }
    else if (tempval == "xerrorbars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_xeb_id, TRUE);
    }
    else if (tempval == "yerrorbars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_yeb_id, TRUE);
    }
    else if (tempval == "xyerrorbars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_xyeb_id, TRUE);
    }
    else if (tempval == "boxes")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_b_id, TRUE);
    }
    else if (tempval == "boxerrorbars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_be_id, TRUE);
    }
    else if (tempval == "boxxyerrorbars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_bxye_id, TRUE);
    }
    else if (tempval == "financebars")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_fin_id, TRUE);
    }
    else if (tempval == "candlesticks")
    {
      clearFileStyleChecks();
      fileStyle->setItemChecked(file_cs_id, TRUE);
    }

    // function style
    tempval = gnuInt->getFuncStyleType();

    if (tempval == "lines")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_l_id, TRUE);
    }
    else if (tempval == "linespoints")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_lp_id, TRUE);
    }
    else if (tempval == "impulses")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_i_id, TRUE);
    }
    else if (tempval == "dots")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_d_id, TRUE);
    }
    else if (tempval == "steps")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_s_id, TRUE);
    }
    else if (tempval == "errorbars")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_eb_id, TRUE);
    }
    else if (tempval == "boxes")
    {
      clearFuncStyleChecks();
      funcStyle->setItemChecked(func_b_id, TRUE);
    }

    // 3d hidden line option
    int d3hidden = gnuInt->getd3HiddenLineFlag();

    if (d3hidden == 1)
      d3Menu->setItemChecked(d3HiddenLine_id, TRUE);
    else if (d3hidden == 0)
      d3Menu->setItemChecked(d3HiddenLine_id, FALSE);
  }
}

void Qplotmaker::xgfeQuit()
{
this->close();
}
void Qplotmaker::closeEvent(QCloseEvent* e)
{
gnuInt->closeGnuplot();
e->accept();
}
void Qplotmaker::setFilePoints()
{
  gnuInt->setFileStyleType("points");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_p_id,TRUE);

}

void Qplotmaker::setFileLines()
{
  gnuInt->setFileStyleType("lines");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_l_id, TRUE);
}

void Qplotmaker::setFileLinesPoints()
{
  gnuInt->setFileStyleType("linespoints");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_lp_id, TRUE);
}

void Qplotmaker::setFileImpulses()
{
  gnuInt->setFileStyleType("impulses");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_i_id, TRUE);
}

void Qplotmaker::setFileDots()
{
  gnuInt->setFileStyleType("dots");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_d_id, TRUE);
}

void Qplotmaker::setFileSteps()
{
  gnuInt->setFileStyleType("steps");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_s_id, TRUE);
}

void Qplotmaker::setFileFsteps()
{
  gnuInt->setFileStyleType("fsteps");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_fs_id, TRUE);
}

void Qplotmaker::setFileHisteps()
{
  gnuInt->setFileStyleType("histeps");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_hs_id, TRUE);
}

void Qplotmaker::setFileErrorbars()
{
  gnuInt->setFileStyleType("errorbars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_eb_id, TRUE);
}

void Qplotmaker::setFileXerrorbars()
{
  gnuInt->setFileStyleType("xerrorbars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_xeb_id, TRUE);
}

void Qplotmaker::setFileYerrorbars()
{
  gnuInt->setFileStyleType("yerrorbars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_yeb_id, TRUE);
}

void Qplotmaker::setFileXyerrorbars()
{
  gnuInt->setFileStyleType("xyerrorbars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_xyeb_id, TRUE);
}

void Qplotmaker::setFileBoxes()
{
  gnuInt->setFileStyleType("boxes");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_b_id, TRUE);
}

void Qplotmaker::setFileBoxerrorbars()
{
  gnuInt->setFileStyleType("boxerrorbars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_be_id, TRUE);
}

void Qplotmaker::setFileBoxxyerrorbars()
{
  gnuInt->setFileStyleType("boxxyerrorbars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_bxye_id, TRUE);
}

void Qplotmaker::setFileFinancebars()
{
  gnuInt->setFileStyleType("financebars");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_fin_id, TRUE);
}

void Qplotmaker::setFileCandlesticks()
{
  gnuInt->setFileStyleType("candlesticks");

  clearFileStyleChecks();
  fileStyle->setItemChecked(file_cs_id, TRUE);
}

void Qplotmaker::setFuncPoints()
{
  gnuInt->setFuncStyleType("points");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_p_id,TRUE);

}

void Qplotmaker::setFuncLines()
{
  gnuInt->setFuncStyleType("lines");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_l_id, TRUE);
}

void Qplotmaker::setFuncLinesPoints()
{
  gnuInt->setFuncStyleType("linespoints");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_lp_id, TRUE);
}

void Qplotmaker::setFuncImpulses()
{
  gnuInt->setFuncStyleType("impulses");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_i_id, TRUE);
}

void Qplotmaker::setFuncDots()
{
  gnuInt->setFuncStyleType("dots");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_d_id, TRUE);
}

void Qplotmaker::setFuncSteps()
{
  gnuInt->setFuncStyleType("steps");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_s_id, TRUE);
}

void Qplotmaker::setFuncErrorbars()
{
  gnuInt->setFuncStyleType("errorbars");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_eb_id, TRUE);
}

void Qplotmaker::setFuncBoxes()
{
  gnuInt->setFuncStyleType("boxes");

  clearFuncStyleChecks();
  funcStyle->setItemChecked(func_b_id, TRUE);
}

void Qplotmaker::clearFileStyleChecks()
{
  int fileStyleTypes[17];

  fileStyleTypes[0] = file_p_id;
  fileStyleTypes[1] = file_l_id;
  fileStyleTypes[2] = file_lp_id;
  fileStyleTypes[3] = file_i_id;
  fileStyleTypes[4] = file_d_id;
  fileStyleTypes[5] = file_s_id;
  fileStyleTypes[6] = file_fs_id;
  fileStyleTypes[7] = file_hs_id;
  fileStyleTypes[8] = file_eb_id;
  fileStyleTypes[9] = file_xeb_id;
  fileStyleTypes[10] = file_yeb_id;
  fileStyleTypes[11] = file_xyeb_id;
  fileStyleTypes[12] = file_b_id;
  fileStyleTypes[13] = file_be_id;
  fileStyleTypes[14] = file_bxye_id;
  fileStyleTypes[15] = file_fin_id;
  fileStyleTypes[16] = file_cs_id;

  for (int i = 0; i < 17; i++)
  {
    if (fileStyle->isItemChecked(fileStyleTypes[i]))  fileStyle->setItemChecked(fileStyleTypes[i],FALSE);
  }
}

void Qplotmaker::clearTermChecks()
{
  int termTypes[16];

  termTypes[0] = bfig_t_id;
  termTypes[1] = corel_t_id;
  termTypes[2] = dxf_t_id;
  termTypes[3] = eepic_t_id;
  termTypes[4] = emtex_t_id;
  termTypes[5] = fig_t_id;
  termTypes[6] = latex_t_id;
  termTypes[7] = pbm_t_id;
  termTypes[8] = ps_t_id;
  termTypes[9] = pslatex_t_id;
  termTypes[10] = pstricks_t_id;
  termTypes[11] = table_t_id;
  termTypes[12] = texdraw_t_id;
  termTypes[13] = tgif_t_id;
  termTypes[14] = tpic_t_id;
  termTypes[15] = x11_t_id;

  for (int i = 0; i < 16; i++)
  {
    if (terminals->isItemChecked(termTypes[i])) terminals->setItemChecked(termTypes[i],FALSE);
  }
}

void Qplotmaker::clearFuncStyleChecks()
{
  int funcStyleTypes[8];

  funcStyleTypes[0] = func_p_id;
  funcStyleTypes[1] = func_l_id;
  funcStyleTypes[2] = func_lp_id;
  funcStyleTypes[3] = func_i_id;
  funcStyleTypes[4] = func_d_id;
  funcStyleTypes[5] = func_s_id;
  funcStyleTypes[6] = func_eb_id;
  funcStyleTypes[7] = func_b_id;

  for (int i = 0; i < 8; i++)
  {
    if (funcStyle->isItemChecked(funcStyleTypes[i])) funcStyle->setItemChecked(funcStyleTypes[i],FALSE);
  }
}

void Qplotmaker::setTermBfig()
{
  gnuInt->setTerminal("bfig");

  clearTermChecks();
  terminals->setItemChecked(bfig_t_id, TRUE);
  termLabel->setText("bfig");
}

void Qplotmaker::setTermCorel()
{
  gnuInt->setTerminal("corel");

  clearTermChecks();
  terminals->setItemChecked(corel_t_id, TRUE);
  termLabel->setText("corel");
}

void Qplotmaker::setTermDxf()
{
  gnuInt->setTerminal("dxf");

  clearTermChecks();
  terminals->setItemChecked(dxf_t_id, TRUE);
  termLabel->setText("dxf");
}

void Qplotmaker::setTermEepic()
{
  gnuInt->setTerminal("eepic");

  clearTermChecks();
  terminals->setItemChecked(eepic_t_id, TRUE);
  termLabel->setText("eepic");
}

void Qplotmaker::setTermEmtex()
{
  // open dialog and capture results
  latexEmtexOp* emtexOptions = new latexEmtexOp(this);

  emtexOptions->setGnuInterface(gnuInt);

  if (emtexOptions->exec())
  {
    // OK was pressed
    // set appropriate checkmark
    clearTermChecks();
    terminals->setItemChecked(emtex_t_id, TRUE);

    gnuInt->setTerminal("emtex");

    // set label
    termLabel->setText("emtex");
  }

}

void Qplotmaker::setTermFig()
{
  gnuInt->setTerminal("fig");

  clearTermChecks();
  terminals->setItemChecked(fig_t_id, TRUE);
  termLabel->setText("fig");
}

void Qplotmaker::setTermLatex()
{
  // open dialog and capture results
  latexEmtexOp* latexOptions = new latexEmtexOp(this);

  latexOptions->setGnuInterface(gnuInt);

  if (latexOptions->exec())
  {
    // OK was pressed
    // set appropriate checkmark
    clearTermChecks();
    terminals->setItemChecked(latex_t_id, TRUE);

    gnuInt->setTerminal("latex");

    // set label
    termLabel->setText("latex");
  }
}

void Qplotmaker::setTermPbm()
{
  // open dialog and capture results
  pbmOp* pbmOptions = new pbmOp(this);

  pbmOptions->setGnuInterface(gnuInt);

  if (pbmOptions->exec())
  {
    // OK was pressed
    // set appropriate checkmark
    clearTermChecks();
    terminals->setItemChecked(pbm_t_id, TRUE);

    gnuInt->setTerminal("pbm");

    // set label
    termLabel->setText("pbm");
  }
}

void Qplotmaker::setTermPostscript()
{
  // open dialog and capture results
  psOpt* psOptions = new psOpt(this);
  psOptions->setGnuInterface(gnuInt);

  if (psOptions->exec())
  {
    // OK was pressed
    // set appropriate checkmark
    clearTermChecks();
    terminals->setItemChecked(ps_t_id, TRUE);

    gnuInt->setTerminal("postscript");

    // set label
    termLabel->setText("postscript");
  }
}

void Qplotmaker::setTermPslatex()
{
  gnuInt->setTerminal("pslatex");

  clearTermChecks();
  terminals->setItemChecked(pslatex_t_id, TRUE);
  termLabel->setText("pslatex");
}

void Qplotmaker::setTermPstricks()
{
  gnuInt->setTerminal("pstricks");

  clearTermChecks();
  terminals->setItemChecked(pstricks_t_id, TRUE);
  termLabel->setText("pstricks");
}

void Qplotmaker::setTermTable()
{
  gnuInt->setTerminal("table");

  clearTermChecks();
  terminals->setItemChecked(table_t_id, TRUE);
  termLabel->setText("table");
}

void Qplotmaker::setTermTexdraw()
{
  gnuInt->setTerminal("texdraw");

  clearTermChecks();
  terminals->setItemChecked(texdraw_t_id, TRUE);
  termLabel->setText("texdraw");
}

void Qplotmaker::setTermTgif()
{
  gnuInt->setTerminal("tgif");

  clearTermChecks();
  terminals->setItemChecked(tgif_t_id, TRUE);
  termLabel->setText("tgif");
}

void Qplotmaker::setTermTpic()
{
  gnuInt->setTerminal("tpic");

  clearTermChecks();
  terminals->setItemChecked(tpic_t_id, TRUE);
  termLabel->setText("tpic");
}

void Qplotmaker::setTermX11()
{
  gnuInt->setTerminal("x11");

  clearTermChecks();
  terminals->setItemChecked(x11_t_id, TRUE);
  termLabel->setText("x11");
}

void Qplotmaker::setPlotSize()
{
  sizeOp* sizeOptions = new sizeOp(this);

  sizeOptions->setGnuInterface(gnuInt);
  sizeOptions->show();
}

void Qplotmaker::getFileOptions()
{
  fileOptions* fileOp = new fileOptions(this);

  fileOp->setGnuInterface(gnuInt);
  fileOp->show();
}

void Qplotmaker::getLegendOps()
{
  legendOp* legendOptions = new legendOp(this);
  legendOptions->setGnuInterface(gnuInt);
  legendOptions->show();
}

void Qplotmaker::getMultiFile()
{
  multiFile* multifile = new multiFile(this);
  multifile->setGnuInterface(gnuInt);
  multifile->show();
}

void Qplotmaker::getMultiFunction()
{
  multiFunc* multifunc = new multiFunc(this);
  multifunc->setGnuInterface(gnuInt);
  multifunc->show();
}

void Qplotmaker::setFileLegendTitle()
{
  fileLegendTitle* fileLegend = new fileLegendTitle(this);
  fileLegend->setGnuInterface(gnuInt);
  fileLegend->show();
}

void Qplotmaker::setFuncLegendTitle()
{
  funcLegendTitle* funcLegend = new funcLegendTitle(this);
  funcLegend->setGnuInterface(gnuInt);
  funcLegend->show();
}

void Qplotmaker::setLogScaleOptions()
{
  logScaleOp* logScale = new logScaleOp(this);
  logScale->setGnuInterface(gnuInt);
  logScale->show();
}

void Qplotmaker::setFileFilter()
{
  fileFilter* fileFilterOp = new fileFilter(this);
  fileFilterOp->setGnuInterface(gnuInt);
  fileFilterOp->show();
}

void Qplotmaker::setBarOptions()
{
  barOp* bar = new barOp(this);
  bar->setGnuInterface(gnuInt);
  bar->show();
}

void Qplotmaker::resetBarOptions()
{
  gnuInt->setBarSizeOption("");
}

void Qplotmaker::getCurveFit()
{
  curveFit* cFit = new curveFit(this);
  cFit->setGnuInterface(gnuInt);
  cFit->show();
}

void Qplotmaker::setBoxWidthOption()
{
  boxWidthOp* bWidth = new boxWidthOp(this);
  bWidth->setGnuInterface(gnuInt);
  bWidth->show();
}

void Qplotmaker::resetBoxWidthOption()
{
  gnuInt->setBoxWidth("");
}

void Qplotmaker::setTicsOptions()
{
  ticsOp* tics = new ticsOp(this);
  tics->setGnuInterface(gnuInt);
  tics->show();
}

void Qplotmaker::getRotation()
{
  rotation* rotOp = new rotation(this);
  rotOp->setGnuInterface(gnuInt);
  rotOp->show();
}

void Qplotmaker::getTicsLevel()
{
  ticsLevel* tlOp = new ticsLevel(this);
  tlOp->setGnuInterface(gnuInt);
  tlOp->show();
}

void Qplotmaker::set3dHiddenLine()
{
  // toggle checkmark and set option

  if (d3Menu->isItemChecked(d3HiddenLine_id) == TRUE)
  {
    d3Menu->setItemChecked(d3HiddenLine_id, FALSE);
    gnuInt->setd3HiddenLineFlag(0);
  }
  else if (d3Menu->isItemChecked(d3HiddenLine_id) == FALSE)
  {
    d3Menu->setItemChecked(d3HiddenLine_id, TRUE);
    gnuInt->setd3HiddenLineFlag(1);
  }
}

void Qplotmaker::setIsolines()
{
  isoLinesOp* isoOp = new isoLinesOp(this);
  isoOp->setGnuInterface(gnuInt);
  isoOp->show();
}


#include "qplotmaker.moc"
