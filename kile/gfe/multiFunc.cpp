/* -------------------------- multiFunc class --------------------------

   This class handles all operations related to the storage and manipulation of 
   multiple functions and their options from the GUI.

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


#include "multiFunc.h"

multiFunc::multiFunc
(
	QWidget* parent,
	const char* name
)
	:
	multiFuncData( parent, name )
{
	setCaption( "Multiple Function" );
}


multiFunc::~multiFunc()
{
}

void multiFunc::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  QString function;

  // get all current functions

  // get first
  function = gnuInt->getMultiFuncFirstFunction();

  // insert first function into list
  if (function != "END")
    multiFuncList->insertItem(function);

  int continueFlag = 1;

  // now insert remaining functions into list

  while (continueFlag)
  {
    function = gnuInt->getMultiFuncNextFunction();

    if (function == "END")
      continueFlag = 0;
    else
      multiFuncList->insertItem(function);
  }

  // grab options for current function in combo box
  if (multiFuncList->count() > 0)
  {
    // get current function
    function = multiFuncList->currentText();

    // get style option
    QString style = gnuInt->getMultiFuncStyleOption(function);

    // put function in edit box
    functionEdit->setText(function);

    // set option for style
    if (style == "points")
      funcStyleList->setCurrentItem(0);
    else if (style == "lines")
      funcStyleList->setCurrentItem(1);
    else if (style == "linespoints")
      funcStyleList->setCurrentItem(2);
    else if (style == "impulses")
      funcStyleList->setCurrentItem(3);
    else if (style == "dots")
      funcStyleList->setCurrentItem(4);
    else if (style == "steps")
      funcStyleList->setCurrentItem(5);
    else if (style == "errorbars")
      funcStyleList->setCurrentItem(6);
    else if (style == "boxes")
      funcStyleList->setCurrentItem(7);
    else if (style == "")
      funcStyleList->setCurrentItem(1);

    // set title options
    QString title = gnuInt->getMultiFuncLegendTitle(function);

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
  }
}

void multiFunc::insertNewFunction()
{
  // get function in edit box
  QString function = functionEdit->text();
    // call gnuInterface
  gnuInt->insertMultiFuncNew(function);

  // insert function in list
  multiFuncList->insertItem(function,0);

  // reset style option
  multiFuncList->setCurrentItem(0);
  QString style = funcStyleList->currentText();

  // set options
  gnuInt->setMultiFuncStyleOption(function,style);


  // set title

  if (legendTitleDefaultButton->isChecked() == TRUE)
  gnuInt->setMultiFuncLegendTitle(function, "default");

  if (legendTitlenotitleButton->isChecked() == TRUE)
  gnuInt->setMultiFuncLegendTitle(function, "notitle");

  QString title = legendTitleEdit->text();
  if (title!="") gnuInt->setMultiFuncLegendTitle(function, title);

  // reset title
  legendTitleEdit->setText("");
  legendTitleDefaultButton->setChecked(TRUE);
  legendTitlenotitleButton->setChecked(FALSE);
  functionEdit->setText("");

}

void multiFunc::setFuncOptions()
{
  // make sure we actually have a function in the combo box
  if (multiFuncList->count() > 0)
  {
    // get function
    QString function = multiFuncList->currentText();

    // get options
    QString style = funcStyleList->currentText();

    // set options
    gnuInt->setMultiFuncStyleOption(function,style);


    // set title

    if (legendTitleDefaultButton->isChecked() == TRUE)
      gnuInt->setMultiFuncLegendTitle(function, "default");

    if (legendTitlenotitleButton->isChecked() == TRUE)
      gnuInt->setMultiFuncLegendTitle(function, "notitle");

    QString title = legendTitleEdit->text();
    if (title!="") gnuInt->setMultiFuncLegendTitle(function, title);

  }
}

void multiFunc::closeMultiFunc()
{
  QDialog::accept();
}

void multiFunc::deleteFunction()
{
  // if there are still items left in the combo box, reset current to item 0,
  // and fill in options
  if (multiFuncList->count() > 0)
  {
    // reset current item for combo box
    //multiFuncList->setCurrentItem(0);

    // get current function in combo box
    QString function = multiFuncList->currentText();

    int currentItem = multiFuncList->currentItem();

    // remove item from combo box
    multiFuncList->removeItem(currentItem);

    // remove item from multiFunc list
    gnuInt->removeMultiFunc(function);

    // make sure we still have files left in the combo box
    if (multiFuncList->count() > 0)
    {
      // reset current item for combo box
      multiFuncList->setCurrentItem(0);

      // get function that is now current in combo box
      function = multiFuncList->currentText();

      // set edit box to current function
      functionEdit->setText(function);

      // get options for this file
      QString style = gnuInt->getMultiFuncStyleOption(function);

      // fill in options in GUI
      // figure out which option index current style corresponds to
      if (style == "points")
        funcStyleList->setCurrentItem(0);
      else if (style == "lines")
        funcStyleList->setCurrentItem(1);
      else if (style == "linespoints")
        funcStyleList->setCurrentItem(2);
      else if (style == "impulses")
        funcStyleList->setCurrentItem(3);
      else if (style == "dots")
        funcStyleList->setCurrentItem(4);
      else if (style == "steps")
        funcStyleList->setCurrentItem(5);
      else if (style == "errorbars")
        funcStyleList->setCurrentItem(6);
      else if (style == "boxes")
        funcStyleList->setCurrentItem(7);
      else if (style == "")
        funcStyleList->setCurrentItem(1);

      // set title options
      QString title = gnuInt->getMultiFuncLegendTitle(function);

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
    }
    else
    {
      // make sure all options are reset
      funcStyleList->setCurrentItem(1);
      functionEdit->setText("");
      legendTitleEdit->setText("");
      legendTitleDefaultButton->setChecked(TRUE);
      legendTitlenotitleButton->setChecked(FALSE);
    }
  }
  else
  {
    // make sure all options are reset
    funcStyleList->setCurrentItem(1);
    functionEdit->setText("");
    legendTitleEdit->setText("");
    legendTitleDefaultButton->setChecked(TRUE);
    legendTitlenotitleButton->setChecked(FALSE);
  }

}

void multiFunc::funcChanged(const QString& func)
{
  // get function
  QString function = func;

  // set function in edit box
  functionEdit->setText(function);

  // get options
  QString style = gnuInt->getMultiFuncStyleOption(function);

  // set options
  // figure out which option index current style corresponds to
  if (style == "points")
    funcStyleList->setCurrentItem(0);
  else if (style == "lines")
    funcStyleList->setCurrentItem(1);
  else if (style == "linespoints")
    funcStyleList->setCurrentItem(2);
  else if (style == "impulses")
    funcStyleList->setCurrentItem(3);
  else if (style == "dots")
    funcStyleList->setCurrentItem(4);
  else if (style == "steps")
    funcStyleList->setCurrentItem(5);
  else if (style == "errorbars")
    funcStyleList->setCurrentItem(6);
  else if (style == "boxes")
    funcStyleList->setCurrentItem(7);
  else if (style == "")
    funcStyleList->setCurrentItem(1);

  // set title options
  QString title = gnuInt->getMultiFuncLegendTitle(function);

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
}

#include "multiFunc.moc"
