/**********************************************************************

	--- Qt Architect generated file ---

	File: psOptData.h

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

#ifndef psOptData_included
#define psOptData_included

#include <qdialog.h>
#include <qlined.h>
#include <qcombo.h>

class psOptData : public QDialog
{
    Q_OBJECT

public:

    psOptData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~psOptData();

public slots:


protected slots:

    virtual void setTerm();

protected:
    QComboBox* modeList;
    QComboBox* colorList;
    QComboBox* dashedList;
    QLineEdit* fontNameEdit;
    QLineEdit* fontSizeEdit;
    QLineEdit* horizSize;
    QLineEdit* vertSize;
    QComboBox* enhancedList;

};

#endif // psOptData_included
