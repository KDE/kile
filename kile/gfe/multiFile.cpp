/* -------------------------- multiFile class --------------------------

   This class handles all operations related to the storage and manipulation of 
   multiple files and their options from the GUI.

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

   ------------------------------------------------------------------------*/

#include "multiFile.h"
#include <iostream>
#include <klocale.h>
#include <kfiledialog.h>


multiFile::multiFile
(
	QWidget* parent,
	const char* name
)
	:
	multiFileData( parent, name )
{
	setCaption( "Multiple Files" );
}


multiFile::~multiFile()
{
}

void multiFile::setGnuInterface(gnuInterface* gnu)
{
  // save gnuInterface object
  gnuInt = gnu;

  // get all current filenames
  
  QString file;

  // get first filename in list
  file = gnuInt->getMultiFileFirstFilename();

  // insert first filename into list
  if (file != "END")
    multiFileList->insertItem(file);

  int continueFlag = 1;

  // now insert remaining files into list
  while (continueFlag)
  {
    file = gnuInt->getMultiFileNextFilename();

    if (file == "END")
      continueFlag = 0;
    else
      multiFileList->insertItem(file);
  }

  // grab options for current file in combo box
  if (multiFileList->count() > 0)
  {
    getCurrentOptions();
  }

}

void multiFile::apply()
{
  // make sure we actually have files in the combo box
  if (multiFileList->count() > 0)
  {
    QString filename = multiFileList->currentText();
    QString dataSetStart = dataSetStartEdit->text();
    QString dataSetEnd = dataSetEndEdit->text();
    QString dataSetInc = dataSetIncEdit->text();
    QString sampPointInc = pointIncEdit->text();
    QString sampLineInc = lineIncEdit->text();
    QString sampStartPoint = startPointEdit->text();
    QString sampStartLine = startLineEdit->text();
    QString sampEndPoint = endPointEdit->text();
    QString sampEndLine = endLineEdit->text();
    QString xcol = xColumnEdit->text();
    QString ycol = yColumnEdit->text();
    QString zcol = zColumnEdit->text();
    QString format = formatEdit->text();
    QString rawformat = rawFormatEdit->text();
    QString smoothType = interpList->currentText();
    QString style = fileStyleList->currentText();
    QString filter = filterEdit->text();

    gnuInt->setMultiFileStyleOption(filename, style);
    gnuInt->setMultiFileDataSetStart(filename,dataSetStart);
    gnuInt->setMultiFileDataSetEnd(filename,dataSetEnd);
    gnuInt->setMultiFileDataSetIncrement(filename,dataSetInc);
    gnuInt->setMultiFileSampPointInc(filename,sampPointInc);
    gnuInt->setMultiFileSampLineInc(filename,sampLineInc);
    gnuInt->setMultiFileSampStartPoint(filename,sampStartPoint);
    gnuInt->setMultiFileSampStartLine(filename,sampStartLine);
    gnuInt->setMultiFileSampEndPoint(filename,sampEndPoint);
    gnuInt->setMultiFileSampEndLine(filename,sampEndLine);
    gnuInt->insertMultiFileXColumnOption(filename, xcol);
    gnuInt->insertMultiFileYColumnOption(filename, ycol);
    gnuInt->insertMultiFileZColumnOption(filename, zcol);
    gnuInt->insertMultiFileFormatOption(filename, format);
    gnuInt->insertMultiFileRawFormatOption(filename, rawformat);
    gnuInt->setMultiFileSmoothType(filename,smoothType);

    if (legendTitleDefaultButton->isChecked() == TRUE)
      gnuInt->setMultiFileLegendTitle(filename, "default");

    if (legendTitlenotitleButton->isChecked() == TRUE)
      gnuInt->setMultiFileLegendTitle(filename, "notitle");

    QString title = legendTitleEdit->text();
    if (title!="") gnuInt->setMultiFileLegendTitle(filename, title);

    if (singleQuoteRB->isChecked() == TRUE)
      gnuInt->setMultiFileFilterQuoteChar(filename, "single");
    else if (doubleQuoteRB->isChecked() == TRUE)
      gnuInt->setMultiFileFilterQuoteChar(filename, "double");

    if (filter != "")
      gnuInt->setMultiFileFilter(filename, filter);
    else
      gnuInt->setMultiFileFilter(filename, "");
  }
}

void multiFile::getNewFile()
{
QString filename=KFileDialog::getOpenFileName(QDir::currentDirPath(), "",this, i18n("Open File"));
  QString temp;

  if ( !filename.isNull() )
  {
    temp = filename;

    multiFileList->insertItem(filename,0);
    gnuInt->insertMultiFileNew(temp);
    clearOptions();
  }
}

void multiFile::deleteFile()
{
  // if there are still items left in the combo box, reset current to item 0,
  // and fill in options
  if (multiFileList->count() > 0)
  {
    // get current file in combo box
    QString filename = multiFileList->currentText();

    int currentItem = multiFileList->currentItem();

    // remove item from combo box
    multiFileList->removeItem(currentItem);

    // remove item from multiFile list
    gnuInt->removeMultiFile(filename);

    // make sure we still have files left in the combo box
    if (multiFileList->count() > 0)
    {
      // reset current item for combo box
      multiFileList->setCurrentItem(0);

      // get options for this file
      getCurrentOptions();
    }
    else
    {
      clearOptions();
    }
  }
  else
  {
    clearOptions();
  }
}

void multiFile::fileChanged(const QString& file)
{
  QString filename = file; // don't need to use this option, but store it to
                          // avoid compiler warnings
  getCurrentOptions();
}

void multiFile::insertCurrentFilename()
{
  QString currentText = filterEdit->text();

  QString filename = multiFileList->currentText();

  QString newString;

  newString += currentText ;
  newString += filename;

  filterEdit->setText(newString);
}

void multiFile::insertNewFilename()
{
  QString currentText = filterEdit->text();

  QString filename = KFileDialog::getOpenFileName(QDir::currentDirPath(), "",this, i18n("Open File"));


  if (!filename.isNull())
  {
    QString newString;
    newString += currentText;
    newString += filename;

    filterEdit->setText(newString);
  }
}

void multiFile::getCurrentOptions()
{
  // get options for this file
  QString filename = multiFileList->currentText();
  QString dataSetStart = gnuInt->getMultiFileDataSetStart(filename);
  QString dataSetEnd = gnuInt->getMultiFileDataSetEnd(filename);
  QString dataSetInc = gnuInt->getMultiFileDataSetIncrement(filename);
  QString sampPointInc = gnuInt->getMultiFileSampPointInc(filename);
  QString sampLineInc = gnuInt->getMultiFileSampLineInc(filename);
  QString sampStartPoint = gnuInt->getMultiFileSampStartPoint(filename);
  QString sampStartLine = gnuInt->getMultiFileSampStartLine(filename);
  QString sampEndPoint = gnuInt->getMultiFileSampEndPoint(filename);
  QString sampEndLine = gnuInt->getMultiFileSampEndLine(filename);
  QString xcol = gnuInt->getMultiFileXColumnOption(filename);
  QString ycol = gnuInt->getMultiFileYColumnOption(filename);
  QString zcol = gnuInt->getMultiFileZColumnOption(filename);
  QString format = gnuInt->getMultiFileFormatOption(filename);
  QString rawformat = gnuInt->getMultiFileRawFormatOption(filename);
  QString smoothType = gnuInt->getMultiFileSmoothType(filename);
  QString style = gnuInt->getMultiFileStyleOption(filename);
  QString filter = gnuInt->getMultiFileFilter(filename);
  QString quoteChar = gnuInt->getMultiFileFilterQuoteChar(filename);

  // fill in options in GUI
  dataSetStartEdit->setText(dataSetStart);
  dataSetEndEdit->setText(dataSetEnd);
  dataSetIncEdit->setText(dataSetInc);
  pointIncEdit->setText(sampPointInc);
  lineIncEdit->setText(sampLineInc);
  startPointEdit->setText(sampStartPoint);
  startLineEdit->setText(sampStartLine);
  endPointEdit->setText(sampEndPoint);
  endLineEdit->setText(sampEndLine);
  xColumnEdit->setText(xcol);
  yColumnEdit->setText(ycol);
  zColumnEdit->setText(zcol);
  formatEdit->setText(format);
  rawFormatEdit->setText(rawformat);
  filterEdit->setText(filter);

  // figure out which option index current style corresponds to
  if (style == "points")
    fileStyleList->setCurrentItem(0);
  else if (style == "lines")
    fileStyleList->setCurrentItem(1);
  else if (style == "linespoints")
    fileStyleList->setCurrentItem(2);
  else if (style == "impulses")
    fileStyleList->setCurrentItem(3);
  else if (style == "dots")
    fileStyleList->setCurrentItem(4);
  else if (style == "steps")
    fileStyleList->setCurrentItem(5);
  else if (style == "fsteps")
    fileStyleList->setCurrentItem(6);
  else if (style == "histeps")
    fileStyleList->setCurrentItem(7);
  else if (style == "errorbars")
    fileStyleList->setCurrentItem(8);
  else if (style == "xerrorbars")
    fileStyleList->setCurrentItem(9);
  else if (style == "yerrorbars")
    fileStyleList->setCurrentItem(10);
  else if (style == "xyerrorbars")
    fileStyleList->setCurrentItem(11);
  else if (style == "boxes")
    fileStyleList->setCurrentItem(12);
  else if (style == "boxerrorbars")
    fileStyleList->setCurrentItem(13);
  else if (style == "boxxyerrorbars")
    fileStyleList->setCurrentItem(14);
  else if (style == "financebars")
    fileStyleList->setCurrentItem(15);
  else if (style == "candlesticks")
    fileStyleList->setCurrentItem(16);
  else if (style == "")
    fileStyleList->setCurrentItem(0);

  // set title options
  QString title = gnuInt->getMultiFileLegendTitle(filename);

  // clear current title
  legendTitleEdit->setText("");

  if ((title != "default") && (title != "notitle"))
    legendTitleEdit->setText(title);
  
  if (title == "default")
  {
    legendTitleDefaultButton->setChecked(TRUE);
    legendTitlenotitleButton->setChecked(FALSE);
  }
  else if (title == "notitle")
  {
    legendTitleDefaultButton->setChecked(FALSE);
    legendTitlenotitleButton->setChecked(TRUE);
  }

  if ((title != "default") && (title != "notitle"))
  {
    legendTitleDefaultButton->setChecked(FALSE);
    legendTitlenotitleButton->setChecked(FALSE);
  }

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

  if (quoteChar == "single")
  {
    singleQuoteRB->setChecked(TRUE);
    doubleQuoteRB->setChecked(FALSE);
  }
    
  if (quoteChar == "double")
  {
    doubleQuoteRB->setChecked(TRUE);
    singleQuoteRB->setChecked(FALSE);
  }
}

void multiFile::clearOptions()
{
  legendTitleEdit->setText("");
  legendTitleDefaultButton->setChecked(TRUE);
  legendTitlenotitleButton->setChecked(FALSE);
  fileStyleList->setCurrentItem(0);
  dataSetStartEdit->setText("");
  dataSetEndEdit->setText("");
  dataSetIncEdit->setText("");
  pointIncEdit->setText("");
  lineIncEdit->setText("");
  startPointEdit->setText("");
  startLineEdit->setText("");
  endPointEdit->setText("");
  endLineEdit->setText("");
  xColumnEdit->setText("");
  yColumnEdit->setText("");
  zColumnEdit->setText("");
  formatEdit->setText("");
  rawFormatEdit->setText("");
  interpList->setCurrentItem(0);
  filterEdit->setText("");
  doubleQuoteRB->setChecked(TRUE);    
  singleQuoteRB->setChecked(FALSE);
}

#include "multiFile.moc"
