/* -------------------------- gnuMultiFunc class --------------------------

   This class handles all operations related to the storage and retrieval of 
   multiple functions and their options. These should be called from
   gnuInterface. 

   It is currently implemented with Qt's dictionary datastructure for keeping
   up with a list of plotFileOb's. 

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

#include "gnuMultiFunc.h"

gnuMultiFunc::gnuMultiFunc()
{
  // create new function list
  funcList = new QDict<gnuPlotFunction>(101,TRUE); // max 100 elements
  funcList->setAutoDelete(TRUE); // autodelete members when removed

  // create new iterator
  funcListIterator = new QDictIterator<gnuPlotFunction>(*funcList);
}

void gnuMultiFunc::insertMultiFuncNew(QString function)
{
  gnuPlotFunction* thisFunc = new gnuPlotFunction; // create a new plot function

  thisFunc->setFunction(function); // set function
  funcList->insert(function,thisFunc); // insert into list
}

void gnuMultiFunc::removeMultiFunc(QString function)
{
  funcList->remove(function);
}

void gnuMultiFunc::setMultiFuncStyleOption(QString function, QString style)
{
  tempFunc = (*funcList)[function];
  tempFunc->setFunctionStyleType(style);
}

QString gnuMultiFunc::getMultiFuncStyleOption(QString function)
{
  tempFunc = (*funcList)[function];
  return tempFunc->getFunctionStyleType();
}

QString gnuMultiFunc::getMultiFuncFirstFunction()
{
  // set iterator to first element
  tempFunc = funcListIterator->toFirst();

  // check for error (empty list = null)
  if (tempFunc == 0)
    return "END";
  else
  {
    return tempFunc->getFunction();
  }

}

QString gnuMultiFunc::getMultiFuncNextFunction()
{
  // increment function list iterator
  tempFunc = ++(*funcListIterator);

  // check for error (end of list = null)
  if (tempFunc == 0)
    return "END";
  else
  {
    // get and return function
    return tempFunc->getFunction();
  }
}

QString gnuMultiFunc::getMultiFuncFirstPlotCmd()
{
  // set iterator to first element
  tempFunc = funcListIterator->toFirst();

  // check for error (empty list = null)
  if (tempFunc == 0)
    return "END";
  else
  {
    // get plot command
    return tempFunc->getPlotCmd();
  }
}

QString gnuMultiFunc::getMultiFuncNextPlotCmd()
{
  // increment function list iterator
  tempFunc = ++(*funcListIterator);

  // check for error (end of list = null)
  if (tempFunc == 0)
    return "END";
  else
  {
    // get and return function plotting command
    return tempFunc->getPlotCmd();
  }
}

void gnuMultiFunc::setLegendTitle(QString function, QString title)
{
  tempFunc = (*funcList)[function];
  tempFunc->setLegendTitle(title);
}

QString gnuMultiFunc::getLegendTitle(QString function)
{
  tempFunc = (*funcList)[function];
  return tempFunc->getLegendTitle();
}
