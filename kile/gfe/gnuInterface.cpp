/**********************************************************************

	File: gnuInterface.cpp

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

#include "gnuInterface.h"

gnuInterface::gnuInterface()
{
  plotFileFlag = 0;
  plotMultipleFileFlag = 0;
  plotFuncFlag = 0;
  plotMultipleFuncFlag = 0;
  filePlotType = "plot";
  funcPlotType = "plot";
  hSize = "";
  vSize = "";
  terminal = "x11";
  termLatexEmtexFont = "roman";
  termLatexEmtexFontSize = "10";
  termPBMfontSize = "small";
  termPBMcolormode = "monochrome";
  termPBMhSize = "1";
  termPBMvSize = "1";
  termPSmode = "landscape";
  termPScolor = "monochrome";
  termPSdashed = "dashed";
  termPSfont = "Helvetica";
  termPSfontSize = "14";
  termPSenhanced = "noenhanced";
  termPShSize = "1";
  termPSvSize = "1";
  legendFlag = "key";
  legendPositionLeft = 0;
  legendPositionRight = 1;
  legendPositionTop = 1;
  legendPositionBottom = 0;
  legendPositionOutside = 0;
  legendPositionBelow = 0;
  legendPositionXVal = "";
  legendPositionYVal = "";
  legendPositionZVal = "";
  legendTextJustify = "Right";
  legendReverse = "noreverse";
  legendBox = "nobox";
  legendLinetype = "";
  legendSampleLength = "4";
  legendSpacing = "1.25";
  legendWidthIncrement = "";
  legendTitle = "";
  xticsOnFlag = 1;
  xticsLocation = "border";
  xticsMirror = "mirror";
  xticsRotation = "norotate";
  xticsPositionType = "SIE";
  xticsStartPos = "";
  xticsIncPos = "";
  xticsEndPos = "";
  xticsLabelsPos = "";
  yticsOnFlag = 1;
  yticsLocation = "border";
  yticsMirror = "mirror";
  yticsRotation = "norotate";
  yticsPositionType = "SIE";
  yticsStartPos = "";
  yticsIncPos = "";
  yticsEndPos = "";
  yticsLabelsPos = "";
  zticsOnFlag = 0;
  zticsMirror = "nomirror";
  zticsRotation = "norotate";
  zticsPositionType = "SIE";
  zticsStartPos = "";
  zticsIncPos = "";
  zticsEndPos = "";
  zticsLabelsPos = "";
  x2ticsOnFlag = 0;
  x2ticsLocation = "border";
  x2ticsMirror = "mirror";
  x2ticsRotation = "norotate";
  x2ticsPositionType = "SIE";
  x2ticsStartPos = "";
  x2ticsIncPos = "";
  x2ticsEndPos = "";
  x2ticsLabelsPos = "";
  y2ticsOnFlag = 0;
  y2ticsLocation = "border";
  y2ticsMirror = "mirror";
  y2ticsRotation = "norotate";
  y2ticsPositionType = "SIE";
  y2ticsStartPos = "";
  y2ticsIncPos = "";
  y2ticsEndPos = "";
  y2ticsLabelsPos = "";
  logScaleBase = 10;
  logScaleXAxisFlag = 0;
  logScaleYAxisFlag = 0;
  logScaleZAxisFlag = 0;
  logScaleX2AxisFlag = 0;
  logScaleY2AxisFlag = 0;
  barSize = "";
  boxWidth = "";
  xAxisRotation = 60;
  zAxisRotation = 30;
  rotationScale = "1";
  zAxisScale = "1";
  ticsLevel = "0.5";
  d3HiddenLineFlag = 0;
  isoLineU = "10";
  isoLineV = "10";

  // open a plot file object
  plotFileOb = new gnuPlotFile;

  // open a new multifile object
  multiFile = new gnuMultiFile;

  // open a plot function object
  plotFunctionOb = new gnuPlotFunction;

  // open a new multifunc object
  multiFunc = new gnuMultiFunc;

  // open a curve fitting object
  curveFitOb = new gnuCurveFit;
}

FILE* gnuInterface::openGnuplot()
{
  gnuCommand = popen("gnuplot","w");
  return gnuCommand;
}

void gnuInterface::doCommand(const QString &c)
{
  fprintf(gnuCommand,c+"\n");
  fflush(gnuCommand);
}

void gnuInterface::setPlotFileFlag(int flag)
{
  plotFileFlag = flag;
}

int gnuInterface::getPlotFileFlag()
{
  return plotFileFlag;
}

void gnuInterface::setPlotMultipleFileFlag(int flag)
{
  plotMultipleFileFlag = flag;
}

int gnuInterface::getPlotMultipleFileFlag()
{
  return plotMultipleFileFlag;
}

void gnuInterface::setPlotMultipleFuncFlag(int flag)
{
  plotMultipleFuncFlag = flag;
}

int gnuInterface::getPlotMultipleFuncFlag()
{
  return plotMultipleFuncFlag;
}

void gnuInterface::setPlotFuncFlag(int flag)
{
  plotFuncFlag = flag;
}

int gnuInterface::getPlotFuncFlag()
{
  return plotFuncFlag;
}

void gnuInterface::doSave()
{
  fprintf(gnuCommand, "save '");
  fprintf(gnuCommand, gnuFileSave);
  fprintf(gnuCommand, "'\n");
  fflush(gnuCommand);
  cout << endl << "Saving to " << gnuFileSave << endl << endl;
}

void gnuInterface::doLoad()
{
  fprintf(gnuCommand, "load '");
  fprintf(gnuCommand, gnuFileLoad);
  fprintf(gnuCommand, "'\n");
  fflush(gnuCommand);
  cout << endl << "Loading  " << gnuFileLoad << endl << endl;
}

void gnuInterface::setPlotFilename(QString filename)
{
  plotFileOb->setFilename(filename);
}

QString gnuInterface::getPlotFilename()
{
  return plotFileOb->getFilename();
}

void gnuInterface::setPlotFunction(QString function)
{
  plotFunctionOb->setFunction(function);
}

QString gnuInterface::getPlotFunction()
{
   return plotFunctionOb->getFunction();
}

void gnuInterface::setGnuFileSave(QString file)
{
  gnuFileSave = file;
}

void gnuInterface::setGnuFileLoad(QString file)
{
  gnuFileLoad = file;
}

void gnuInterface::setFilePlotType(QString type)
{
  filePlotType = type;
}

QString gnuInterface::getFilePlotType()
{
  return filePlotType;
}

void gnuInterface::setFileStyleType(QString type)
{
  plotFileOb->setFileStyleType(type);
}

QString gnuInterface::getFileStyleType()
{
  return plotFileOb->getFileStyleType();
}

void gnuInterface::setFileDataSetStart(QString start)
{
  plotFileOb->setFileDataSetStart(start);
}

QString gnuInterface::getFileDataSetStart()
{
  return plotFileOb->getFileDataSetStart();
}

void gnuInterface::setFileDataSetEnd(QString end)
{
  plotFileOb->setFileDataSetEnd(end);
}

QString gnuInterface::getFileDataSetEnd()
{
  return plotFileOb->getFileDataSetEnd();
}

void gnuInterface::setFileDataSetIncrement(QString inc)
{
  plotFileOb->setFileDataSetIncrement(inc);
}

QString gnuInterface::getFileDataSetIncrement()
{
  return plotFileOb->getFileDataSetIncrement();
}

void gnuInterface::setFileSampPointInc(QString pinc)
{
  plotFileOb->setFileSampPointInc(pinc);
}

QString gnuInterface::getFileSampPointInc()
{
  return plotFileOb->getFileSampPointInc();
}

void gnuInterface::setFileSampLineInc(QString linc)
{
  plotFileOb->setFileSampLineInc(linc);
}

QString gnuInterface::getFileSampLineInc()
{
  return plotFileOb->getFileSampLineInc();
}

void gnuInterface::setFileSampStartPoint(QString startp)
{
  plotFileOb->setFileSampStartPoint(startp);
}

QString gnuInterface::getFileSampStartPoint()
{
  return plotFileOb->getFileSampStartPoint();
}

void gnuInterface::setFileSampStartLine(QString startl)
{
  plotFileOb->setFileSampStartLine(startl);
}

QString gnuInterface::getFileSampStartLine()
{
  return plotFileOb->getFileSampStartLine();
}

void gnuInterface::setFileSampEndPoint(QString endp)
{
  plotFileOb->setFileSampEndPoint(endp);
}

QString gnuInterface::getFileSampEndPoint()
{
  return plotFileOb->getFileSampEndPoint();
}

void gnuInterface::setFileSampEndLine(QString endl)
{
  plotFileOb->setFileSampEndLine(endl);
}

QString gnuInterface::getFileSampEndLine()
{
  return plotFileOb->getFileSampEndLine();
}

void gnuInterface::setFileSmoothType(QString type)
{
  plotFileOb->setFileSmoothType(type);
}

QString gnuInterface::getFileSmoothType()
{
  return plotFileOb->getFileSmoothType();
}

void gnuInterface::setFileXColumn(QString column)
{
  plotFileOb->setFileXColumn(column);
}

QString gnuInterface::getFileXColumn()
{
  return plotFileOb->getFileXColumn();
}

void gnuInterface::setFileYColumn(QString column)
{
  plotFileOb->setFileYColumn(column);
}

QString gnuInterface::getFileYColumn()
{
  return plotFileOb->getFileYColumn();
}

void gnuInterface::setFileZColumn(QString column)
{
  plotFileOb->setFileZColumn(column);
}

QString gnuInterface::getFileZColumn()
{
  return plotFileOb->getFileZColumn();
}

void gnuInterface::setFileFormatString(QString format)
{
  plotFileOb->setFileFormatString(format);
}

QString gnuInterface::getFileFormatString()
{
  return plotFileOb->getFileFormatString();
}

void gnuInterface::setRawFileFormatString(QString format)
{
  plotFileOb->setRawFileFormatString(format);
}

QString gnuInterface::getRawFileFormatString()
{
  return plotFileOb->getRawFileFormatString();
}

void gnuInterface::setFileFilter(QString filter)
{
  plotFileOb->setFileFilter(filter);
}

void gnuInterface::setFileFilterQuoteChar(QString quote)
{
  plotFileOb->setFileFilterQuoteChar(quote);
}

QString gnuInterface::getFileFilterQuoteChar()
{
  return plotFileOb->getFileFilterQuoteChar();
}

QString gnuInterface::getFileFilter()
{
  return plotFileOb->getFileFilter();
}

void gnuInterface::setFuncPlotType(QString type)
{
  funcPlotType = type;
}

QString gnuInterface::getFuncPlotType()
{
  return funcPlotType;
}

void gnuInterface::setFuncStyleType(QString type)
{
  plotFunctionOb->setFunctionStyleType(type);
}

QString gnuInterface::getFuncStyleType()
{
  return plotFunctionOb->getFunctionStyleType();
}

void gnuInterface::setXVariableName(QString name)
{
  XVariableName = name;
}

QString gnuInterface::getXVariableName()
{
  return XVariableName;
}

void gnuInterface::setXRangeStart(QString range)
{
  XRangeStart = range;
}

QString gnuInterface::getXRangeStart()
{
  return XRangeStart;
}

void gnuInterface::setXRangeEnd(QString range)
{
  XRangeEnd = range;
}

QString gnuInterface::getXRangeEnd()
{
  return XRangeEnd;
}

void gnuInterface::setYVariableName(QString name)
{
  YVariableName = name;
}

QString gnuInterface::getYVariableName()
{
  return YVariableName;
}

void gnuInterface::setYRangeStart(QString range)
{
  YRangeStart = range;
}

QString gnuInterface::getYRangeStart()
{
  return YRangeStart;
}

void gnuInterface::setYRangeEnd(QString range)
{
  YRangeEnd = range;
}

QString gnuInterface::getYRangeEnd()
{
  return YRangeEnd;
}

void gnuInterface::setZRangeStart(QString range)
{
  ZRangeStart = range;
}

QString gnuInterface::getZRangeStart()
{
  return ZRangeStart;
}

void gnuInterface::setZRangeEnd(QString range)
{
  ZRangeEnd = range;
}

QString gnuInterface::getZRangeEnd()
{
  return ZRangeEnd;
}

void gnuInterface::setXlabel(QString label)
{
  XLabel = label;
}

QString gnuInterface::getXlabel()
{
  return XLabel;
}

void gnuInterface::setXOffset_X(QString offset)
{
  XOffset_X = offset;
}

QString gnuInterface::getXOffset_X()
{
  return XOffset_X;
}

void gnuInterface::setXOffset_Y(QString offset)
{
  XOffset_Y = offset;
}

QString gnuInterface::getXOffset_Y()
{
  return XOffset_Y;
}

void gnuInterface::setYlabel(QString label)
{
  YLabel = label;
}

QString gnuInterface::getYlabel()
{
  return YLabel;
}

void gnuInterface::setYOffset_X(QString offset)
{
  YOffset_X = offset;
}

QString gnuInterface::getYOffset_X()
{
  return YOffset_X;
}

void gnuInterface::setYOffset_Y(QString offset)
{
  YOffset_Y = offset;
}

QString gnuInterface::getYOffset_Y()
{
  return YOffset_Y;
}

void gnuInterface::setZlabel(QString label)
{
  ZLabel = label;
}

QString gnuInterface::getZlabel()
{
  return ZLabel;
}

void gnuInterface::setZOffset_X(QString offset)
{
  ZOffset_X = offset;
}

QString gnuInterface::getZOffset_X()
{
  return ZOffset_X;
}

void gnuInterface::setZOffset_Y(QString offset)
{
  ZOffset_Y = offset;
}

QString gnuInterface::getZOffset_Y()
{
  return ZOffset_Y;
}

void gnuInterface::setTitle(QString intitle)
{
  title = intitle;
}

QString gnuInterface::getTitle()
{
  return title;
}

void gnuInterface::setTitleOffset_X(QString offset)
{
  titleOffset_X = offset;
}

QString gnuInterface::getTitleOffset_X()
{
  return titleOffset_X;
}

void gnuInterface::setTitleOffset_Y(QString offset)
{
  titleOffset_Y = offset;
}

QString gnuInterface::getTitleOffset_Y()
{
 return  titleOffset_Y;
}

void gnuInterface::setTerminal(QString term)
{
  terminal = term;
}

QString gnuInterface::getTerminal()
{
  return terminal;
}

void gnuInterface::setTermLateXEmtexFont(QString font)
{
  termLatexEmtexFont = font;
}

QString gnuInterface::getTermLateXEmtexFont()
{
  return termLatexEmtexFont;
}

void gnuInterface::setTermLateXEmtexFontSize(QString size)
{
  termLatexEmtexFontSize = size;
}

QString gnuInterface::getTermLateXEmtexFontSize()
{
  return termLatexEmtexFontSize;
}

void gnuInterface::setTermPBMFontSize(QString size)
{
  termPBMfontSize = size;
}

QString gnuInterface::getTermPBMFontSize()
{
  return termPBMfontSize;
}

void gnuInterface::setTermPBMColormode(QString color)
{
  termPBMcolormode = color;
}

QString gnuInterface::getTermPBMColormode()
{
  return termPBMcolormode;
}

void gnuInterface::setTermPBMhSize(QString size)
{
  termPBMhSize = size;
  hSize = size;
}

QString gnuInterface::getTermPBMhSize()
{
  return termPBMhSize;
}

void gnuInterface::setTermPBMvSize(QString size)
{
  termPBMvSize = size;
  vSize = size;
}

QString gnuInterface::getTermPBMvSize()
{
  return termPBMvSize;
}

void gnuInterface::setTermPSmode(QString mode)
{
  termPSmode = mode;
}

QString gnuInterface::getTermPSmode()
{
  return termPSmode;
}

void gnuInterface::setTermPScolor(QString color)
{
  termPScolor = color;
}

QString gnuInterface::getTermPScolor()
{
  return termPScolor;
}

void gnuInterface::setTermPSdashed(QString type)
{
  termPSdashed = type;
}

QString gnuInterface::getTermPSdashed()
{
  return termPSdashed;
}

void gnuInterface::setTermPSfont(QString font)
{
  termPSfont = font;
}

QString gnuInterface::getTermPSfont()
{
  return termPSfont;
}

void gnuInterface::setTermPSfontSize(QString size)
{
  termPSfontSize = size;
}

QString gnuInterface::getTermPSfontSize()
{
  return termPSfontSize;
}

void gnuInterface::setTermPSenhanced(QString inenhanced)
{
  termPSenhanced = inenhanced;
}

QString gnuInterface::getTermPSenhanced()
{
  return termPSenhanced;
}

void gnuInterface::setTermPShSize(QString size)
{
  termPShSize = size;
  hSize = size;
}

QString gnuInterface::getTermPShSize()
{
  return termPShSize;
}

void gnuInterface::setTermPSvSize(QString size)
{
  termPSvSize = size;
  vSize = size;
}

QString gnuInterface::getTermPSvSize()
{
  return termPSvSize;
}

void gnuInterface::setOutput(QString inoutput)
{
  output = inoutput;
}

QString gnuInterface::getOutput()
{
  return output;
}

void gnuInterface::setHorizSize(QString size)
{
  hSize = size;
}

QString gnuInterface::getHorizSize()
{
  return hSize;
}

void gnuInterface::setVertSize(QString size)
{
  vSize = size;
}

QString gnuInterface::getVertSize()
{
  return vSize;
}

void gnuInterface::closeGnuplot()
{
  pclose(gnuCommand);
}

void gnuInterface::setReplotFlag(int flag)
{
  replotFlag = flag;
}

void gnuInterface::setLegendFlag(QString flag)
{
  legendFlag = flag;
}

QString gnuInterface::getLegendFlag()
{
  return legendFlag;
}

void gnuInterface::setLegendPositionLeft(int position)
{
  legendPositionLeft = position;
}

int gnuInterface::getLegendPositionLeft()
{
  return legendPositionLeft;
}

void gnuInterface::setLegendPositionRight(int position)
{
  legendPositionRight = position;
}

int gnuInterface::getLegendPositionRight()
{
  return legendPositionRight;
}

void gnuInterface::setLegendPositionTop(int position)
{
  legendPositionTop = position;
}

int gnuInterface::getLegendPositionTop()
{
  return legendPositionTop;
}

void gnuInterface::setLegendPositionBottom(int position)
{
  legendPositionBottom = position;
}

int gnuInterface::getLegendPositionBottom()
{
  return legendPositionBottom;
}

void gnuInterface::setLegendPositionOutside(int position)
{
  legendPositionOutside = position;
}

int gnuInterface::getLegendPositionOutside()
{
  return legendPositionOutside;
}

void gnuInterface::setLegendPositionBelow(int position)
{
  legendPositionBelow = position;
}

int gnuInterface::getLegendPositionBelow()
{
  return legendPositionBelow;
}

void gnuInterface::setLegendPositionXVal(QString val)
{
  legendPositionXVal = val;
}

QString gnuInterface::getLegendPositionXVal()
{
  return legendPositionXVal;
}

void gnuInterface::setLegendPositionYVal(QString val)
{
  legendPositionYVal = val;
}

QString gnuInterface::getLegendPositionYVal()
{
  return legendPositionYVal;
}

void gnuInterface::setLegendPositionZVal(QString val)
{
  legendPositionZVal = val;
}

QString gnuInterface::getLegendPositionZVal()
{
  return legendPositionZVal;
}

void gnuInterface::setLegendTextJustify(QString justify)
{
  legendTextJustify = justify;
}

QString gnuInterface::getLegendTextJustify()
{
  return legendTextJustify;
}

void gnuInterface::setLegendReverse(QString reverse)
{
  legendReverse = reverse;
}

QString gnuInterface::getLegendReverse()
{
  return legendReverse;
}

void gnuInterface::setLegendBox(QString box)
{
  legendBox = box;
}

QString gnuInterface::getLegendBox()
{
  return legendBox;
}

void gnuInterface::setLegendLinetype(QString type)
{
  legendLinetype = type;
}

QString gnuInterface::getLegendLinetype()
{
  return legendLinetype;
}

void gnuInterface::setLegendSampleLength(QString length)
{
  legendSampleLength = length;
}

QString gnuInterface::getLegendSampleLength()
{
  return legendSampleLength;
}

void gnuInterface::setLegendSpacing(QString spacing)
{
  legendSpacing = spacing;
}

QString gnuInterface::getLegendSpacing()
{
  return legendSpacing;
}

void gnuInterface::setLegendWidthIncrement(QString width)
{
  legendWidthIncrement = width;
}

QString gnuInterface::getLegendWidthIncrement()
{
  return legendWidthIncrement;
}

void gnuInterface::setLegendTitle(QString title)
{
  legendTitle = title;
}

QString gnuInterface::getLegendTitle()
{
  return legendTitle;
}

void gnuInterface::setXticsOnFlag(int flag)
{
  xticsOnFlag = flag;
}

int gnuInterface::getXticsOnFlag()
{
  return xticsOnFlag;
}

void gnuInterface::setXticsLocation(QString location)
{
  xticsLocation = location;
}

QString gnuInterface::getXticsLocation()
{
  return xticsLocation;
}

void gnuInterface::setXticsMirror(QString mirror)
{
  xticsMirror = mirror;
}

QString gnuInterface::getXticsMirror()
{
  return xticsMirror;
}

void gnuInterface::setXticsRotation(QString rotation)
{
  xticsRotation = rotation;
}

QString gnuInterface::getXticsRotation()
{
  return xticsRotation;
}

void gnuInterface::setXticsPositionType(QString type)
{
  xticsPositionType = type;
}

QString gnuInterface::getXticsPositionType()
{
  return xticsPositionType;
}

void gnuInterface::setXticsStartPos(QString pos)
{
  xticsStartPos = pos;
}

QString gnuInterface::getXticsStartPos()
{
  return xticsStartPos;
}

void gnuInterface::setXticsIncPos(QString pos)
{
  xticsIncPos = pos;
}

QString gnuInterface::getXticsIncPos()
{
  return xticsIncPos;
}

void gnuInterface::setXticsEndPos(QString pos)
{
  xticsEndPos = pos;
}

QString gnuInterface::getXticsEndPos()
{
  return xticsEndPos;
}

void gnuInterface::setXticsLabelsPos(QString labels)
{
  xticsLabelsPos = labels;
}

QString gnuInterface::getXticsLabelsPos()
{
  return xticsLabelsPos;
}

void gnuInterface::setYticsOnFlag(int flag)
{
  yticsOnFlag = flag;
}

int gnuInterface::getYticsOnFlag()
{
  return yticsOnFlag;
}

void gnuInterface::setYticsLocation(QString location)
{
  yticsLocation = location;
}

QString gnuInterface::getYticsLocation()
{
  return yticsLocation;
}

void gnuInterface::setYticsMirror(QString mirror)
{
  yticsMirror = mirror;
}

QString gnuInterface::getYticsMirror()
{
  return yticsMirror;
}

void gnuInterface::setYticsRotation(QString rotation)
{
  yticsRotation = rotation;
}

QString gnuInterface::getYticsRotation()
{
  return yticsRotation;
}

void gnuInterface::setYticsPositionType(QString type)
{
  yticsPositionType = type;
}

QString gnuInterface::getYticsPositionType()
{
  return yticsPositionType;
}

void gnuInterface::setYticsStartPos(QString pos)
{
  yticsStartPos = pos;
}

QString gnuInterface::getYticsStartPos()
{
  return yticsStartPos;
}

void gnuInterface::setYticsIncPos(QString pos)
{
  yticsIncPos = pos;
}

QString gnuInterface::getYticsIncPos()
{
  return yticsIncPos;
}

void gnuInterface::setYticsEndPos(QString pos)
{
  yticsEndPos = pos;
}

QString gnuInterface::getYticsEndPos()
{
  return yticsEndPos;
}

void gnuInterface::setYticsLabelsPos(QString labels)
{
  yticsLabelsPos = labels;
}

QString gnuInterface::getYticsLabelsPos()
{
  return yticsLabelsPos;
}

void gnuInterface::setZticsOnFlag(int flag)
{
  zticsOnFlag = flag;
}

int gnuInterface::getZticsOnFlag()
{
  return zticsOnFlag;
}

void gnuInterface::setZticsMirror(QString mirror)
{
  zticsMirror = mirror;
}

QString gnuInterface::getZticsMirror()
{
  return zticsMirror;
}

void gnuInterface::setZticsRotation(QString rotation)
{
  zticsRotation = rotation;
}

QString gnuInterface::getZticsRotation()
{
  return zticsRotation;
}

void gnuInterface::setZticsPositionType(QString type)
{
  zticsPositionType = type;
}

QString gnuInterface::getZticsPositionType()
{
  return zticsPositionType;
}

void gnuInterface::setZticsStartPos(QString pos)
{
  zticsStartPos = pos;
}

QString gnuInterface::getZticsStartPos()
{
  return zticsStartPos;
}

void gnuInterface::setZticsIncPos(QString pos)
{
  zticsIncPos = pos;
}

QString gnuInterface::getZticsIncPos()
{
  return zticsIncPos;
}

void gnuInterface::setZticsEndPos(QString pos)
{
  zticsEndPos = pos;
}

QString gnuInterface::getZticsEndPos()
{
  return zticsEndPos;
}

void gnuInterface::setZticsLabelsPos(QString labels)
{
  zticsLabelsPos = labels;
}

QString gnuInterface::getZticsLabelsPos()
{
  return zticsLabelsPos;
}

void gnuInterface::setX2ticsOnFlag(int flag)
{
  x2ticsOnFlag = flag;
}

int gnuInterface::getX2ticsOnFlag()
{
  return x2ticsOnFlag;
}

void gnuInterface::setX2ticsLocation(QString location)
{
  x2ticsLocation = location;
}

QString gnuInterface::getX2ticsLocation()
{
  return x2ticsLocation;
}

void gnuInterface::setX2ticsMirror(QString mirror)
{
  x2ticsMirror = mirror;
}

QString gnuInterface::getX2ticsMirror()
{
  return x2ticsMirror;
}

void gnuInterface::setX2ticsRotation(QString rotation)
{
  x2ticsRotation = rotation;
}

QString gnuInterface::getX2ticsRotation()
{
  return x2ticsRotation;
}

void gnuInterface::setX2ticsPositionType(QString type)
{
  x2ticsPositionType = type;
}

QString gnuInterface::getX2ticsPositionType()
{
  return x2ticsPositionType;
}

void gnuInterface::setX2ticsStartPos(QString pos)
{
  x2ticsStartPos = pos;
}

QString gnuInterface::getX2ticsStartPos()
{
  return x2ticsStartPos;
}

void gnuInterface::setX2ticsIncPos(QString pos)
{
  x2ticsIncPos = pos;
}

QString gnuInterface::getX2ticsIncPos()
{
  return x2ticsIncPos;
}

void gnuInterface::setX2ticsEndPos(QString pos)
{
  x2ticsEndPos = pos;
}

QString gnuInterface::getX2ticsEndPos()
{
  return x2ticsEndPos;
}

void gnuInterface::setX2ticsLabelsPos(QString labels)
{
  x2ticsLabelsPos = labels;
}

QString gnuInterface::getX2ticsLabelsPos()
{
  return x2ticsLabelsPos;
}

void gnuInterface::setY2ticsOnFlag(int flag)
{
  y2ticsOnFlag = flag;
}

int gnuInterface::getY2ticsOnFlag()
{
  return y2ticsOnFlag;
}

void gnuInterface::setY2ticsLocation(QString location)
{
  y2ticsLocation = location;
}

QString gnuInterface::getY2ticsLocation()
{
  return y2ticsLocation;
}

void gnuInterface::setY2ticsMirror(QString mirror)
{
  y2ticsMirror = mirror;
}

QString gnuInterface::getY2ticsMirror()
{
  return y2ticsMirror;
}

void gnuInterface::setY2ticsRotation(QString rotation)
{
  y2ticsRotation = rotation;
}

QString gnuInterface::getY2ticsRotation()
{
  return y2ticsRotation;
}

void gnuInterface::setY2ticsPositionType(QString type)
{
  y2ticsPositionType = type;
}

QString gnuInterface::getY2ticsPositionType()
{
  return y2ticsPositionType;
}

void gnuInterface::setY2ticsStartPos(QString pos)
{
  y2ticsStartPos = pos;
}

QString gnuInterface::getY2ticsStartPos()
{
  return y2ticsStartPos;
}

void gnuInterface::setY2ticsIncPos(QString pos)
{
  y2ticsIncPos = pos;
}

QString gnuInterface::getY2ticsIncPos()
{
  return y2ticsIncPos;
}

void gnuInterface::setY2ticsEndPos(QString pos)
{
  y2ticsEndPos = pos;
}

QString gnuInterface::getY2ticsEndPos()
{
  return y2ticsEndPos;
}

void gnuInterface::setY2ticsLabelsPos(QString labels)
{
  y2ticsLabelsPos = labels;
}

QString gnuInterface::getY2ticsLabelsPos()
{
  return y2ticsLabelsPos;
}

void gnuInterface::insertMultiFileNew(QString filename)
{
  multiFile->insertMultiFileNew(filename);
}

void gnuInterface::removeMultiFile(QString filename)
{
  multiFile->removeMultiFile(filename);
}

void gnuInterface::setMultiFileDataSetStart(QString filename, QString start)
{
  multiFile->setMultiFileDataSetStart(filename, start);
}

QString gnuInterface::getMultiFileDataSetStart(QString filename)
{
  return multiFile->getMultiFileDataSetStart(filename);
}

void gnuInterface::setMultiFileDataSetEnd(QString filename,QString end)
{
  multiFile->setMultiFileDataSetEnd(filename, end);
}

QString gnuInterface::getMultiFileDataSetEnd(QString filename)
{
  return multiFile->getMultiFileDataSetEnd(filename);
}

void gnuInterface::setMultiFileDataSetIncrement(QString filename,QString inc)
{
  multiFile->setMultiFileDataSetIncrement( filename, inc);
}

QString gnuInterface::getMultiFileDataSetIncrement(QString filename)
{
  return multiFile->getMultiFileDataSetIncrement( filename);
}

void gnuInterface::setMultiFileSampPointInc(QString filename,QString pinc)
{
  multiFile->setMultiFileSampPointInc( filename, pinc);
}

QString gnuInterface::getMultiFileSampPointInc(QString filename)
{
  return multiFile->getMultiFileSampPointInc(filename);
}

void gnuInterface::setMultiFileSampLineInc(QString filename,QString linc)
{
  multiFile->setMultiFileSampLineInc(filename, linc);
}

QString gnuInterface::getMultiFileSampLineInc(QString filename)
{
  return multiFile->getMultiFileSampLineInc( filename);
}

void gnuInterface::setMultiFileSampStartPoint(QString filename,QString startp)
{
  multiFile->setMultiFileSampStartPoint( filename, startp);
}

QString gnuInterface::getMultiFileSampStartPoint(QString filename)
{
  return multiFile->getMultiFileSampStartPoint(filename);
}

void gnuInterface::setMultiFileSampStartLine(QString filename,QString startl)
{
  multiFile->setMultiFileSampStartLine( filename, startl);
}

QString gnuInterface::getMultiFileSampStartLine(QString filename)
{
  return multiFile->getMultiFileSampStartLine(filename);
}

void gnuInterface::setMultiFileSampEndPoint(QString filename,QString endp)
{
  multiFile->setMultiFileSampEndPoint( filename, endp);
}

QString gnuInterface::getMultiFileSampEndPoint(QString filename)
{
  return multiFile->getMultiFileSampEndPoint(filename);
}

void gnuInterface::setMultiFileSampEndLine(QString filename,QString endl)
{
  multiFile->setMultiFileSampEndLine( filename, endl);
}

QString gnuInterface::getMultiFileSampEndLine(QString filename)
{
  return multiFile->getMultiFileSampEndLine(filename);
}

void gnuInterface::setMultiFileSmoothType(QString filename,QString type)
{
  multiFile->setMultiFileSmoothType( filename, type);
}

QString gnuInterface::getMultiFileSmoothType(QString filename)
{
  return multiFile->getMultiFileSmoothType(filename);
}

void gnuInterface::insertMultiFileXColumnOption(QString filename, QString xcol)
{
  multiFile->insertMultiFileXColumnOption(filename, xcol);
}

QString gnuInterface::getMultiFileXColumnOption(QString filename)
{
  return multiFile->getMultiFileXColumnOption(filename);
}

void gnuInterface::insertMultiFileYColumnOption(QString filename, QString ycol)
{
  multiFile->insertMultiFileYColumnOption(filename,ycol);
}

QString gnuInterface::getMultiFileYColumnOption(QString filename)
{
  return multiFile->getMultiFileYColumnOption(filename);
}

void gnuInterface::insertMultiFileZColumnOption(QString filename, QString zcol)
{
  multiFile->insertMultiFileZColumnOption(filename,zcol);
}

QString gnuInterface::getMultiFileZColumnOption(QString filename)
{
  return multiFile->getMultiFileZColumnOption(filename);
}

void gnuInterface::setMultiFileLegendTitle(QString filename, QString title)
{
  multiFile->setLegendTitle(filename,title);
}

QString gnuInterface::getMultiFileLegendTitle(QString filename)
{
  return multiFile->getLegendTitle(filename);
}

void gnuInterface::insertMultiFileFormatOption(QString filename, QString format)
{
  multiFile->insertMultiFileFormatOption(filename,format);
}

QString gnuInterface::getMultiFileFormatOption(QString filename)
{
  return multiFile->getMultiFileFormatOption(filename);
}

void gnuInterface::insertMultiFileRawFormatOption(QString filename, QString format)
{
  multiFile->insertMultiFileRawFormatOption(filename,format);
}

QString gnuInterface::getMultiFileRawFormatOption(QString filename)
{
  return multiFile->getMultiFileRawFormatOption(filename);
}

void gnuInterface::setMultiFileStyleOption(QString filename, QString style)
{
  multiFile->setMultiFileStyleOption(filename,style);
}

QString gnuInterface::getMultiFileStyleOption(QString filename)
{
  return multiFile->getMultiFileStyleOption(filename);
}

void gnuInterface::setMultiFileFilter(QString filename, QString filter)
{
  multiFile->setMultiFileFilter(filename, filter);
}

QString gnuInterface::getMultiFileFilter(QString filename)
{
  return multiFile->getMultiFileFilter(filename);
}

void gnuInterface::setMultiFileFilterQuoteChar(QString filename, QString quote)
{
  multiFile->setMultiFileFilterQuoteChar(filename, quote);
}

QString gnuInterface::getMultiFileFilterQuoteChar(QString filename)
{
  return multiFile->getMultiFileFilterQuoteChar(filename);
}

QString gnuInterface::getMultiFileFirstFilename()
{
  return multiFile->getMultiFileFirstFilename();
}

QString gnuInterface::getMultiFileNextFilename()
{
  return multiFile->getMultiFileNextFilename();
}

void gnuInterface::insertMultiFuncNew(QString function)
{
  multiFunc->insertMultiFuncNew(function);
}

void gnuInterface::removeMultiFunc(QString function)
{
  multiFunc->removeMultiFunc(function);
}

void gnuInterface::setMultiFuncStyleOption(QString function, QString style)
{
  multiFunc->setMultiFuncStyleOption(function,style);
}

QString gnuInterface::getMultiFuncStyleOption(QString function)
{
  return multiFunc->getMultiFuncStyleOption(function);
}

QString gnuInterface::getMultiFuncFirstFunction()
{
  return multiFunc->getMultiFuncFirstFunction();
}

QString gnuInterface::getMultiFuncNextFunction()
{
  return multiFunc->getMultiFuncNextFunction();
}

QString gnuInterface::getMultiFuncFirstPlotCmd()
{
  return multiFunc->getMultiFuncFirstPlotCmd();
}

QString gnuInterface::getMultiFuncNextPlotCmd()
{
  return multiFunc->getMultiFuncNextPlotCmd();
}

void gnuInterface::setFileLegendTitle(QString title)
{
  plotFileOb->setLegendTitle(title);
}

QString gnuInterface::getFileLegendTitle()
{
  return plotFileOb->getLegendTitle();
}

void gnuInterface::setFuncLegendTitle(QString title)
{
  plotFunctionOb->setLegendTitle(title);
}

void gnuInterface::setMultiFuncLegendTitle(QString function, QString title)
{
  multiFunc->setLegendTitle(function, title);
}

QString gnuInterface::getFuncLegendTitle()
{
  return plotFunctionOb->getLegendTitle();
}

QString gnuInterface::getMultiFuncLegendTitle(QString function)
{
  return multiFunc->getLegendTitle(function);
}

void gnuInterface::setLogScaleBase(int base)
{
  logScaleBase = base;
}

int gnuInterface::getLogScaleBase()
{
  return logScaleBase;
}

void gnuInterface::setLogScaleXAxis(int xAxis)
{
  logScaleXAxisFlag = xAxis;
}

int gnuInterface::getLogScaleXAxis()
{
  return logScaleXAxisFlag;
}

void gnuInterface::setLogScaleYAxis(int yAxis)
{
  logScaleYAxisFlag = yAxis;
}

int gnuInterface::getLogScaleYAxis()
{
  return logScaleYAxisFlag;
}

void gnuInterface::setLogScaleZAxis(int zAxis)
{
  logScaleZAxisFlag = zAxis;
}

int gnuInterface::getLogScaleZAxis()
{
  return logScaleZAxisFlag;
}

void gnuInterface::setLogScaleX2Axis(int x2Axis)
{
  logScaleX2AxisFlag = x2Axis;
}

int gnuInterface::getLogScaleX2Axis()
{
  return logScaleX2AxisFlag;
}

void gnuInterface::setLogScaleY2Axis(int y2Axis)
{
  logScaleY2AxisFlag = y2Axis;
}

int gnuInterface::getLogScaleY2Axis()
{
  return logScaleY2AxisFlag;
}

void gnuInterface::setBarSizeOption(QString size)
{
  barSize = size;
}

QString gnuInterface::getBarSizeOption()
{
  return barSize;
}

void gnuInterface::setCurveFitFunctionName(QString name)
{
  curveFitOb->setFunctionName(name);
}

QString gnuInterface::getCurveFitFunctionName()
{
  return curveFitOb->getFunctionName();
}

void gnuInterface::setCurveFitFunctionValue(QString function)
{
  curveFitOb->setFunctionValue(function);
}

QString gnuInterface::getCurveFitFunctionValue()
{
  return curveFitOb->getFunctionValue();
}

void gnuInterface::setCurveFitDataFile(QString file)
{
  curveFitOb->setDataFile(file);
}

QString gnuInterface::getCurveFitDataFile()
{
  return curveFitOb->getDataFile();
}

void gnuInterface::setCurveFitVarXRangeName(QString range)
{
  curveFitOb->setVarXRangeName(range);
}

QString gnuInterface::getCurveFitVarXRangeName()
{
  return curveFitOb->getVarXRangeName();
}

void gnuInterface::setCurveFitVarXRangeMin(QString min)
{
  curveFitOb->setVarXRangeMin(min);
}

QString gnuInterface::getCurveFitVarXRangeMin()
{
  return curveFitOb->getVarXRangeMin();
}

void gnuInterface::setCurveFitVarXRangeMax(QString max)
{
  curveFitOb->setVarXRangeMax(max);
}

QString gnuInterface::getCurveFitVarXRangeMax()
{
  return curveFitOb->getVarXRangeMax();
}

void gnuInterface::setCurveFitVarYRangeName(QString range)
{
  curveFitOb->setVarYRangeName(range);
}

QString gnuInterface::getCurveFitVarYRangeName()
{
  return curveFitOb->getVarYRangeName();
}

void gnuInterface::setCurveFitVarYRangeMin(QString min)
{
  curveFitOb->setVarYRangeMin(min);
}

QString gnuInterface::getCurveFitVarYRangeMin()
{
  return curveFitOb->getVarYRangeMin();
}

void gnuInterface::setCurveFitVarYRangeMax(QString max)
{
  curveFitOb->setVarYRangeMax(max);
}

QString gnuInterface::getCurveFitVarYRangeMax()
{
  return curveFitOb->getVarYRangeMax();
}

void gnuInterface::setCurveFitParamFile(QString file)
{
  curveFitOb->setParamFile(file);
}

QString gnuInterface::getCurveFitParamFile()
{
  return curveFitOb->getParamFile();
}

void gnuInterface::setCurveFitParamFileFlag(int flag)
{
  curveFitOb->setParamFileFlag(flag);
}

int gnuInterface::getCurveFitParamFileFlag()
{
  return curveFitOb->getParamFileFlag();
}

void gnuInterface::setCurveFitParamCSLFlag(int flag)
{
  curveFitOb->setParamCSLFlag(flag);
}

int gnuInterface::getCurveFitParamCSLFlag()
{
  return curveFitOb->getParamCSLFlag();
}

void gnuInterface::setCurveFitParamCSL(QString list)
{
  curveFitOb->setParamCSL(list);
}

QString gnuInterface::getCurveFitParamCSL()
{
  return curveFitOb->getParamCSL();
}

void gnuInterface::setCurveFitFitLimit(QString limit)
{
  curveFitOb->setFitLimit(limit);
}

QString gnuInterface::getCurveFitFitLimit()
{
  return curveFitOb->getFitLimit();
}

void gnuInterface::setCurveFitFitMaxIter(QString iter)
{
  curveFitOb->setFitMaxIter(iter);
}

QString gnuInterface::getCurveFitFitMaxIter()
{
  return curveFitOb->getFitMaxIter();
}

void gnuInterface::setCurveFitDataSetStart(QString start)
{
  curveFitOb->setDataSetStart(start);
}

QString gnuInterface::getCurveFitDataSetStart()
{
  return curveFitOb->getDataSetStart();
}

void gnuInterface::setCurveFitDataSetEnd(QString end)
{
  curveFitOb->setDataSetEnd(end);
}

QString gnuInterface::getCurveFitDataSetEnd()
{
  return curveFitOb->getDataSetEnd();
}

void gnuInterface::setCurveFitDataSetInc(QString inc)
{
  curveFitOb->setDataSetInc(inc);
}

QString gnuInterface::getCurveFitDataSetInc()
{
  return curveFitOb->getDataSetInc();
}

void gnuInterface::setCurveFitPointInc(QString inc)
{
  curveFitOb->setPointInc(inc);
}

QString gnuInterface::getCurveFitPointInc()
{
  return curveFitOb->getPointInc();
}

void gnuInterface::setCurveFitLineInc(QString inc)
{
  curveFitOb->setLineInc(inc);
}

QString gnuInterface::getCurveFitLineInc()
{
  return curveFitOb->getLineInc();
}

void gnuInterface::setCurveFitStartPoint(QString start)
{
  curveFitOb->setStartPoint(start);
}

QString gnuInterface::getCurveFitStartPoint()
{
  return curveFitOb->getStartPoint();
}

void gnuInterface::setCurveFitStartLine(QString start)
{
  curveFitOb->setStartLine(start);
}

QString gnuInterface::getCurveFitStartLine()
{
  return curveFitOb->getStartLine();
}

void gnuInterface::setCurveFitEndPoint(QString end)
{
  curveFitOb->setEndPoint(end);
}

QString gnuInterface::getCurveFitEndPoint()
{
  return curveFitOb->getEndPoint();
}

void gnuInterface::setCurveFitEndLine(QString end)
{
  curveFitOb->setEndLine(end);
}

QString gnuInterface::getCurveFitEndLine()
{
  return curveFitOb->getEndLine();
}

void gnuInterface::setCurveFitXColumn(QString col)
{
  curveFitOb->setXColumn(col);
}

QString gnuInterface::getCurveFitXColumn()
{
  return curveFitOb->getXColumn();
}

void gnuInterface::setCurveFitYColumn(QString col)
{
  curveFitOb->setYColumn(col);
}

QString gnuInterface::getCurveFitYColumn()
{
  return curveFitOb->getYColumn();
}

void gnuInterface::setCurveFitZColumn(QString col)
{
  curveFitOb->setZColumn(col);
}

QString gnuInterface::getCurveFitZColumn()
{
  return curveFitOb->getZColumn();
}

void gnuInterface::setCurveFitFormat(QString informat)
{
  curveFitOb->setFormat(informat);
}

QString gnuInterface::getCurveFitFormat()
{
  return curveFitOb->getFormat();
}

void gnuInterface::setCurveFitRawFormat(QString format)
{
  curveFitOb->setRawFormat(format);
}

QString gnuInterface::getCurveFitRawFormat()
{
  return curveFitOb->getRawFormat();
}

void gnuInterface::doCurveFit()
{
  QString fitcmd;

  fitcmd = curveFitOb->getFitCmd();

  cout << fitcmd << endl;

  fprintf(gnuCommand,fitcmd);
  fprintf(gnuCommand,"\n");
  fflush(gnuCommand);
}

void gnuInterface::setBoxWidth(QString width)
{
  boxWidth = width;
}

QString gnuInterface::getBoxWidth()
{
  return boxWidth;
}

void gnuInterface::setRotationXAxis(int value)
{
  xAxisRotation = value;
}

int gnuInterface::getRotationXAxis()
{
  return xAxisRotation;
}

void gnuInterface::setRotationZAxis(int value)
{
  zAxisRotation = value;
}

int gnuInterface::getRotationZAxis()
{
  return zAxisRotation;
}

void gnuInterface::setRotationScaling(QString scale)
{
  rotationScale = scale;
}

QString gnuInterface::getRotationScaling()
{
  return rotationScale;
}

void gnuInterface::setRotationZAxisScaling(QString scale)
{
  zAxisScale = scale;
}

QString gnuInterface::getRotationZAxisScaling()
{
  return zAxisScale;
}

void gnuInterface::setTicsLevel(QString level)
{
  ticsLevel = level;
}

QString gnuInterface::getTicsLevel()
{
  return ticsLevel;
}

void gnuInterface::setd3HiddenLineFlag(int flag)
{
  d3HiddenLineFlag = flag;
}

int gnuInterface::getd3HiddenLineFlag()
{
  return d3HiddenLineFlag;
}

void gnuInterface::setIsolineU(QString isoU)
{
  isoLineU = isoU;
}

QString gnuInterface::getIsolineU()
{
  return isoLineU;
}

void gnuInterface::setIsolineV(QString isoV)
{
  isoLineV = isoV;
}

QString gnuInterface::getIsolineV()
{
  return isoLineV;
}

void gnuInterface::savePlotData(QString filename)
{
  cout << endl << "Saving Xgfe plot data to " << filename << endl;

  QFile file(filename);
  if ( !file.open( IO_WriteOnly ) ) return;
  QTextStream outfile( &file );
  // open output file
//  ofstream outfile(filename);

  // save all options to the file

  // save header to identify this file
  outfile << "XGFE-FILE" << endl;

  // single file flag
  outfile << "singleFileFlag " << plotFileFlag << " ;" << endl;

  // single function flag
  outfile << "singleFuncFlag " << plotFuncFlag << " ;" << endl;

  // multiple file flag
  outfile << "multiFileFlag " << plotMultipleFileFlag << " ;" << endl;

  // multiple function flag
  outfile << "multiFuncFlag " << plotMultipleFuncFlag << " ;" << endl;

  // single plot file filename
  outfile << "singleFilename " << getPlotFilename() << " ;" << endl;

  // single plot function
  outfile <<  "singleFunction " << getPlotFunction() << " ;" << endl;

  // single file plot type
  outfile << "singleFilePlotType " << getFilePlotType() << " ;" << endl;

  // single file style type
  outfile << "singleFileStyleType " << getFileStyleType() << " ;" << endl;

  // single file modifiers
  outfile << "singleFileDataSetStart " << getFileDataSetStart() << " ;" << endl;
  outfile << "singleFileDataSetEnd " << getFileDataSetEnd() << " ;" << endl;
  outfile << "singleFileDataSetIncrement " << getFileDataSetIncrement()
          << " ;" << endl;
  outfile << "singleFileSampPointInc " << getFileSampPointInc() << " ;" << endl;
  outfile << "singleFileSampLineInc " << getFileSampLineInc() << " ;" << endl;
  outfile << "singleFileSampStartPoint " << getFileSampStartPoint()
          << " ;" << endl;
  outfile << "singleFileSampStartLine " << getFileSampStartLine() << " ;"
          << endl;
  outfile << "singleFileSampEndPoint " << getFileSampEndPoint() << " ;" << endl;
  outfile << "singleFileSampEndLine " << getFileSampEndLine() << " ;" << endl;
  outfile << "singleFileSmoothType " << getFileSmoothType() << " ;" << endl;
  outfile << "singleFileXColumn " << getFileXColumn() << " ;" << endl;
  outfile << "singleFileYColumn " << getFileYColumn() << " ;" << endl;
  outfile << "singleFileZColumn " << getFileZColumn() << " ;" << endl;
  outfile << "singleFileFormatString " << getFileFormatString()  << " ;" << endl;
  outfile << "singleRawFileFormatString " << getRawFileFormatString()  << " ;" << endl;
  outfile << "singleFileFilter " << getFileFilter()  << " ;" << endl;
  outfile << "singleFileFilterQuoteChar " << getFileFilterQuoteChar()  << " ;" << endl;
  outfile << "singleFileLegendTitle " << getFileLegendTitle()  << " ;" << endl;

  // get overall options
  outfile << "XVariableName " << XVariableName  << " ;" << endl;
  outfile << "XRangeStart " << XRangeStart  << " ;" << endl;
  outfile << "XRangeEnd " << XRangeEnd  << " ;" << endl;
  outfile << "YVariableName " << YVariableName  << " ;" << endl;
  outfile << "YRangeStart " << YRangeStart  << " ;" << endl;
  outfile << "YRangeEnd " << YRangeEnd  << " ;" << endl;
  outfile << "ZRangeStart " << ZRangeStart  << " ;" << endl;
  outfile << "ZRangeEnd " << ZRangeEnd  << " ;" << endl;
  outfile << "XLabel " << XLabel  << " ;" << endl;
  outfile << "XOffset_X " << XOffset_X  << " ;" << endl;
  outfile << "XOffset_Y " << XOffset_Y  << " ;" << endl;
  outfile << "YLabel " << YLabel  << " ;" << endl;
  outfile << "YOffset_X " << YOffset_X  << " ;" << endl;
  outfile << "YOffset_Y " << YOffset_Y  << " ;" << endl;
  outfile << "ZLabel " << ZLabel  << " ;" << endl;
  outfile << "ZOffset_X " << ZOffset_X  << " ;" << endl;
  outfile << "ZOffset_Y " << ZOffset_Y  << " ;" << endl;
  outfile << "title " << title  << " ;" << endl;
  outfile << "titleOffset_X " << titleOffset_X  << " ;" << endl;
  outfile << "titleOffset_Y " << titleOffset_Y  << " ;" << endl;
  outfile << "terminal " << terminal  << " ;" << endl;
  outfile << "termLatexEmtexFont " << termLatexEmtexFont  << " ;" << endl;
  outfile << "termLatexEmtexFontSize " << termLatexEmtexFontSize  << " ;" << endl;
  outfile << "termPBMfontSize " << termPBMfontSize  << " ;" << endl;
  outfile << "termPBMcolormode " << termPBMcolormode  << " ;" << endl;
  outfile << "termPBMhSize " << termPBMhSize  << " ;" << endl;
  outfile << "termPBMvSize " << termPBMvSize  << " ;" << endl;
  outfile << "termPSmode " << termPSmode  << " ;" << endl;
  outfile << "termPScolor " << termPScolor  << " ;" << endl;
  outfile << "termPSdashed " << termPSdashed  << " ;" << endl;
  outfile << "termPSfont " << termPSfont  << " ;" << endl;
  outfile << "termPSfontSize " << termPSfontSize  << " ;" << endl;
  outfile << "termPSenhanced " << termPSenhanced  << " ;" << endl;
  outfile << "termPShSize " << termPShSize  << " ;" << endl;
  outfile << "termPSvSize " << termPSvSize  << " ;" << endl;
  outfile << "output " << output  << " ;" << endl;
  outfile << "hSize " << hSize  << " ;" << endl;
  outfile << "vSize " << vSize  << " ;" << endl;
  outfile << "legendFlag " << getLegendFlag()  << " ;" << endl;
  outfile << "legendPositionLeftFlag " << legendPositionLeft  << " ;" << endl;
  outfile << "legendPositionRightFlag " << legendPositionRight  << " ;" << endl;
  outfile << "legendPositionTopFlag " << legendPositionTop  << " ;" << endl;
  outfile << "legendPositionBottomFlag " << legendPositionBottom  << " ;" << endl;
  outfile << "legendPositionOutsideFlag " << legendPositionOutside  << " ;" << endl;
  outfile << "legendPositionBelowFlag " << legendPositionBelow  << " ;" << endl;
  outfile << "legendPositionXVal " << legendPositionXVal  << " ;" << endl;
  outfile << "legendPositionYVal " << legendPositionYVal  << " ;" << endl;
  outfile << "legendPositionZVal " << legendPositionZVal  << " ;" << endl;
  outfile << "legendTextJustify " << legendTextJustify  << " ;" << endl;
  outfile << "legendReverse " << legendReverse  << " ;" << endl;
  outfile << "legendBox " << legendBox  << " ;" << endl;
  outfile << "legendLinetype " << legendLinetype  << " ;" << endl;
  outfile << "legendSampleLength " << legendSampleLength  << " ;" << endl;
  outfile << "legendSpacing " << legendSpacing  << " ;" << endl;
  outfile << "legendWidthIncrement " << legendWidthIncrement  << " ;" << endl;
  outfile << "legendTitle " << legendTitle  << " ;" << endl;
  outfile << "logScaleBase " << logScaleBase  << " ;" << endl;
  outfile << "logScaleXAxisFlag " << logScaleXAxisFlag  << " ;" << endl;
  outfile << "logScaleYAxisFlag " << logScaleYAxisFlag  << " ;" << endl;
  outfile << "logScaleZAxisFlag " << logScaleZAxisFlag  << " ;" << endl;
  outfile << "logScaleX2AxisFlag " << logScaleX2AxisFlag  << " ;" << endl;
  outfile << "logScaleY2AxisFlag " << logScaleY2AxisFlag  << " ;" << endl;
  outfile << "barSize " << barSize  << " ;" << endl;
  outfile << "boxWidth " << boxWidth  << " ;" << endl;
  outfile << "xticsOnFlag " << xticsOnFlag  << " ;" << endl;
  outfile << "xticsLocation " << xticsLocation  << " ;" << endl;
  outfile << "xticsMirror " << xticsMirror  << " ;" << endl;
  outfile << "xticsRotation " << xticsRotation  << " ;" << endl;
  outfile << "xticsPositionType " << xticsPositionType  << " ;" << endl;
  outfile << "xticsStartPos " << xticsStartPos  << " ;" << endl;
  outfile << "xticsIncPos " << xticsIncPos  << " ;" << endl;
  outfile << "xticsEndPos " << xticsEndPos  << " ;" << endl;
  outfile << "xticsLabelsPos " << xticsLabelsPos  << " ;" << endl;
  outfile << "yticsOnFlag " << yticsOnFlag  << " ;" << endl;
  outfile << "yticsLocation " << yticsLocation  << " ;" << endl;
  outfile << "yticsMirror " << yticsMirror  << " ;" << endl;
  outfile << "yticsRotation " << yticsRotation  << " ;" << endl;
  outfile << "yticsPositionType " << yticsPositionType  << " ;" << endl;
  outfile << "yticsStartPos " << yticsStartPos  << " ;" << endl;
  outfile << "yticsIncPos " << yticsIncPos  << " ;" << endl;
  outfile << "yticsEndPos " << yticsEndPos  << " ;" << endl;
  outfile << "yticsLabelsPos " << yticsLabelsPos  << " ;" << endl;
  outfile << "zticsOnFlag " << zticsOnFlag  << " ;" << endl;
  outfile << "zticsMirror " << zticsMirror  << " ;" << endl;
  outfile << "zticsRotation " << zticsRotation  << " ;" << endl;
  outfile << "zticsPositionType " << zticsPositionType  << " ;" << endl;
  outfile << "zticsStartPos " << zticsStartPos  << " ;" << endl;
  outfile << "zticsIncPos " << zticsIncPos  << " ;" << endl;
  outfile << "zticsEndPos " << zticsEndPos  << " ;" << endl;
  outfile << "zticsLabelsPos " << zticsLabelsPos  << " ;" << endl;
  outfile << "x2ticsOnFlag " << x2ticsOnFlag  << " ;" << endl;
  outfile << "x2ticsLocation " << x2ticsLocation  << " ;" << endl;
  outfile << "x2ticsMirror " << x2ticsMirror  << " ;" << endl;
  outfile << "x2ticsRotation " << x2ticsRotation  << " ;" << endl;
  outfile << "x2ticsPositionType " << x2ticsPositionType  << " ;" << endl;
  outfile << "x2ticsStartPos " << x2ticsStartPos  << " ;" << endl;
  outfile << "x2ticsIncPos " << x2ticsIncPos  << " ;" << endl;
  outfile << "x2ticsEndPos " << x2ticsEndPos  << " ;" << endl;
  outfile << "x2ticsLabelsPos " << x2ticsLabelsPos  << " ;" << endl;
  outfile << "y2ticsOnFlag " << y2ticsOnFlag  << " ;" << endl;
  outfile << "y2ticsLocation " << y2ticsLocation  << " ;" << endl;
  outfile << "y2ticsMirror " << y2ticsMirror  << " ;" << endl;
  outfile << "y2ticsRotation " << y2ticsRotation  << " ;" << endl;
  outfile << "y2ticsPositionType " << y2ticsPositionType  << " ;" << endl;
  outfile << "y2ticsStartPos " << y2ticsStartPos  << " ;" << endl;
  outfile << "y2ticsIncPos " << y2ticsIncPos  << " ;" << endl;
  outfile << "y2ticsEndPos " << y2ticsEndPos  << " ;" << endl;
  outfile << "y2ticsLabelsPos " << y2ticsLabelsPos  << " ;" << endl;

  // 3D plot options
  outfile << "xAxisRotation " << xAxisRotation << " ;" << endl;
  outfile << "zAxisRotation " << zAxisRotation << " ;" << endl;
  outfile << "rotationScale " << rotationScale << " ;" << endl;
  outfile << "zAxisScale " << zAxisScale << " ;" << endl;
  outfile << "ticsLevel " << ticsLevel << " ;" << endl;
  outfile << "d3HiddenLineFlag " << d3HiddenLineFlag << " ;" << endl;
  outfile << "isoLineU " << isoLineU << " ;" << endl;
  outfile << "isoLineV " << isoLineV << " ;" << endl;

  // write out multiple file data

  // get the first file
  QString tempfile = getMultiFileFirstFilename();

  // get the remaining files
  while (tempfile != "END")
  {
    outfile << "MultiFileNew " << tempfile << " ;" << endl;

    outfile << "MultiFileDataSetStart " << tempfile << " ; "
            << getMultiFileDataSetStart(tempfile) << " ;" << endl;

    outfile << "MultiFileDataSetEnd " << tempfile << " ; "
            << getMultiFileDataSetEnd(tempfile) << " ;" << endl;

    outfile << "MultiFileDataSetIncrement " << tempfile << " ; "
            << getMultiFileDataSetIncrement(tempfile)  << " ;" << endl;

    outfile << "MultiFileSampPointInc " << tempfile << " ; "
            << getMultiFileSampPointInc(tempfile)  << " ;" << endl;

    outfile << "MultiFileSampLineInc " << tempfile << " ; "
            << getMultiFileSampLineInc(tempfile)  << " ;" << endl;

    outfile << "MultiFileSampStartPoint " << tempfile << " ; "
            <<  getMultiFileSampStartPoint(tempfile) << " ;" << endl;

    outfile << "MultiFileSampStartLine " << tempfile << " ; "
            <<  getMultiFileSampStartLine(tempfile) << " ;" << endl;

    outfile << "MultiFileSampEndPoint " << tempfile << " ; "
            <<  getMultiFileSampEndPoint(tempfile) << " ;" << endl;

    outfile << "MultiFileSampEndLine " << tempfile << " ; "
            << getMultiFileSampEndLine(tempfile)  << " ;" << endl;

    outfile << "MultiFileSmoothType " << tempfile << " ; "
            <<  getMultiFileSmoothType(tempfile) << " ;" << endl;

    outfile << "MultiFileXColumnOption " << tempfile << " ; "
            <<  getMultiFileXColumnOption(tempfile) << " ;" << endl;

    outfile << "MultiFileYColumnOption " << tempfile << " ; "
            <<  getMultiFileYColumnOption(tempfile) << " ;" << endl;

    outfile << "MultiFileZColumnOption " << tempfile << " ; "
            <<  getMultiFileZColumnOption(tempfile) << " ;" << endl;

    outfile << "MultiFileFormatOption " << tempfile << " ; "
            <<  getMultiFileFormatOption(tempfile) << " ;" << endl;

    outfile << "MultiFileRawFormatOption " << tempfile << " ; "
            <<  getMultiFileRawFormatOption(tempfile) << " ;" << endl;

    outfile << "MultiFileStyleOption " << tempfile << " ; "
            <<  getMultiFileStyleOption(tempfile) << " ;" << endl;

    outfile << "MultiFileLegendTitle " << tempfile << " ; "
            <<  getMultiFileLegendTitle(tempfile) << " ;" << endl;

    outfile << "MultiFileFilter " << tempfile << " ; "
            <<  getMultiFileFilter(tempfile) << " ;" << endl;

    outfile << "MultiFileFilterQuoteChar " << tempfile << " ; "
            <<  getMultiFileFilterQuoteChar(tempfile) << " ;" << endl;

    tempfile = getMultiFileNextFilename();
  }

  // write single function options
  outfile << "singleFuncPlotType " << getFuncPlotType()  << " ;" << endl;
  outfile << "singleFuncStyleType " << getFuncStyleType()  << " ;" << endl;
  outfile << "singleFuncLegendTitle " << getFuncLegendTitle()  << " ;" << endl;

  // write multiple function options

  QString tempfunc = getMultiFuncFirstFunction();

  while (tempfunc != "END")
  {
    outfile << "MultiFuncNew " << tempfunc << " ;" << endl;

    outfile << "MultiFuncStyleOption " << tempfunc << " ; "
            << getMultiFuncStyleOption(tempfunc) << " ;" << endl;

    outfile << "MultiFuncLegendTitle " << tempfunc << " ; "
            << getMultiFuncLegendTitle(tempfunc) << " ;" << endl;

    tempfunc = getMultiFuncNextFunction();
  }
file.close();
cout << "Done." << endl;
}



void gnuInterface::loadPlotData(QString filename)
{
  cout << endl << "Loading Xgfe plot data from " << filename << endl;

  QString token;
  QString buffer;
  QString tempfile;
  QString tempfunc;
  int tempint;
  bool ok;

  QFile file(filename);
  if ( !file.open( IO_ReadOnly ) ) return;
  QTextStream infile( &file );

  // check header
    infile >> token;

  if (token != "XGFE-FILE")
    cout << endl << "Error! This is not an Xgfe saved file!" << endl;
  else
  {
    while ( !infile.atEnd() )
    {
      infile >> token;
      if (token == "singleFileFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setPlotFileFlag(tempint);
      }
      else if (token == "singleFuncFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setPlotFuncFlag(tempint);
      }
      else if (token == "multiFileFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setPlotMultipleFileFlag(tempint);
      }
      else if (token == "multiFuncFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setPlotMultipleFuncFlag(tempint);
      }
      else if (token == "singleFilename")
      {
       buffer=getValue(infile);
        setPlotFilename(buffer);
      }
      else if (token == "singleFunction")
      {
       buffer=getValue(infile);
        setPlotFunction(buffer);
      }
      else if (token == "singleFilePlotType")
      {
       buffer=getValue(infile);
        setFilePlotType(buffer);
      }
      else if (token == "singleFileStyleType")
      {
       buffer=getValue(infile);
        setFileStyleType(buffer);
      }
      else if (token == "singleFuncPlotType")
      {
       buffer=getValue(infile);
        setFuncPlotType(buffer);
      }
      else if (token == "singleFuncStyleType")
      {
       buffer=getValue(infile);
        setFuncStyleType(buffer);
      }
      else if (token == "singleFuncLegendTitle")
      {
       buffer=getValue(infile);
        setFuncLegendTitle(buffer);
      }
      else if (token == "singleFileDataSetStart")
      {
       buffer=getValue(infile);
        setFileDataSetStart(buffer);
      }
      else if (token == "singleFileDataSetEnd")
      {
       buffer=getValue(infile);
        setFileDataSetEnd(buffer);
      }
      else if (token == "singleFileDataSetIncrement")
      {
       buffer=getValue(infile);
        setFileDataSetIncrement(buffer);
      }
      else if (token == "singleFileSampPointInc")
      {
       buffer=getValue(infile);
        setFileSampPointInc(buffer);
      }
      else if (token == "singleFileSampLineInc")
      {
       buffer=getValue(infile);
        setFileSampLineInc(buffer);
      }
      else if (token == "singleFileSampStartPoint")
      {
       buffer=getValue(infile);
        setFileSampStartPoint(buffer);
      }
      else if (token == "singleFileSampStartLine")
      {
       buffer=getValue(infile);
        setFileSampStartLine(buffer);
      }
      else if (token == "singleFileSampEndPoint")
      {
       buffer=getValue(infile);
        setFileSampEndPoint(buffer);
      }
      else if (token == "singleFileSampEndLine")
      {
       buffer=getValue(infile);
        setFileSampEndLine(buffer);
      }
      else if (token == "singleFileSmoothType")
      {
       buffer=getValue(infile);
        setFileSmoothType(buffer);
      }
      else if (token == "singleFileXColumn")
      {
       buffer=getValue(infile);
        setFileXColumn(buffer);
      }
      else if (token == "singleFileYColumn")
      {
       buffer=getValue(infile);
        setFileYColumn(buffer);
      }
      else if (token == "singleFileZColumn")
      {
       buffer=getValue(infile);
        setFileZColumn(buffer);
      }
      else if (token == "singleFileFormatString")
      {
       buffer=getValue(infile);
        setFileFormatString(buffer);
      }
      else if (token == "singleRawFileFormatString")
      {
       buffer=getValue(infile);
        setRawFileFormatString(buffer);
      }
      else if (token == "singleFileFilter")
      {
       buffer=getValue(infile);
        setFileFilter(buffer);
      }
      else if (token == "singleFileFilterQuoteChar")
      {
       buffer=getValue(infile);
        setFileFilterQuoteChar(buffer);
      }
      else if (token == "singleFileLegendTitle")
      {
       buffer=getValue(infile);
        setFileLegendTitle(buffer);
      }
      else if (token == "XVariableName")
      {
       buffer=getValue(infile);
        setXVariableName(buffer);
      }
      else if (token == "XRangeStart")
      {
       buffer=getValue(infile);
        setXRangeStart(buffer);
      }
      else if (token == "XRangeEnd")
      {
       buffer=getValue(infile);
        setXRangeEnd(buffer);
      }
      else if (token == "YVariableName")
      {
       buffer=getValue(infile);
        setYVariableName(buffer);
      }
      else if (token == "YRangeStart")
      {
       buffer=getValue(infile);
        setYRangeStart(buffer);
      }
      else if (token == "YRangeEnd")
      {
       buffer=getValue(infile);
        setYRangeEnd(buffer);
      }
      else if (token == "ZRangeStart")
      {
       buffer=getValue(infile);
        setZRangeStart(buffer);
      }
      else if (token == "ZRangeEnd")
      {
       buffer=getValue(infile);
        setZRangeEnd(buffer);
      }
      else if (token == "XLabel")
      {
       buffer=getValue(infile);
        setXlabel(buffer);
      }
      else if (token == "XOffset_X")
      {
       buffer=getValue(infile);
        setXOffset_X(buffer);
      }
      else if (token == "XOffset_Y")
      {
       buffer=getValue(infile);
        setXOffset_Y(buffer);
      }
      else if (token == "YLabel")
      {
       buffer=getValue(infile);
        setYlabel(buffer);
      }
      else if (token == "YOffset_X")
      {
       buffer=getValue(infile);
        setYOffset_X(buffer);
      }
      else if (token == "YOffset_Y")
      {
       buffer=getValue(infile);
        setYOffset_Y(buffer);
      }
      else if (token == "ZLabel")
      {
       buffer=getValue(infile);
        setZlabel(buffer);
      }
      else if (token == "ZOffset_X")
      {
       buffer=getValue(infile);
        setZOffset_X(buffer);
      }
      else if (token == "ZOffset_Y")
      {
       buffer=getValue(infile);
        setZOffset_Y(buffer);
      }
      else if (token == "title")
      {
       buffer=getValue(infile);
        setTitle(buffer);
      }
      else if (token == "titleOffset_X")
      {
       buffer=getValue(infile);
        setTitleOffset_X(buffer);
      }
      else if (token == "titleOffset_Y")
      {
       buffer=getValue(infile);
        setTitleOffset_Y(buffer);
      }
      else if (token == "terminal")
      {
       buffer=getValue(infile);
        setTerminal(buffer);
      }
      else if (token == "termLatexEmtexFont")
      {
       buffer=getValue(infile);
        setTermLateXEmtexFont(buffer);
      }
      else if (token == "termLatexEmtexFontSize")
      {
       buffer=getValue(infile);
        setTermLateXEmtexFontSize(buffer);
      }
      else if (token == "termPBMfontSize")
      {
       buffer=getValue(infile);
        setTermPBMFontSize(buffer);
      }
      else if (token == "termPBMcolormode")
      {
       buffer=getValue(infile);
        setTermPBMColormode(buffer);
      }
      else if (token == "termPBMhSize")
      {
       buffer=getValue(infile);
        setTermPBMhSize(buffer);
      }
      else if (token == "termPBMvSize")
      {
       buffer=getValue(infile);
        setTermPBMvSize(buffer);
      }
      else if (token == "termPSmode")
      {
       buffer=getValue(infile);
        setTermPSmode(buffer);
      }
      else if (token == "termPScolor")
      {
       buffer=getValue(infile);
        setTermPScolor(buffer);
      }
      else if (token == "termPSdashed")
      {
       buffer=getValue(infile);
        setTermPSdashed(buffer);
      }
      else if (token == "termPSfont")
      {
       buffer=getValue(infile);
        setTermPSfont(buffer);
      }
      else if (token == "termPSfontSize")
      {
       buffer=getValue(infile);
        setTermPSfontSize(buffer);
      }
      else if (token == "termPSenhanced")
      {
       buffer=getValue(infile);
        setTermPSenhanced(buffer);
      }
      else if (token == "termPShSize")
      {
       buffer=getValue(infile);
        setTermPShSize(buffer);
      }
      else if (token == "termPSvSize")
      {
       buffer=getValue(infile);
        setTermPSvSize(buffer);
      }
      else if (token == "output")
      {
       buffer=getValue(infile);
        setOutput(buffer);
      }
      else if (token == "hSize")
      {
       buffer=getValue(infile);
        setHorizSize(buffer);
      }
      else if (token == "vSize")
      {
       buffer=getValue(infile);
        setVertSize(buffer);
      }
      else if (token == "legendFlag")
      {
       buffer=getValue(infile);
        setLegendFlag(buffer);
      }
      else if (token == "legendPositionLeftFlag")
      {
       buffer=getValue(infile);
      tempint=buffer.toInt(&ok,10);
      if (!ok) tempint=0;
      setLegendPositionLeft(tempint);
      }
      else if (token == "legendPositionRightFlag")
      {
       buffer=getValue(infile);
      tempint=buffer.toInt(&ok,10);
      if (!ok) tempint=0;
      setLegendPositionRight(tempint);
      }
      else if (token == "legendPositionTopFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLegendPositionTop(tempint);
      }
      else if (token == "legendPositionBottomFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLegendPositionBottom(tempint);
      }
      else if (token == "legendPositionOutsideFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLegendPositionOutside(tempint);
      }
      else if (token == "legendPositionBelowFlag")
      {
       buffer=getValue(infile);
      tempint=buffer.toInt(&ok,10);
      if (!ok) tempint=0;
      setLegendPositionBelow(tempint);
      }
      else if (token == "legendPositionXVal")
      {
       buffer=getValue(infile);
        setLegendPositionXVal(buffer);
      }
      else if (token == "legendPositionYVal")
      {
       buffer=getValue(infile);
        setLegendPositionYVal(buffer);
      }
      else if (token == "legendPositionZVal")
      {
       buffer=getValue(infile);
        setLegendPositionZVal(buffer);
      }
      else if (token == "legendTextJustify")
      {
       buffer=getValue(infile);
        setLegendTextJustify(buffer);
      }
      else if (token == "legendReverse")
      {
       buffer=getValue(infile);
        setLegendReverse(buffer);
      }
      else if (token == "legendBox")
      {
       buffer=getValue(infile);
        setLegendBox(buffer);
      }
      else if (token == "legendLinetype")
      {
       buffer=getValue(infile);
        setLegendLinetype(buffer);
      }
      else if (token == "legendSampleLength")
      {
       buffer=getValue(infile);
        setLegendSampleLength(buffer);
      }
      else if (token == "legendSpacing")
      {
       buffer=getValue(infile);
        setLegendSpacing(buffer);
      }
      else if (token == "legendWidthIncrement")
      {
       buffer=getValue(infile);
        setLegendWidthIncrement(buffer);
      }
      else if (token == "legendTitle")
      {
       buffer=getValue(infile);
        setLegendTitle(buffer);
      }
      else if (token == "logScaleBase")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLogScaleBase(tempint);
      }
      else if (token == "logScaleXAxisFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLogScaleYAxis(tempint);
      }
      else if (token == "logScaleYAxisFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLogScaleYAxis(tempint);
      }
      else if (token == "logScaleZAxisFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLogScaleZAxis(tempint);
      }
      else if (token == "logScaleX2AxisFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLogScaleX2Axis(tempint);
      }
      else if (token == "logScaleY2AxisFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setLogScaleY2Axis(tempint);
      }
      else if (token == "barSize")
      {
       buffer=getValue(infile);
        setBarSizeOption(buffer);
      }
      else if (token == "boxWidth")
      {
       buffer=getValue(infile);
        setBoxWidth(buffer);
      }
      else if (token == "xticsOnFlag")
      {
       buffer=getValue(infile);
        tempint=buffer.toInt(&ok,10);
        if (!ok) tempint=0;
        setXticsOnFlag(tempint);
      }
      else if (token == "xticsLocation")
      {
       buffer=getValue(infile);
        setXticsLocation(buffer);
      }
      else if (token == "xticsMirror")
      {
       buffer=getValue(infile);
        setXticsMirror(buffer);
      }
      else if (token == "xticsRotation")
      {
       buffer=getValue(infile);
        setXticsRotation(buffer);
      }
      else if (token == "xticsPositionType")
      {
       buffer=getValue(infile);
        setXticsPositionType(buffer);
      }
      else if (token == "xticsStartPos")
      {
       buffer=getValue(infile);
        setXticsStartPos(buffer);
      }
      else if (token == "xticsIncPos")
      {
       buffer=getValue(infile);
        setXticsIncPos(buffer);
      }
      else if (token == "xticsEndPos")
      {
       buffer=getValue(infile);
        setXticsEndPos(buffer);
      }
      else if (token == "xticsLabelsPos")
      {
       buffer=getValue(infile);
        setXticsLabelsPos(buffer);
      }
      else if (token == "yticsOnFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setYticsOnFlag(tempint);
      }
      else if (token == "yticsLocation")
      {
       buffer=getValue(infile);
        setYticsLocation(buffer);
      }
      else if (token == "yticsMirror")
      {
       buffer=getValue(infile);
        setYticsMirror(buffer);
      }
      else if (token == "yticsRotation")
      {
       buffer=getValue(infile);
        setYticsRotation(buffer);
      }
      else if (token == "yticsPositionType")
      {
       buffer=getValue(infile);
        setYticsPositionType(buffer);
      }
      else if (token == "yticsStartPos")
      {
       buffer=getValue(infile);
        setYticsStartPos(buffer);
      }
      else if (token == "yticsIncPos")
      {
       buffer=getValue(infile);
        setYticsIncPos(buffer);
      }
      else if (token == "yticsEndPos")
      {
       buffer=getValue(infile);
        setYticsEndPos(buffer);
      }
      else if (token == "yticsLabelsPos")
      {
       buffer=getValue(infile);
        setYticsLabelsPos(buffer);
      }
      else if (token == "zticsOnFlag")
      {
       buffer=getValue(infile);
        tempint=buffer.toInt(&ok,10);
        if (!ok) tempint=0;
        setZticsOnFlag(tempint);
      }
      else if (token == "zticsMirror")
      {
       buffer=getValue(infile);
        setZticsMirror(buffer);
      }
      else if (token == "zticsRotation")
      {
       buffer=getValue(infile);
        setZticsRotation(buffer);
      }
      else if (token == "zticsPositionType")
      {
       buffer=getValue(infile);
        setZticsPositionType(buffer);
      }
      else if (token == "zticsStartPos")
      {
       buffer=getValue(infile);
        setZticsStartPos(buffer);
      }
      else if (token == "zticsIncPos")
      {
       buffer=getValue(infile);
        setZticsIncPos(buffer);
      }
      else if (token == "zticsEndPos")
      {
       buffer=getValue(infile);
        setZticsEndPos(buffer);
      }
      else if (token == "zticsLabelsPos")
      {
       buffer=getValue(infile);
        setZticsLabelsPos(buffer);
      }
      else if (token == "x2ticsOnFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setX2ticsOnFlag(tempint);
      }
      else if (token == "x2ticsLocation")
      {
       buffer=getValue(infile);
        setX2ticsLocation(buffer);
      }
      else if (token == "x2ticsMirror")
      {
       buffer=getValue(infile);
        setX2ticsMirror(buffer);
      }
      else if (token == "x2ticsRotation")
      {
       buffer=getValue(infile);
        setX2ticsRotation(buffer);
      }
      else if (token == "x2ticsPositionType")
      {
       buffer=getValue(infile);
        setX2ticsPositionType(buffer);
      }
      else if (token == "x2ticsStartPos")
      {
       buffer=getValue(infile);
        setX2ticsStartPos(buffer);
      }
      else if (token == "x2ticsIncPos")
      {
       buffer=getValue(infile);
        setX2ticsIncPos(buffer);
      }
      else if (token == "x2ticsEndPos")
      {
       buffer=getValue(infile);
        setX2ticsEndPos(buffer);
      }
      else if (token == "x2ticsLabelsPos")
      {
        buffer=getValue(infile);
        setX2ticsLabelsPos(buffer);
      }
      else if (token == "y2ticsOnFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setY2ticsOnFlag(tempint);
      }
      else if (token == "y2ticsLocation")
      {
       buffer=getValue(infile);
        setY2ticsLocation(buffer);
      }
      else if (token == "y2ticsMirror")
      {
       buffer=getValue(infile);
        setY2ticsMirror(buffer);
      }
      else if (token == "y2ticsRotation")
      {
       buffer=getValue(infile);
        setY2ticsRotation(buffer);
      }
      else if (token == "y2ticsPositionType")
      {
       buffer=getValue(infile);
        setY2ticsPositionType(buffer);
      }
      else if (token == "y2ticsStartPos")
      {
       buffer=getValue(infile);
        setY2ticsStartPos(buffer);
      }
      else if (token == "y2ticsIncPos")
      {
       buffer=getValue(infile);
        setY2ticsIncPos(buffer);
      }
      else if (token == "y2ticsEndPos")
      {
       buffer=getValue(infile);
        setY2ticsEndPos(buffer);
      }
      else if (token == "y2ticsLabelsPos")
      {
       buffer=getValue(infile);
        setY2ticsLabelsPos(buffer);
      }
      else if (token == "xAxisRotation")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setRotationXAxis(tempint);
      }
      else if (token == "zAxisRotation")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setRotationZAxis(tempint);
      }
      else if (token == "rotationScale")
      {
       buffer=getValue(infile);
        setRotationScaling(buffer);
      }
      else if (token == "zAxisScale")
      {
       buffer=getValue(infile);
        setRotationZAxisScaling(buffer);
      }
      else if (token == "ticsLevel")
      {
       buffer=getValue(infile);
        setTicsLevel(buffer);
      }
      else if (token == "d3HiddenLineFlag")
      {
       buffer=getValue(infile);
       tempint=buffer.toInt(&ok,10);
       if (!ok) tempint=0;
       setd3HiddenLineFlag(tempint);
      }
      else if (token == "isoLineU")
      {
       buffer=getValue(infile);
        setIsolineU(buffer);
      }
      else if (token == "isoLineV")
      {
       buffer=getValue(infile);
        setIsolineV(buffer);
      }
      else if (token == "MultiFileNew")
      {
       buffer=getValue(infile);
        insertMultiFileNew(buffer);
      }
      else if (token == "MultiFileDataSetStart")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileDataSetStart(tempfile, buffer);
      }
      else if (token == "MultiFileDataSetEnd")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileDataSetEnd(tempfile, buffer);
      }
      else if (token == "MultiFileDataSetIncrement")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileDataSetIncrement(tempfile, buffer);
      }
      else if (token == "MultiFileSampPointInc")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSampPointInc(tempfile, buffer);
      }
      else if (token == "MultiFileSampLineInc")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSampLineInc(tempfile, buffer);
      }
      else if (token == "MultiFileSampStartPoint")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSampStartPoint(tempfile, buffer);
      }
      else if (token == "MultiFileSampStartLine")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSampStartLine(tempfile, buffer);
      }
      else if (token == "MultiFileSampEndPoint")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSampEndPoint(tempfile, buffer);
      }
      else if (token == "MultiFileSampEndLine")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSampEndLine(tempfile, buffer);
      }
      else if (token == "MultiFileSmoothType")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileSmoothType(tempfile, buffer);
      }
      else if (token == "MultiFileXColumnOption")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        insertMultiFileXColumnOption(tempfile, buffer);
      }
      else if (token == "MultiFileYColumnOption")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        insertMultiFileYColumnOption(tempfile, buffer);
      }
      else if (token == "MultiFileZColumnOption")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        insertMultiFileZColumnOption(tempfile, buffer);
      }
      else if (token == "MultiFileFormatOption")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        insertMultiFileFormatOption(tempfile, buffer);
      }
      else if (token == "MultiFileRawFormatOption")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        insertMultiFileRawFormatOption(tempfile, buffer);
      }
      else if (token == "MultiFileStyleOption")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileStyleOption(tempfile, buffer);
      }
      else if (token == "MultiFileLegendTitle")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileLegendTitle(tempfile, buffer);
      }
      else if (token == "MultiFileFilter")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileFilter(tempfile, buffer);
      }
      else if (token == "MultiFileFilterQuoteChar")
      {
       tempfile=getValue(infile);
       buffer=getValue(infile);
        setMultiFileFilterQuoteChar(tempfile, buffer);
      }
      else if (token == "MultiFuncNew")
      {
       buffer=getValue(infile);
        insertMultiFuncNew(buffer);
      }
      else if (token == "MultiFuncStyleOption")
      {
       tempfunc=getValue(infile);
       buffer=getValue(infile);
        setMultiFuncStyleOption(tempfunc, buffer);
      }
      else if (token == "MultiFuncLegendTitle")
      {
       tempfunc=getValue(infile);
       buffer=getValue(infile);
        setMultiFuncLegendTitle(tempfunc, buffer);
      }
    }
  }

file.close();

  cout << "Done." << endl;
}

QString gnuInterface::getValue(QTextStream& infile)
{
  QString buffer="";
  QString result="";
  int i=0;
  while ( !infile.atEnd() && buffer!=";")
    {
    infile >> buffer;
    if (buffer!=";")
       {
        if (i>0) result+=" ";
        result+=buffer;
        i++;
       } 
    }
  return result;
}

QString gnuInterface::getCommands()
{
  QString result="";
  QString tmp="";
  QString xrange = "";           // reset range temp variables
  QString yrange = "";
  QString zrange = "";

  QString filePlotCMD = "";      // reset plot subQStrings
  QString funcPlotCMD = "";
  QString rangePlotCMD = "";


  /* There are several things we need to do in order to create a plot.
     (1) Set x,y,z labels, output, terminal, size, title, key, etc as
         individual "set" commands that can be executed individually
     (2) Choose plot, splot, or replot command, specify x,y,z axis ranges
         as head of plot command.
     (3) Issue commands for datafiles and functions and their options as the
         tail of the command. */


  // Do task (1): setup x,y,z labels, output, terminal, size, title, key, etc

  // add x,y,z labels if given

  // setup xlabel
  QString label = "";

  if (XLabel != "")
  {
    result+="set xlabel '";
    result+=XLabel;
    result+="' ";
    // set offsets
    QString offset = "";

    // both xlabel offsets are specified
    if ((XOffset_X != "") && (XOffset_Y != ""))
    {
      offset = XOffset_X;
      offset += ",";
      offset += XOffset_Y;
    }

    // xlabel x offset only specified
    if ((XOffset_X != "") && (XOffset_Y == ""))
    {
      result+= XOffset_X;
       label = XOffset_X;
    }

    // xlabel y offset only specified
    if ((XOffset_X == "") && (XOffset_Y != ""))
    {
      result+= ",";
      result+= XOffset_Y;
    }

    result+="\n";
  }
  else
  {
    result+= "set xlabel\n";
  }

  // setup ylabel

  if (YLabel != "")
  {
    result+= "set ylabel '";
    result+=YLabel;
    result+="' ";

    // set ylabel offsets

    // both ylabel offsets are specified
    if ((YOffset_X != "") && (YOffset_Y != ""))
    {
      result+= YOffset_X;
      result+= ",";
      result+= YOffset_Y;
    }

    // ylabel x offset only specified
    if ((YOffset_X != "") && (YOffset_Y == ""))
    {
      result+= YOffset_X;
     }

    // ylabel y offset only specified
    if ((YOffset_X == "") && (YOffset_Y != ""))
    {
      result+= ",";
      result+= YOffset_Y;
    }

    result+="\n";
  }
  else
  {
    result+= "set ylabel\n";
  }

  // setup zlabel

  if (ZLabel != "")
  {
    result+= "set zlabel '";
    result+=ZLabel;
    result+="' ";
    // set zlabel offsets

    // both offsets are specified
    if ((ZOffset_X != "") && (ZOffset_Y != ""))
    {
      result+= ZOffset_X;
      result+= ",";
      result+= ZOffset_Y;
    }

    // zlabel x offset only specified
    if ((ZOffset_X != "") && (ZOffset_Y == ""))
    {
      result+= ZOffset_X;
     }

    // zlabel y offset only specified
    if ((ZOffset_X == "") && (ZOffset_Y != ""))
    {
      result+= ",";
      result+= ZOffset_Y;
    }

    result+="\n";
  }
  else
  {
    result+= "set zlabel\n";
  }

  // set output
  if (output == "")
  {
    result+= "set output\n";
  }
  else
  {
    result+= "set output '";
    result+=output;
    result+="'";
    result+="\n";
  }

  // set terminal
  if (terminal != "")
  {
    result+= "set terminal ";
    result+= terminal;

    if ((terminal == "emtex") || (terminal == "latex"))
    {

      result+= " ";
      result+= termLatexEmtexFont;
      result+= " ";
      result+= termLatexEmtexFontSize;
      result+= "\n";
    }
    else if (terminal == "pbm")
    {
      result+= " ";
      result+= termPBMfontSize;
      result+= " ";
      result+= termPBMcolormode;
      result+= "\n";
     }
    else if (terminal == "postscript")
    {
      result+= " ";
      result+=termPSmode;
      result+= " ";
      result+=termPSenhanced;
      result+= " ";
      result+=termPScolor;
      result+= " ";
      result+=termPSdashed;
      result+= " ";
      result+= "\"";
      result+=termPSfont;
      result+="\"";
      result+= " ";
      result+=termPSfontSize;
      result+= "\n";
    }
    else
    {
      result+= "\n";
    }
  }

  // set size
  result+= "set size ";
  if ((hSize != "") && (vSize != ""))
  {
    result+= hSize;
    result+= ",";
    result+= vSize;
  }
  result+= "\n";
  // setup title

  // set title
  if (title != "")
  {
    result+= "set title '";
    result+=title;
    result+="' ";
    // set title offsets

    // both title offsets are specified
    if ((titleOffset_X != "") && (titleOffset_Y != ""))
    {
      result+= titleOffset_X;
      result+= ",";
      result+= titleOffset_Y;
    }

    // title x offset only specified
    if ((titleOffset_X != "") && (titleOffset_Y == ""))
    {
      result+= titleOffset_X;
    }

    // title y offset only specified
    if ((titleOffset_X == "") && (titleOffset_Y != ""))
    {
      result+= ",";
      result+= titleOffset_Y;
    }

    result+="\n";
  }
  else
  {
    result+= "set title ''\n";
  }

  // setup legend command
  QString legend = "set ";

  legend += legendFlag; // key or nokey

  if (legendFlag != "nokey")
  {
    if (legendPositionLeft == 1)
      legend += " left ";

    if (legendPositionRight == 1)
      legend += " right ";

    if (legendPositionTop == 1)
      legend += " top ";

    if (legendPositionBottom == 1)
      legend += " bottom ";

    if (legendPositionOutside == 1)
      legend += " outside ";

    if (legendPositionBelow == 1)
      legend += " below ";

    if ((legendPositionXVal != "") && (legendPositionYVal != ""))
    {
      legend += legendPositionXVal;
      legend += ",";
      legend += legendPositionYVal;

      if (legendPositionZVal != "")
      {
        legend += ",";
        legend += legendPositionZVal;
      }

      legend += " ";
    }

    legend += legendTextJustify;
    legend += " ";

    legend += legendReverse;
    legend += " ";

    legend += "samplen ";
    legend += legendSampleLength;
    legend += " ";

    legend += "spacing ";
    legend += legendSpacing;
    legend += " ";

    if (legendWidthIncrement != "")
    {
      legend += "width ";
      legend += legendWidthIncrement;
      legend += " ";
    }

    legend += "title ";
    legend += " '";
    legend += legendTitle;
    legend += "' ";

    legend += legendBox;
    legend += " ";

    legend += legendLinetype;
  }

  legend += "\n";

  result+=legend;  // issue command

  // set xtics options
  if (xticsOnFlag == 1)
  {
    result+="set xtics ";

    result+=xticsLocation;
    result+=" ";
    result+=xticsMirror;
    result+=" ";
    result+=xticsRotation;
    result+=" ";
    if (xticsPositionType == "SIE")
    {
      if (xticsStartPos != "")
      {
        result+=xticsStartPos;
        result+= ",";
      }

      if (xticsStartPos == "")
      {
        result+= " ";
      }

      result+=xticsIncPos;
      if (xticsEndPos != "")
      {
        result+=",";
        result+=xticsEndPos;
      }
    }
    else if (xticsPositionType == "LABELS")
    {
      result+=" ";
      result+=xticsLabelsPos;
    }

    result+="\n";
  }
  else
  {
    result+="set noxtics\n";
  }


  // set ytics options
  if (yticsOnFlag == 1)
  {
    result+="set ytics ";
    result+=yticsLocation;
    result+=" ";
    result+=yticsMirror;
    result+=" ";
    result+=yticsRotation;
    result+=" ";
    if (yticsPositionType == "SIE")
    {
      if (yticsStartPos != "")
      {
        result+=yticsStartPos;
        result+= ",";
      }

      if (yticsStartPos == "")
      {
        result+= " ";
      }

      result+=yticsIncPos;
      if (yticsEndPos != "")
      {
        result+=",";
        result+=yticsEndPos;
      }
    }
    else if (yticsPositionType == "LABELS")
    {
      result+=" ";
      result+=yticsLabelsPos;
     }

    result+="\n";
    
  }
  else
  {
    result+="set noytics\n";
   }


  // set ztics options
  if (zticsOnFlag == 1)
  {
    result+="set ztics ";
    result+=zticsMirror;
    result+=" ";
    result+=zticsRotation;
    result+=" ";
    if (zticsPositionType == "SIE")
    {
      if (zticsStartPos != "")
      {
        result+=zticsStartPos;
        result+= ",";
      }

      if (zticsStartPos == "")
      {
        result+= " ";
       }

      result+=zticsIncPos;
      if (zticsEndPos != "")
      {
        result+=",";
        result+=zticsEndPos;
      }
    }
    else if (zticsPositionType == "LABELS")
    {
      result+=" ";
      result+=zticsLabelsPos;
    }

    result+="\n";
  }
  else
  {
    result+="set noztics\n";
  }


  // set x2tics options
  if (x2ticsOnFlag == 1)
  {
    result+="set x2tics ";
    result+=x2ticsLocation;
    result+=" ";
    result+=x2ticsMirror;
    result+=" ";
    result+=x2ticsRotation;
    result+=" ";
    if (x2ticsPositionType == "SIE")
    {
      if (x2ticsStartPos != "")
      {
        result+=x2ticsStartPos;
        result+= ",";
      }

      if (x2ticsStartPos == "")
      {
        result+= " ";
      }

      result+=x2ticsIncPos;
      if (x2ticsEndPos != "")
      {
        result+=",";
        result+=x2ticsEndPos;
      }
    }
    else if (x2ticsPositionType == "LABELS")
    {
      result+=" ";
      result+=x2ticsLabelsPos;
    }

    result+="\n";
  }
  else
  {
    result+="set nox2tics\n";
  }

  // set y2tics options
  if (y2ticsOnFlag == 1)
  {
    result+="set y2tics ";
    result+=y2ticsLocation;
    result+=" ";
    result+=y2ticsMirror;
    result+=" ";
    result+=y2ticsRotation;
    result+=" ";
    if (y2ticsPositionType == "SIE")
    {
      if (y2ticsStartPos != "")
      {
        result+=y2ticsStartPos;
        result+= ",";
       }

      if (y2ticsStartPos == "")
      {
        result+= " ";
      }

      result+=y2ticsIncPos;
      if (y2ticsEndPos != "")
      {
        result+=",";
        result+=y2ticsEndPos;
      }
    }
    else if (y2ticsPositionType == "LABELS")
    {
      result+=" ";
      result+=y2ticsLabelsPos;
    }

    result+="\n";
  }
  else
  {
    result+="set noy2tics\n";
  }

  // set log scale axis

  QString logScaleCMD = "set logscale ";
  QString noLogScaleCMD = "set nologscale ";

  // append axes to appropriate command

  if (logScaleXAxisFlag == 1)
    logScaleCMD += "x";
  else
    noLogScaleCMD += "x";

  if (logScaleYAxisFlag == 1)
    logScaleCMD += "y";
  else
    noLogScaleCMD += "y";

  if (logScaleZAxisFlag == 1)
    logScaleCMD += "z";
  else
    noLogScaleCMD += "z";

  // if any axes are checked log scale, print and append base to command.

  if ((logScaleXAxisFlag == 1) || (logScaleYAxisFlag == 1) ||
      (logScaleZAxisFlag == 1) || (logScaleX2AxisFlag == 1) ||
      (logScaleY2AxisFlag == 1))
  {
    result+= logScaleCMD;
    result+=" ";
    tmp="";
    tmp.sprintf("%d",logScaleBase);
    result+=tmp;
    result+= "\n";
    
  }

  // only print nologscale command if all axes are not set to log
  if ((logScaleXAxisFlag != 1) || (logScaleYAxisFlag != 1) ||
      (logScaleZAxisFlag != 1) || (logScaleX2AxisFlag != 1) ||
      (logScaleY2AxisFlag != 1))
  {
    result+= noLogScaleCMD;
    result+= "\n";
  }

  // print x2 axis seperately
  if (logScaleX2AxisFlag == 1)
  {
    result+="set logscale x2";
    tmp="";
    tmp.sprintf("%d",logScaleBase);
    result+=tmp;
    result+= "\n";
  }
  else
  {
    result+="set nologscale x2\n";
  }

  // print y2 axis seperately
  if (logScaleY2AxisFlag == 1)
  {
    result+="set logscale y2";
    tmp="";
    tmp.sprintf("%d",logScaleBase);
    result+=tmp;
    result+= "\n";
  }
  else
  {
    result+="set nologscale y2\n";
  }

  // set bar options
  result+="set bar ";
  result+= barSize;
  result+="\n";
  // set box width options
  result+="set boxwidth ";
  result+= boxWidth;
  result+= "\n";
  // set 3d rotation options (view command)
  result+="set view ";
  tmp="";
  tmp.sprintf("%d",xAxisRotation);
  result+=tmp;
  result+=", ";
  tmp="";
  tmp.sprintf("%d",zAxisRotation);
  result+=tmp;
  result+=", ";
  result+=rotationScale;
  result+=", ";
  result+=zAxisScale;
  result+="\n";
  // tics level for 3d plots
  result+="set ticslevel ";
  result+=ticsLevel;
  result+="\n";
  // hidden line removal for 3D plots
  if (d3HiddenLineFlag == 1)
  {
    result+= "set hidden3d\n";
  }
  else if (d3HiddenLineFlag == 0)
  {
    result+= "set nohidden3d\n";
  }

  // isolines for 3D plots
  result+= "set isosamples ";
  result+= isoLineU;
  result+= ", ";
  result+= isoLineV;
  result+= "\n";
  // setup plotting range QString that comes after plot/splot

  // check to see if any range info is given
  if (replotFlag == 0)
  {
    if ((XVariableName != "") || (XRangeStart != "") || (XRangeEnd != ""))
    {
      // make sure a variable is not given without ranges
      if ((XVariableName != "") && (XRangeStart == "") && (XRangeEnd == ""))
      {
        cout << "X var specified, but no range given.\n";
      }
      else
      {
        xrange += "[";       // we have an x variable or range to specify
        xrange += XVariableName;    // insert x range

        if (XVariableName != "")
          xrange += "=";

        xrange += XRangeStart;
        xrange += ":";
        xrange += XRangeEnd;
        xrange += "]";
      }
    }

    // check to see if any range info is given
    if ((YVariableName != "") || (YRangeStart != "") || (YRangeEnd != ""))
    {
      // make sure a variable is not given without ranges
      if ((YVariableName != "") && (YRangeStart == "") && (YRangeEnd == ""))
      {
        cout << "Y var specified, but no range given. \n";
      }
      else
      {
        yrange += "[";       // we have a y variable or range to specify
        yrange += YVariableName;    // insert y range

        if (YVariableName != "")
          yrange += "=";

        yrange += YRangeStart;
        yrange += ":";
        yrange += YRangeEnd;
        yrange += "]";
      }
    }

    // check to see if any range info is given
    if ((ZRangeStart != "") || (ZRangeEnd != ""))
    {
      zrange += "[";       // we have a z range to specify
      zrange += ZRangeStart;
      zrange += ":";
      zrange += ZRangeEnd;
      zrange += "]";
    }

    rangePlotCMD += xrange;       // setup range QString

    if (xrange != "")
      rangePlotCMD += " ";

    rangePlotCMD += yrange;

    if (yrange != "")
      rangePlotCMD += " ";

    rangePlotCMD += zrange;
  }

  // As a convention, plot files first then functions

    // start file plotting command generation code
    // filename or multiple filename option is checked

  if ((plotFileFlag == 1) || (plotMultipleFileFlag == 1))
  {
    // make sure single file is not "none" or null and is checked
    if ((plotFileOb->getFilename() != "") &&
        (plotFileOb->getFilename() != "none") && (plotFileFlag == 1))
    {
      filePlotCMD += plotFileOb->getPlotCmd(); // get command QString
    }
  }

  // now plot multiple files
  QString tempFile;

  // get first file plotting command in list
  tempFile = multiFile->getMultiFileFirstPlotCmd();

  // make sure flag is set
  if (plotMultipleFileFlag == 1)
  {
    // if already plotted a single file insert comma
    if ((plotFileFlag == 1) && (tempFile != "END"))
    {
      filePlotCMD += ", "; // insert comma
    }

    if (tempFile != "END")
      filePlotCMD += tempFile;

      // now iterate over the remaining files in list inserting commas before
    tempFile = multiFile->getMultiFileNextPlotCmd();

    while (tempFile != "END")
    {
      filePlotCMD += ",";
      filePlotCMD += tempFile;
      tempFile = multiFile->getMultiFileNextPlotCmd();
    }
  }

  // start function plotting command generation code

  if (plotFuncFlag == 1) // function option is checked
  {
    if (plotFunctionOb->getFunction() != "")
    {
      funcPlotCMD += plotFunctionOb->getPlotCmd(); // get command QString
    }
  }

  // now plot multiple functions
  QString tempFunc;

  // make sure flag is set
  if (plotMultipleFuncFlag == 1)
  {
    // now get first function plotting command in list
    tempFunc = multiFunc->getMultiFuncFirstPlotCmd();

      // if already plotted a single function insert comma
    if (plotFuncFlag == 1)
    {
      if (tempFunc != "END")
        funcPlotCMD += ", "; // insert comma
    }

    if (tempFunc != "END")
      funcPlotCMD += tempFunc;

      // now iterate over the remaining functions in list
      // inserting commas before each
    tempFunc = multiFunc->getMultiFuncNextPlotCmd();

    while (tempFunc != "END")
    {
      funcPlotCMD += ", ";
      funcPlotCMD += tempFunc;
      tempFunc = multiFunc->getMultiFuncNextPlotCmd();
    }
  }


  // build plotting command based on what is selected: file(s)/function(s)/both
  // ------------------- issue file plotting command if present ----------
  if (filePlotCMD != "")
  {
    if (replotFlag == 1) // check for replot command
    {
      result+="replot";
    }
    else
    {
      result+=filePlotType; // plot/splot
    }

    // insert range info if present
    if (rangePlotCMD != "")
    {
      result+=" ";
      result+=rangePlotCMD;
    }

    // add a space before file plotting command
    result+=" ";
    result+=filePlotCMD;
    
  }

  // add newline to end command if no function is being plotted
  if (funcPlotCMD == "")
  {
    result+="\n";
  }

  // ---------- issue function plotting command if present ---------------

  if (funcPlotCMD != "")
  {
    // if not file command is present, insert plot/splot/replot command
    if (filePlotCMD == "")
    {
      if (replotFlag == 1) // check for replot command
      {
        result+="replot";
      }
      else
      {
        result+=funcPlotType; // plot/splot
      }

      // insert range info if present
      if (rangePlotCMD != "")
      {
        result+=" ";
        result+=rangePlotCMD;
      }

      // add a space before function plotting command
      result+=" ";

      // acutally plot functions
      result+=funcPlotCMD;
      result+="\n";
      
    }

    // if a file command is present, insert comma first before function command
    if (filePlotCMD != "")
    {
      result+=", ";
      result+=funcPlotCMD;
      result+="\n";
     
    }
  }
  return result;
}
