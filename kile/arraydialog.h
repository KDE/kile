/***************************************************************************
                          arraydialog.h  -  description
                             -------------------
    begin                : ven sep 27 2002
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

#ifndef ARRAYDIALOG_H
#define ARRAYDIALOG_H


/**
  *@author Pascal Brachet
  */

#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qtable.h>
#include <qpushbutton.h>
#include <qdialog.h>


class arraydialog : public QDialog  {
   Q_OBJECT
public:
	arraydialog(QWidget *parent=0, const char *name=0);
	~arraydialog();

public:
  QTable* Table1;
	QSpinBox *spinBoxRows;
	QSpinBox *spinBoxCollums;
	QLabel *QLabel_1;
	QLabel *QLabel_2;
	QLabel *QLabel_3;
  QLabel *QLabel_4;
  QComboBox *combo, *combo2;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;

protected slots:
  void NewRows(int num);
  void NewCollums(int num);
};


#endif
