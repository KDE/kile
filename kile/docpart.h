/***************************************************************************
                          docpart.h  -  description
                             -------------------
    begin                : Sun Jul 29 2001
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

#ifndef DOCPART_H
#define DOCPART_H

#include <khtml_part.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <qstringlist.h>

/**
  *@author Brachet Pascal
  */

class docpart : public KHTMLPart  {
   Q_OBJECT
public:
	docpart(QWidget *parent=0, const char *name=0);
	~docpart();
	bool backEnable();
	bool forwardEnable();

public slots:
  void home();
	void forward();
	void back();
 	void addToHistory( QString url );

signals:
	void updateStatus( bool back, bool forward );

protected:
  virtual void urlSelected( const QString &url, int button=0, int state=0,const QString &_target= QString::null, KParts::URLArgs args = KParts::URLArgs());

private:
	QStringList history;
	unsigned int hpos;

};

#endif
