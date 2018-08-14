/********************************************************************************************
    begin                : sam sep 28 2002
    edit		 : 12/10/2007
    copyright            : (C) 2002 - 2003 by Pascal Brachet (Jeroen.Wijnhout@kdemail.net)
                               2003 by Jeroen Wijnhout
                               2007-2008 by Thomas Braun
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KILEAPPIFACE_H
#define KILEAPPIFACE_H

#include <QObject>

/*
 * 	This files servers as our source for the xml file net.sourceforge.kile.main.xml
 *	After changing this file execute
 *	qdbuscpp2xml -a kileappIface.h -o net.sourceforge.kile.main.xml
 *
 *	This API is subject to change at anytime, however it will be finalized before the next major release
 *
 */

class MainAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "net.sourceforge.kile.main")

public Q_SLOTS:
    Q_SCRIPTABLE void openDocument(const QString &path);
    Q_SCRIPTABLE void closeDocument();

    Q_SCRIPTABLE void openProject(const QString &path);

    Q_SCRIPTABLE void insertText(const QString &path);
    Q_SCRIPTABLE void setLine(const QString& line);
    Q_SCRIPTABLE void setActive();

    Q_SCRIPTABLE int runTool(const QString &tool);
    Q_SCRIPTABLE int runToolWithConfig(const QString &tool, const QString & config);
};

#endif
