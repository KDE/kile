/**********************************************************************

	--- Qt Architect generated file ---

	File: ticsOp.h

    Xgfe: X Windows GUI front end to Gnuplot
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

#ifndef ticsOp_included
#define ticsOp_included

using namespace std;

#include "ticsOpData.h"
#include "gnuInterface.h"
#include <iostream>
#include <qstring.h>

class ticsOp : public ticsOpData
{
    Q_OBJECT

public:

    ticsOp
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~ticsOp();

  void setGnuInterface(gnuInterface* gnu);
  void setTicsOptions();

private:
  gnuInterface* gnuInt;

};
#endif // ticsOp_included
