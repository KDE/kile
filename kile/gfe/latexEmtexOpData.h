/**********************************************************************

	--- Qt Architect generated file ---

	File: latexEmtexOpData.h

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

#ifndef latexEmtexOpData_included
#define latexEmtexOpData_included

#include <qdialog.h>
#include <qlined.h>
#include <qcombo.h>

class latexEmtexOpData : public QDialog
{
    Q_OBJECT

public:

    latexEmtexOpData
    (
        QWidget* parent = 0,
        const char* name = 0
    );

    virtual ~latexEmtexOpData();

public slots:


protected slots:

    virtual void setTerm();

protected:
    QComboBox* FontList;
    QComboBox* FontSizeList;
    QLineEdit* FontSizeOther;

};

#endif // latexEmtexOpData_included
