/***************************************************************************
                          refdialog.h  -  description
                             -------------------
    begin                : dim déc 1 2002
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

#ifndef REFDIALOG_H
#define REFDIALOG_H
#include <qdialog.h>

class QComboBox;
class QPushButton;
/**
  *@author Pascal Brachet
  */

class refdialog : public QDialog  {
   Q_OBJECT
public:
	refdialog(QWidget *parent=0, const char *name=0);
	~refdialog();

public:
  QComboBox *combo1;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
};


#endif
