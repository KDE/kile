/***************************************************************************
                          kilesidebar.cpp  -  description
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
 
#include <qwidgetstack.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kmultitabbar.h>

#include "symbolview.h"
#include "kilesidebar.h"

KileSideBar::KileSideBar(QWidget *parent, const char *name, Qt::Orientation orientation /*= Vertical*/, bool alwaysShowLabels /*= false*/) : 
	QFrame(parent, name),
	m_symbolTab(0L),
	m_nTabs(0),
	m_nCurrent(0),
	m_bMinimized(false),
	m_nMinSize(0),
	m_nMaxSize(1000),
	m_nSize(400)
{
	setFrameStyle(QFrame::Box|QFrame::Plain);
	setLineWidth(1);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
 
	QLayout *layout;

	m_tabStack = new QWidgetStack(this);

	KMultiTabBar::KMultiTabBarMode tabbarori;
	KMultiTabBar::KMultiTabBarPosition tabbarpos;
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
	m_tabBar->setStyle(alwaysShowLabels ? KMultiTabBar::KDEV3 : KMultiTabBar::VSNET);

	if ( orientation == Qt::Horizontal )
	{
		setMinimumHeight(m_tabBar->height());
		layout->add(m_tabBar);
		layout->add(m_tabStack);
	}
	else if ( orientation == Qt::Vertical )
	{
		setMinimumWidth(m_tabBar->width());
		layout->add(m_tabStack);
		layout->add(m_tabBar);
	}

	m_symbolTab = new SymbolView(m_tabStack);
	m_tabStack->addWidget(m_symbolTab, 0);
	m_nTabs++;
}


KileSideBar::~KileSideBar()
{
}

int KileSideBar::addTab(QWidget *tab, const QPixmap &pic, const QString &text /* = QString::null*/)
{
	m_isSymbolView[m_nTabs] = false;
	m_widgetToIndex[tab] = m_nTabs;

	m_tabBar->appendTab(pic, m_nTabs, text);
	m_tabStack->addWidget(tab, m_nTabs);
	connect(m_tabBar->tab(m_nTabs), SIGNAL(clicked(int)), this, SLOT(showTab(int)));

	showTab(m_nTabs);

	return m_nTabs++;
}

int KileSideBar::addSymbolTab(int page, const QPixmap &pic, const QString &text /* = QString::null*/)
{
	m_widgetToIndex[m_symbolTab] = m_nTabs;
	m_isSymbolView[m_nTabs] = true;
	m_indexToPage[m_nTabs] = page;

	m_tabBar->appendTab(pic, m_nTabs, text);
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

void KileSideBar::showPage(QWidget *widget)
{
	if ( m_widgetToIndex.contains(widget) )
		showTab(m_widgetToIndex[widget], false);
}

void KileSideBar::showTab(int id, bool toggle /*= true*/)
{
	if ( toggle & (id == m_nCurrent) )
	{
		if (m_bMinimized) expand();
		else shrink();

		return;
	}

	if (m_bMinimized) { expand(); m_bMinimized = false; }

	m_tabBar->setTab(m_nCurrent, false);
	m_tabBar->setTab(id, true);

	int tabId = isSymbolView(id) ? 0 : id;
	if ( isSymbolView(id) ) m_symbolTab->showPage(m_indexToPage[id]);

	m_tabStack->raiseWidget(tabId);

	m_nCurrent = id;
}

KileBottomBar::KileBottomBar(QWidget *parent, const char *name) :
	KileSideBar(parent, name, Qt::Horizontal, true)
{}

void KileBottomBar::shrink()
{
	m_bMinimized = true;

	m_nSize = height();
	m_nMinSize = minimumHeight();
	m_nMaxSize = maximumHeight();

	m_tabStack->hide();
	setFixedHeight(m_tabBar->height());
}

void KileBottomBar::expand()
{
	m_bMinimized = false;

	m_tabStack->show();

	resize(width(), m_nSize);
	setMinimumHeight(m_nMinSize);
	setMaximumHeight(m_nMaxSize);
}

#include "kilesidebar.moc"
