/***************************************************************************
                          kileinfointerface.h  -  description
                             -------------------
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#ifndef KILEINFOINTERFACE_H
#define KILEINFOINTERFACE_H

#include <qstring.h>

class KileInfoInterface
{

public:
	KileInfoInterface() {m_name=QString::null;}
	virtual ~KileInfoInterface() {}

public:
	virtual QString getName() =0;
	virtual QString getShortName() =0;

private:
	QString m_name;
};

#endif
