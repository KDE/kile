/* -------------------------- gnuInterface class --------------------------

   This class contains all the code necessary to issue commands to gnuplot
   through a named pipe. It does not check for error messages as a result
   of a bad command.

   The plotting command is reasonably smart. All you have to do is set the
   options you want and call doPlot().

   It is intended that all commands be passed through gnuInterface so one
   should only have to look at this header file to see all commands that
   are available. This class may pass the commands on and call equivalent
   commands from other classes if necessary.

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

#ifndef gnuInterface_included
#define gnuInterface_included

using namespace std;

#include <qstring.h>
#include <stdio.h>
#include <qtextstream.h>
#include <qfile.h>
#include "gnuPlotFile.h"
#include "gnuPlotFunction.h"
#include "gnuMultiFile.h"
#include "gnuMultiFunc.h"
#include "gnuCurveFit.h"

class gnuInterface
{
public:
  gnuInterface();
  /* Description:
       Constructor function */

  FILE* openGnuplot();
  /* Incoming arguments:
       none
     Outgoing arguments:
       FILE*: standard I/O stream returned by popen. If the return value
              is NULL, then an error opening the pipe occurred.
              See "man popen" for details
      Description:
        Opens a write only named pipe to gnuplot.*/

  void doCommand(const QString &c);
  /* Incoming arguments:
       none
     Outgoing arguments:
       none
     Description:
       Issues all necessary commands for a plot. This does all the work of
       setting labels, variables, etc. */

  void setPlotFileFlag(int flag);
  /* Incoming arguments:
       int flag: 0 for false or 1 for true
     Outgoing arguments:
       none
     Description:
       Sets a flag to control whether a datafile will be plotted.
       If set to 1, make sure filename is set! */

  int getPlotFileFlag();
  /* Description:
       Gets flag */

  void setPlotMultipleFileFlag(int flag);
  /* Incoming arguments:
       int flag: 0 for false or 1 for true
     Outgoing arguments:
       none
     Description:
       Sets a flag to control whether multiple datafiles will be plotted.
       If set to 1, make sure multiple filenames are set! */

  int getPlotMultipleFileFlag();
  /* Description:
       Gets flag */

  void setPlotFuncFlag(int flag);
  /* Incoming arguments:
       int flag: 0 for false or 1 for true
     Outgoing arguments:
       none
     Description:
       Sets a flag to control whether a function will be plotted.
       If set to 1, make sure the function is set! */

  int getPlotFuncFlag();
  /* Description:
       Gets flag */

  void setPlotMultipleFuncFlag(int flag);
  /* Incoming arguments:
       int flag: 0 for false or 1 for true
     Outgoing arguments:
       none
     Description:
       Sets a flag to control whether multiple functions will be plotted.
       If set to 1, make sure the functions are set! */

  int getPlotMultipleFuncFlag();
  /* Description:
       Gets flag */

  void setPlotFilename(QString filename);
  /* Incoming arguments:
       QString filename: Path and name of file to plot
     Outgoing arguments:
       none
     Description:
       Sets the datafile name to plot */

  QString getPlotFilename();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString filename: Path and name of file to plot
     Description:
       Gets the single datafile name to plot */
  QString getCommands();

  void setPlotFunction(QString function);
  /* Incoming arguments:
       QString function: Function or comma seperated multiple functions
     Outgoing arguments:
       none
     Description:
       Sets the function or functions to plot*/

  QString getPlotFunction();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString function: Function to plot
     Description:
       Gets the single function to plot*/

  void setGnuFileSave(QString file);
  /* Incoming arguments:
       QString file: Filename to save plot to
     Outgoing arguments:
       none
     Description:
       Sets the filename to save a gnuplot plot to */

  void setGnuFileLoad(QString file);
  /* Incoming arguments:
       QString file: Filename to load plot from
     Outgoing arguments:
       none
     Description:
       Sets the filename to load a gnuplot plot from */

  void setFilePlotType(QString type);
  /* Incoming arguments:
       QString type: plot or splot
     Outgoing arguments:
       none
     Description:
       This sets the type of plot for the data file. Use plot for 2D plots,
       and splot for 3D plots */

  QString getFilePlotType();
  /* Description:
       Gets type of plot for the single data file. */

  void setFileStyleType(QString type);
  /* Incoming arguments:
       QString type: style of plotting.
     Outgoing arguments:
       none
     Description:
       Sets the style of plotting the datafile */

  QString getFileStyleType();
  /* Description:
       Gets style type of plot for the single data file. */

  void setFileDataSetStart(QString start);
  /*  Description:
       Sets the start of the data set for the index command */

  QString getFileDataSetStart();
  /*  Description:
       Gets the start of the data set for the index command */

  void setFileDataSetEnd(QString end);
  /*  Description:
       Sets the end of the data set for the index command */

  QString getFileDataSetEnd();
  /*  Description:
       Gets the end of the data set for the index command */

  void setFileDataSetIncrement(QString inc);
  /*  Description:
       Sets the increment value of the data set for the index command */

  QString getFileDataSetIncrement();
  /*  Description:
       Gets the increment value of the data set for the index command */

  void setFileSampPointInc(QString pinc);
  /*  Description:
       Sets the point increment value of the periodic sampling
       for the every command */

  QString getFileSampPointInc();
  /*  Description:
       Gets the point increment value of the periodic sampling
       for the every command */

  void setFileSampLineInc(QString linc);
  /*  Description:
       Sets the line increment value of the periodic sampling
       for the every command */

  QString getFileSampLineInc();
  /*  Description:
       Gets the line increment value of the periodic sampling
       for the every command */

  void setFileSampStartPoint(QString startp);
  /*  Description:
       Sets the point start value of the periodic sampling
       for the every command */

  QString getFileSampStartPoint();
  /*  Description:
       Gets the point start value of the periodic sampling
       for the every command */

  void setFileSampStartLine(QString startl);
  /*  Description:
       Sets the line start value of the periodic sampling
       for the every command */

  QString getFileSampStartLine();
  /*  Description:
       Gets the line start value of the periodic sampling
       for the every command */

  void setFileSampEndPoint(QString endp);
  /*  Description:
       Sets the point end value of the periodic sampling
       for the every command */

  QString getFileSampEndPoint();
  /*  Description:
       Gets the point end value of the periodic sampling
       for the every command */

  void setFileSampEndLine(QString endl);
  /*  Description:
       Sets the line end value of the periodic sampling
       for the every command */

  QString getFileSampEndLine();
  /*  Description:
       Gets the line end value of the periodic sampling
       for the every command */

  void setFileSmoothType(QString type);
  /*  Description:
       Sets the smoothing type for the smooth command */

  QString getFileSmoothType();
  /*  Description:
       Gets the smoothing type for the smooth command */

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

  void setFileFilter(QString filter);
  /* Incoming arguments:
       QString filter: filter command
     Outgoing arguments:
       none
     Description:
       Sets filter command for the single file */

  QString getFileFilter();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: filter command
     Description:
       Gets filter command for the single file */

  void setFileFilterQuoteChar(QString quote);
  /* Incoming arguments:
       char quote: quoting char
     Outgoing arguments:
       none
     Description:
       Sets quoting character for the single file filter cmd */

  QString getFileFilterQuoteChar();
  /* Incoming arguments:
       none
     Outgoing arguments:
       none
     Description:
       Gets quoting character for the single file filter cmd */

  void setFuncPlotType(QString type);
  /* Incoming arguments:
       QString type: plot or splot
     Outgoing arguments:
       none
     Description:
       This sets the type of plot for the function. Use plot for 2D plots,
       and splot for 3D plots */

  QString getFuncPlotType();
  /* Description:
       Gets plot type */

  void setFuncStyleType(QString type);
  /* Incoming arguments:
       QString type: style of plotting. With Gnuplot 3.5 the following types
       are supported: points, lines, linespoints, impulses, dots, steps,
       errorbars, and boxes. Others can be specified if Gnuplot will
       understand it.
     Outgoing arguments:
       none
     Description:
       Sets the style of plotting the function */

  QString getFuncStyleType();
  /* Description:
       Gets style type */

  void setXVariableName(QString name);
  /* Incoming arguments:
       QString name: X variable name
     Outgoing arguments:
       none
     Description:
       Sets the X variable name */

  QString getXVariableName();

  void setXRangeStart(QString range);
  /* Incoming arguments:
       QString range: Starting range for the X axis
     Outgoing arguments:
       none
     Description:
       Sets the starting range for the X axis */

  QString getXRangeStart();

  void setXRangeEnd(QString range);
  /* Incoming arguments:
       QString range: Ending range for the X axis
     Outgoing arguments:
       none
     Description:
       Sets the ending range for the X axis */

  QString getXRangeEnd();

  void setYVariableName(QString name);
  /* Incoming arguments:
       QString name: Y variable name
     Outgoing arguments:
       none
     Description:
       Sets the Y variable name */

  QString getYVariableName();

  void setYRangeStart(QString range);
  /* Incoming arguments:
       QString range: Starting range for the Y axis
     Outgoing arguments:
       none
     Description:
       Sets the starting range for the Y axis */

  QString getYRangeStart();

  void setYRangeEnd(QString range);
  /* Incoming arguments:
       QString range: Ending range for the Y axis
     Outgoing arguments:
       none
     Description:
       Sets the ending range for the Y axis */

  QString getYRangeEnd();

  void setZRangeStart(QString range);
  /* Incoming arguments:
       QString range: Starting range for the Z axis
     Outgoing arguments:
       none
     Description:
       Sets the starting range for the Z axis */

  QString getZRangeStart();

  void setZRangeEnd(QString range);
  /* Incoming arguments:
       QString range: Ending range for the Z axis
     Outgoing arguments:
       none
     Description:
       Sets the ending range for the Z axis */

  QString getZRangeEnd();

  void setXlabel(QString label);
  /* Incoming arguments:
       QString label: Label for the X axis
     Outgoing arguments:
       none
     Description:
       Sets the label for the X axis */

  QString getXlabel();

  void setXOffset_X(QString offset);
  /* Incoming arguments:
       QString offset: X offset value for X axis
     Outgoing arguments:
       none
     Description:
       Sets X offset value for X axis */

  QString getXOffset_X();

  void setXOffset_Y(QString offset);
  /* Incoming arguments:
       QString offset: Y offset value for X axis
     Outgoing arguments:
       none
     Description:
       Sets Y offset value for X axis */

  QString getXOffset_Y();

  void setYlabel(QString label);
  /* Incoming arguments:
       QString label: Label for the Y axis
     Outgoing arguments:
       none
     Description:
       Sets the label for the Y axis */

  QString getYlabel();

  void setYOffset_X(QString offset);
  /* Incoming arguments:
       QString offset: X offset value for Y axis
     Outgoing arguments:
       none
     Description:
       Sets X offset value for Y axis */

  QString getYOffset_X();

  void setYOffset_Y(QString offset);
  /* Incoming arguments:
       QString offset: Y offset value for Y axis
     Outgoing arguments:
       none
     Description:
       Sets Y offset value for Y axis */

  QString getYOffset_Y();

  void setZlabel(QString label);
  /* Incoming arguments:
       QString label: Label for the Z axis
     Outgoing arguments:
       none
     Description:
       Sets the label for the Z axis */

  QString getZlabel();

  void setZOffset_X(QString offset);
  /* Incoming arguments:
       QString offset: X offset value for Z axis
     Outgoing arguments:
       none
     Description:
       Sets X offset value for Z axis */

  QString getZOffset_X();

  void setZOffset_Y(QString offset);
  /* Incoming arguments:
       QString offset: X offset value for Z axis
     Outgoing arguments:
       none
     Description:
       Sets Y offset value for Z axis */

  QString getZOffset_Y();

  void setTitle(QString intitle);
  /* Incoming arguments:
       QString intitle: Title of plot
     Outgoing arguments:
       none
     Description:
       Sets title of plot */

  QString getTitle();

  void setTitleOffset_X(QString offset);
  /* Incoming arguments:
       QString offset: X offset value for the title
     Outgoing arguments:
       none
     Description:
       Sets X offset value for the title */

  QString getTitleOffset_X();

  void setTitleOffset_Y(QString offset);
  /* Incoming arguments:
       QString offset: Y offset value for the title
     Outgoing arguments:
       none
     Description:
       Sets Y offset value for the title */

  QString getTitleOffset_Y();

  void setTerminal(QString terminal);
  /* Description:
       Sets the terminal name */

  QString getTerminal();
  /* Description:
       Gets the terminal name */

  void setTermLateXEmtexFont(QString font);
  /* Description:
       Sets the latex/emtex terminal font with any options needed */

  QString getTermLateXEmtexFont();
  /* Description:
       Returns the current value of the terminal font */

  void setTermLateXEmtexFontSize(QString size);
  /* Description:
       Sets the latex/emtex terminal size */

  QString getTermLateXEmtexFontSize();
  /* Description:
       Gets the latex/emtex terminal size */

  void setTermPBMFontSize(QString size);
  /* Description:
       Sets the PBM terminal font size */

  QString getTermPBMFontSize();
  /* Description:
       Gets the PBM terminal font size */

  void setTermPBMColormode(QString color);
  /* Description:
       Sets the PBM terminal colormode */

  QString getTermPBMColormode();
  /* Description:
       Gets the PBM terminal colormode */

  void setTermPBMhSize(QString size);
  /* Description:
       Sets the PBM terminal horizontal size (pixels) */

  QString getTermPBMhSize();
  /* Description:
       Gets the PBM terminal horizontal size (pixels) */

  void setTermPBMvSize(QString size);
  /* Description:
       Sets the PBM terminal vertical size (pixels) */

  QString getTermPBMvSize();
  /* Description:
       Gets the PBM terminal vertical size (pixels) */

  void setTermPSmode(QString mode);
  /* Description:
       Sets the postscript terminal mode */

  QString getTermPSmode();
  /* Description:
       Gets the postscript terminal mode */

  void setTermPScolor(QString color);
  /* Description:
       Sets the postscript terminal color */

  QString getTermPScolor();
  /* Description:
       Gets the postscript terminal color */

  void setTermPSdashed(QString type);
  /* Description:
       Sets the postscript terminal line-type */

  QString getTermPSdashed();
  /* Description:
       Gets the postscript terminal line-type */

  void setTermPSfont(QString font);
  /* Description:
       Sets the postscript terminal font */

  QString getTermPSfont();
  /* Description:
       Gets the postscript terminal font */

  void setTermPSfontSize(QString size);
  /* Description:
       Sets the postscript terminal font size */

  QString getTermPSfontSize();
  /* Description:
       Gets the postscript terminal font size */

  void setTermPSenhanced(QString inenhanced);
  /* Description:
       Sets the postscript terminal enhanced */

  QString getTermPSenhanced();
  /* Description:
       Gets the postscript terminal enhanced */

  void setTermPShSize(QString size);
  /* Description:
       Sets the postscript terminal horizontal size (inches) */

  QString getTermPShSize();
  /* Description:
       Gets the postscript terminal horizontal size (inches) */

  void setTermPSvSize(QString size);
  /* Description:
       Sets the postscript terminal vertical size (inches) */

  QString getTermPSvSize();
  /* Description:
       Gets the postscript terminal vertical size (inches) */

  void setHorizSize(QString size);
  /* Incoming arguments:
       QString size: Horizontal size of plot
     Outgoing arguments:
       none
     Description:
       Sets horizontal size of plot. Depending on the terminal type,
       the units could be pixels, inches, or scaling factor */

  QString getHorizSize();
  /* Description:
       Gets horizontal size of plot. */

  void setVertSize(QString size);
  /* Incoming arguments:
       QString size: Vertical size of plot
     Outgoing arguments:
       none
     Description:
       Sets vertical size of plot. Depending on the terminal type,
       the units could be pixels, inches, or scaling factor */

  QString getVertSize();
  /* Description:
       Gets vertical size of plot. */

  void setOutput(QString output);
  /* Incoming arguments:
       QString output: Output filename for plot
     Outgoing arguments:
       none
     Description:
       Sets the output filename for the plot */

  QString getOutput();

  void doSave();
  /* Incoming arguments:
       none
     Outgoing arguments:
       none
     Description:
       Uses the "save" command of gnuplot to the file specified by the output
       variable */

  void doLoad();
  /* Incoming arguments:
       none
     Outgoing arguments:
       none
     Description:
       Uses the "load" command of gnuplot to the file specified by the output
       variable */

  void closeGnuplot();
  /* Incoming arguments:
       none
     Outgoing arguments:
       none
     Description:
       Closes the named pipe to gnuplot */

  void setReplotFlag(int flag);
  /* Incoming arguments:
       int flag: 0 for false or 1 for true
     Outgoing arguments:
       none
     Description:
       Sets a flag to control whether a function will use the replot command. */

  void setLegendFlag(QString flag);
  /* Incoming arguments:
       QString flag: key for a legend, nokey for no legend
     Outgoing arguments:
       none
     Description:
       Specifies if a legend will be used */

  QString getLegendFlag();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: value of legend flag
     Description:
       Returns current value of legend flag */

  void setLegendPositionLeft(int position);
  /* Incoming arguments:
       int position: 1 or 0 : 1 = true, 0 = false
     Outgoing arguments:
       none
     Description:
       Sets value of legend position if it is on the left or not*/

  int getLegendPositionLeft();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: value of current legend position if it is left
     Description:
       Gets current value of legend position if it is left */

  void setLegendPositionRight(int position);
  /* Incoming arguments:
       int position: 1 or 0 : 1 = true, 0 = false
     Outgoing arguments:
       none
     Description:
       Sets value of legend position if it is on the right or not*/

  int getLegendPositionRight();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: value of current legend position if it is right
     Description:
       Gets current value of legend position if it is right */

  void setLegendPositionTop(int position);
  /* Incoming arguments:
       int position: 1 or 0 : 1 = true, 0 = false
     Outgoing arguments:
       none
     Description:
       Sets value of legend position if it is on the top or not*/

  int getLegendPositionTop();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: value of current legend position if it is top
     Description:
       Gets current value of legend position if it is top */

  void setLegendPositionBottom(int position);
  /* Incoming arguments:
       int position: 1 or 0 : 1 = true, 0 = false
     Outgoing arguments:
       none
     Description:
       Sets value of legend position if it is on the bottom or not*/

  int getLegendPositionBottom();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: value of current legend position if it is bottom
     Description:
       Gets current value of legend position if it is bottom */

  void setLegendPositionOutside(int position);
  /* Incoming arguments:
       int position: 1 or 0 : 1 = true, 0 = false
     Outgoing arguments:
       none
     Description:
       Sets value of legend position if it is on the outside or not*/

  int getLegendPositionOutside();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: value of current legend position if it is outside
     Description:
       Gets current value of legend position if it is outside */

  void setLegendPositionBelow(int position);
  /* Incoming arguments:
       int position: 1 or 0 : 1 = true, 0 = false
     Outgoing arguments:
       none
     Description:
       Sets value of legend position if it is below or not*/

  int getLegendPositionBelow();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: value of current legend position if it is below
     Description:
       Gets current value of legend position if it is below */

  void setLegendPositionXVal(QString val);
  /* Incoming arguments:
       QString val: X position of legend
     Outgoing arguments:
       none
     Description:
       Sets current value of X coordinate for legend position */

  QString getLegendPositionXVal();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of X coordinate
     Description:
       Gets current value of X coordinate for legend position */

  void setLegendPositionYVal(QString val);
  /* Incoming arguments:
       QString val: Y position of legend
     Outgoing arguments:
       none
     Description:
       Sets current value of Y coordinate for legend position */

  QString getLegendPositionYVal();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of Y coordinate
     Description:
       Gets current value of Y coordinate for legend position */

  void setLegendPositionZVal(QString val);
  /* Incoming arguments:
       QString val: Z position of legend
     Outgoing arguments:
       none
     Description:
       Sets current value of Z coordinate for legend position */

  QString getLegendPositionZVal();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of Z coordinate
     Description:
       Gets current value of Z coordinate for legend position */

  void setLegendTextJustify(QString justify);
  /* Incoming arguments:
       QString justify: Justification of legend text
     Outgoing arguments:
       none
     Description:
       Sets current value of justification for legend text */

  QString getLegendTextJustify();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of text justification
     Description:
       Gets current value of justification for legend text */

  void setLegendReverse(QString reverse);
  /* Incoming arguments:
       QString reverse: reverse/noreverse for legend sample and text
     Outgoing arguments:
       none
     Description:
       Sets current value of reverse for legend text and samples */

  QString getLegendReverse();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of reverse
     Description:
       Gets current value of reverse for legend text and samples */

  void setLegendBox(QString box);
  /* Incoming arguments:
       QString box: box/nobox value to use a box or not
     Outgoing arguments:
       none
     Description:
       Sets current value of box for legend  */

  QString getLegendBox();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of box
     Description:
       Gets current value of box for legend  */

  void setLegendLinetype(QString type);
  /* Incoming arguments:
       QString type: linetype for legend box
     Outgoing arguments:
       none
     Description:
       Sets current value of linetype for box around legend  */

  QString getLegendLinetype();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of linetype
     Description:
       Gets current value of linetype for box around legend  */

  void setLegendSampleLength(QString length);
  /* Incoming arguments:
       QString length: length of line sample in legend
     Outgoing arguments:
       none
     Description:
       Sets current value of line sample length for legend  */

  QString getLegendSampleLength();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of sample length
     Description:
       Gets current value of line sample length for legend  */

  void setLegendSpacing(QString spacing);
  /* Incoming arguments:
       QString spacing: vertical spacing of lines in legend
     Outgoing arguments:
       none
     Description:
       Sets current value of vertical spacing between lines for legend */

  QString getLegendSpacing();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of legend spacing
     Description:
       Gets current value of vertical spacing between lines for legend */

  void setLegendWidthIncrement(QString width);
  /* Incoming arguments:
       QString width: extra width for box around legend
     Outgoing arguments:
       none
     Description:
       Sets current value of extra spacing for box around legend */

  QString getLegendWidthIncrement();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of extra spacing
     Description:
       Gets current value of extra spacing for box around legend */

  void setLegendTitle(QString title);
  /* Incoming arguments:
       QString title: title of legend
     Outgoing arguments:
       none
     Description:
       Sets current value of title for legend */

  QString getLegendTitle();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current value of legend title
     Description:
       Gets current value of title for legend */

  void setXticsOnFlag(int flag);
  int getXticsOnFlag();
  void setXticsLocation(QString location);
  QString getXticsLocation();
  void setXticsMirror(QString mirror);
  QString getXticsMirror();
  void setXticsRotation(QString rotation);
  QString getXticsRotation();
  void setXticsPositionType(QString type);
  /* Description:
       Sets position type
       Position = SIE for start/inc/edit type or LABELS = labels/pos type */
  QString getXticsPositionType();
  void setXticsStartPos(QString pos);
  QString getXticsStartPos();
  void setXticsIncPos(QString pos);
  QString getXticsIncPos();
  void setXticsEndPos(QString pos);
  QString getXticsEndPos();
  void setXticsLabelsPos(QString labels);
  QString getXticsLabelsPos();

  void setYticsOnFlag(int flag);
  int getYticsOnFlag();
  void setYticsLocation(QString location);
  QString getYticsLocation();
  void setYticsMirror(QString mirror);
  QString getYticsMirror();
  void setYticsRotation(QString rotation);
  QString getYticsRotation();
  void setYticsPositionType(QString type);
  /* Description:
       Sets position type
       Position = SIE for start/inc/edit type or LABELS = labels/pos type */
  QString getYticsPositionType();
  void setYticsStartPos(QString pos);
  QString getYticsStartPos();
  void setYticsIncPos(QString pos);
  QString getYticsIncPos();
  void setYticsEndPos(QString pos);
  QString getYticsEndPos();
  void setYticsLabelsPos(QString labels);
  QString getYticsLabelsPos();

  void setZticsOnFlag(int flag);
  int getZticsOnFlag();
  void setZticsMirror(QString mirror);
  QString getZticsMirror();
  void setZticsRotation(QString rotation);
  QString getZticsRotation();
  void setZticsPositionType(QString type);
  /* Description:
       Sets position type
       Position = SIE for start/inc/edit type or LABELS = labels/pos type */
  QString getZticsPositionType();
  void setZticsStartPos(QString pos);
  QString getZticsStartPos();
  void setZticsIncPos(QString pos);
  QString getZticsIncPos();
  void setZticsEndPos(QString pos);
  QString getZticsEndPos();
  void setZticsLabelsPos(QString labels);
  QString getZticsLabelsPos();

  void setX2ticsOnFlag(int flag);
  int getX2ticsOnFlag();
  void setX2ticsLocation(QString location);
  QString getX2ticsLocation();
  void setX2ticsMirror(QString mirror);
  QString getX2ticsMirror();
  void setX2ticsRotation(QString rotation);
  QString getX2ticsRotation();
  void setX2ticsPositionType(QString type);
  /* Description:
       Sets position type
       Position = SIE for start/inc/edit type or LABELS = labels/pos type */
  QString getX2ticsPositionType();
  void setX2ticsStartPos(QString pos);
  QString getX2ticsStartPos();
  void setX2ticsIncPos(QString pos);
  QString getX2ticsIncPos();
  void setX2ticsEndPos(QString pos);
  QString getX2ticsEndPos();
  void setX2ticsLabelsPos(QString labels);
  QString getX2ticsLabelsPos();

  void setY2ticsOnFlag(int flag);
  int getY2ticsOnFlag();
  void setY2ticsLocation(QString location);
  QString getY2ticsLocation();
  void setY2ticsMirror(QString mirror);
  QString getY2ticsMirror();
  void setY2ticsRotation(QString rotation);
  QString getY2ticsRotation();
  void setY2ticsPositionType(QString type);
  /* Description:
       Sets position type
       Position = SIE for start/inc/edit type or LABELS = labels/pos type */
  QString getY2ticsPositionType();
  void setY2ticsStartPos(QString pos);
  QString getY2ticsStartPos();
  void setY2ticsIncPos(QString pos);
  QString getY2ticsIncPos();
  void setY2ticsEndPos(QString pos);
  QString getY2ticsEndPos();
  void setY2ticsLabelsPos(QString labels);
  QString getY2ticsLabelsPos();


  void insertMultiFileNew(QString filename);
  /* Incoming arguments:
       QString filename: new filename
     Outgoing arguments:
       none
     Description:
       Inserts a new file into multiple file list */

  void removeMultiFile(QString filename);
  /* Incoming arguments:
       QString filename: file to remove
     Outgoing arguments:
       none
     Description:
       Removes file from multiple file list */

  QString getMultiFileFirstFilename();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: filename for the first file
     Description:
       Returns filename for the first file in the multiple file list
       Returns END if empty list */

  QString getMultiFileNextFilename();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: filename for the next file
     Description:
       Returns filename for the next file in the multiple file list
       Returns END if at the end of the list */

  void setMultiFileDataSetStart(QString filename, QString start);
  /*  Description:
       Sets the start of the data set for the index command */

  QString getMultiFileDataSetStart(QString filename);
  /*  Description:
       Gets the start of the data set for the index command */

  void setMultiFileDataSetEnd(QString filename,QString end);
  /*  Description:
       Sets the end of the data set for the index command */

  QString getMultiFileDataSetEnd(QString filename);
  /*  Description:
       Gets the end of the data set for the index command */

  void setMultiFileDataSetIncrement(QString filename,QString inc);
  /*  Description:
       Sets the increment value of the data set for the index command */

  QString getMultiFileDataSetIncrement(QString filename);
  /*  Description:
       Gets the increment value of the data set for the index command */

  void setMultiFileSampPointInc(QString filename,QString pinc);
  /*  Description:
       Sets the point increment value of the periodic sampling
       for the every command */

  QString getMultiFileSampPointInc(QString filename);
  /*  Description:
       Gets the point increment value of the periodic sampling
       for the every command */

  void setMultiFileSampLineInc(QString filename,QString linc);
  /*  Description:
       Sets the line increment value of the periodic sampling
       for the every command */

  QString getMultiFileSampLineInc(QString filename);
  /*  Description:
       Gets the line increment value of the periodic sampling
       for the every command */

  void setMultiFileSampStartPoint(QString filename,QString startp);
  /*  Description:
       Sets the point start value of the periodic sampling
       for the every command */

  QString getMultiFileSampStartPoint(QString filename);
  /*  Description:
       Gets the point start value of the periodic sampling
       for the every command */

  void setMultiFileSampStartLine(QString filename,QString startl);
  /*  Description:
       Sets the line start value of the periodic sampling
       for the every command */

  QString getMultiFileSampStartLine(QString filename);
  /*  Description:
       Gets the line start value of the periodic sampling
       for the every command */

  void setMultiFileSampEndPoint(QString filename,QString endp);
  /*  Description:
       Sets the point end value of the periodic sampling
       for the every command */

  QString getMultiFileSampEndPoint(QString filename);
  /*  Description:
       Gets the point end value of the periodic sampling
       for the every command */

  void setMultiFileSampEndLine(QString filename,QString endl);
  /*  Description:
       Sets the line end value of the periodic sampling
       for the every command */

  QString getMultiFileSampEndLine(QString filename);
  /*  Description:
       Gets the line end value of the periodic sampling
       for the every command */

  void setMultiFileSmoothType(QString filename,QString type);
  /*  Description:
       Sets the smoothing type for the smooth command */

  QString getMultiFileSmoothType(QString filename);
  /*  Description:
       Gets the smoothing type for the smooth command */

  void insertMultiFileXColumnOption(QString filename, QString xcol);
  /* Incoming arguments:
       QString filename: file to operate on
       QString xcol: X column to plot
     Outgoing arguments:
       none
     Description:
       Sets x column for a given file in the multiple file list */

  QString getMultiFileXColumnOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: x column for given file
     Description:
       Return x column for a given file in the multiple file list */

  void insertMultiFileYColumnOption(QString filename, QString ycol);
  /* Incoming arguments:
       QString filename: file to operate on
       QString ycol: Y column to plot
     Outgoing arguments:
       none
     Description:
       Sets y column for a given file in the multiple file list */

  QString getMultiFileYColumnOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: y column for a given file
     Description:
       Return y column for a given file in the multiple file list */

  void insertMultiFileZColumnOption(QString filename, QString zcol);
  /* Incoming arguments:
       QString filename: file to operate on
       QString zcol: Z column to plot
     Outgoing arguments:
       none
     Description:
       Sets z column for a given file in the multiple file list */

  QString getMultiFileZColumnOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: z column for the given file
     Description:
       Return z column for a given file in the multiple file list */

  void insertMultiFileFormatOption(QString filename, QString format);
  /* Incoming arguments:
       QString filename: file to operate on
       QString format: format for plotting
     Outgoing arguments:
       none
     Description:
       Set format for the given file in the multiple file list */

  QString getMultiFileFormatOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: format for given file
     Description:
       Returns format for the given file in the multiple file list */

  void insertMultiFileRawFormatOption(QString filename, QString format);
  /* Incoming arguments:
       QString filename: file to operate on
       QString format: raw format QString for plotting
     Outgoing arguments:
       none
     Description:
       Set raw format for the given file in the multiple file list */

  QString getMultiFileRawFormatOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: raw format for given file
     Description:
       Returns raw format for the given file in the multiple file list */

  void setMultiFileStyleOption(QString filename, QString style);
  /* Incoming arguments:
       QString filename: file to operate on
       QString format: style QString for plotting
     Outgoing arguments:
       none
     Description:
       Set style for the given file in the multiple file list */

  QString getMultiFileStyleOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: style for given file
     Description:
       Returns style for the given file in the multiple file list */

  void insertMultiFuncNew(QString function);
  /* Incoming arguments:
       QString: function to insert
     Outgoing arguments:
       none
     Description:
       Inserts a new function into list */

  void removeMultiFunc(QString function);
  /* Incoming arguments:
       QString: function to remove
     Outgoing arguments:
       none
     Description:
       Removes a function from list */

  void setMultiFuncStyleOption(QString function, QString style);
  /* Incoming arguments:
       function: function to operate on
       style: plotting style to use
     Outgoing arguments:
       none
     Description:
       Sets the plotting style for the specified function */

  QString getMultiFuncStyleOption(QString function);
  /* Incoming arguments:
       function: function to operate on
     Outgoing arguments:
       QString: plotting style
     Description:
       Gets the plotting style for the specified function */

  QString getMultiFuncFirstFunction();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: First function in list
               Returns END if list is empty
     Description:
       Gets the first function in the list */

  QString getMultiFuncNextFunction();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: Next function in list
               Returns END if the end of the list is reached
     Description:
       Gets the next function in the list */

  QString getMultiFuncFirstPlotCmd();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: First function's plotting command in list
               Returns END if list is empty
     Description:
       Gets the first function's plotting command in the list */

  QString getMultiFuncNextPlotCmd();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: Next function's plotting command in list
               Returns END if the end of the list is reached
     Description:
       Gets the next function's plotting command in the list */

  void setFileLegendTitle(QString title);
  /* Incoming arguments:
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend for the single file */

  void setMultiFileLegendTitle(QString filename, QString title);
  /* Incoming arguments:
       filename: filename to operate on
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend for the given file */

  QString getFileLegendTitle();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: title for current file
     Description:
       Gets title to be used in legend for the single file */

  QString getMultiFileLegendTitle(QString filename);
  /* Incoming arguments:
       filename: filename to operate on
     Outgoing arguments:
       QString: title for given file
     Description:
       Gets title to be used in legend for the given file */

  void setFuncLegendTitle(QString title);
  /* Incoming arguments:
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend for the single function */

  void setMultiFuncLegendTitle(QString function, QString title);
  /* Incoming arguments:
       function: function to operate on
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend for the given function */

  QString getFuncLegendTitle();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: title for legend
     Description:
       Gets title to be used in legend for the single function */

  QString getMultiFuncLegendTitle(QString function);
  /* Incoming arguments:
       function: function to operate on
     Outgoing arguments:
       QString: title for given function
     Description:
       Gets title to be used in legend for the given function */

  void setMultiFileFilter(QString filename, QString filter);
  /* Incoming arguments:
       QString filename: filename to operate on
       QString filter: filter command
     Outgoing arguments:
       none
     Description:
       Sets filter command for the file */

  QString getMultiFileFilter(QString filename);
  /* Incoming arguments:
       QString: filename to operate on
     Outgoing arguments:
       QString: filter command
     Description:
       Gets filter command for the file */

  void setMultiFileFilterQuoteChar(QString filename, QString quote);
  /* Incoming arguments:
       QString filename: filename to operate on
       QString quote: quoting char
     Outgoing arguments:
       none
     Description:
       Sets quoting character for the file's filter cmd */

  QString getMultiFileFilterQuoteChar(QString filename);
  /* Incoming arguments:
       QString filename: filename to operate on
     Outgoing arguments:
       QString: quoting character
     Description:
       Gets quoting character for the file's filter cmd */

  void setLogScaleBase(int base);

  int getLogScaleBase();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: base of log scale
     Description:
       Gets the base for log scale */

  void setLogScaleXAxis(int xAxis);

  int getLogScaleXAxis();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: flag for x axis
     Description:
       Gets the flag to determine if x axis is to be plotted log scale */

  void setLogScaleYAxis(int yAxis);

  int getLogScaleYAxis();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: flag for y axis
     Description:
       Gets the flag to determine if y axis is to be plotted log scale */

  void setLogScaleZAxis(int zAxis);

  int getLogScaleZAxis();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: flag for z axis
     Description:
       Gets the flag to determine if z axis is to be plotted log scale */

  void setLogScaleX2Axis(int x2Axis);

  int getLogScaleX2Axis();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: flag for x2 axis
     Description:
       Gets the flag to determine if x2 axis is to be plotted log scale */

  void setLogScaleY2Axis(int y2Axis);

  int getLogScaleY2Axis();
  /* Incoming arguments:
       none
     Outgoing arguments:
       int: flag for y2 axis
     Description:
       Gets the flag to determine if y2 axis is to be plotted log scale */

  void setBarSizeOption(QString size);
  /* Incoming arguments:
       QString: size of bars
     Outgoing arguments:
       none
     Description:
       Sets the size of bars for bar styles of plotting */

  QString getBarSizeOption();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: bar size
     Description:
       Gets the size of bars for bar styles of plotting */

  void setCurveFitFunctionName(QString name);
  /* Description:
       Sets function name (left side of equal sign) to fit to */

  QString getCurveFitFunctionName();
  /* Description:
       Gets function name to fit to */

  void setCurveFitFunctionValue(QString function);
  /* Description:
       Sets function value (right side of equal sign) to fit to */

  QString getCurveFitFunctionValue();
  /* Description:
       Gets function value (right side of equal sign) to fit to  */

  void setCurveFitDataFile(QString file);
  /* Description:
       Sets data file for plotting */

  QString getCurveFitDataFile();
  /* Description:
       Gets data file for plotting  */

  void setCurveFitVarXRangeName(QString range);
  /* Description:
       Sets x variable name for xrange */

  QString getCurveFitVarXRangeName();
  /* Description:
       Gets x variable name for xrange  */

  void setCurveFitVarXRangeMin(QString min);
  /* Description:
       Sets x variable min value */

  QString getCurveFitVarXRangeMin();
  /* Description:
       Gets x variable min value */

  void setCurveFitVarXRangeMax(QString max);
  /* Description:
       Sets x variable max value */

  QString getCurveFitVarXRangeMax();
  /* Description:
       Gets x variable max value */

  void setCurveFitVarYRangeName(QString range);
  /* Description:
       Sets y variable name for yrange */

  QString getCurveFitVarYRangeName();
  /* Description:
       Gets y variable name for yrange */

  void setCurveFitVarYRangeMin(QString min);
  /* Description:
       Sets y variable min value */

  QString getCurveFitVarYRangeMin();
  /* Description:
       Gets y variable min value */

  void setCurveFitVarYRangeMax(QString max);
  /* Description:
       Sets y variable max value */

  QString getCurveFitVarYRangeMax();
  /* Description:
       Gets y variable max value */

  void setCurveFitParamFile(QString file);
  /* Description:
       Sets parameter file for plotting parameters */

  QString getCurveFitParamFile();
  /* Description:
       Gets parameter file for plotting parameters */

  void setCurveFitParamFileFlag(int flag);
  /* Description:
       Sets flag for selecting parameter file vs comma seperated
       value parameters */

  int getCurveFitParamFileFlag();
  /* Description:
       Gets flag for selecting parameter file vs comma seperated
       value parameters */

  void setCurveFitParamCSLFlag(int flag);
  /* Description:
       Sets flag for selecting parameter file vs comma seperated
       value parameters */

  int getCurveFitParamCSLFlag();
  /* Description:
       Gets flag for selecting parameter file vs comma seperated
       value parameters */

  void setCurveFitParamCSL(QString list);
  /* Description:
       Sets comma seperated list of plotting parameters */

  QString getCurveFitParamCSL();
  /* Description:
       Gets comma seperated list of plotting parameters */

  void setCurveFitFitLimit(QString limit);
  /* Description:
       Sets fit limit value for convergence */

  QString getCurveFitFitLimit();
  /* Description:
       Gets fit limit value for convergence */

  void setCurveFitFitMaxIter(QString iter);
  /* Description:
       Sets maximum number of iterations for non-convergence case */

  QString getCurveFitFitMaxIter();
  /* Description:
       Gets maximum number of iterations for non-convergence case */

  void setCurveFitDataSetStart(QString start);
  /* Description:
       Sets starting data set of the data file modifiers */

  QString getCurveFitDataSetStart();
  /* Description:
       Gets starting data set of the data file modifiers */

  void setCurveFitDataSetEnd(QString end);
  /* Description:
       Sets ending data set of the data file modifiers */

  QString getCurveFitDataSetEnd();
  /* Description:
       Gets ending data set of the data file modifiers */

  void setCurveFitDataSetInc(QString inc);
  /* Description:
       Sets data set increment of the data file modifiers */

  QString getCurveFitDataSetInc();
  /* Description:
       Gets data set increment of the data file modifiers */

  void setCurveFitPointInc(QString inc);
  /* Description:
       Sets the data point increment of the data file modifiers */

  QString getCurveFitPointInc();
  /* Description:
       Gets the data point increment of the data file modifiers */

  void setCurveFitLineInc(QString inc);
  /* Description:
       Sets the increment value for lines in data file (modifier) */

  QString getCurveFitLineInc();
  /* Description:
       Gets the increment value for lines in data file (modifier) */

  void setCurveFitStartPoint(QString start);
  /* Description:
       Sets the starting point in data file (modifier) */

  QString getCurveFitStartPoint();
  /* Description:
       Gets the starting point in data file (modifier) */

  void setCurveFitStartLine(QString start);
  /* Description:
       Sets the starting line in data file (modifier) */

  QString getCurveFitStartLine();
  /* Description:
       Gets the starting line in data file (modifier) */

  void setCurveFitEndPoint(QString end);
  /* Description:
       Sets the end point in data file (modifier) */

  QString getCurveFitEndPoint();
  /* Description:
       Gets the end point in data file (modifier) */

  void setCurveFitEndLine(QString end);
  /* Description:
       Sets the ending line in data file (modifier) */

  QString getCurveFitEndLine();
  /* Description:
       Gets the ending line in data file (modifier) */

  void setCurveFitXColumn(QString col);
  /* Description:
       Sets the x column from data file to plot (modifier) */

  QString getCurveFitXColumn();
  /* Description:
       Gets the x column from data file to plot (modifier) */

  void setCurveFitYColumn(QString col);
  /* Description:
       Sets the y column from data file to plot (modifier)  */

  QString getCurveFitYColumn();
  /* Description:
       Gets the y column from data file to plot (modifier) */

  void setCurveFitZColumn(QString col);
  /* Description:
       Sets the z column from data file to plot (modifier) */

  QString getCurveFitZColumn();
  /* Description:
       Gets the z column from data file to plot (modifier) */

  void setCurveFitFormat(QString informat);
  /* Description:
       Sets scanf format QString for plotting from data file (modifier)
       (format should be surrounded with double quotes) */

  QString getCurveFitFormat();
  /* Description:
       Gets format QString for plotting from data file (modifier)
       (format will be surrounded with double quotes) */

  void setCurveFitRawFormat(QString format);
  /* Description:
       Sets format QString for plotting from data file (modifier)
       (no automatic quoting, must be supplied by user) */

  QString getCurveFitRawFormat();
  /* Description:
       Sets format QString for plotting from data file (modifier)
       (no automatic quoting, must be supplied by user) */

  void doCurveFit();
  /* Description:
       Issue curve fitting command */

  void setBoxWidth(QString width);
  /* Description:
       Sets box width for boxes plotting styles */

  QString getBoxWidth();
  /* Description:
       Gets box width for boxes plotting styles */

  void setRotationXAxis(int value);
  /* Description:
       Sets rotation value about X axis for 3d plots; should be from 0-180 */

  int getRotationXAxis();

  void setRotationZAxis(int value);
  /* Description:
       Sets rotation value about Z axis for 3d plots; should be from 0-360 */

  int getRotationZAxis();

  void setRotationScaling(QString scale);
  /* Description:
       Sets scaling factor for entire plot  */

  QString getRotationScaling();

  void setRotationZAxisScaling(QString scale);
  /* Description:
       Sets scaling factor for Z axis  */

  QString getRotationZAxisScaling();

  void setTicsLevel(QString level);
  /* Description:
       Sets the tics level for 3d plots */

  QString getTicsLevel();

  void setd3HiddenLineFlag(int flag);
  /* Description:
       Sets hidden line flag for showing hidden lines in 3D plots or not */

  int getd3HiddenLineFlag();

  void setIsolineU(QString isoU);
  /* Description:
       Sets number of isolines in U direction for 3D plots */

  QString getIsolineU();

  void setIsolineV(QString isoV);
  /* Description:
       Sets number of isolines in V direction for 3D plots */

  QString getIsolineV();

  void savePlotData(QString filename);
  /* Description:
       Saves plotting variables individually to the given filename
       as a specially formatted file */

  void loadPlotData(QString filename);
  /* Description:
       Loads plotting variables from the given filename that
       corresponds to the special format */

private:
  QString gnuFileSave;           // filename to save plot to
  QString gnuFileLoad;           // filename to load plot from

  gnuPlotFile* plotFileOb;      // plot file object
  gnuMultiFile* multiFile;      // multiple file object
  gnuPlotFunction* plotFunctionOb; // plot function object
  gnuMultiFunc* multiFunc;      // multiple function object

  QString filePlotType;          // plot or splot for datafile
  QString funcPlotType;          // plot or splot for the function

  QString XVariableName;         // variable name for x axis
  QString XRangeStart;           // starting value for x axis
  QString XRangeEnd;             // ending value for x axis

  QString YVariableName;         // variable name for y axis
  QString YRangeStart;           // starting value for y axis
  QString YRangeEnd;             // ending value for y axis

  QString ZRangeStart;           // starting value for z axis
  QString ZRangeEnd;             // ending value for z axis

  QString XLabel;                // x label
  QString XOffset_X;             // x label offset in x direction
  QString XOffset_Y;             // x label offset in y direction

  QString YLabel;                // y label
  QString YOffset_X;             // y label offset in x direction
  QString YOffset_Y;             // y label offset in y direction

  QString ZLabel;                // z label
  QString ZOffset_X;             // z label offset in x direction
  QString ZOffset_Y;             // z label offset in y direction

  QString title;                 // title of plot
  QString titleOffset_X;         // title offset in x direction
  QString titleOffset_Y;         // title offset in y direction

  QString terminal;              // terminal type

  QString termLatexEmtexFont;    // latex/Emtex terminal font name
  QString termLatexEmtexFontSize; // latex/Emtex terminal size

  QString termPBMfontSize;       // PBM terminal font size
  QString termPBMcolormode;      // PBM terminal colormode
  QString termPBMhSize;          // PBM terminal horiz size (pixels)
  QString termPBMvSize;          // PBM terminal vertical size (pixels)

  QString termPSmode;            // postscript terminal mode
  QString termPScolor;           // postscript terminal color
  QString termPSdashed;          // postscript terminal line type
  QString termPSfont;            // postscript terminal font
  QString termPSfontSize;        // postscript terminal font size
  QString termPSenhanced;        // postscript terminal enhanced/noenhanced
  QString termPShSize;           // postscript terminal horiz size
  QString termPSvSize;           // postscript terminal vert size

  QString output;                // output filename

  QString hSize;                 // horizontal size of plot
  QString vSize;                 // vertical size of plot

  QString legendFlag;            // key/nokey to specify if legend is present
  int legendPositionLeft;       // position of legend
  int legendPositionRight;      // position of legend
  int legendPositionTop;        // position of legend
  int legendPositionBottom;     // position of legend
  int legendPositionOutside;    // position of legend
  int legendPositionBelow;      // position of legend
  QString legendPositionXVal;    // position of legend x coordinate
  QString legendPositionYVal;    // position of legend y coordinate
  QString legendPositionZVal;    // position of legend z coordinate
  QString legendTextJustify;     // justification of legend text
  QString legendReverse;         // reverse/noreverse of legend
  QString legendBox;             // box/nobox of legend
  QString legendLinetype;        // linetype of legend box
  QString legendSampleLength;    // length of line sample for legend
  QString legendSpacing;         // vertical spacing of elements in legend
  QString legendWidthIncrement;  // extra spacing around elements for box
  QString legendTitle;           // title of legend

  int xticsOnFlag;              // X tics options
  QString xticsLocation;
  QString xticsMirror;
  QString xticsRotation;
  QString xticsPositionType;
  QString xticsStartPos;
  QString xticsIncPos;
  QString xticsEndPos;
  QString xticsLabelsPos;

  int yticsOnFlag;              // Y tics options
  QString yticsLocation;
  QString yticsMirror;
  QString yticsRotation;
  QString yticsPositionType;
  QString yticsStartPos;
  QString yticsIncPos;
  QString yticsEndPos;
  QString yticsLabelsPos;

  int zticsOnFlag;              // Z tics options
  QString zticsMirror;
  QString zticsRotation;
  QString zticsPositionType;
  QString zticsStartPos;
  QString zticsIncPos;
  QString zticsEndPos;
  QString zticsLabelsPos;

  int x2ticsOnFlag;            // X2 tics options
  QString x2ticsLocation;
  QString x2ticsMirror;
  QString x2ticsRotation;
  QString x2ticsPositionType;
  QString x2ticsStartPos;
  QString x2ticsIncPos;
  QString x2ticsEndPos;
  QString x2ticsLabelsPos;

  int y2ticsOnFlag;            // Y2 tics options
  QString y2ticsLocation;
  QString y2ticsMirror;
  QString y2ticsRotation;
  QString y2ticsPositionType;
  QString y2ticsStartPos;
  QString y2ticsIncPos;
  QString y2ticsEndPos;
  QString y2ticsLabelsPos;

  int plotFileFlag;             // flag to specify plotting file or not
  int plotMultipleFileFlag;     // flag to specify plotting multiple files
  int plotFuncFlag;             // flag to specify plotting function or not
  int plotMultipleFuncFlag;     // flag to specify plotting multiple functions
  int replotFlag;               // flag to specify replot command

  int logScaleBase;             // log scale base
  int logScaleXAxisFlag;        // flag to plot log scale on X axis
  int logScaleYAxisFlag;        // flag to plot log scale on Y axis
  int logScaleZAxisFlag;        // flag to plot log scale on Z axis
  int logScaleX2AxisFlag;       // flag to plot log scale on X2 axis
  int logScaleY2AxisFlag;       // flag to plot log scale on Y2 axis

  QString barSize;               // size of bar in bar styles of plotting

  QString boxWidth;              // with of boxes in boxes type of plotting

  int xAxisRotation;            // rotation about x axis for 3d plots
  int zAxisRotation;            // rotation about z axis for 3d plots
  QString rotationScale;         // scaling for entire plot for 3d plots
  QString zAxisScale;            // scaling for z axis for 3d plots

  QString ticsLevel;             // tics level for 3d plots

  int d3HiddenLineFlag;         // flag for hidden lines in 3D plots

  QString isoLineU;              // number of isolines in U direction for 3D
  QString isoLineV;              // number of isolines in V direction for 3D

  FILE* gnuCommand;             // named pipe to gnuplot

  gnuCurveFit* curveFitOb;      // curve fitting object

  QString getValue(QTextStream& in);



};

#endif // gnuInterface_included

