/**************************************************************************************
    Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
              (C) 2006 by Thomas Braun (thomas.braun@virtuell-zuhause.de)
              (C) 2006-2011 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sidebar.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayout>

#include "kiledebug.h"

namespace KileWidget {

SideBar::SideBar(QWidget *parent, Qt::Orientation orientation /*= Vertical*/) :
	QWidget(parent),
	m_orientation(orientation),
	m_minimized(true),
	m_directionalSize(0)
{
	QBoxLayout *layout = NULL, *extraLayout = NULL;
	KMultiTabBar::KMultiTabBarPosition tabbarpos = KMultiTabBar::Top;
	m_extraWidget = new QWidget(this);

	if (orientation == Qt::Horizontal) {
		layout = new QVBoxLayout(this);
		extraLayout = new QHBoxLayout(m_extraWidget);
		tabbarpos = KMultiTabBar::Top;
	}
	else if(orientation == Qt::Vertical) {
		layout = new QHBoxLayout(this);
		extraLayout = new QVBoxLayout(m_extraWidget);
		tabbarpos = KMultiTabBar::Left;
	}

	m_tabStack = new QStackedWidget(this);
	m_tabStack->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	m_tabStack->setVisible(false);

	m_tabBar = new KMultiTabBar(tabbarpos, this);
	m_tabBar->setStyle(KMultiTabBar::KDEV3ICON);

	m_extraWidget->setLayout(extraLayout);
	extraLayout->addWidget(m_tabBar);

	if(orientation == Qt::Horizontal) {
		layout->addWidget(m_extraWidget);
		layout->addWidget(m_tabStack);
		m_tabBar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	}
	else if(orientation == Qt::Vertical) {
		layout->addWidget(m_tabStack);
		layout->addWidget(m_extraWidget);
		m_tabBar->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	}

	layout->setMargin(0);
	layout->setSpacing(0);
	extraLayout->setMargin(0);
	extraLayout->setSpacing(0);

	setLayout(layout);
}

SideBar::~SideBar()
{
}

int SideBar::addPage(QWidget *widget, const QPixmap &pic, const QString &text /* = QString()*/)
{
	int index = m_tabStack->addWidget(widget);
	m_tabBar->appendTab(pic, index, text);
	connect(m_tabBar->tab(index), SIGNAL(clicked(int)), this, SLOT(tabClicked(int)));

	switchToTab(index);

	return index;
}

void SideBar::removePage(QWidget *w)
{
	int nTabs = m_tabStack->count();
	int index = m_tabStack->indexOf(w);
	int currentIndex = currentTab();
	m_tabStack->removeWidget(w);
	disconnect(m_tabBar->tab(index), SIGNAL(clicked(int)), this, SLOT(showTab(int)));
	m_tabBar->removeTab(index);
	if(index == currentIndex && nTabs >= 2) {
		switchToTab(findNextShownTab(index));
	}
}

QWidget* SideBar::currentPage()
{
	if(isMinimized()) {
		return NULL;
	}

	return m_tabStack->currentWidget();
}

int SideBar::currentTab()
{
	if(m_minimized) {
		return -1;
	}

	return m_tabStack->currentIndex();
}

bool SideBar::isMinimized()
{
	return m_minimized;
}

int SideBar::count()
{
	return m_tabStack->count();
}

void SideBar::shrink()
{
	KILE_DEBUG();
	if(isMinimized()) {
		return;
	}

	int currentIndex = currentTab(); // before changing m_minimized!
	m_tabStack->setVisible(false);
	m_minimized = true;

	if(m_orientation == Qt::Horizontal) {
		m_directionalSize = height();
		setFixedHeight(m_tabBar->sizeHint().height());
	}
	else if(m_orientation == Qt::Vertical) {
		m_directionalSize = width();
		setFixedWidth(m_tabBar->sizeHint().width());
	}

	// deselect the currect tab
	m_tabBar->setTab(currentIndex, false);

	emit visibilityChanged(false);
}

void SideBar::expand()
{
	KILE_DEBUG();
	if(!isMinimized()) {
		return;
	}

	KILE_DEBUG() << "directional size = " << m_directionalSize;
	if(m_orientation == Qt::Horizontal) {
		setMinimumHeight(0);
		setMaximumHeight(QWIDGETSIZE_MAX);
		m_tabStack->resize(m_tabStack->width(), m_directionalSize);
	}
	else if(m_orientation == Qt::Vertical) {
		setMinimumWidth(0);
		setMaximumWidth(QWIDGETSIZE_MAX);
		m_tabStack->resize(m_directionalSize, m_tabStack->height());
	}

	m_tabStack->setVisible(true);
	m_minimized = false;

	emit visibilityChanged(true);
}

void SideBar::tabClicked(int i)
{
	int currentIndex = currentTab();

	if(i == currentIndex && !isMinimized()) {
		shrink();
	}
	else {
		switchToTab(i);
	}
}

int SideBar::findNextShownTab(int i)
{
	int nTabs = m_tabStack->count();
	if(nTabs <= 0) {
		return -1;
	}
	for(int j = 1; j < nTabs; ++j) {
		int index = (i + j) % nTabs;

		if(m_tabBar->tab(index)->isVisible()) {
			return index;
		}
	}
	return -1;
}

void SideBar::setPageVisible(QWidget *w, bool b)
{
	int nTabs = m_tabStack->count();
	int index = m_tabStack->indexOf(w);
	int currentIndex = currentTab();

	KMultiTabBarTab *tab = m_tabBar->tab(index);
	tab->setVisible(b);
	if(!b && index == currentIndex && nTabs >= 2) {
		switchToTab(findNextShownTab(index));
	}
}

void SideBar::showPage(QWidget *widget)
{
	KILE_DEBUG() << "===SideBar::showPage(" << widget << ")";
	int i = m_tabStack->indexOf(widget);
	KILE_DEBUG() << "i is " << i;
	if(i >= 0) {
		switchToTab(i);
	}
}

int SideBar::directionalSize()
{
	if(m_minimized) {
		return m_directionalSize;
	}

	if(m_orientation == Qt::Horizontal) {
		return m_tabStack->height();
	}
	else if(m_orientation == Qt::Vertical) {
		return m_tabStack->width();
	}

	return 0;
}

void SideBar::setDirectionalSize(int i)
{
	KILE_DEBUG() << "size = " << i;
	m_directionalSize = i;
	if(m_orientation == Qt::Horizontal) {
		m_tabStack->resize(m_tabStack->width(), i);
	}
	else if(m_orientation == Qt::Vertical) {
		m_tabStack->resize(i, m_tabStack->height());
	}
}

void SideBar::addExtraWidget(QWidget *w)
{
	m_extraWidget->layout()->addWidget(w);
}

void SideBar::switchToTab(int id)
{
	KILE_DEBUG() << "id = " << id;
	int nTabs = m_tabStack->count();
	int currentIndex = currentTab();

	if(id >= nTabs || id < 0 || m_tabBar->tab(id)->isHidden()) {
		shrink();
		return;
	}
	// currentIndex == id is allowed if we are expanding, for example
	if(currentIndex >= 0) {
		m_tabBar->setTab(currentIndex, false);
	}
	m_tabBar->setTab(id, true);

	m_tabStack->setCurrentIndex(id);
	expand();
}

BottomBar::BottomBar(QWidget *parent) : SideBar(parent, Qt::Horizontal)
{
}

}

#include "sidebar.moc"
