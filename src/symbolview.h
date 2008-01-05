/***************************************************************************
    begin                : Fri Aug 1 2003
    edit   : Fri April 6 2007
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

#include <QListWidget>

class QMouseEvent;

static const QString MFUSGroup = "MostFrequentlyUsedSymbols";
static const QString MFUSprefix = "MFUS";

class SymbolView : public QListWidget
{
		Q_OBJECT

	public:
		SymbolView(QWidget *parent = 0, int type = -1, const char *name = 0);
		~SymbolView();
		enum { MFUS = 0, Relation, Operator, Arrow, MiscMath, MiscText, Delimiters, Greek, Special, Cyrillic, User };
		void writeConfig();

	private:
		void fillWidget(const QString &prefix);
		//void contentsMousePressEvent(QMouseEvent *e);
		void extract(const QString& key, int& count, QString &cmd, QStringList &args, QStringList &pkgs);
		void extract(const QString& key, int& count);
		void initPage(int page);
		QString getToolTip(const QString &key);

	protected:
		virtual void mousePressEvent(QMouseEvent *event);

	Q_SIGNALS:
		void insertText(const QString& text, const QStringList &pkgs);
		void addToList(const QListWidgetItem *item);

	public Q_SLOTS:
		void slotAddToList(const QListWidgetItem *item);
};

#endif
