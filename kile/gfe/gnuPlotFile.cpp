/* -------------------------- gnuPlotFile class --------------------------
   
   This is a class to create an object to plot datafiles. It contains all
   variables necessary for plotting files and knows how to issue the correct
   command to gnuPlot to plot the file with its options. 

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

#include "gnuPlotFile.h"

gnuPlotFile::gnuPlotFile()
{
  filename = "";
  styleType = "points";
  dataSetStart = "";
  dataSetEnd = "";
  dataSetInc = "";
  samplePointInc = "";
  sampleLineInc = "";
  sampleStartPoint = "";
  sampleStartLine = "";
  sampleEndPoint = "";
  sampleEndLine = "";
  fileSmoothType = "none";  
  xColumn = "";
  yColumn = "";
  zColumn = "";
  formatString = "";
  rawFormatString = "";
  styleType = "points";
  legendTitle = "default";
  filter = "";
  filterQuoteChar = "double";
}

QString gnuPlotFile::getPlotCmd()
{
  QString plotQString;

  if (filter == "")
  {
    // insert filename with quotes
    plotQString += " '";
    plotQString += filename;
    plotQString += "' ";
  }
  else
  {
    if (filterQuoteChar == "single")
      plotQString += "'";
    else if (filterQuoteChar == "double")
      plotQString += '"';

    plotQString += "< ";
    plotQString += filter;

    if (filterQuoteChar == "single")
      plotQString += "'";
    else if (filterQuoteChar == "double")
      plotQString += '"';
  }

  // setup datafile modifiers

  // insert "index" command
  if ((dataSetStart != "") && (dataSetEnd != "") && (dataSetInc != ""))
  {
    plotQString += " index ";
    plotQString += dataSetStart;
    plotQString += ":";
    plotQString += dataSetEnd;
    plotQString += ":";
    plotQString += dataSetInc;
  }

  if ((dataSetStart != "") && (dataSetEnd != "") && (dataSetInc == ""))
  {
    plotQString += " index ";
    plotQString += dataSetStart;
    plotQString += ":";
    plotQString += dataSetEnd;
  }

  if ((dataSetStart != "") && (dataSetEnd == "") && (dataSetInc == ""))
  {
    plotQString += " index ";
    plotQString += dataSetStart;
  }

  // insert "every" command
  if ((samplePointInc != "") || (sampleLineInc != "") ||
      (sampleStartPoint != "") || (sampleStartLine != "") ||
      (sampleEndPoint != "") || (sampleEndLine != ""))
  {
    if (sampleEndLine != "")
    {
      plotQString += " every ";
      plotQString += samplePointInc;
      plotQString += ":";
      plotQString += sampleLineInc;
      plotQString += ":";
      plotQString += sampleStartPoint;
      plotQString += ":";
      plotQString += sampleStartLine;
      plotQString += ":";
      plotQString += sampleEndPoint;
      plotQString += ":";
      plotQString += sampleEndLine;
    }
    else if (sampleEndPoint != "")
    {
      plotQString += " every ";
      plotQString += samplePointInc;
      plotQString += ":";
      plotQString += sampleLineInc;
      plotQString += ":";
      plotQString += sampleStartPoint;
      plotQString += ":";
      plotQString += sampleStartLine;
      plotQString += ":";
      plotQString += sampleEndPoint;
    }
    else if (sampleStartLine != "")
    {
      plotQString += " every ";
      plotQString += samplePointInc;
      plotQString += ":";
      plotQString += sampleLineInc;
      plotQString += ":";
      plotQString += sampleStartPoint;
      plotQString += ":";
      plotQString += sampleStartLine;
    }
    else if (sampleStartPoint != "")
    {
      plotQString += " every ";
      plotQString += samplePointInc;
      plotQString += ":";
      plotQString += sampleLineInc;
      plotQString += ":";
      plotQString += sampleStartPoint;
    }
    else if (sampleLineInc != "")
    {
      plotQString += " every ";
      plotQString += samplePointInc;
      plotQString += ":";
      plotQString += sampleLineInc;
    }
    else if (samplePointInc != "")
    {
      plotQString += " every ";
      plotQString += samplePointInc;
    }
  }

  // insert using command if columns or format QString is specified
  // and a raw format QString is not specified
  if (rawFormatString == "")
  {
    if ((xColumn != "") || (yColumn != "") || (zColumn != "") ||
        (formatString != ""))
    {
      plotQString += " using ";
      plotQString += xColumn;
      plotQString += ":";
      plotQString += yColumn;

      if (zColumn != "")
      {
        plotQString += ":";
        plotQString += zColumn;
      }

      plotQString += " ";
      plotQString += formatString;
    }
  }
  else
  {
    plotQString += " using ";
    plotQString += rawFormatString;
  }

  // insert "smooth" command
  if (fileSmoothType != "none")
  {
    plotQString += " smooth ";
    plotQString += fileSmoothType;
  }

  // insert title for legend
  if (legendTitle == "notitle")
    plotQString += " notitle";

  if ((legendTitle != "default") && (legendTitle != "notitle"))
  {
    plotQString += " title ";
    plotQString += '"';
    plotQString += legendTitle;
    plotQString += '"';
  }

  // insert plotting style
  plotQString += " with ";
  plotQString += styleType;

  return plotQString;
}

void gnuPlotFile::setFilename(QString file)
{
  filename = file;
}

QString gnuPlotFile::getFilename()
{
  return filename;
}


void gnuPlotFile::setFileStyleType(QString type)
{
  styleType = type;
}

QString gnuPlotFile::getFileStyleType()
{
  return styleType;
}

void gnuPlotFile::setFileDataSetStart(QString start)
{
  dataSetStart = start;
}

QString gnuPlotFile::getFileDataSetStart()
{
  return dataSetStart;
}

void gnuPlotFile::setFileDataSetEnd(QString end)
{
  dataSetEnd = end;
}

QString gnuPlotFile::getFileDataSetEnd()
{
  return dataSetEnd;
}

void gnuPlotFile::setFileDataSetIncrement(QString inc)
{
  dataSetInc = inc;
}

QString gnuPlotFile::getFileDataSetIncrement()
{
  return dataSetInc;
}

void gnuPlotFile::setFileSampPointInc(QString pinc)
{
  samplePointInc = pinc;
}

QString gnuPlotFile::getFileSampPointInc()
{
  return samplePointInc;
}

void gnuPlotFile::setFileSampLineInc(QString linc)
{
  sampleLineInc = linc;
}

QString gnuPlotFile::getFileSampLineInc()
{
  return sampleLineInc;
}

void gnuPlotFile::setFileSampStartPoint(QString startp)
{
  sampleStartPoint = startp;
}

QString gnuPlotFile::getFileSampStartPoint()
{
  return sampleStartPoint;
}

void gnuPlotFile::setFileSampStartLine(QString startl)
{
  sampleStartLine = startl;
}

QString gnuPlotFile::getFileSampStartLine()
{
  return sampleStartLine;
}

void gnuPlotFile::setFileSampEndPoint(QString endp)
{
  sampleEndPoint = endp;
}

QString gnuPlotFile::getFileSampEndPoint()
{
  return sampleEndPoint;
}

void gnuPlotFile::setFileSampEndLine(QString endl)
{
  sampleEndLine = endl;
}

QString gnuPlotFile::getFileSampEndLine()
{
  return sampleEndLine;
}

void gnuPlotFile::setFileSmoothType(QString type)
{
  fileSmoothType = type;
}

QString gnuPlotFile::getFileSmoothType()
{
  return fileSmoothType;
}

void gnuPlotFile::setFileXColumn(QString column)
{
  xColumn = column;
}

QString gnuPlotFile::getFileXColumn()
{
  return xColumn;
}

void gnuPlotFile::setFileYColumn(QString column)
{
  yColumn = column;
}

QString gnuPlotFile::getFileYColumn()
{
  return yColumn;
}

void gnuPlotFile::setFileZColumn(QString column)
{
  zColumn = column;
}

QString gnuPlotFile::getFileZColumn()
{
  return zColumn;
}

void gnuPlotFile::setFileFormatString(QString format)
{
  formatString = format;
}

QString gnuPlotFile::getFileFormatString()
{
  return formatString;
}

void gnuPlotFile::setRawFileFormatString(QString format)
{
  rawFormatString = format;
}

QString gnuPlotFile::getRawFileFormatString()
{
  return rawFormatString;
}

void gnuPlotFile::setLegendTitle(QString title)
{
  legendTitle = title;
}

QString gnuPlotFile::getLegendTitle()
{
  return legendTitle;
}

void gnuPlotFile::setFileFilter(QString thefilter)
{
  filter = thefilter;
}

QString gnuPlotFile::getFileFilter()
{
  return filter;
}

void gnuPlotFile::setFileFilterQuoteChar(QString quote)
{
  filterQuoteChar = quote;
}

QString gnuPlotFile::getFileFilterQuoteChar()
{
  return filterQuoteChar;
}
