/***************************************************************************
                          addoptiondialog.h  -  description
                             -------------------
    begin                : Sun Oct 20 2002
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

#ifndef ADDOPTIONDIALOG_H
#define ADDOPTIONDIALOG_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>

/**
  *@author Pascal Brachet
  */

class AddOptionDialog : public QDialog
{
    Q_OBJECT
public: 
	AddOptionDialog(QWidget *parent = 0, const char *name = 0);
	~AddOptionDialog();
   QLineEdit *lineEdit;
private:
	  QPushButton *buttonOk;
	  QPushButton *buttonCancel;
};

#endif
