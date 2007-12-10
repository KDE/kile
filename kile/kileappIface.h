/***************************************************************************
    begin                : sam sep 28 2002
    edit		 : 12/10/2007
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout, 2007 Thomas Braun
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

#include <QObject>

// THIS API is subject to change at anytime, however it will be finalized before the next major release

class KileAppDBusIface : QObject
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "net.sourceforge.kile.main")

	public Q_SLOTS:
		Q_SCRIPTABLE void openDocument(const QString &path);
		Q_SCRIPTABLE void closeDocument();

		Q_SCRIPTABLE void openProject(const QString &path);

		Q_SCRIPTABLE void insertText(const QString &path);
		Q_SCRIPTABLE void setLine( const int line);
		Q_SCRIPTABLE void setActive();

		Q_SCRIPTABLE int runTool(const QString &tool);
		Q_SCRIPTABLE int runToolWithConfig(const QString &tool, const QString & config);
};

#endif
