/**********************************************************************

	--- Qt Architect generated file ---

	File: logScaleOp.cpp

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
using namespace std;
                               
#include "logScaleOp.h"
#include <qstring.h>
#include <klocale.h>

logScaleOp::logScaleOp
(
	QWidget* parent,
	const char* name
)
	:
	logScaleOpData( parent, name )
{
	setCaption( i18n("Log Scale Options") );
}


logScaleOp::~logScaleOp()
{
}

void logScaleOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  QString tempBase;
  int base;
  int xAxis;
  int yAxis;
  int zAxis;
  int x2Axis;
  int y2Axis;

  base = gnuInt->getLogScaleBase();
  xAxis = gnuInt->getLogScaleXAxis();
  yAxis = gnuInt->getLogScaleYAxis();
  zAxis = gnuInt->getLogScaleZAxis();
  x2Axis = gnuInt->getLogScaleX2Axis();
  y2Axis = gnuInt->getLogScaleY2Axis();

  tempBase = QString::number( base );

  logBaseEdit->setText(tempBase);

  if (xAxis == 1)
    logAxisX->setChecked(TRUE);

  if (yAxis == 1)
    logAxisY->setChecked(TRUE);

  if (zAxis == 1)
    logAxisZ->setChecked(TRUE);

  if (x2Axis == 1)
    logAxisX2->setChecked(TRUE);

  if (y2Axis == 1)
    logAxisY2->setChecked(TRUE);
  
}

void logScaleOp::setLogScaleOp()
{
  QString tempBase;
  int base = 10;
  int xAxis = 0;
  int yAxis = 0;
  int zAxis = 0;
  int x2Axis = 0;
  int y2Axis = 0;
  
  
  // fill in which axes are selected

  if (logAxisX->isChecked() == TRUE)
    xAxis = 1;
  
  if (logAxisY->isChecked() == TRUE)
    yAxis = 1;

  if (logAxisZ->isChecked() == TRUE)
    zAxis = 1;

  if (logAxisX2->isChecked() == TRUE)
    x2Axis = 1;

  if (logAxisY2->isChecked() == TRUE)
    y2Axis = 1;

  // fill in base

  tempBase = logBaseEdit->text();
  base = tempBase.toInt();

  gnuInt->setLogScaleXAxis(xAxis);
  gnuInt->setLogScaleYAxis(yAxis);
  gnuInt->setLogScaleZAxis(zAxis);
  gnuInt->setLogScaleX2Axis(x2Axis);
  gnuInt->setLogScaleY2Axis(y2Axis);
  gnuInt->setLogScaleBase(base);
  
  QDialog::accept();
}

#include "logScaleOp.moc"
