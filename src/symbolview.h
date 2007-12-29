/***************************************************************************
    begin                : Fri Aug 1 2003
    edit		 : Fri April 6 2007
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout, 2006 - 2007 Thomas Braun
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

#include <q3iconview.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QHideEvent>

#include <k3iconview.h>
#include <QString>
#include <QLabel>

static const QString MFUSGroup = "MostFrequentlyUsedSymbols";
static const QString MFUSprefix = "MFUS";

class SymbolView : public K3IconView
{	
	Q_OBJECT
	
public:
	SymbolView(QWidget *parent=0,int type = -1, const char *name=0);
	~SymbolView();
	enum { MFUS = 0, Relation, Operator, Arrow, MiscMath, MiscText, Delimiters, Greek, Special, Cyrillic, User };
	void writeConfig();
	
private:
	void fillWidget(const QString &prefix);
	void hideEvent( QHideEvent * );
	void contentsMousePressEvent(QMouseEvent *e);
	void extract(const QString& key, int& count, QString &cmd, QStringList &args, QStringList &pkgs);
	void extract(const QString& key, int& count);
	void initPage(int page);

signals:
	void insertText(const QString& text,const QStringList &pkgs);
	void addToList(const Q3IconViewItem *item);

private slots:
	void showToolTip( Q3IconViewItem *item );
	void slotAddToList(const Q3IconViewItem *item);
	void removeToolTip();

private:
	QLabel *m_toolTip;
};

#endif
