/**********************************************************************

	--- Qt Architect generated file ---

   This class sets the bar options

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

 *********************************************************************/
using namespace std;

#ifndef barOp_included
#define barOp_included

#include "barOpData.h"
#include "gnuInterface.h"
#include <qstring.h>

class barOp : public barOpData
{
    Q_OBJECT

public:

    barOp
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~barOp();

  void setGnuInterface(gnuInterface* gnu);
  void setBarOption();

private:
  gnuInterface* gnuInt;

};
#endif // barOp_included
