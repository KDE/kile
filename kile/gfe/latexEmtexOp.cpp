/**********************************************************************

	--- Qt Architect generated file ---

	File: latexEmtexOp.cpp

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

#include "latexEmtexOp.h"
#include <klocale.h>

latexEmtexOp::latexEmtexOp
(
	QWidget* parent,
	const char* name
)
	:
	latexEmtexOpData( parent, name )
{
	setCaption( i18n("LaTeX/EmTeX Options") );
}


latexEmtexOp::~latexEmtexOp()
{
}

void latexEmtexOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // get and load current options
  QString font = gnuInt->getTermLateXEmtexFont();
  QString size = gnuInt->getTermLateXEmtexFontSize();

  if (font == "roman")
    FontList->setCurrentItem(0);

  if (font == "courier")
    FontList->setCurrentItem(1);

  if (size == "10")
    FontSizeList->setCurrentItem(0);
  else if (size == "11")
    FontSizeList->setCurrentItem(1);
  else if (size == "12")
    FontSizeList->setCurrentItem(2);
  else
    FontSizeOther->setText(size);

}

void latexEmtexOp::setTerm()
{
  // set options
  QString font = FontList->currentText();
  QString size = FontSizeList->currentText();
  QString otherSize = FontSizeOther->text();

  gnuInt->setTermLateXEmtexFont(font);
    
  if (otherSize == "")
    gnuInt->setTermLateXEmtexFontSize(size);
  else
    gnuInt->setTermLateXEmtexFontSize(otherSize);
  
  // close dialog for options
  QDialog::accept();
}


#include "latexEmtexOp.moc"
