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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   ------------------------------------------------------------------------*/
using namespace std;

#include <qstring.h>

#ifndef gnuCurveFit_included
#define gnuCurveFit_included

class gnuCurveFit
{
public:
  gnuCurveFit();
  /* Description:
       Constructor function */

  QString getFitCmd();
  /* Description:
       Gets curve fitting command */

  void setFunctionName(QString name);
  /* Description:
       Sets function name (left side of equal sign) to fit to */

  QString getFunctionName();
  /* Description:
       Gets function name to fit to */

  void setFunctionValue(QString function);
  /* Description:
       Sets function value (right side of equal sign) to fit to */

  QString getFunctionValue();
  /* Description:
       Gets function value (right side of equal sign) to fit to  */

  void setDataFile(QString file);
  /* Description:
       Sets data file for plotting */

  QString getDataFile();
  /* Description:
       Gets data file for plotting  */

  void setVarXRangeName(QString range);
  /* Description:
       Sets x variable name for xrange */

  QString getVarXRangeName();
  /* Description:
       Gets x variable name for xrange  */

  void setVarXRangeMin(QString min);
  /* Description:
       Sets x variable min value */

  QString getVarXRangeMin();
  /* Description:
       Gets x variable min value */

  void setVarXRangeMax(QString max);
  /* Description:
       Sets x variable max value */

  QString getVarXRangeMax();
  /* Description:
       Gets x variable max value */

  void setVarYRangeName(QString range);
  /* Description:
       Sets y variable name for yrange */

  QString getVarYRangeName();
  /* Description:
       Gets y variable name for yrange */

  void setVarYRangeMin(QString min);
  /* Description:
       Sets y variable min value */

  QString getVarYRangeMin();
  /* Description:
       Gets y variable min value */

  void setVarYRangeMax(QString max);
  /* Description:
       Sets y variable max value */

  QString getVarYRangeMax();
  /* Description:
       Gets y variable max value */

  void setParamFile(QString file);
  /* Description:
       Sets parameter file for plotting parameters */

  QString getParamFile();
  /* Description:
       Gets parameter file for plotting parameters */

  void setParamFileFlag(int flag);
  /* Description:
       Sets flag for selecting parameter file vs comma seperated
       value parameters */

  int getParamFileFlag();
  /* Description:
       Gets flag for selecting parameter file vs comma seperated
       value parameters */

  void setParamCSLFlag(int flag);
  /* Description:
       Sets flag for selecting parameter file vs comma seperated
       value parameters */

  int getParamCSLFlag();
  /* Description:
       Gets flag for selecting parameter file vs comma seperated
       value parameters */

  void setParamCSL(QString list);
  /* Description:
       Sets comma seperated list of plotting parameters */

  QString getParamCSL();
  /* Description:
       Gets comma seperated list of plotting parameters */

  void setFitLimit(QString limit);
  /* Description:
       Sets fit limit value for convergence */

  QString getFitLimit();
  /* Description:
       Gets fit limit value for convergence */

  void setFitMaxIter(QString iter);
  /* Description:
       Sets maximum number of iterations for non-convergence case */

  QString getFitMaxIter();
  /* Description:
       Gets maximum number of iterations for non-convergence case */

  void setDataSetStart(QString start);
  /* Description:
       Sets starting data set of the data file modifiers */

  QString getDataSetStart();
  /* Description:
       Gets starting data set of the data file modifiers */

  void setDataSetEnd(QString end);
  /* Description:
       Sets ending data set of the data file modifiers */

  QString getDataSetEnd();
  /* Description:
       Gets ending data set of the data file modifiers */

  void setDataSetInc(QString inc);
  /* Description:
       Sets data set increment of the data file modifiers */

  QString getDataSetInc();
  /* Description:
       Gets data set increment of the data file modifiers */

  void setPointInc(QString inc);
  /* Description:
       Sets the data point increment of the data file modifiers */

  QString getPointInc();
  /* Description:
       Gets the data point increment of the data file modifiers */

  void setLineInc(QString inc);
  /* Description:
       Sets the increment value for lines in data file (modifier) */

  QString getLineInc();
  /* Description:
       Gets the increment value for lines in data file (modifier) */

  void setStartPoint(QString start);
  /* Description:
       Sets the starting point in data file (modifier) */

  QString getStartPoint();
  /* Description:
       Gets the starting point in data file (modifier) */

  void setStartLine(QString start);
  /* Description:
       Sets the starting line in data file (modifier) */

  QString getStartLine();
  /* Description:
       Gets the starting line in data file (modifier) */

  void setEndPoint(QString end);
  /* Description:
       Sets the end point in data file (modifier) */

  QString getEndPoint();
  /* Description:
       Gets the end point in data file (modifier) */

  void setEndLine(QString end);
  /* Description:
       Sets the ending line in data file (modifier) */

  QString getEndLine();
  /* Description:
       Gets the ending line in data file (modifier) */

  void setXColumn(QString col);
  /* Description:
       Sets the x column from data file to plot (modifier) */

  QString getXColumn();
  /* Description:
       Gets the x column from data file to plot (modifier) */

  void setYColumn(QString col);
  /* Description:
       Sets the y column from data file to plot (modifier)  */

  QString getYColumn();
  /* Description:
       Gets the y column from data file to plot (modifier) */

  void setZColumn(QString col);
  /* Description:
       Sets the z column from data file to plot (modifier) */

  QString getZColumn();
  /* Description:
       Gets the z column from data file to plot (modifier) */

  void setFormat(QString informat);
  /* Description:
       Sets scanf format QString for plotting from data file (modifier)
       (format should be surrounded with double quotes) */

  QString getFormat();
  /* Description:
       Gets format QString for plotting from data file (modifier)
       (format will be surrounded with double quotes) */

  void setRawFormat(QString format);
  /* Description:
       Sets format QString for plotting from data file (modifier)
       (no automatic quoting, must be supplied by user) */

  QString getRawFormat();
  /* Description:
       Sets format QString for plotting from data file (modifier)
       (no automatic quoting, must be supplied by user) */

private:
  QString functionName;
  QString functionValue;
  QString dataFile;
  QString varXRangeName;
  QString varXRangeMin;
  QString varXRangeMax;
  QString varYRangeName;
  QString varYRangeMin;
  QString varYRangeMax;
  QString paramFile;
  int paramFileFlag;
  int paramCSLFlag;
  QString paramCSL;
  QString fitLimit;
  QString fitMaxIter;
  QString dataSetStart;
  QString dataSetEnd;
  QString dataSetInc;
  QString pointInc;
  QString lineInc;
  QString startPoint;
  QString startLine;
  QString endPoint;
  QString endLine;
  QString xColumn;
  QString yColumn;
  QString zColumn;
  QString format;
  QString rawFormat;
};

#endif // gnuCurveFit_included
