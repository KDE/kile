/* -------------------------- gnuMultiFunc class --------------------------

   This class handles all operations related to the storage and retrieval of 
   multiple functions and their options. These should be called from
   gnuInterface. This class allows you to implement the storage in any way
   you choose. 

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
using namespace std;

#ifndef gnuMultiFunc_included
#define gnuMultiFunc_included

#include <qstring.h>
#include "gnuPlotFunction.h"
#include <qdict.h> // Qt's dictionary data structure

class gnuMultiFunc
{
public:
  gnuMultiFunc();
  // Constructor function

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

  void setLegendTitle(QString function, QString title);
  /* Incoming arguments:
       function: function to operate on
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend */

  QString getLegendTitle(QString function);
  /* Incoming arguments:
       function: function to operate on
     Outgoing arguments:
       QString: title for current function
     Description:
       Gets title to be used in legend for the current function */

private:
  QDict<gnuPlotFunction>* funcList;
  QDictIterator<gnuPlotFunction>* funcListIterator;
  gnuPlotFunction* tempFunc;
};

#endif // gnuMultiFunc_included
