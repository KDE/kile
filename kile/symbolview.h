/***************************************************************************
    begin                : Fri Aug 1 2003
    edit		 : Wed Jun 1 2006
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout, 2006 Thomas Braun
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

#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include <kiconview.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qlabel.h>


class SymbolView : public KIconView
{	
	Q_OBJECT
	
public:
	SymbolView(QWidget *parent=0, const char *name=0,int type = -1);
	~SymbolView();
	enum {Relation = 0, Operator, Arrow, MiscMath, MiscText, Delimiters, Greek, Special, Cyrillic, User};
	
private:
	void fillWidget(const QString &addition);
	void hideEvent( QHideEvent * );
	void contentsMousePressEvent(QMouseEvent *e);
	void initPage(int page);

signals:
	void requestedText(const QString& text);
	void insertText(const QString& text);

private slots:
	void showToolTip( QIconViewItem *item );
	void removeToolTip();

private:
	QLabel *toolTip;
	QString name;
	
};


#endif
