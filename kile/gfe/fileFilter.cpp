/**********************************************************************

	--- Qt Architect generated file ---

	File: fileFilter.cpp
	Last generated: Sat May 2 22:35:39 1998

 *********************************************************************/
using namespace std;

#include "fileFilter.h"
#include <qstring.h>
#include <qfiledlg.h>
#include <klocale.h>
#include <kfiledialog.h>


fileFilter::fileFilter
(
	QWidget* parent,
	const char* name
)
	:
	fileFilterData( parent, name )
{
	setCaption( "Data File Filtering Command" );
}


fileFilter::~fileFilter()
{
}

void fileFilter::setGnuInterface(gnuInterface* gnu)
{
  gnuInt = gnu;

  QString temp = gnuInt->getFileFilter();
  filterEdit->setText(temp);

  QString quoteChar = gnuInt->getFileFilterQuoteChar();

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

void fileFilter::insertCurrentFilename()
{
  QString currentText = filterEdit->text();

  QString temp = gnuInt->getPlotFilename();
  QString filename = temp;

  QString newString;

  newString += currentText ;
  newString += filename;
  
  filterEdit->setText(newString);
}

void fileFilter::insertNewFilename()
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

void fileFilter::setFilter()
{
  QString filterCmd;
  
  if (singleQuoteRB->isChecked() == TRUE)
    gnuInt->setFileFilterQuoteChar("single");
  else if (doubleQuoteRB->isChecked() == TRUE)
    gnuInt->setFileFilterQuoteChar("double");
  
  filterCmd += filterEdit->text();
  
  gnuInt->setFileFilter(filterCmd);

  QDialog::accept();
}

#include "fileFilter.moc"
