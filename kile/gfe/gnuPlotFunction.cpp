/* -------------------------- gnuPlotFunction class --------------------------
   
   This is a class to create an object to plot functions. It contains all
   variables necessary for plotting function and knows how to issue the correct
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

#include "gnuPlotFunction.h"

gnuPlotFunction::gnuPlotFunction()
{
  function = "";
  styleType = "lines";
  legendTitle = "default";
}

QString gnuPlotFunction::getPlotCmd()
{
  QString plotcmd = "";

  plotcmd += function;

  // insert title for legend
  if (legendTitle == "notitle")
    plotcmd += " notitle";

  if ((legendTitle != "default") && (legendTitle != "notitle"))
  {
    plotcmd += " title ";
    plotcmd += '"';
    plotcmd += legendTitle;
    plotcmd += '"';
  }

  plotcmd += " with ";
  plotcmd += styleType;

  return plotcmd;
}

void gnuPlotFunction::setFunction(QString func)
{
  function = func;
}

QString gnuPlotFunction::getFunction()
{
  return function;
}

void gnuPlotFunction::setFunctionStyleType(QString type)
{
  styleType = type;
}

QString gnuPlotFunction::getFunctionStyleType()
{
  return styleType;
}

void gnuPlotFunction::setLegendTitle(QString title)
{
  legendTitle = title;
}

QString gnuPlotFunction::getLegendTitle()
{
  return legendTitle;
}
