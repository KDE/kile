/**********************************************************************

	--- Qt Architect generated file ---

	File: fileFilterData.h

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

#ifndef fileFilterData_included
#define fileFilterData_included

#include <qdialog.h>
#include <qlined.h>
#include <qradiobt.h>

class fileFilterData : public QDialog
{
    Q_OBJECT

public:

    fileFilterData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

  virtual ~fileFilterData();

public slots:


protected slots:

    virtual void insertCurrentFilename();
    virtual void insertNewFilename();
    virtual void setFilter();

protected:
    QLineEdit* filterEdit;
    QRadioButton* singleQuoteRB;
    QRadioButton* doubleQuoteRB;

};

#endif // fileFilterData_included
