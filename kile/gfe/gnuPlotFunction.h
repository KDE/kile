/* -------------------------- gnuPlotFunction class --------------------------
   
   This is a class to create an object to plot functions. It contains all
   variables necessary for plotting functions and knows how to issue the correct
   command to gnuPlot to plot the function with its options. 

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

#ifndef gnuPlotFunction_included
#define gnuPlotFunction_included

using namespace std;

#include <qstring.h>
#include <iostream>

class gnuPlotFunction
{
public:
  gnuPlotFunction();
  /* Description:
       Constructor function */

  QString getPlotCmd();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: plotting command for the function
     Description:
       Builds a QString to plot the function */

  void setFunction(QString func);
  /* Incoming arguments:
       QString function: Function to plot
     Outgoing arguments:
       none
     Description:
       Sets the function to plot */

  QString getFunction();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: current function
     Description:
       Gets the function to plot */

  void setFunctionStyleType(QString type);
  /* Incoming arguments:
       QString type: style of plotting. With Gnuplot 3.5 the following types
       are supported: points, lines, linespoints, impulses, dots, steps,
       errorbars, and boxes. Others can be specified if Gnuplot will
       understand it.
     Outgoing arguments:
       none
     Description:
       Sets the style of plotting the function */

QString getFunctionStyleType();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: function style option
     Description:
       Gets the style of plotting the function */

  void setLegendTitle(QString title);
  /* Incoming arguments:
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend */

  QString getLegendTitle();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: title for current function
     Description:
       Gets title to be used in legend for the function */

private:

  QString function;
  QString styleType;
  QString legendTitle;
};

#endif // gnuPlotFunction_included
