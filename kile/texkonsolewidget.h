/***************************************************************************
                          texkonsolewidget.h  -  description
                             -------------------
    begin                : Sat Dec 8 2001
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

#ifndef TEXKONSOLEWIDGET_H
#define TEXKONSOLEWIDGET_H

#include <qwidget.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <kparts/part.h>

/**
  *@author Brachet Pascal
  @author Jeroen Wijnhout
  */

class TexKonsoleWidget : public QWidget
{
  Q_OBJECT

public:

  TexKonsoleWidget(QWidget* parent, const char* name=0);
  ~TexKonsoleWidget();


public slots:
  void SetDirectory(QString dirname);
  void activate();

protected:

  virtual void showEvent(QShowEvent *ev);
	void spawn();

private:

    KParts::ReadOnlyPart *part;
    QVBoxLayout *vbox;
    bool present;
private slots:
    void slotDestroyed ();

};


#endif
