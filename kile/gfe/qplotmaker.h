/***************************************************************************
                          qplotmaker.h  -  description
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

#ifndef QPLOTMAKER_H
#define QPLOTMAKER_H

using namespace std;

#include "qplotdialog.h"
#include "latexEmtexOp.h"
#include "pbmOp.h"
#include "psOpt.h"
#include "sizeOp.h"
#include "rawGnu.h"
#include "fileOptions.h"
#include "legendOp.h"
#include "multiFile.h"
#include "multiFunc.h"
#include "fileLegendTitle.h"
#include "funcLegendTitle.h"
#include "logScaleOp.h"
#include "fileFilter.h"
#include "barOp.h"
#include "curveFit.h"
#include "boxWidthOp.h"
#include "ticsOp.h"
#include "rotation.h"
#include "ticsLevel.h"
#include "isoLinesOp.h"
#include <qapplication.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <iostream>
#include <stdio.h>

class Qplotmaker : public Qplotdialog
{
    Q_OBJECT

public:

    Qplotmaker
    (
        QWidget* parent = 0,
        const char* name = 0
    );

    virtual ~Qplotmaker();

 public slots:

 void plot();
  void replot();
  void dataFileOpen();
  void save();
  void load();
  void saveXgfe();
  void loadXgfe();
  void xgfeQuit();
  void setFilePoints();
  void setFileLines();
  void setFileLinesPoints();
  void setFileImpulses();
  void setFileDots();
  void setFileSteps();
  void setFileFsteps();
  void setFileHisteps();
  void setFileErrorbars();
  void setFileXerrorbars();
  void setFileYerrorbars();
  void setFileXyerrorbars();
  void setFileBoxes();
  void setFileBoxerrorbars();
  void setFileBoxxyerrorbars();
  void setFileFinancebars();
  void setFileCandlesticks();
  void setFuncPoints();
  void setFuncLines();
  void setFuncLinesPoints();
  void setFuncImpulses();
  void setFuncDots();
  void setFuncSteps();
  void setFuncErrorbars();
  void setFuncBoxes();
  void setFilePlotType2d();
  void setFilePlotType3d();
  void setFuncPlotType2d();
  void setFuncPlotType3d();
  void setTermBfig();
  void setTermCorel();
  void setTermDxf();
  void setTermEepic();
  void setTermEmtex();
  void setTermFig();
  void setTermLatex();
  void setTermPbm();
  void setTermPostscript();
  void setTermPslatex();
  void setTermPstricks();
  void setTermTable();
  void setTermTexdraw();
  void setTermTgif();
  void setTermTpic();
  void setTermX11();
  void setPlotSize();
  void getFileOptions();
  void getLegendOps();
  void getMultiFile();
  void getMultiFunction();
  void setFileLegendTitle();
  void setFuncLegendTitle();
  void setLogScaleOptions();
  void setFileFilter();
  void setBarOptions();
  void resetBarOptions();
  void getCurveFit();
  void setBoxWidthOption();
  void resetBoxWidthOption();
  void setTicsOptions();
  void getRotation();
  void getTicsLevel();
  void set3dHiddenLine();
  void setIsolines();

private:
  void clearFileStyleChecks();
  void clearFuncStyleChecks();
  void clearTermChecks();

protected:
  virtual void closeEvent(QCloseEvent *e);
};
#endif
