/**********************************************************************

	--- Qt Architect generated file ---

	File: fileLegendTitleData.h

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 *********************************************************************/

#ifndef fileLegendTitleData_included
#define fileLegendTitleData_included

#include <qdialog.h>
#include <qlined.h>
#include <qchkbox.h>

class fileLegendTitleData : public QDialog
{
    Q_OBJECT

public:

    fileLegendTitleData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~fileLegendTitleData();

public slots:


protected slots:

    virtual void setFileLegendTitleOK();

protected:
    QLineEdit* fileLegendTitleEdit;
    QCheckBox* defaultCButton;
    QCheckBox* noTitleCButton; 
};

#endif // fileLegendTitleData_included
