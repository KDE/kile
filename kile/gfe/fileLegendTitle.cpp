/**********************************************************************

	--- Qt Architect generated file ---

	File: fileLegendTitle.cpp

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

#include "fileLegendTitle.h"
#include <klocale.h>

fileLegendTitle::fileLegendTitle
(
	QWidget* parent,
	const char* name
)
	:
	fileLegendTitleData( parent, name )
{
	setCaption( i18n("File Legend Title") );
}


fileLegendTitle::~fileLegendTitle()
{
}

void fileLegendTitle::setFileLegendTitleOK()
{
  if (defaultCButton->isChecked() == TRUE)
    gnuInt->setFileLegendTitle("default");

  if (noTitleCButton->isChecked() == TRUE)
    gnuInt->setFileLegendTitle("notitle");

  QString title = fileLegendTitleEdit->text();
  if (title!="") gnuInt->setFileLegendTitle(title);

  QDialog::accept();
}

void fileLegendTitle::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;
  
  QString title = gnuInt->getFileLegendTitle();

  if ((title != "default") && (title != "notitle"))
    fileLegendTitleEdit->setText(title);

  if (title == "default")
  {
    defaultCButton->setChecked(TRUE);
    noTitleCButton->setChecked(FALSE);
  }
  else if (title == "notitle")
  {
    defaultCButton->setChecked(FALSE);
    noTitleCButton->setChecked(TRUE);
  }

  if ((title != "default") && (title != "notitle"))
  {
    defaultCButton->setChecked(FALSE);
    noTitleCButton->setChecked(FALSE);
  }
}

#include "fileLegendTitle.moc"
