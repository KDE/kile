/***************************************************************************
                          commandprocess.cpp  -  description
                             -------------------
    begin                : Wed Apr 23 2003
    copyright            : (C) 2003 by Pascal Brachet
    email                : pascal.brachet@club-internet.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "commandprocess.h"

CommandProcess::CommandProcess() : KShellProcess("/bin/sh") {
}

CommandProcess::~CommandProcess(){
}

void CommandProcess::terminate() {
   this->kill();
}


QString CommandProcess::command()
{
	QValueList<QCString> list = this->args();
	QString ret;
	for ( QValueListIterator<QCString> item=list.begin();
		item != list.end();
		item++ )
	{
		ret += QString(*item)+" ";
	}

	return ret;
}

#include "commandprocess.moc"
