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

class KAction;
class KMainWindow;

namespace KileStdActions
{
	void setupStdTags(KileInfoInterface *kii, KMainWindow *parent, QPtrList<KAction>*, QStringList*);
	void setupBibTags(KMainWindow *parent);
	void setupMathTags(KMainWindow *parent, QPtrList<KAction>*);
};
