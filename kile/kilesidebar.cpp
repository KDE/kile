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
 
#include <qwidgetstack.h>
#include <qlayout.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kmultitabbar.h>

#include "symbolview.h"
#include "kilesidebar.h"

KileSideBar::KileSideBar(int size, QWidget *parent, const char *name, Qt::Orientation orientation /*= Vertical*/) : 
	QFrame(parent, name),
	m_nTabs(0),
	m_nCurrent(0),
	m_bMinimized(false),
	m_nMinSize(0),
	m_nMaxSize(1000),
	m_nSize(size)
{
	setFrameStyle(QFrame::Box|QFrame::Plain);
	setLineWidth(1);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
 
	QLayout *layout;

	m_tabStack = new QWidgetStack(this);
	m_tabStack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	KMultiTabBar::KMultiTabBarMode tabbarori = KMultiTabBar::Horizontal;
	KMultiTabBar::KMultiTabBarPosition tabbarpos = KMultiTabBar::Top;
	if ( orientation == Qt::Horizontal ) 
	{
		layout = new QVBoxLayout(this);
		tabbarori = KMultiTabBar::Horizontal;
		tabbarpos = KMultiTabBar::Top;
	}
	else if ( orientation == Qt::Vertical ) 
	{
		layout = new QHBoxLayout(this);
		tabbarori = KMultiTabBar::Vertical;
		tabbarpos = KMultiTabBar::Right;
	}

	m_tabBar = new KMultiTabBar(tabbarori, this);
	m_tabBar->setPosition(tabbarpos);

	#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
		m_tabBar->setStyle(KMultiTabBar::KDEV3ICON);
 	#else
 		m_tabBar->setStyle(KMultiTabBar::KDEV3);
 	#endif

	if ( orientation == Qt::Horizontal )
	{
		setMinimumHeight(m_tabBar->height());
		m_nMinSize = m_tabBar->height();
		m_nMaxSize = m_tabBar->maximumHeight();
		layout->add(m_tabBar);
		layout->add(m_tabStack);
	}
	else if ( orientation == Qt::Vertical )
	{
		setMinimumWidth(m_tabBar->width());
		m_nMinSize = m_tabBar->width();
		m_nMaxSize = m_tabBar->maximumWidth();
		layout->add(m_tabStack);
		layout->add(m_tabBar);
	}
}

KileSideBar::~KileSideBar()
{
}

int KileSideBar::addTab(QWidget *tab, const QPixmap &pic, const QString &text /* = QString::null*/)
{
	m_widgetToIndex[tab] = m_nTabs;

	m_tabBar->appendTab(pic, m_nTabs, text);
	m_tabStack->addWidget(tab, m_nTabs);
	connect(m_tabBar->tab(m_nTabs), SIGNAL(clicked(int)), this, SLOT(showTab(int)));

	return m_nTabs++;
}


void KileSideBar::setVisible(bool show)
{
	kdDebug() << "==KileSideBar::setVisible(" << show << ")===========" << endl;
	if (show) expand();
	else shrink();
}

void KileSideBar::shrink()
{
	if ( !isVisible() ) return;

	m_bMinimized = true;

	m_nSize = width();
	m_nMinSize = minimumWidth();
	m_nMaxSize = maximumWidth();

	m_tabStack->hide();
	setFixedWidth(m_tabBar->width());

	emit visibilityChanged(false);
}

void KileSideBar::expand()
{
	if ( isVisible() ) return;

	m_bMinimized = false;

	m_tabStack->show();

	resize(m_nSize, height());
	setMinimumWidth(m_nMinSize);
	setMaximumWidth(m_nMaxSize);

	emit visibilityChanged(true);
}

QWidget* KileSideBar::currentPage()
{
	return m_tabStack->visibleWidget();
}

void KileSideBar::removePage(QWidget *w) 
{
	QMap<QWidget*,int>::iterator it = m_widgetToIndex.find(w);
	if(it == m_widgetToIndex.end())
	{
		return;
	}
	int index = *it;
	m_tabStack->removeWidget(w);
	disconnect(m_tabBar->tab(index), SIGNAL(clicked(int)), this, SLOT(showTab(int)));
	m_tabBar->removeTab(index);
	m_widgetToIndex.remove(it);
	if(index == m_nCurrent && m_nTabs >= 2)
	{
		showTab((m_nCurrent + 1) % m_nTabs);
	}
	--m_nTabs;
}

void KileSideBar::setPageVisible(QWidget *w, bool b) {
	QMap<QWidget*,int>::iterator it = m_widgetToIndex.find(w);
	if(it == m_widgetToIndex.end())
	{
		return;
	}
	int index = *it;
	KMultiTabBarTab *tab = m_tabBar->tab(index);
	if(tab->isShown() == b) {
		return;
	}
  	tab->setShown(b);
	if(!b && index == m_nCurrent && m_nTabs >= 2)
	{
		showTab((m_nCurrent + 1) % m_nTabs);
	}
}

void KileSideBar::showPage(QWidget *widget)
{
	if ( m_widgetToIndex.contains(widget) )
		switchToTab(m_widgetToIndex[widget]);
}

void KileSideBar::showTab(int id)
{
	if(id >= m_nTabs || id < 0)
	{
		return;
	}
	if ( id != m_nCurrent)
	{
		switchToTab(id);
		if (m_bMinimized) expand();
	}
	else
		toggleTab();
}

void KileSideBar::toggleTab()
{
	if (m_bMinimized) expand();
	else shrink();
}

void KileSideBar::switchToTab(int id)
{
	if(id >= m_nTabs || id < 0) {
		return;
	}
	m_tabBar->setTab(m_nCurrent, false);
	m_tabBar->setTab(id, true);

	m_tabStack->raiseWidget(id);

	m_nCurrent = id;
}

KileBottomBar::KileBottomBar(int size, QWidget *parent, const char *name) :
	KileSideBar(size, parent, name, Qt::Horizontal)
{}

void KileBottomBar::shrink()
{
	m_bMinimized = true;

	m_nSize = height();
	m_nMinSize = minimumHeight();
	m_nMaxSize = maximumHeight();

	m_tabStack->hide();
	setFixedHeight(m_tabBar->height());

	emit visibilityChanged(false);
}

void KileBottomBar::expand()
{
	m_bMinimized = false;

	m_tabStack->show();

	resize(width(), m_nSize);
	setMinimumHeight(m_nMinSize);
	setMaximumHeight(m_nMaxSize);
	
	emit visibilityChanged(true);
}

#include "kilesidebar.moc"
