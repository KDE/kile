/***************************************************************************
                          messagewidget.h  -  description
                             -------------------
    begin                : Sun Dec 30 2001
    copyright            : (C) 2001 by Pascal Brachet
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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <qwidget.h>
#include <ktextedit.h>

/**
  *@author Pascal Brachet
  */

class MessageWidget : public KTextEdit  {
   Q_OBJECT
public: 
	MessageWidget(QWidget *parent, const char *name=0);
	~MessageWidget();

public slots:
   void highlight();
   void insertLine(QString l);
};

#endif
