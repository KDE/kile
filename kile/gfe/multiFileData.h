/* -------------------------- multiFileData class --------------------------

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   ------------------------------------------------------------------------*/

#ifndef multiFileData_included
#define multiFileData_included

#include <qdialog.h>
#include <qlined.h>
#include <qcombo.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include <qtabdlg.h>

class multiFileData : public QTabDialog
{
    Q_OBJECT

public:

    multiFileData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~multiFileData();

public slots:


protected slots:

  virtual void getNewFile();
  virtual void deleteFile();
  virtual void fileChanged(const QString&);
  virtual void insertCurrentFilename();
  virtual void insertNewFilename();
  virtual void apply();
  
protected:
  QComboBox* multiFileList;
  QComboBox* fileStyleList;
  QLineEdit* dataSetStartEdit;
  QLineEdit* dataSetEndEdit;
  QLineEdit* dataSetIncEdit;
  QLineEdit* pointIncEdit;
  QLineEdit* lineIncEdit;
  QLineEdit* startPointEdit;
  QLineEdit* startLineEdit;
  QLineEdit* endPointEdit;
  QLineEdit* endLineEdit;
  QLineEdit* xColumnEdit;
  QLineEdit* yColumnEdit;
  QLineEdit* zColumnEdit;
  QLineEdit* formatEdit;
  QLineEdit* rawFormatEdit;
  QComboBox* interpList;
  QLineEdit* legendTitleEdit;
  QCheckBox* legendTitleDefaultButton;
  QCheckBox* legendTitlenotitleButton;  
  QLineEdit* filterEdit;
  QRadioButton* singleQuoteRB;
  QRadioButton* doubleQuoteRB;

};

#endif // multiFileData_included
