/***************************************************************************
                          structdialog.h  -  description
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


#ifndef STRUCTDIALOG_H
#define STRUCTDIALOG_H


/**
  *@author Pascal Brachet
  */

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdialog.h>


class structdialog : public QDialog  {
   Q_OBJECT
public:
	structdialog(QWidget *parent=0, const char *name=0);
	~structdialog();

public:
	QLabel *QLabel_1;
  QLineEdit* title_edit;
  QCheckBox* checkbox;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;
private:
};

#endif
