/**********************************************************************

	--- Qt Architect generated file ---

	File: isoLinesOp.h

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 *********************************************************************/

#ifndef isoLinesOp_included
#define isoLinesOp_included

#include "isoLinesOpData.h"
#include "gnuInterface.h"

class isoLinesOp : public isoLinesOpData
{
    Q_OBJECT

public:

    isoLinesOp
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~isoLinesOp();

  void setGnuInterface(gnuInterface* gnu);
  void setIsolineDefaults();
  void setIsolinesOp();

private:
  gnuInterface* gnuInt;

};
#endif // isoLinesOp_included
