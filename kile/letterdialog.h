/***************************************************************************
                          letterdialog.h  -  description
                             -------------------
    begin                : Tue Oct 30 2001
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

#ifndef LETTERDIALOG_H
#define LETTERDIALOG_H
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdialog.h>


/**
  *@author Brachet Pascal
  */

class letterdialog : public QDialog  {
   Q_OBJECT
public:
	letterdialog(QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
	~letterdialog();

public:
	QLabel *QLabel_2;
	QLabel *QLabel_3;
	QLabel *QLabel_4;

  QComboBox *combo2;
  QComboBox *combo3;
  QComboBox *combo4;
  QCheckBox* checkbox1;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;

};


#endif

