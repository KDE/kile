/**********************************************************************

	--- Qt Architect generated file ---

	File: fileOptions.cpp

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

#include "fileOptions.h"
#include <klocale.h>

fileOptions::fileOptions
(
	QWidget* parent,
	const char* name
)
	:
	fileOptionsData( parent, name )
{
	setCaption( i18n("Data File Options") );
}


fileOptions::~fileOptions()
{
}

void fileOptions::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;
  
  // get options and insert into widgets

  QString dataSetStart = gnuInt->getFileDataSetStart();
  QString dataSetEnd = gnuInt->getFileDataSetEnd();
  QString dataSetInc = gnuInt->getFileDataSetIncrement();
  QString sampPointInc = gnuInt->getFileSampPointInc();
  QString sampLineInc = gnuInt->getFileSampLineInc();
  QString sampStartPoint = gnuInt->getFileSampStartPoint();
  QString sampStartLine = gnuInt->getFileSampStartLine();
  QString sampEndPoint = gnuInt->getFileSampEndPoint();
  QString sampEndLine = gnuInt->getFileSampEndLine();
  QString xCol = gnuInt->getFileXColumn();
  QString yCol = gnuInt->getFileYColumn();
  QString zCol = gnuInt->getFileZColumn();
  QString format = gnuInt->getFileFormatString();
  QString rawFormat = gnuInt->getRawFileFormatString();
  QString smoothType = gnuInt->getFileSmoothType();

  dataSetStartEdit->setText(dataSetStart);
  dataSetEndEdit->setText(dataSetEnd);
  dataSetIncEdit->setText(dataSetInc);
  pointIncEdit->setText(sampPointInc);
  lineIncEdit->setText(sampLineInc);
  startPointEdit->setText(sampStartPoint);
  startLineEdit->setText(sampStartLine);
  endPointEdit->setText(sampEndPoint);
  endLineEdit->setText(sampEndLine);
  xColumnEdit->setText(xCol);
  yColumnEdit->setText(yCol);
  zColumnEdit->setText(zCol);
  formatEdit->setText(format);
  rawFormatEdit->setText(rawFormat);

  if (smoothType == "none")
    interpList->setCurrentItem(0);
  else if (smoothType == "unique")
    interpList->setCurrentItem(1);
  else if (smoothType == "csplines")
    interpList->setCurrentItem(2);
  else if (smoothType == "acsplines")
    interpList->setCurrentItem(3);
  else if (smoothType == "bezier")
    interpList->setCurrentItem(4);
  else if (smoothType == "sbezier")
    interpList->setCurrentItem(5);
}

void fileOptions::setFormat()
{
  QString dataSetStart = dataSetStartEdit->text();
  QString dataSetEnd = dataSetEndEdit->text();
  QString dataSetInc = dataSetIncEdit->text();
  QString sampPointInc = pointIncEdit->text();
  QString sampLineInc = lineIncEdit->text();
  QString sampStartPoint = startPointEdit->text();
  QString sampStartLine = startLineEdit->text();
  QString sampEndPoint = endPointEdit->text();
  QString sampEndLine = endLineEdit->text();
  QString xCol = xColumnEdit->text();
  QString yCol = yColumnEdit->text();
  QString zCol = zColumnEdit->text();
  QString format = formatEdit->text();
  QString rawFormat = rawFormatEdit->text();
  QString smoothType = interpList->currentText();

  gnuInt->setFileDataSetStart(dataSetStart);
  gnuInt->setFileDataSetEnd(dataSetEnd);
  gnuInt->setFileDataSetIncrement(dataSetInc);
  gnuInt->setFileSampPointInc(sampPointInc);
  gnuInt->setFileSampLineInc(sampLineInc);
  gnuInt->setFileSampStartPoint(sampStartPoint);
  gnuInt->setFileSampStartLine(sampStartLine);
  gnuInt->setFileSampEndPoint(sampEndPoint);
  gnuInt->setFileSampEndLine(sampEndLine);
  gnuInt->setFileXColumn(xCol);
  gnuInt->setFileYColumn(yCol);
  gnuInt->setFileZColumn(zCol);
  gnuInt->setFileFormatString(format);
  gnuInt->setRawFileFormatString(rawFormat);
  gnuInt->setFileSmoothType(smoothType);

  QDialog::accept();
}

#include "fileOptions.moc"
