/***************************************************************************
                          qplotdialog.h  -  description
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
using namespace std;

#ifndef QPLOTDIALOG_H
#define QPLOTDIALOG_H

#include <qmainwindow.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qpopmenu.h>
#include <qapp.h>
#include <qframe.h>
#include <qfiledlg.h>
#include <qmsgbox.h>
#include <qstring.h>
#include <stdio.h>
#include "gnuInterface.h"

/**
  *@author Pascal Brachet
  */

class Qplotdialog : public QMainWindow  {
   Q_OBJECT
public: 
	Qplotdialog(QWidget *parent=0, const char *name=0);
	~Qplotdialog();
protected slots:

  virtual void plot();
  virtual void replot();
  virtual void dataFileOpen();
  virtual void save();
  virtual void load();
  virtual void saveXgfe();
  virtual void loadXgfe();
  virtual void xgfeQuit();
  virtual void setFilePoints();
  virtual void setFileLines();
  virtual void setFileLinesPoints();
  virtual void setFileImpulses();
  virtual void setFileDots();
  virtual void setFileSteps();
  virtual void setFileFsteps();
  virtual void setFileHisteps();
  virtual void setFileErrorbars();
  virtual void setFileXerrorbars();
  virtual void setFileYerrorbars();
  virtual void setFileXyerrorbars();
  virtual void setFileBoxes();
  virtual void setFileBoxerrorbars();
  virtual void setFileBoxxyerrorbars();
  virtual void setFileFinancebars();
  virtual void setFileCandlesticks();
  virtual void setFuncPoints();
  virtual void setFuncLines();
  virtual void setFuncLinesPoints();
  virtual void setFuncImpulses();
  virtual void setFuncDots();
  virtual void setFuncSteps();
  virtual void setFuncErrorbars();
  virtual void setFuncBoxes();
  virtual void setFilePlotType2d();
  virtual void setFilePlotType3d();
  virtual void setFuncPlotType2d();
  virtual void setFuncPlotType3d();
  virtual void setTermBfig();
  virtual void setTermCorel();
  virtual void setTermDxf();
  virtual void setTermEepic();
  virtual void setTermEmtex();
  virtual void setTermFig();
  virtual void setTermLatex();
  virtual void setTermPbm();
  virtual void setTermPostscript();
  virtual void setTermPslatex();
  virtual void setTermPstricks();
  virtual void setTermTable();
  virtual void setTermTexdraw();
  virtual void setTermTgif();
  virtual void setTermTpic();
  virtual void setTermX11();
  virtual void setPlotSize();
  virtual void getFileOptions();
  virtual void getLegendOps();
  virtual void getMultiFile();
  virtual void getMultiFunction();
  virtual void setFileLegendTitle();
  virtual void setFuncLegendTitle();
  virtual void setLogScaleOptions();
  virtual void setFileFilter();
  virtual void setBarOptions();
  virtual void resetBarOptions();
  virtual void getCurveFit();
  virtual void setBoxWidthOption();
  virtual void resetBoxWidthOption();
  virtual void setTicsOptions();
  virtual void getRotation();
  virtual void getTicsLevel();
  virtual void set3dHiddenLine();
  virtual void setIsolines();
  void resetSize();
  void getOutput();
  void resetOutput();
  void showAbout();

protected:
  gnuInterface* gnuInt;
  QFrame* Line1;
  QFrame* Line2;
  QFrame* Line3;
  QLineEdit* filenameEdit;
  QLabel* GfeLabel;
  QLineEdit* outputLabel;
  QLabel* termLabel;
  QLabel* plotSizeLabel;
  QCheckBox* filenameCB;
  QCheckBox* functionCB;
  QCheckBox* multiFileCheckbox;
  QCheckBox* multiFuncCheckbox;
  QLineEdit* functionEdit;
  QLineEdit* varX;
  QLineEdit* xStart;
  QLineEdit* xEnd;
  QLineEdit* zLabel;
  QLineEdit* yLabel;
  QLineEdit* xLabel;
  QLineEdit* varY;
  QLineEdit* yStart;
  QLineEdit* yEnd;
  QLineEdit* zStart;
  QLineEdit* zEnd;
  QLineEdit* XLabelOffset_X;
  QLineEdit* XLabelOffset_Y;
  QLineEdit* YLabelOffset_X;
  QLineEdit* YLabelOffset_Y;
  QLineEdit* ZLabelOffset_X;
  QLineEdit* ZLabelOffset_Y;
  QLineEdit* titleLabel;
  QLineEdit* titleOffset_X;
  QLineEdit* titleOffset_Y;
  QPopupMenu* file;
  QPopupMenu* fileStyle;
  QPopupMenu* funcStyle;
  QPopupMenu* filePlotType;
  QPopupMenu* funcPlotType;
  QPopupMenu* terminals;
  QPopupMenu* options;
  QPopupMenu* datafileOpMenu;
  QPopupMenu* funcOpMenu;
  QPopupMenu* help;
  int file_p_id;
  int file_l_id;
  int file_lp_id;
  int file_i_id;
  int file_d_id;
  int file_s_id;
  int file_fs_id;
  int file_hs_id;
  int file_eb_id;
  int file_xeb_id;
  int file_yeb_id;
  int file_xyeb_id;
  int file_b_id;
  int file_be_id;
  int file_bxye_id;
  int file_fin_id;
  int file_cs_id;
  int func_p_id;
  int func_l_id;
  int func_lp_id;
  int func_i_id;
  int func_d_id;
  int func_s_id;
  int func_eb_id;
  int func_b_id;
  int file2d_id;
  int file3d_id;
  int func2d_id;
  int func3d_id;
  int bfig_t_id;
  int corel_t_id;
  int dxf_t_id;
  int eepic_t_id;
  int emtex_t_id;
  int fig_t_id;
  int latex_t_id;
  int pbm_t_id;
  int ps_t_id;
  int pslatex_t_id;
  int pstricks_t_id;
  int table_t_id;
  int texdraw_t_id;
  int tgif_t_id;
  int tpic_t_id;
  int x11_t_id;
  QPopupMenu* d3Menu;
  int d3HiddenLine_id;
  QString output;
  FILE* gnuRC;
};

#endif
