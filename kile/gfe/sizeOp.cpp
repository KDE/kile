/**********************************************************************

	--- Qt Architect generated file ---

	File: sizeOp.cpp

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

#include "sizeOp.h"
#include <klocale.h>

sizeOp::sizeOp
(
	QWidget* parent,
	const char* name
)
	:
	sizeOpData( parent, name )
{
	setCaption( i18n("Plot Size") );
}


sizeOp::~sizeOp()
{
}

void sizeOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // get and load current size
  QString vSize = gnuInt->getVertSize();
  QString hSize = gnuInt->getHorizSize();

  vSizeEdit->setText(vSize);
  hSizeEdit->setText(hSize);
}

void sizeOp::setSize()
{
  QString hSize = hSizeEdit->text();
  QString vSize = vSizeEdit->text();

  gnuInt->setHorizSize(hSize);
  gnuInt->setVertSize(vSize);

  // close option window
  QDialog::accept();
}

#include "sizeOp.moc"
