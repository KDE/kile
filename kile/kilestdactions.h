//
//
// C++ Interface: kilestdactions
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2003

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILESTDACTIONS_H
#define KILESTDACTIONS_H
class KAction;
class KMainWindow;
 
namespace KileStdActions
{
	void setupStdTags(KileInfo *ki, KMainWindow *parent, KActionCollection *ac);
	void setupBibTags(KMainWindow *parent, KActionCollection *ac);
	void setupMathTags(KMainWindow *parent, KActionCollection *ac);
	void setupActionlistTags(KileInfo *ki, KMainWindow *parent, KActionCollection *ac);
}

#endif
