/***************************************************************************
                          commandprocess.h  -  description
                             -------------------
    begin                : Wed Apr 23 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMMANDPROCESS_H
#define COMMANDPROCESS_H

#include <kprocess.h>

/**
  *@author Jeroen Wijnhout
  */

class CommandProcess : public KShellProcess  {
   Q_OBJECT
   
public: 
	CommandProcess();
	~CommandProcess();

	QString command();
	
public slots:
   void terminate();
};

#endif
