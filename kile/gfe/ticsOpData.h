/**********************************************************************

	--- Qt Architect generated file ---

	File: ticsOpData.h

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

#ifndef ticsOpData_included
#define ticsOpData_included

#include <qtabdlg.h>
#include <qlined.h>
#include <qradiobt.h>
#include <qcombo.h>

class ticsOpData : public QTabDialog
{
    Q_OBJECT

public:

    ticsOpData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~ticsOpData();

public slots:


protected slots:
     virtual void setTicsOptions();


protected:
    QRadioButton* xticsOnRButton;
    QRadioButton* xticsOffRButton;
    QComboBox* xticsLocationCBox;
    QComboBox* xticsMirrorCBox;
    QComboBox* xticsRotationCBox;
    QLineEdit* xticsStartPosEdit;
    QLineEdit* xticsIncPosEdit;
    QLineEdit* xticsEndPosEdit;
    QRadioButton* xticsSIERadioButton;
    QRadioButton* xticsLabelPosRButton;
    QLineEdit* xticsLabelsPosEdit;

    QComboBox* yticsLocationCBox;
    QRadioButton* yticsOnRButton;
    QRadioButton* yticsOffRButton;
    QComboBox* yticsMirrorCBox;
    QComboBox* yticsRotationCBox;
    QLineEdit* yticsStartPosEdit;
    QLineEdit* yticsIncPosEdit;
    QLineEdit* yticsEndPosEdit;
    QRadioButton* yticsSIERadioButton;
    QRadioButton* yticsLabelPosRButton;
    QLineEdit* yticsLabelsPosEdit;

    QRadioButton* zticsOnRButton;
//    QComboBox* zticsLocationCBox;
    QRadioButton* zticsOffRButton;
    QComboBox* zticsMirrorCBox;
    QComboBox* zticsRotationCBox;
    QLineEdit* zticsStartPosEdit;
    QLineEdit* zticsIncPosEdit;
    QLineEdit* zticsEndPosEdit;
    QRadioButton* zticsSIERadioButton;
    QRadioButton* zticsLabelPosRButton;
    QLineEdit* zticsLabelsPosEdit;

    QComboBox* x2ticsLocationCBox;
    QRadioButton* x2ticsOnRButton;
    QRadioButton* x2ticsOffRButton;
    QComboBox* x2ticsMirrorCBox;
    QComboBox* x2ticsRotationCBox;
    QLineEdit* x2ticsStartPosEdit;
    QLineEdit* x2ticsIncPosEdit;
    QLineEdit* x2ticsEndPosEdit;
    QRadioButton* x2ticsSIERadioButton;
    QRadioButton* x2ticsLabelPosRButton;
    QLineEdit* x2ticsLabelsPosEdit;

    QComboBox* y2ticsLocationCBox;
    QRadioButton* y2ticsOnRButton;
    QRadioButton* y2ticsOffRButton;
    QComboBox* y2ticsMirrorCBox;
    QComboBox* y2ticsRotationCBox;
    QLineEdit* y2ticsStartPosEdit;
    QLineEdit* y2ticsIncPosEdit;
    QLineEdit* y2ticsEndPosEdit;
    QRadioButton* y2ticsSIERadioButton;
    QRadioButton* y2ticsLabelPosRButton;
    QLineEdit* y2ticsLabelsPosEdit;

};

#endif // ticsOpData_included
