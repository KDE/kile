/***************************************************************************
    begin                : Sun Jul 29 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal, 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
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
#include <qstringlist.h>

class DocumentationViewer : public KHTMLPart
{
	Q_OBJECT

public:
	DocumentationViewer(QWidget *parent=0, const char *name=0);
	~DocumentationViewer();
	bool backEnable();
	bool forwardEnable();

public slots:
	void home();
	void forward();
	void back();
	void addToHistory( const QString & url );

signals:
	void updateStatus( bool back, bool forward );

protected:
	virtual void urlSelected( const QString &url, int button=0, int state=0,const QString &_target= QString::null, KParts::URLArgs args = KParts::URLArgs());

private:
	QStringList	m_history;
	unsigned int	m_hpos;

};

#endif
