/**********************************************************************

	--- Qt Architect generated file ---

	File: ticsOp.cpp

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

#include <klocale.h>
#include "ticsOp.h"

ticsOp::ticsOp
(
	QWidget* parent,
	const char* name
)
	:
	ticsOpData( parent, name )
{
	setCaption( i18n("Tics Options") );
}


ticsOp::~ticsOp()
{
}

void ticsOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // get options
  int xticsOnFlag = gnuInt->getXticsOnFlag();
  QString xticsLocation = gnuInt->getXticsLocation();
  QString xticsMirror = gnuInt->getXticsMirror();
  QString xticsRotation = gnuInt->getXticsRotation();
  QString xticsPositionType = gnuInt->getXticsPositionType();
  QString xticsStartPos = gnuInt->getXticsStartPos();
  QString xticsIncPos = gnuInt->getXticsIncPos();
  QString xticsEndPos = gnuInt->getXticsEndPos();
  QString xticsLabelsPos = gnuInt->getXticsLabelsPos();

  int yticsOnFlag = gnuInt->getYticsOnFlag();
  QString yticsLocation = gnuInt->getYticsLocation();
  QString yticsMirror = gnuInt->getYticsMirror();
  QString yticsRotation = gnuInt->getYticsRotation();
  QString yticsPositionType = gnuInt->getYticsPositionType();
  QString yticsStartPos = gnuInt->getYticsStartPos();
  QString yticsIncPos = gnuInt->getYticsIncPos();
  QString yticsEndPos = gnuInt->getYticsEndPos();
  QString yticsLabelsPos = gnuInt->getYticsLabelsPos();

  int zticsOnFlag = gnuInt->getZticsOnFlag();
  QString zticsMirror = gnuInt->getZticsMirror();
  QString zticsRotation = gnuInt->getZticsRotation();
  QString zticsPositionType = gnuInt->getZticsPositionType();
  QString zticsStartPos = gnuInt->getZticsStartPos();
  QString zticsIncPos = gnuInt->getZticsIncPos();
  QString zticsEndPos = gnuInt->getZticsEndPos();
  QString zticsLabelsPos = gnuInt->getZticsLabelsPos();

  int x2ticsOnFlag = gnuInt->getX2ticsOnFlag();
  QString x2ticsLocation = gnuInt->getX2ticsLocation();
  QString x2ticsMirror = gnuInt->getX2ticsMirror();
  QString x2ticsRotation = gnuInt->getX2ticsRotation();
  QString x2ticsPositionType = gnuInt->getX2ticsPositionType();
  QString x2ticsStartPos = gnuInt->getX2ticsStartPos();
  QString x2ticsIncPos = gnuInt->getX2ticsIncPos();
  QString x2ticsEndPos = gnuInt->getX2ticsEndPos();
  QString x2ticsLabelsPos = gnuInt->getX2ticsLabelsPos();

  int y2ticsOnFlag = gnuInt->getY2ticsOnFlag();
  QString y2ticsLocation = gnuInt->getY2ticsLocation();
  QString y2ticsMirror = gnuInt->getY2ticsMirror();
  QString y2ticsRotation = gnuInt->getY2ticsRotation();
  QString y2ticsPositionType = gnuInt->getY2ticsPositionType();
  QString y2ticsStartPos = gnuInt->getY2ticsStartPos();
  QString y2ticsIncPos = gnuInt->getY2ticsIncPos();
  QString y2ticsEndPos = gnuInt->getY2ticsEndPos();
  QString y2ticsLabelsPos = gnuInt->getY2ticsLabelsPos();

  // set xtics options
  if (xticsOnFlag == 1)
  {
    xticsOnRButton->setChecked(TRUE);
    xticsOffRButton->setChecked(FALSE);
  }
  else if (xticsOnFlag == 0)
  {
    xticsOnRButton->setChecked(FALSE);
    xticsOffRButton->setChecked(TRUE);
  }

  if (xticsLocation == "border")
    xticsLocationCBox->setCurrentItem(0);

  if (xticsLocation == "axis")
    xticsLocationCBox->setCurrentItem(1);

  if (xticsMirror == "mirror")
    xticsMirrorCBox->setCurrentItem(0);

  if (xticsMirror == "nomirror")
    xticsMirrorCBox->setCurrentItem(1);

  if (xticsRotation == "norotate")
    xticsRotationCBox->setCurrentItem(0);

  if (xticsRotation == "rotate")
    xticsRotationCBox->setCurrentItem(1);

  if (xticsPositionType == "SIE")
  {
    xticsSIERadioButton->setChecked(TRUE);
    xticsLabelPosRButton->setChecked(FALSE);

    xticsStartPosEdit->setText(xticsStartPos);
    xticsIncPosEdit->setText(xticsIncPos);
    xticsEndPosEdit->setText(xticsEndPos);
  }
  else if (xticsPositionType == "LABELS")
  {
    xticsSIERadioButton->setChecked(FALSE);
    xticsLabelPosRButton->setChecked(TRUE);

    xticsLabelsPosEdit->setText(xticsLabelsPos);
  }

  // set ytics options
  if (yticsOnFlag == 1)
  {
    yticsOnRButton->setChecked(TRUE);
    yticsOffRButton->setChecked(FALSE);
  }
  else if (yticsOnFlag == 0)
  {
    yticsOnRButton->setChecked(FALSE);
    yticsOffRButton->setChecked(TRUE);
  }

  if (yticsLocation == "border")
    yticsLocationCBox->setCurrentItem(0);

  if (yticsLocation == "axis")
    yticsLocationCBox->setCurrentItem(1);

  if (yticsMirror == "mirror")
    yticsMirrorCBox->setCurrentItem(0);

  if (yticsMirror == "nomirror")
    yticsMirrorCBox->setCurrentItem(1);

  if (yticsRotation == "norotate")
    yticsRotationCBox->setCurrentItem(0);

  if (yticsRotation == "rotate")
    yticsRotationCBox->setCurrentItem(1);

  if (yticsPositionType == "SIE")
  {
    yticsSIERadioButton->setChecked(TRUE);
    yticsLabelPosRButton->setChecked(FALSE);

    yticsStartPosEdit->setText(yticsStartPos);
    yticsIncPosEdit->setText(yticsIncPos);
    yticsEndPosEdit->setText(yticsEndPos);
  }
  else if (yticsPositionType == "LABELS")
  {
    yticsSIERadioButton->setChecked(FALSE);
    yticsLabelPosRButton->setChecked(TRUE);

    yticsLabelsPosEdit->setText(yticsLabelsPos);
  }

  // set ztics options
  if (zticsOnFlag == 1)
  {
    zticsOnRButton->setChecked(TRUE);
    zticsOffRButton->setChecked(FALSE);
  }
  else if (zticsOnFlag == 0)
  {
    zticsOnRButton->setChecked(FALSE);
    zticsOffRButton->setChecked(TRUE);
  }

  if (zticsMirror == "mirror")
    zticsMirrorCBox->setCurrentItem(0);

  if (zticsMirror == "nomirror")
    zticsMirrorCBox->setCurrentItem(1);

  if (zticsRotation == "norotate")
    zticsRotationCBox->setCurrentItem(0);

  if (zticsRotation == "rotate")
    zticsRotationCBox->setCurrentItem(1);

  if (zticsPositionType == "SIE")
  {
    zticsSIERadioButton->setChecked(TRUE);
    zticsLabelPosRButton->setChecked(FALSE);

    zticsStartPosEdit->setText(zticsStartPos);
    zticsIncPosEdit->setText(zticsIncPos);
    zticsEndPosEdit->setText(zticsEndPos);
  }
  else if (zticsPositionType == "LABELS")
  {
    zticsSIERadioButton->setChecked(FALSE);
    zticsLabelPosRButton->setChecked(TRUE);

    zticsLabelsPosEdit->setText(zticsLabelsPos);
  }

  // set x2tics options
  if (x2ticsOnFlag == 1)
  {
    x2ticsOnRButton->setChecked(TRUE);
    x2ticsOffRButton->setChecked(FALSE);
  }
  else if (x2ticsOnFlag == 0)
  {
    x2ticsOnRButton->setChecked(FALSE);
    x2ticsOffRButton->setChecked(TRUE);
  }

  if (x2ticsLocation == "border")
    x2ticsLocationCBox->setCurrentItem(0);

  if (x2ticsLocation == "axis")
    x2ticsLocationCBox->setCurrentItem(1);

  if (x2ticsMirror == "mirror")
    x2ticsMirrorCBox->setCurrentItem(0);

  if (x2ticsMirror == "nomirror")
    x2ticsMirrorCBox->setCurrentItem(1);

  if (x2ticsRotation == "norotate")
    x2ticsRotationCBox->setCurrentItem(0);

  if (x2ticsRotation == "rotate")
    x2ticsRotationCBox->setCurrentItem(1);

  if (x2ticsPositionType == "SIE")
  {
    x2ticsSIERadioButton->setChecked(TRUE);
    x2ticsLabelPosRButton->setChecked(FALSE);

    x2ticsStartPosEdit->setText(x2ticsStartPos);
    x2ticsIncPosEdit->setText(x2ticsIncPos);
    x2ticsEndPosEdit->setText(x2ticsEndPos);
  }
  else if (x2ticsPositionType == "LABELS")
  {
    x2ticsSIERadioButton->setChecked(FALSE);
    x2ticsLabelPosRButton->setChecked(TRUE);

    x2ticsLabelsPosEdit->setText(x2ticsLabelsPos);
  }

  // set y2tics options
  if (y2ticsOnFlag == 1)
  {
    y2ticsOnRButton->setChecked(TRUE);
    y2ticsOffRButton->setChecked(FALSE);
  }
  else if (y2ticsOnFlag == 0)
  {
    y2ticsOnRButton->setChecked(FALSE);
    y2ticsOffRButton->setChecked(TRUE);
  }

  if (y2ticsLocation == "border")
    y2ticsLocationCBox->setCurrentItem(0);

  if (y2ticsLocation == "axis")
    y2ticsLocationCBox->setCurrentItem(1);

  if (y2ticsMirror == "mirror")
    y2ticsMirrorCBox->setCurrentItem(0);

  if (y2ticsMirror == "nomirror")
    y2ticsMirrorCBox->setCurrentItem(1);

  if (y2ticsRotation == "norotate")
    y2ticsRotationCBox->setCurrentItem(0);

  if (y2ticsRotation == "rotate")
    y2ticsRotationCBox->setCurrentItem(1);

  if (y2ticsPositionType == "SIE")
  {
    y2ticsSIERadioButton->setChecked(TRUE);
    y2ticsLabelPosRButton->setChecked(FALSE);

    y2ticsStartPosEdit->setText(y2ticsStartPos);
    y2ticsIncPosEdit->setText(y2ticsIncPos);
    y2ticsEndPosEdit->setText(y2ticsEndPos);
  }
  else if (y2ticsPositionType == "LABELS")
  {
    y2ticsSIERadioButton->setChecked(FALSE);
    y2ticsLabelPosRButton->setChecked(TRUE);

    y2ticsLabelsPosEdit->setText(y2ticsLabelsPos);
  }

}

void ticsOp::setTicsOptions()
{
  // get and set options

  // xtics
  if (xticsOnRButton->isChecked() == TRUE)
    gnuInt->setXticsOnFlag(1);

  if (xticsOnRButton->isChecked() == FALSE)
    gnuInt->setXticsOnFlag(0);

  QString xticsLocation = xticsLocationCBox->currentText();
  QString xticsMirror = xticsMirrorCBox->currentText();
  QString xticsRotation = xticsRotationCBox->currentText();

  QString xticsPositionType;

  if (xticsSIERadioButton->isChecked() == TRUE)
    xticsPositionType = "SIE";

  if (xticsLabelPosRButton->isChecked() == TRUE)
    xticsPositionType = "LABELS";

  QString xticsStartPos = xticsStartPosEdit->text();
  QString xticsIncPos = xticsIncPosEdit->text();
  QString xticsEndPos = xticsEndPosEdit->text();
  QString xticsLabelsPos = xticsLabelsPosEdit->text();

  gnuInt->setXticsLocation(xticsLocation);
  gnuInt->setXticsMirror(xticsMirror);
  gnuInt->setXticsRotation(xticsRotation);
  gnuInt->setXticsPositionType(xticsPositionType);
  gnuInt->setXticsStartPos(xticsStartPos);
  gnuInt->setXticsIncPos(xticsIncPos);
  gnuInt->setXticsEndPos(xticsEndPos);
  gnuInt->setXticsLabelsPos(xticsLabelsPos);

  // ytics
  if (yticsOnRButton->isChecked() == TRUE)
    gnuInt->setYticsOnFlag(1);

  if (yticsOnRButton->isChecked() == FALSE)
    gnuInt->setYticsOnFlag(0);

  QString yticsLocation = yticsLocationCBox->currentText();
  QString yticsMirror = yticsMirrorCBox->currentText();
  QString yticsRotation = yticsRotationCBox->currentText();

  QString yticsPositionType;

  if (yticsSIERadioButton->isChecked() == TRUE)
    yticsPositionType = "SIE";

  if (yticsLabelPosRButton->isChecked() == TRUE)
    yticsPositionType = "LABELS";

  QString yticsStartPos = yticsStartPosEdit->text();
  QString yticsIncPos = yticsIncPosEdit->text();
  QString yticsEndPos = yticsEndPosEdit->text();
  QString yticsLabelsPos = yticsLabelsPosEdit->text();

  gnuInt->setYticsLocation(yticsLocation);
  gnuInt->setYticsMirror(yticsMirror);
  gnuInt->setYticsRotation(yticsRotation);
  gnuInt->setYticsPositionType(yticsPositionType);
  gnuInt->setYticsStartPos(yticsStartPos);
  gnuInt->setYticsIncPos(yticsIncPos);
  gnuInt->setYticsEndPos(yticsEndPos);
  gnuInt->setYticsLabelsPos(yticsLabelsPos);

  // ztics
  if (zticsOnRButton->isChecked() == TRUE)
    gnuInt->setZticsOnFlag(1);

  if (zticsOnRButton->isChecked() == FALSE)
    gnuInt->setZticsOnFlag(0);

  QString zticsMirror = zticsMirrorCBox->currentText();
  QString zticsRotation = zticsRotationCBox->currentText();

  QString zticsPositionType;

  if (zticsSIERadioButton->isChecked() == TRUE)
    zticsPositionType = "SIE";

  if (zticsLabelPosRButton->isChecked() == TRUE)
    zticsPositionType = "LABELS";

  QString zticsStartPos = zticsStartPosEdit->text();
  QString zticsIncPos = zticsIncPosEdit->text();
  QString zticsEndPos = zticsEndPosEdit->text();
  QString zticsLabelsPos = zticsLabelsPosEdit->text();

  gnuInt->setZticsMirror(zticsMirror);
  gnuInt->setZticsRotation(zticsRotation);
  gnuInt->setZticsPositionType(zticsPositionType);
  gnuInt->setZticsStartPos(zticsStartPos);
  gnuInt->setZticsIncPos(zticsIncPos);
  gnuInt->setZticsEndPos(zticsEndPos);
  gnuInt->setZticsLabelsPos(zticsLabelsPos);

  // x2tics
  if (x2ticsOnRButton->isChecked() == TRUE)
    gnuInt->setX2ticsOnFlag(1);

  if (x2ticsOnRButton->isChecked() == FALSE)
    gnuInt->setX2ticsOnFlag(0);

  QString x2ticsLocation = x2ticsLocationCBox->currentText();
  QString x2ticsMirror = x2ticsMirrorCBox->currentText();
  QString x2ticsRotation = x2ticsRotationCBox->currentText();

  QString x2ticsPositionType;

  if (x2ticsSIERadioButton->isChecked() == TRUE)
    x2ticsPositionType = "SIE";

  if (x2ticsLabelPosRButton->isChecked() == TRUE)
    x2ticsPositionType = "LABELS";

  QString x2ticsStartPos = x2ticsStartPosEdit->text();
  QString x2ticsIncPos = x2ticsIncPosEdit->text();
  QString x2ticsEndPos = x2ticsEndPosEdit->text();
  QString x2ticsLabelsPos = x2ticsLabelsPosEdit->text();

  gnuInt->setX2ticsLocation(x2ticsLocation);
  gnuInt->setX2ticsMirror(x2ticsMirror);
  gnuInt->setX2ticsRotation(x2ticsRotation);
  gnuInt->setX2ticsPositionType(x2ticsPositionType);
  gnuInt->setX2ticsStartPos(x2ticsStartPos);
  gnuInt->setX2ticsIncPos(x2ticsIncPos);
  gnuInt->setX2ticsEndPos(x2ticsEndPos);
  gnuInt->setX2ticsLabelsPos(x2ticsLabelsPos);

  // y2tics
  if (y2ticsOnRButton->isChecked() == TRUE)
    gnuInt->setY2ticsOnFlag(1);

  if (y2ticsOnRButton->isChecked() == FALSE)
    gnuInt->setY2ticsOnFlag(0);

  QString y2ticsLocation = y2ticsLocationCBox->currentText();
  QString y2ticsMirror = y2ticsMirrorCBox->currentText();
  QString y2ticsRotation = y2ticsRotationCBox->currentText();

  QString y2ticsPositionType;

  if (y2ticsSIERadioButton->isChecked() == TRUE)
    y2ticsPositionType = "SIE";

  if (y2ticsLabelPosRButton->isChecked() == TRUE)
    y2ticsPositionType = "LABELS";

  QString y2ticsStartPos = y2ticsStartPosEdit->text();
  QString y2ticsIncPos = y2ticsIncPosEdit->text();
  QString y2ticsEndPos = y2ticsEndPosEdit->text();
  QString y2ticsLabelsPos = y2ticsLabelsPosEdit->text();

  gnuInt->setY2ticsLocation(y2ticsLocation);
  gnuInt->setY2ticsMirror(y2ticsMirror);
  gnuInt->setY2ticsRotation(y2ticsRotation);
  gnuInt->setY2ticsPositionType(y2ticsPositionType);
  gnuInt->setY2ticsStartPos(y2ticsStartPos);
  gnuInt->setY2ticsIncPos(y2ticsIncPos);
  gnuInt->setY2ticsEndPos(y2ticsEndPos);
  gnuInt->setY2ticsLabelsPos(y2ticsLabelsPos);
}

#include "ticsOp.moc"
