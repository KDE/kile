/***************************************************************************
                          l2hdialog.h  -  description
                             -------------------
    begin                : Sun Jun 3 2001
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

#ifndef L2HDIALOG_H
#define L2HDIALOG_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>

/**
  *@author Brachet Pascal
  */

class l2hdialog : public QDialog  {
   Q_OBJECT
public: 
	l2hdialog(QWidget *parent=0, const char *name=0);
	~l2hdialog();
public:
	QLabel *QLabel_1;
  QLineEdit* options_edit;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;
};

#endif
