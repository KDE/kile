/**********************************************************************

	--- Qt Architect generated file ---

	File: curveFit.h

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

 *********************************************************************/

#ifndef curveFit_included
#define curveFit_included

#include "curveFitData.h"
#include "gnuInterface.h"
#include <qfiledlg.h>

class curveFit : public curveFitData
{
    Q_OBJECT

public:

    curveFit
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

  virtual ~curveFit();
  void setGnuInterface(gnuInterface* gnu);
  void doFit();
  void dataFileOpen();
  void paramFileOpen();
  
private:
  gnuInterface* gnuInt;
  

};
#endif // curveFit_included
