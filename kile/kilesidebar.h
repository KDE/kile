/***************************************************************************
                          kilesidebar.h  -  description
                             -------------------
    begin                : Fri 18-06-2004
    copyright            : (C) 2004 by Jeroen Wijnhout
    email                :  Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef KILESIDEBAR_H
#define KILESIDEBAR_H

#include <qframe.h>
#include <qmap.h>

class QWidgetStack;
class KMultiTabBar;
class SymbolView;

/**
@author Jeroen Wijnhout
*/
class KileSideBar : public QFrame
{
	Q_OBJECT

public:
	KileSideBar(QWidget *parent = 0, const char *name = 0, Qt::Orientation orientation = Qt::Vertical, bool alwaysShowLabels = false);
	~KileSideBar();

	int addTab(QWidget *tab, const QPixmap &pic, const QString &text = QString::null);
	int addSymbolTab(int page, const QPixmap &pic, const QString &text = QString::null);

	int currentTab() { return m_nCurrent; }
	SymbolView* symbolView() { return m_symbolTab; }

	bool isVisible() { return !m_bMinimized; }

	QWidget* currentPage();

signals:
	void visibilityChanged(bool );

public slots:
	void setVisible(bool );

	virtual void shrink();
	virtual void expand();

	void showTab(int, bool = true);
	void showPage(QWidget *);
	bool isSymbolView(int id) { return m_isSymbolView[id]; }

protected:
	QWidgetStack		*m_tabStack;
	KMultiTabBar		*m_tabBar;
	SymbolView			*m_symbolTab;
	int					m_nTabs;
	int					m_nCurrent;
	QMap<int,int>		m_indexToPage;
	QMap<QWidget*,int>	m_widgetToIndex;
	QMap<int,bool>		m_isSymbolView;
	bool				m_bMinimized;
	int					m_nMinSize, m_nMaxSize, m_nSize;
};

class KileBottomBar : public KileSideBar
{
	Q_OBJECT

public:
	KileBottomBar(QWidget *parent = 0, const char *name = 0);

	void shrink();
	void expand();
};

#endif
