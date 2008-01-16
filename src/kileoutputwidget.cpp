/***************************************************************************
    begin                : Sun Dec 21 2003
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
 ****************************************************************************/

#include "kileoutputwidget.h"
#include "kiledebug.h"

namespace KileWidget
{
Output::Output(QWidget *parent) : KTextEdit(parent)
{
	setReadOnly(true);
}

Output::~Output()
{
}

void Output::receive(const QString & str)
{
	static QString line = "";

	//find newline symbol
	//only output if we have receive one or more
	//full lines of text
	int newLineAt = str.findRev('\n');
	if(newLineAt != -1)
	{
		line += str.left(newLineAt); //don't copy the newline char
		line.replace('<', "&lt;");
		line.replace('>', "&gt;");
		append(line);
		line = str.mid(newLineAt + 1);
	}
	else
		line += str;
}
}

#include "kileoutputwidget.moc"
