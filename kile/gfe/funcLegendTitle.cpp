/**********************************************************************

	--- Qt Architect generated file ---

	File: funcLegendTitle.cpp

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

#include "funcLegendTitle.h"

funcLegendTitle::funcLegendTitle
(
	QWidget* parent,
	const char* name
)
	:
	funcLegendTitleData( parent, name )
{
}


funcLegendTitle::~funcLegendTitle()
{
}

void funcLegendTitle::setFuncLegendTitleOK()
{
  if (defaultCButton->isChecked() == TRUE)
    gnuInt->setFuncLegendTitle("default");

  if (notitleCButton->isChecked() == TRUE)
    gnuInt->setFuncLegendTitle("notitle");

  QString title = funcLegendTitleEdit->text();
  if (title!="") gnuInt->setFuncLegendTitle(title);

  QDialog::accept();
}

void funcLegendTitle::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;
  
  QString title = gnuInt->getFuncLegendTitle();

  if ((title != "default") && (title != "notitle"))
    funcLegendTitleEdit->setText(title);

  if (title == "default")
  {
    defaultCButton->setChecked(TRUE);
    notitleCButton->setChecked(FALSE);
  }
  else if (title == "notitle")
  {
    defaultCButton->setChecked(FALSE);
    notitleCButton->setChecked(TRUE);
  }

  if ((title != "default") && (title != "notitle"))
  {
    defaultCButton->setChecked(FALSE);
    notitleCButton->setChecked(FALSE);
  }
  
}

#include "funcLegendTitle.moc"
