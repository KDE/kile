/* -------------------------- multiFunc class --------------------------

   This class handles all operations related to the storage and manipulation of 
   multiple functions and their options from the GUI.

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

#ifndef multiFunc_included
#define multiFunc_included

#include "multiFuncData.h"
#include "gnuInterface.h"

class multiFunc : public multiFuncData
{
    Q_OBJECT

public:

    multiFunc
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~multiFunc();

  void setGnuInterface(gnuInterface* gnu);
  void insertNewFunction();
  void setFuncOptions();
  void closeMultiFunc();
  void deleteFunction();
  void funcChanged(const QString& func);

private:
  gnuInterface* gnuInt;
};
#endif // multiFunc_included
