/***************************************************************************
                          kileoutputwidget.cpp  -  description
                             -------------------
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

namespace KileWidget
{
	Output::Output(QWidget *parent, const char * name) : KTextEdit(parent, name)
	{
	}

	Output::~Output()
	{}

	void Output::receive(const QString & str)
	{
		int row = (paragraphs() == 0)? 0 : paragraphs()-1;
		int col = paragraphLength(row);
		setCursorPosition(row,col);
		insertAt(str, row, col);
	}
}

#include "kileoutputwidget.moc"
