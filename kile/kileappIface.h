/***************************************************************************
    begin                : sam sep 28 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
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
#ifndef KILEAPPDCOPIFACE_H
#define KILEAPPDCOPIFACE_H

#include <dcopobject.h>

class KileAppDCOPIface : virtual public DCOPObject
{
	K_DCOP

	k_dcop:
		virtual void openDocument(const QString &)=0;
		virtual void insertText(const QString &)=0;
		virtual void fileSelected(const QString &)=0; //backwards compatibility
		virtual void closeDocument()=0;
		virtual void openProject(const QString &)=0;
		virtual void setLine( const QString &)=0;
		virtual void setActive()=0;
		virtual int run(const QString &)=0;
		virtual int runWith(const QString &, const QString &)=0;
};

#endif
