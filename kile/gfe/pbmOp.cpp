/**********************************************************************

	--- Qt Architect generated file ---

	File: pbmOp.cpp

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

#include "pbmOp.h"

pbmOp::pbmOp
(
	QWidget* parent,
	const char* name
)
	:
	pbmOpData( parent, name )
{
	setCaption( "PBM Options" );
}


pbmOp::~pbmOp()
{
}

void pbmOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;  

  // get current options and fill in GUI
  QString fontsize = gnuInt->getTermPBMFontSize();
  QString colormode = gnuInt->getTermPBMColormode();
  QString hSize = gnuInt->getTermPBMhSize();
  QString vSize = gnuInt->getTermPBMvSize();

  if (fontsize == "small")
    fontSize->setCurrentItem(0);
  else if (fontsize == "medium")
    fontSize->setCurrentItem(1);
  else if (fontsize == "large")
    fontSize->setCurrentItem(2);

  if (colormode == "monochrome")
    colorMode->setCurrentItem(0);
  else if (colormode == "gray")
    colorMode->setCurrentItem(1);
  else if (colormode == "color")
    colorMode->setCurrentItem(2);

  horizSize->setText(hSize);
  vertSize->setText(vSize);
}

void pbmOp::setTerm()
{
  // get options
  QString size = fontSize->currentText();
  QString colormode = colorMode->currentText();
  QString hSize = horizSize->text();
  QString vSize = vertSize->text();

  gnuInt->setTermPBMFontSize(size);
  gnuInt->setTermPBMColormode(colormode);
  gnuInt->setTermPBMhSize(hSize);
  gnuInt->setTermPBMvSize(vSize);
  
  // close dialog for options
  QDialog::accept();
}


#include "pbmOp.moc"
