/**********************************************************************

	--- Qt Architect generated file ---

	File: boxWidthOp.h

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

#ifndef boxWidthOp_included
#define boxWidthOp_included

#include "boxWidthOpData.h"
#include "gnuInterface.h"

class boxWidthOp : public boxWidthOpData
{
    Q_OBJECT

public:

    boxWidthOp
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~boxWidthOp();
  void setGnuInterface(gnuInterface* gnu);
  void setBoxWidth();

private:
  gnuInterface* gnuInt;

};
#endif // boxWidthOp_included
