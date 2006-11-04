/**************************************************************************************
    begin                : Fri 18-06-2004
    edit 		 : Wed 1 Jun 2006
    copyright            : (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2006 by Thomas Braun (braun@physik.fu-berlin.de)
                           (C) 2006 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

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
	KileSideBar(int size, QWidget *parent = 0, const char *name = 0, Qt::Orientation orientation = Qt::Vertical, bool alwaysShowLabels = false);
	~KileSideBar();

	int addTab(QWidget *tab, const QPixmap &pic, const QString &text = QString::null);

	int currentTab() { return m_nCurrent; }

	bool isVisible() { return !m_bMinimized; }

        void setSize(int sz) { m_nSize = sz; }
        int size() { return m_nSize; }

	QWidget* currentPage();
	void removePage(QWidget *w);

	/**
	 * Shows or hides the tab connected to the widget "w". If the tab to be hidden is
	 * currently selected, the next tab will be shown (cyclically).
	 *
	 * Due to limitations in KMultiTabBar, the result only looks nice if the tab to
	 * be hidden is the last tab in the KMultiTabBar.
	 * @param b set to "true" to show the tab connected to the widget "w", "false" to
	 *          hide it
	 **/
	void setPageVisible(QWidget *w, bool b);

signals:
	void visibilityChanged(bool );

public slots:
	void setVisible(bool );

	virtual void shrink();
	virtual void expand();

	void showTab(int);
	void showPage(QWidget *);
	void toggleTab();
	void switchToTab(int id);

protected:
	QWidgetStack		*m_tabStack;
	KMultiTabBar		*m_tabBar;
	int			m_nTabs;
	int			m_nCurrent;
	QMap<int,int>		m_indexToPage;
	QMap<QWidget*,int>	m_widgetToIndex;
	bool			m_bMinimized;
	int			m_nMinSize, m_nMaxSize, m_nSize;
};

class KileBottomBar : public KileSideBar
{
	Q_OBJECT

public:
	KileBottomBar(int size, QWidget *parent = 0, const char *name = 0);

	void shrink();
	void expand();
};

#endif
