/*************************************************************************************
    begin                : Sun Dec 21 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include "widgets/outputview.h"
#include "kiledebug.h"

namespace KileWidget {

OutputView::OutputView(QWidget *parent) : KTextEdit(parent)
{
	setReadOnly(true);
	QPalette customPalette = palette();
	customPalette.setColor(QPalette::Base, QColor(Qt::white));
	customPalette.setColor(QPalette::Window, QColor(Qt::white));
	setPalette(customPalette);
}

OutputView::~OutputView()
{
}

void OutputView::receive(const QString& str)
{
	static QString line;

	//find newline symbol
	//only output if we have receive one or more
	//full lines of text
	int newLineAt = str.lastIndexOf('\n');
	if(newLineAt != -1) {
		line += str.left(newLineAt); //don't copy the newline char
		line.replace('<', "&lt;");
		line.replace('>', "&gt;");
		append(line);
		line = str.mid(newLineAt + 1);
	}
	else {
		line += str;
	}
}

}

#include "outputview.moc"
