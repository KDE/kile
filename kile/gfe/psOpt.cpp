/**********************************************************************

	--- Qt Architect generated file ---

	File: psOpt.cpp

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

#include "psOpt.h"
#include <klocale.h>

psOpt::psOpt
(
	QWidget* parent,
	const char* name
)
	:
	psOptData( parent, name )
{
	setCaption( i18n("PostScript Options") );
}


psOpt::~psOpt()
{
}

void psOpt::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // get options and fill in GUI
  QString mode = gnuInt->getTermPSmode();
  QString color = gnuInt->getTermPScolor();
  QString dashed = gnuInt->getTermPSdashed();
  QString fontName = gnuInt->getTermPSfont();
  QString fontSize = gnuInt->getTermPSfontSize();
  QString enhanced = gnuInt->getTermPSenhanced();
  QString hSize = gnuInt->getTermPShSize();
  QString vSize = gnuInt->getTermPSvSize();

  if (mode == "landscape")
    modeList->setCurrentItem(0);
  else if (mode == "portrait")
    modeList->setCurrentItem(1);
  else if (mode == "eps")
    modeList->setCurrentItem(2);

  if (color == "monochrome")
    colorList->setCurrentItem(0);
  else if (color == "color")
    colorList->setCurrentItem(1);

  if (dashed == "dashed")
    dashedList->setCurrentItem(0);
  else if (dashed == "solid")
    dashedList->setCurrentItem(1);

  fontNameEdit->setText(fontName);
  fontSizeEdit->setText(fontSize);

  if (enhanced == "noenhanced")
    enhancedList->setCurrentItem(0);
  else if (enhanced == "enhanced")
    enhancedList->setCurrentItem(1);

  horizSize->setText(hSize);
  vertSize->setText(vSize);
}

void psOpt::setTerm()
{
  QString mode = modeList->currentText();
  QString color = colorList->currentText();
  QString dashed = dashedList->currentText();
  QString enhanced = enhancedList->currentText();
  QString fontName = fontNameEdit->text();
  QString fontSize = fontSizeEdit->text();
  QString hSize = horizSize->text();
  QString vSize = vertSize->text();

  gnuInt->setTermPSmode(mode);
  gnuInt->setTermPScolor(color);
  gnuInt->setTermPSdashed(dashed);
  gnuInt->setTermPSfont(fontName);
  gnuInt->setTermPSfontSize(fontSize);
  gnuInt->setTermPSenhanced(enhanced);
  gnuInt->setTermPShSize(hSize);
  gnuInt->setTermPSvSize(vSize);

  // close dialog for options
  QDialog::accept();
}

#include "psOpt.moc"
