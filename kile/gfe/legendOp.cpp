/**********************************************************************

	--- Qt Architect generated file ---

	File: legendOp.cpp
	Last generated: Sun Feb 22 19:45:48 1998

 *********************************************************************/

#include "legendOp.h"
#include <qmsgbox.h>

legendOp::legendOp
(
	QWidget* parent,
	const char* name
)
	:
	legendOpData( parent, name )
{
	setCaption( "Legend Options" );
}


legendOp::~legendOp()
{
}

void legendOp::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  // recall current status of variables and set appropriate checks and values

  QString legendFlag = gnuInt->getLegendFlag();
  int legendPositionLeft = gnuInt->getLegendPositionLeft();
  int legendPositionRight = gnuInt->getLegendPositionRight();
  int legendPositionTop = gnuInt->getLegendPositionTop();
  int legendPositionBottom = gnuInt->getLegendPositionBottom();
  int legendPositionOutside = gnuInt->getLegendPositionOutside();
  int legendPositionBelow = gnuInt->getLegendPositionBelow();
  QString legendPositionXVal = gnuInt->getLegendPositionXVal();
  QString legendPositionYVal = gnuInt->getLegendPositionYVal();
  QString legendPositionZVal = gnuInt->getLegendPositionZVal();
  QString legendTextJustify = gnuInt->getLegendTextJustify();
  QString legendReverse = gnuInt->getLegendReverse();
  QString legendBox = gnuInt->getLegendBox();
  QString legendLinetype = gnuInt->getLegendLinetype();
  QString legendSampleLength = gnuInt->getLegendSampleLength();
  QString legendSpacing = gnuInt->getLegendSpacing();
  QString legendWidthIncrement = gnuInt->getLegendWidthIncrement();
  QString legendTitle = gnuInt->getLegendTitle();

  if (legendFlag == "key")
  {
    keyButton->setChecked(TRUE);
    noKeyButton->setChecked(FALSE);
  }

  if (legendFlag == "nokey")
  {
    noKeyButton->setChecked(TRUE);
    keyButton->setChecked(FALSE);
  }

  if (legendPositionLeft == 1)
    positionLeftButton->setChecked(TRUE);
  else
    positionLeftButton->setChecked(FALSE);

  if (legendPositionRight == 1)
    positionRightButton->setChecked(TRUE);
  else
    positionRightButton->setChecked(FALSE);

  if (legendPositionTop == 1)
    positionTopButton->setChecked(TRUE);
  else
    positionTopButton->setChecked(FALSE);

  if (legendPositionBottom == 1)
    positionBottomButton->setChecked(TRUE);
  else
    positionBottomButton->setChecked(FALSE);

  if (legendPositionOutside == 1)
    positionOutsideButton->setChecked(TRUE);
  else
    positionOutsideButton->setChecked(FALSE);

  if (legendPositionBelow == 1)
    positionBelowButton->setChecked(TRUE);
  else
    positionBelowButton->setChecked(FALSE);

  positionXEdit->setText(legendPositionXVal);
  positionYEdit->setText(legendPositionYVal);
  positionZEdit->setText(legendPositionZVal);

  if (legendTextJustify == "Left")
    textJustifyList->setCurrentItem(0);

  if (legendTextJustify == "Right")
    textJustifyList->setCurrentItem(1);

  if (legendReverse == "noreverse")
    reverseList->setCurrentItem(0);

  if (legendReverse == "reverse")
    reverseList->setCurrentItem(1);

  if (legendBox == "nobox")
    boxList->setCurrentItem(0);

  if (legendBox == "box")
    boxList->setCurrentItem(1);

  lineTypeEdit->setText(legendLinetype);
  sampleLengthEdit->setText(legendSampleLength);
  spacingEdit->setText(legendSpacing);
  widthIncrementEdit->setText(legendWidthIncrement);
  legendTitleEdit->setText(legendTitle);

}


void legendOp::setLegendOpDefaults()
{
  noKeyButton->setChecked(FALSE);
  keyButton->setChecked(TRUE);
  positionLeftButton->setChecked(FALSE);
  positionRightButton->setChecked(TRUE);
  positionTopButton->setChecked(TRUE);
  positionBottomButton->setChecked(FALSE);
  positionOutsideButton->setChecked(FALSE);
  positionBelowButton->setChecked(FALSE);
  positionXEdit->setText("");
  positionYEdit->setText("");
  positionZEdit->setText("");
  textJustifyList->setCurrentItem(0);
  reverseList->setCurrentItem(0);
  boxList->setCurrentItem(0);
  lineTypeEdit->setText("");
  sampleLengthEdit->setText("4");
  spacingEdit->setText("1.25");
  widthIncrementEdit->setText("");
  legendTitleEdit->setText("");
}

void legendOp::setLegendOptions()
{

  // make sure at least one position is checked
  if ((positionLeftButton->isChecked() == FALSE)  &&
      (positionRightButton->isChecked() == FALSE) &&
      (positionTopButton->isChecked() == FALSE) &&
      (positionBottomButton->isChecked() == FALSE) &&
      (positionOutsideButton->isChecked() == FALSE) &&
      (positionBelowButton->isChecked() == FALSE))
  {
    QMessageBox::information( this, "Xgfe",
                              "You must select at least one position button!");
    return;
  }


  if (keyButton->isChecked() == TRUE)
    gnuInt->setLegendFlag("key");

  if (noKeyButton->isChecked() == TRUE)
    gnuInt->setLegendFlag("nokey");

  if (positionLeftButton->isChecked() == TRUE)
    gnuInt->setLegendPositionLeft(1);
  else
    gnuInt->setLegendPositionLeft(0);

  if (positionRightButton->isChecked() == TRUE)
    gnuInt->setLegendPositionRight(1);
  else
    gnuInt->setLegendPositionRight(0);

  if (positionTopButton->isChecked() == TRUE)
    gnuInt->setLegendPositionTop(1);
  else
    gnuInt->setLegendPositionTop(0);

  if (positionBottomButton->isChecked() == TRUE)
    gnuInt->setLegendPositionBottom(1);
  else
    gnuInt->setLegendPositionBottom(0);

  if (positionOutsideButton->isChecked() == TRUE)
    gnuInt->setLegendPositionOutside(1);
  else
    gnuInt->setLegendPositionOutside(0);

  if (positionBelowButton->isChecked() == TRUE)
    gnuInt->setLegendPositionBelow(1);
  else
    gnuInt->setLegendPositionBelow(0);


  QString legendPositionXVal = positionXEdit->text();
  QString legendPositionYVal = positionYEdit->text();
  QString legendPositionZVal = positionZEdit->text();

  gnuInt->setLegendPositionXVal(legendPositionXVal);
  gnuInt->setLegendPositionYVal(legendPositionYVal);
  gnuInt->setLegendPositionZVal(legendPositionZVal);

  QString justify = textJustifyList->currentText();
  gnuInt->setLegendTextJustify(justify);

  QString reverse = reverseList->currentText();
  gnuInt->setLegendReverse(reverse);

  QString box = boxList->currentText();
  gnuInt->setLegendBox(box);

  QString linetype = lineTypeEdit->text();
  gnuInt->setLegendLinetype(linetype);

  QString sample = sampleLengthEdit->text();
  gnuInt->setLegendSampleLength(sample);

  QString spacing = spacingEdit->text();
  gnuInt->setLegendSpacing(spacing);

  QString width = widthIncrementEdit->text();
  gnuInt->setLegendWidthIncrement(width);

  QString title = legendTitleEdit->text();
  gnuInt->setLegendTitle(title);
  
  QDialog::accept();
}

#include "legendOp.moc"
