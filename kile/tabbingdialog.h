/***************************************************************************
                          tabbingdialog.h  -  description
                             -------------------
    begin                : dim jui 14 2002
    copyright            : (C) 2002 by Pascal Brachet
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TABBINGDIALOG_H
#define TABBINGDIALOG_H

#include <qdialog.h>


class QSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
/**
  *@author Pascal Brachet
  */

class tabbingdialog : public QDialog  {
   Q_OBJECT
public:
	tabbingdialog(QWidget *parent=0, const char *name=0);
	~tabbingdialog();

public:
	QSpinBox *spinBoxCollums, *spinBoxRows;
	QLabel *Label1, *Label2, *Label3;
  QLineEdit *LineEdit1;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;
};

#endif
