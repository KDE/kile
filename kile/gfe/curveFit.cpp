/**********************************************************************

	--- Qt Architect generated file ---

	File: curveFit.cpp

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

#include "curveFit.h"
#include <qstring.h>
#include <klocale.h>
#include <kfiledialog.h>

curveFit::curveFit
(
	QWidget* parent,
	const char* name
)
	:
	curveFitData( parent, name )
{
	setCaption( "Curve Fitting" );
}


curveFit::~curveFit()
{
}

void curveFit::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // insert current options

  QString functionName = gnuInt->getCurveFitFunctionName();
  QString dataFile = gnuInt->getCurveFitDataFile();
  QString functionValue = gnuInt->getCurveFitFunctionValue();
  QString varXRangeName = gnuInt->getCurveFitVarXRangeName();
  QString varXRangeMin = gnuInt->getCurveFitVarXRangeMin();
  QString varXRangeMax = gnuInt->getCurveFitVarXRangeMax();
  QString varYRangeName = gnuInt->getCurveFitVarYRangeName();
  QString varYRangeMin = gnuInt->getCurveFitVarYRangeMin();
  QString varYRangeMax = gnuInt->getCurveFitVarYRangeMax();
  QString paramFile = gnuInt->getCurveFitParamFile();
  int paramFileFlag = gnuInt->getCurveFitParamFileFlag();
  int paramCSLFlag = gnuInt->getCurveFitParamCSLFlag();
  QString paramCSL = gnuInt->getCurveFitParamCSL();
  QString fitLimit = gnuInt->getCurveFitFitLimit();
  QString fitMaxIter = gnuInt->getCurveFitFitMaxIter();
  QString dataSetStart = gnuInt->getCurveFitDataSetStart();
  QString dataSetEnd = gnuInt->getCurveFitDataSetEnd();
  QString dataSetInc = gnuInt->getCurveFitPointInc();
  QString pointInc = gnuInt->getCurveFitPointInc();
  QString lineInc = gnuInt->getCurveFitLineInc();
  QString startPoint = gnuInt->getCurveFitStartPoint();
  QString startLine = gnuInt->getCurveFitStartLine();
  QString endPoint = gnuInt->getCurveFitEndPoint();
  QString endLine = gnuInt->getCurveFitEndLine();
  QString xColumn = gnuInt->getCurveFitXColumn();
  QString yColumn = gnuInt->getCurveFitYColumn();
  QString zColumn = gnuInt->getCurveFitZColumn();
  QString format = gnuInt->getCurveFitFormat();
  QString rawFormat = gnuInt->getCurveFitRawFormat();

  functionNameEdit->setText(functionName);
  dataFileEdit->setText(dataFile);
  functionValueEdit->setText(functionValue);
  varXRangeNameEdit->setText(varXRangeName);
  varXRangeMinEdit->setText(varXRangeMin);
  varXRangeMaxEdit->setText(varXRangeMax);
  varYRangeNameEdit->setText(varYRangeName);
  varYRangeMinEdit->setText(varYRangeMin);
  varYRangeMaxEdit->setText(varYRangeMax);
  paramFileEdit->setText(paramFile);

  if (paramFileFlag == 1)
  {
    paramFileRB->setChecked(TRUE);
    paramCSLRB->setChecked(FALSE);
  }
  else if (paramCSLFlag == 1)
  {
    paramCSLRB->setChecked(TRUE);
    paramFileRB->setChecked(FALSE);
  }

  paramCSLEdit->setText(paramCSL);
  fitLimitEdit->setText(fitLimit);
  fitMaxIterEdit->setText(fitMaxIter);
  dataSetStartEdit->setText(dataSetStart);
  dataSetEndEdit->setText(dataSetEnd);
  dataSetIncEdit->setText(dataSetInc);
  pointIncEdit->setText(pointInc);
  lineIncEdit->setText(lineInc);
  startPointEdit->setText(startPoint);
  startLineEdit->setText(startLine);
  endPointEdit->setText(endPoint);
  endLineEdit->setText(endLine);
  xColumnEdit->setText(xColumn);
  yColumnEdit->setText(yColumn);
  zColumnEdit->setText(zColumn);
  formatEdit->setText(format);
  rawFormatEdit->setText(rawFormat);
}

void curveFit::dataFileOpen()
{
  QString fileName = KFileDialog::getOpenFileName(QDir::currentDirPath(), "",this, i18n("Open File"));

  if ( !fileName.isNull() ) // got a file name
  {
    dataFileEdit->setText(fileName);
  }
}

void curveFit::paramFileOpen()
{
  QString fileName = KFileDialog::getOpenFileName(QDir::currentDirPath(), "",this, i18n("Open File"));

  if ( !fileName.isNull() ) // got a file name
  {
    paramFileEdit->setText(fileName);
  }
}

void curveFit::doFit()
{
  // get current options

  QString functionName = functionNameEdit->text();
  QString functionValue = functionValueEdit->text();
  QString dataFile = dataFileEdit->text();
  QString varXRangeName = varXRangeNameEdit->text();
  QString varXRangeMin = varXRangeMinEdit->text();
  QString varXRangeMax = varXRangeMaxEdit->text();
  QString varYRangeName = varYRangeNameEdit->text();
  QString varYRangeMin = varYRangeMinEdit->text();
  QString varYRangeMax = varYRangeMaxEdit->text();
  QString paramFile = paramFileEdit->text();

  int paramFileFlag;
  int paramCSLFlag;

  if ((paramFileRB->isChecked() == TRUE) &&
      (paramCSLRB->isChecked() == FALSE))
  {
    paramFileFlag = 1;
    paramCSLFlag = 0;
  }
  else
  {
    paramFileFlag = 0;
    paramCSLFlag = 1;
  }

  QString paramCSL = paramCSLEdit->text();
  QString fitLimit = fitLimitEdit->text();
  QString fitMaxIter = fitMaxIterEdit->text();
  QString dataSetStart = dataSetStartEdit->text();
  QString dataSetEnd = dataSetEndEdit->text();
  QString dataSetInc = dataSetIncEdit->text();
  QString pointInc = pointIncEdit->text();
  QString lineInc = lineIncEdit->text();
  QString startPoint = startPointEdit->text();
  QString startLine = startLineEdit->text();
  QString endPoint = endPointEdit->text();
  QString endLine = endLineEdit->text();
  QString xColumn = xColumnEdit->text();
  QString yColumn = yColumnEdit->text();
  QString zColumn = zColumnEdit->text();
  QString format = formatEdit->text();
  QString rawFormat = rawFormatEdit->text();

  // set options

  gnuInt->setCurveFitFunctionName(functionName);
  gnuInt->setCurveFitFunctionValue(functionValue);
  gnuInt->setCurveFitDataFile(dataFile);
  gnuInt->setCurveFitVarXRangeName(varXRangeName);
  gnuInt->setCurveFitVarXRangeMin(varXRangeMin);
  gnuInt->setCurveFitVarXRangeMax(varXRangeMax);
  gnuInt->setCurveFitVarYRangeName(varYRangeName);
  gnuInt->setCurveFitVarYRangeMin(varYRangeMin);
  gnuInt->setCurveFitVarYRangeMax(varYRangeMax);
  gnuInt->setCurveFitParamFile(paramFile);
  gnuInt->setCurveFitParamFileFlag(paramFileFlag);
  gnuInt->setCurveFitParamCSLFlag(paramCSLFlag);
  gnuInt->setCurveFitParamCSL(paramCSL);
  gnuInt->setCurveFitFitLimit(fitLimit);
  gnuInt->setCurveFitFitMaxIter(fitMaxIter);
  gnuInt->setCurveFitDataSetStart(dataSetStart);
  gnuInt->setCurveFitDataSetEnd(dataSetEnd);
  gnuInt->setCurveFitDataSetInc(dataSetInc);
  gnuInt->setCurveFitPointInc(pointInc);
  gnuInt->setCurveFitLineInc(lineInc);
  gnuInt->setCurveFitStartPoint(startPoint);
  gnuInt->setCurveFitStartLine(startLine);
  gnuInt->setCurveFitEndPoint(endPoint);
  gnuInt->setCurveFitEndLine(endLine);
  gnuInt->setCurveFitXColumn(xColumn);
  gnuInt->setCurveFitYColumn(yColumn);
  gnuInt->setCurveFitZColumn(zColumn);
  gnuInt->setCurveFitFormat(format);
  gnuInt->setCurveFitRawFormat(rawFormat);
  
  gnuInt->doCurveFit();
}

#include "curveFit.moc"
