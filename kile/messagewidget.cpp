/***************************************************************************
                          messagewidget.cpp  -  description
                             -------------------
    begin                : Sun Dec 30 2001
    copyright            : (C) 2001 by Pascal Brachet
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "messagewidget.h"
#include <qregexp.h>
#include <qpainter.h>
#include <qrect.h>
#include <qcolor.h>
#include <qbrush.h>
#include <kmessagebox.h>

MessageWidget::MessageWidget(QWidget *parent, const char *name ) : KTextEdit(parent,name)
{
	setColor(black);
	setPaper(white);
}

MessageWidget::~MessageWidget(){
}

void MessageWidget::highlight()
{
	blockSignals(true); // block signals to avoid recursion
	setUpdatesEnabled(false);
	int cursorParagraph, cursorIndex;

	getCursorPosition( &cursorParagraph, &cursorIndex );

	selectAll();
	setColor(Qt::black);
	removeSelection();
	int index=0;

	for(int i = 0 ; i < paragraphs() ; i++ )
	{
		QString line=text(i);


		///////////// LaTeX error ///////////
		index=line.find("!",0);
		if (index>=0)
		{
			setSelection( i,0, i,paragraphLength(i) );
			setColor(QColor(0xCC, 0x00, 0x00));
			removeSelection();
		}
		///////////// LaTeX warning ///////////
		index=line.find("LaTeX Warning",0);
		if (index>=0)
		{
			setSelection( i, 0, i,paragraphLength(i) );
			setColor(QColor(0x00, 0x00, 0xCC ));
			removeSelection();
		}
		///////////// TeX files ///////////
		index=line.find(".tex",0);
		if (index>=0)
		{
			setSelection( i, 0, i,paragraphLength(i) );
			setColor(QColor(0x00, 0x80, 0x00));
			removeSelection();
		}
   	}
	setCursorPosition( cursorParagraph, cursorIndex );
	setUpdatesEnabled(true);
	blockSignals(false); // block signals to avoid recursion
}

void MessageWidget::insertLine(const QString &l)
{
	int para=0;
  	int index=0;
  	getCursorPosition( &para, &index);
	setColor(Qt::black);
	setPaper(white);
  	insertAt(l+"\n",para,index);
  	setCursorPosition(para+1,0);
}

void MessageWidget::append(const QString &l)
{
	setColor(black);
	setPaper(white);
  	KTextEdit::append(l);
}

void MessageWidget::insertAt(const QString &l, int para, int index)
{
	setColor(black);
	setPaper(white);
  	KTextEdit::insertAt(l,para,index);
}
#include "messagewidget.moc"
