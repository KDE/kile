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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   ------------------------------------------------------------------------*/

#ifndef gnuPlotFile_included
#define gnuPlotFile_included

using namespace std;

#include <qstring.h>

class gnuPlotFile
{
public:
  gnuPlotFile();
  /* Description:
       Constructor function */

  QString getPlotCmd();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: plotting command for the file
     Description:
       Builds a QString to plot the file */

  void setFilename(QString file);
  /* Incoming arguments:
       QString file: Filename to plot
     Outgoing arguments:
       none
     Description:
       Sets the filename to plot */

  QString getFilename();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current filename
     Description:
       Gets the current filename */

  void setFileStyleType(QString type);
  /* Incoming arguments:
       QString type: style of plotting. With Gnuplot 3.5 the following types
       are supported: points, lines, linespoints, impulses, dots, steps,
       errorbars, and boxes. Others can be specified if Gnuplot will
       understand it.
     Outgoing arguments:
       none
     Description:
       Sets the style of plotting the datafile */

  QString getFileStyleType();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current style
     Description:
       Gets the current style */

  void setFileDataSetStart(QString start);
  QString getFileDataSetStart();
  void setFileDataSetEnd(QString end);
  QString getFileDataSetEnd();
  void setFileDataSetIncrement(QString inc);
  QString getFileDataSetIncrement();
  void setFileSampPointInc(QString pinc);
  QString getFileSampPointInc();
  void setFileSampLineInc(QString linc);
  QString getFileSampLineInc();
  void setFileSampStartPoint(QString startp);
  QString getFileSampStartPoint();
  void setFileSampStartLine(QString startl);
  QString getFileSampStartLine();
  void setFileSampEndPoint(QString endp);
  QString getFileSampEndPoint();
  void setFileSampEndLine(QString endl);
  QString getFileSampEndLine();
  void setFileSmoothType(QString type);
  QString getFileSmoothType();

  void setFileXColumn(QString column);
  /* Incoming arguments:
       QString column: column to use for x variable
     Outgoing arguments:
       none
     Description:
       Sets the column to use for the x variable */

  QString getFileXColumn();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: x column that is currently specified
     Description:
       Gets the column to use for the x variable */

  void setFileYColumn(QString column);
  /* Incoming arguments:
       QString column: column to use for y variable
     Outgoing arguments:
       none
     Description:
       Sets the column to use for the y variable */

  QString getFileYColumn();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: y column that is currently specified
     Description:
       Gets the column to use for the y variable */

  void setFileZColumn(QString column);
  /* Incoming arguments:
       QString column: column to use for z variable
     Outgoing arguments:
       none
     Description:
       Sets the column to use for the z variable */

  QString getFileZColumn();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: z column that is currently specified
     Description:
       Gets the column to use for the z variable */

  void setFileFormatString(QString format);
  /* Incoming arguments:
       QString format: scanf format QString including quotes
     Outgoing arguments:
       none
     Description:
       Sets the scanf format QString to read the data file with */

  QString getFileFormatString();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current format QString
     Description:
       Gets the current scanf format QString */

  void setRawFileFormatString(QString format);
  /* Incoming arguments:
       QString format: format for plotting file
     Outgoing arguments:
       none
     Description:
       Sets the format QString to whatever is specified. If this is set, it
       will be used instead of building one from the columns and format
       QString specifications. This allows complex formats to be specified. */

  QString getRawFileFormatString();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current raw file format
     Description:
       Gets the current format QString */

  void setLegendTitle(QString title);
  /* Incoming arguments:
       title: title for legend
              if = notitle, notitle command is issued
              if = default, title option is not given,
                   gnuplot will use default
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend */

  QString getLegendTitle();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: title for current file
     Description:
       Gets title to be used in legend for the file */

  void setFileFilter(QString thefilter);
  /* Incoming arguments:
       QString: filter command
     Outgoing arguments:
       none
     Description:
       Sets filter command for the file */

  QString getFileFilter();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: filter command
     Description:
       Gets filter command for the file */

  void setFileFilterQuoteChar(QString quote);
  /* Incoming arguments:
       QString quote: quoting char
     Outgoing arguments:
       none
     Description:
       Sets quoting character for the single file filter cmd */

  QString getFileFilterQuoteChar();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: quoting character
     Description:
       Gets quoting character for the single file filter cmd */

private:

  QString filename;
  QString styleType;
  QString dataSetStart;
  QString dataSetEnd;
  QString dataSetInc;
  QString samplePointInc;
  QString sampleLineInc;
  QString sampleStartPoint;
  QString sampleStartLine;
  QString sampleEndPoint;
  QString sampleEndLine;
  QString fileSmoothType;
  QString xColumn;
  QString yColumn;
  QString zColumn;
  QString formatString;
  QString rawFormatString;
  QString legendTitle;
  QString filter;
  QString filterQuoteChar;

};



#endif // gnuPlotFile_included
