/**********************************************************************

	--- Qt Architect generated file ---

	File: barOp.cpp

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

#include "barOp.h"
#include <klocale.h>

barOp::barOp
(
	QWidget* parent,
	const char* name
)
	:
	barOpData( parent, name )
{
	setCaption( i18n("Bar Options") );
}


barOp::~barOp()
{
}

void barOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  QString size = gnuInt->getBarSizeOption();
  
  if (size == "large")
  {
    synList->setCurrentItem(1);
  }
  else if ((size != "") && (size != "small"))
    barSizeEdit->setText(size);
}

void barOp::setBarOption()
{
  QString barSize = barSizeEdit->text();

  if (barSize != "")
  {
    gnuInt->setBarSizeOption(barSize);
  }
  else
  {
    barSize = synList->currentText();
    gnuInt->setBarSizeOption(barSize);
  }
  
  QDialog::accept();
}


#include "barOp.moc"
