/**********************************************************************

	--- Qt Architect generated file ---

	File: rotation.cpp

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

#include "rotation.h"
#include <klocale.h>

rotation::rotation
(
	QWidget* parent,
	const char* name
)
	:
	rotationData( parent, name )
{
	setCaption( i18n("3D Rotation") );
}


rotation::~rotation()
{
}

void rotation::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  int xAxisValue = gnuInt->getRotationXAxis();
  int zAxisValue = gnuInt->getRotationZAxis();
  QString scale = gnuInt->getRotationScaling();
  QString zscale = gnuInt->getRotationZAxisScaling();

  xAxis->setValue(xAxisValue);
  xAxisLCD->display(xAxisValue);
  zAxis->setValue(zAxisValue);
  zAxisLCD->display(zAxisValue);
  plotScalingEdit->setText(scale);
  zAxisScalingEdit->setText(zscale);
}

void rotation::doOK()
{
  QString scale = plotScalingEdit->text();
  QString zscale = zAxisScalingEdit->text();

  gnuInt->setRotationXAxis(xAxisRotation);
  gnuInt->setRotationZAxis(zAxisRotation);
  gnuInt->setRotationScaling(scale);
  gnuInt->setRotationZAxisScaling(zscale);

  QDialog::accept();
}

void rotation::setDefaults()
{
  xAxis->setValue(60);
  xAxisLCD->display(60);
  zAxis->setValue(30);
  zAxisLCD->display(30);
  plotScalingEdit->setText("1");
  zAxisScalingEdit->setText("1");
}

void rotation::xAxisChanged(int num)
{
  xAxisLCD->display(num);
  xAxisRotation = num;
}

void rotation::zAxisChanged(int num)
{
  zAxisLCD->display(num);
  zAxisRotation = num;
}

#include "rotation.moc"
