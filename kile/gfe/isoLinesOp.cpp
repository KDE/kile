/**********************************************************************

	--- Qt Architect generated file ---

	File: isoLinesOp.cpp

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

#include "isoLinesOp.h"
#include <klocale.h>

isoLinesOp::isoLinesOp
(
	QWidget* parent,
	const char* name
)
	:
	isoLinesOpData( parent, name )
{
	setCaption( i18n("Isoline Options") );
}


isoLinesOp::~isoLinesOp()
{
}

void isoLinesOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // get options
  QString isoU = gnuInt->getIsolineU();
  QString isoV = gnuInt->getIsolineV();

  isoUEdit->setText(isoU);
  isoVEdit->setText(isoV);
}

void isoLinesOp::setIsolineDefaults()
{
  isoUEdit->setText("10");
  isoVEdit->setText("10");
}

void isoLinesOp::setIsolinesOp()
{
  QString isoU = isoUEdit->text();
  QString isoV = isoVEdit->text();
  
  gnuInt->setIsolineU(isoU);
  gnuInt->setIsolineV(isoV);

  QDialog::accept();
}

#include "isoLinesOp.moc"
