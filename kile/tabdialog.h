/***************************************************************************
                          tabdialog.h  -  description
                             -------------------
    begin                : Mon Apr 30 2001
    copyright            : (C) 2001 by Brachet Pascal
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

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qtable.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdialog.h>


class tabdialog : public QDialog  {
   Q_OBJECT
public:
	tabdialog(QWidget *parent=0, const char *name=0);
	~tabdialog();

public:
  QTable* Table1;
	QSpinBox *spinBoxRows;
	QSpinBox *spinBoxCollums;
	QLabel *QLabel_1;
	QLabel *QLabel_2;
	QLabel *QLabel_3;
  QLabel *QLabel_4;
  QComboBox *combo1;
  QComboBox *combo2;
  QCheckBox* checkbox1;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;

protected slots:
  void NewRows(int num);
  void NewCollums(int num);
};


#endif


