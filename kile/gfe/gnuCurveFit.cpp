/* -------------------------- gnuCurveFit class --------------------------
   
   This is a class to create an object to handle curve fitting. 

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

#include "gnuCurveFit.h"

gnuCurveFit::gnuCurveFit()
{
  functionName = "";
  functionValue = "";
  dataFile = "";
  varXRangeName = "";
  varXRangeMin = "";
  varXRangeMax = "";
  varYRangeName = "";
  varYRangeMin = "";
  varYRangeMax = "";
  paramFile = "";
  paramFileFlag = 0;
  paramCSLFlag = 1;
  paramCSL = "";
  fitLimit = "";
  fitMaxIter = "";
  dataSetStart = "";
  dataSetEnd = "";
  dataSetInc = "";
  pointInc = "";
  lineInc = "";
  startPoint = "";
  startLine = "";
  endPoint = "";
  endLine = "";
  xColumn = "";
  yColumn = "";
  zColumn = "";
  format = "";
  rawFormat = "";
}

QString gnuCurveFit::getFitCmd()
{
  QString fitcmd;

  // define function
  fitcmd += functionName;
  fitcmd += " = ";
  fitcmd += functionValue;
  fitcmd += "\n";
  
  // define fitting variables

  // ---- fit limit
  fitcmd += "FIT_LIMIT = ";
  
  if (fitLimit != "")
    fitcmd += fitLimit;
  else
    fitcmd += "1e-5";

  
  fitcmd += "\n";
  
  // ---- fit maxiter
  fitcmd += "FIT_MAXITER = ";
  
  if (fitMaxIter != "")
    fitcmd += fitMaxIter;
  else
    fitcmd += "0";
  
  fitcmd += "\n";
  
  // define start of command
  fitcmd += "fit ";
  
  // insert x range

  if ((varXRangeName != "") || (varXRangeMin != "") || (varXRangeMax != ""))
  {
    fitcmd += " [";

    if (varXRangeName != "")
    {
      fitcmd += varXRangeName;
      fitcmd += "=";
    }
    
    if ((varXRangeMin != "") || (varXRangeMax != ""))
    {
      fitcmd += varXRangeMin;
      fitcmd += ":";
      fitcmd += varXRangeMax;
    }

    fitcmd += "]";
  }

  // insert Y range
  if ((varYRangeName != "") || (varYRangeMin != "") || (varYRangeMax != ""))
  {
    fitcmd += " [";
    
    if (varYRangeName != "")
    {
      fitcmd += varYRangeName;
      fitcmd += "=";
    }
    
    if ((varYRangeMin != "") || (varYRangeMax != ""))
    {
      fitcmd += varYRangeMin;
      fitcmd += ":";
      fitcmd += varYRangeMax;
    }

    fitcmd += "]";
  }

  // insert function 
  fitcmd += " ";
  fitcmd += functionName;

  // define datafile
  fitcmd += " '";
  fitcmd += dataFile;
  fitcmd += "' ";
  
  // define datafile modifiers
  // ---- insert "index" command
  if ((dataSetStart != "") && (dataSetEnd != "") && (dataSetInc != ""))
  {
    fitcmd += " index ";
    fitcmd += dataSetStart;
    fitcmd += ":";
    fitcmd += dataSetEnd;
    fitcmd += ":";
    fitcmd += dataSetInc;
  }
  
  if ((dataSetStart != "") && (dataSetEnd != "") && (dataSetInc == ""))
  {
    fitcmd += " index ";
    fitcmd += dataSetStart;
    fitcmd += ":";
    fitcmd += dataSetEnd;
  }
  
  if ((dataSetStart != "") && (dataSetEnd == "") && (dataSetInc == ""))
  {
    fitcmd += " index ";
    fitcmd += dataSetStart;
  }
  
  // insert "every" command
  if ((pointInc != "") || (lineInc != "") || 
      (startPoint != "") || (startLine != "") || 
      (endPoint != "") || (endLine != ""))
  {
    if (endLine != "")
    {
      fitcmd += " every ";
      fitcmd += pointInc;
      fitcmd += ":";
      fitcmd += lineInc;
      fitcmd += ":";
      fitcmd += startPoint;
      fitcmd += ":";
      fitcmd += startLine;
      fitcmd += ":";
      fitcmd += endPoint;
      fitcmd += ":";
      fitcmd += endLine;
    }
    else if (endPoint != "")
    {
      fitcmd += " every ";
      fitcmd += pointInc;
      fitcmd += ":";
      fitcmd += lineInc;
      fitcmd += ":";
      fitcmd += startPoint;
      fitcmd += ":";
      fitcmd += startLine;
      fitcmd += ":";
      fitcmd += endPoint;
    }
    else if (startLine != "")
    {
      fitcmd += " every ";
      fitcmd += pointInc;
      fitcmd += ":";
      fitcmd += lineInc;
      fitcmd += ":";
      fitcmd += startPoint;
      fitcmd += ":";
      fitcmd += startLine;
    }
    else if (startPoint != "")
    {
      fitcmd += " every ";
      fitcmd += pointInc;
      fitcmd += ":";
      fitcmd += lineInc;
      fitcmd += ":";
      fitcmd += startPoint;
    }
    else if (lineInc != "")
    {
      fitcmd += " every ";
      fitcmd += pointInc;
      fitcmd += ":";
      fitcmd += lineInc;
    }
    else if (pointInc != "")
    {
      fitcmd += " every ";
      fitcmd += pointInc;
    }
  }
  
  // ---- insert using command if columns or format QString is specified 
  // ---- and a raw format QString is not specified
  if (rawFormat == "")
  {      
    if ((xColumn != "") || (yColumn != "") || (zColumn != "") || 
        (format != ""))
    {
      fitcmd += " using ";
      fitcmd += xColumn;
      fitcmd += ":";
      fitcmd += yColumn;
      
      if (zColumn != "")
      {
        fitcmd += ":";
        fitcmd += zColumn;
      }
      
      fitcmd += " ";
      fitcmd += format;
    }      
  }
  else 
  {
    fitcmd += " using ";
    fitcmd += rawFormat;
  }

  // insert via parameters
  fitcmd += " via ";

  if (paramFileFlag == 1)
  {
    fitcmd += "'";
    fitcmd += paramFile;
    fitcmd += "'";
  }
  else if (paramCSLFlag == 1)
  {
    fitcmd += paramCSL;
  }

  return fitcmd;
}

void gnuCurveFit::setFunctionName(QString name)
{
  functionName = name;
}

QString gnuCurveFit::getFunctionName()
{
  return functionName;
}

void gnuCurveFit::setFunctionValue(QString function)
{
  functionValue = function;
}

QString gnuCurveFit::getFunctionValue()
{
  return functionValue;
}

void gnuCurveFit::setDataFile(QString file)
{
  dataFile = file;
}

QString gnuCurveFit::getDataFile()
{
  return dataFile;
}

void gnuCurveFit::setVarXRangeName(QString range)
{
  varXRangeName = range;
}

QString gnuCurveFit::getVarXRangeName()
{
  return varXRangeName;
}

void gnuCurveFit::setVarXRangeMin(QString min)
{
  varXRangeMin = min;
}

QString gnuCurveFit::getVarXRangeMin()
{
  return varXRangeMin;
}

void gnuCurveFit::setVarXRangeMax(QString max)
{
  varXRangeMax = max;
}

QString gnuCurveFit::getVarXRangeMax()
{
  return varXRangeMax;
}

void gnuCurveFit::setVarYRangeName(QString range)
{
  varYRangeName = range;
}

QString gnuCurveFit::getVarYRangeName()
{
  return varYRangeName;
}

void gnuCurveFit::setVarYRangeMin(QString min)
{
  varYRangeMin = min;
}

QString gnuCurveFit::getVarYRangeMin()
{
  return varYRangeMin;
}

void gnuCurveFit::setVarYRangeMax(QString max)
{
  varYRangeMax = max;
}

QString gnuCurveFit::getVarYRangeMax()
{
  return varYRangeMax;
}

void gnuCurveFit::setParamFile(QString file)
{
  paramFile = file;
}

QString gnuCurveFit::getParamFile()
{
  return paramFile;
}

void gnuCurveFit::setParamFileFlag(int flag)
{
  paramFileFlag = flag;
}

int gnuCurveFit::getParamFileFlag()
{
  return paramFileFlag;
}

void gnuCurveFit::setParamCSLFlag(int flag)
{
  paramCSLFlag = flag;
}

int gnuCurveFit::getParamCSLFlag()
{
  return paramCSLFlag;
}

void gnuCurveFit::setParamCSL(QString list)
{
  paramCSL = list;
}

QString gnuCurveFit::getParamCSL()
{
  return paramCSL;
}

void gnuCurveFit::setFitLimit(QString limit)
{
  fitLimit = limit;
}

QString gnuCurveFit::getFitLimit()
{
  return fitLimit;
}

void gnuCurveFit::setFitMaxIter(QString iter)
{
  fitMaxIter = iter;
}

QString gnuCurveFit::getFitMaxIter()
{
  return fitMaxIter;
}

void gnuCurveFit::setDataSetStart(QString start)
{
  dataSetStart = start;
}

QString gnuCurveFit::getDataSetStart()
{
  return dataSetStart;
}

void gnuCurveFit::setDataSetEnd(QString end)
{
  dataSetEnd = end;
}

QString gnuCurveFit::getDataSetEnd()
{
  return dataSetEnd;
}

void gnuCurveFit::setDataSetInc(QString inc)
{
  dataSetInc = inc;
}

QString gnuCurveFit::getDataSetInc()
{
  return dataSetInc;
}

void gnuCurveFit::setPointInc(QString inc)
{
  pointInc = inc;
}

QString gnuCurveFit::getPointInc()
{
  return pointInc;
}

void gnuCurveFit::setLineInc(QString inc)
{
  lineInc = inc;
}

QString gnuCurveFit::getLineInc()
{
  return lineInc;
}

void gnuCurveFit::setStartPoint(QString start)
{
  startPoint = start;
}

QString gnuCurveFit::getStartPoint()
{
  return startPoint;
}

void gnuCurveFit::setStartLine(QString start)
{
  startLine = start;
}

QString gnuCurveFit::getStartLine()
{
  return startLine;
}

void gnuCurveFit::setEndPoint(QString end)
{
  endPoint = end;
}

QString gnuCurveFit::getEndPoint()
{
  return endPoint;
}

void gnuCurveFit::setEndLine(QString end)
{
  endLine = end;
}

QString gnuCurveFit::getEndLine()
{
  return endLine;
}

void gnuCurveFit::setXColumn(QString col)
{
  xColumn = col;
}

QString gnuCurveFit::getXColumn()
{
  return xColumn;
}

void gnuCurveFit::setYColumn(QString col)
{
  yColumn = col;
}

QString gnuCurveFit::getYColumn()
{
  return yColumn;
}

void gnuCurveFit::setZColumn(QString col)
{
  zColumn = col;
}

QString gnuCurveFit::getZColumn()
{
  return zColumn;
}

void gnuCurveFit::setFormat(QString informat)
{
  format = informat;
}

QString gnuCurveFit::getFormat()
{
  return format;
}

void gnuCurveFit::setRawFormat(QString format)
{
  rawFormat = format;
}

QString gnuCurveFit::getRawFormat()
{
  return rawFormat;
}
