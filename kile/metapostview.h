/***************************************************************************
                          metapostview.h  -  description
                             -------------------
    begin                : mar avr 8 2003
    copyright            : (C) 2003 by Pascal Brachet
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

#ifndef METAPOSTVIEW_H
#define METAPOSTVIEW_H

#include <qlistbox.h>
#include <qstring.h>

/**
  *@author Pascal Brachet
  */
//////////////////////////////  
class metapostview : public QListBox  {
  Q_OBJECT
public:
	metapostview(QWidget *parent=0, const char *name=0);
	~metapostview();
};

#endif
