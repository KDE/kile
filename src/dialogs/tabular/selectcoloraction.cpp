/********************************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
    copyright            : (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "selectcoloraction.h"

#include <QVBoxLayout>
#include <QWidgetAction>

#include <KColorCells>
#include <KColorDialog>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KPushButton>

namespace KileDialog {

SelectColorAction::SelectColorAction(const KIcon &icon, const QString &text, QWidget *parent)
	: KToolBarPopupAction(icon, text, parent)
{
	QWidget *page = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	page->setLayout(layout);

	m_ccColors = new KColorCells(page, 4, 4);
	m_ccColors->setSelectionMode(QAbstractItemView::SingleSelection);
	m_ccColors->setColor(0, Qt::white);
	m_ccColors->setColor(1, Qt::black);
	m_ccColors->setColor(2, Qt::red);
	m_ccColors->setColor(3, Qt::darkRed);
	m_ccColors->setColor(4, Qt::green);
	m_ccColors->setColor(5, Qt::darkGreen);
	m_ccColors->setColor(6, Qt::blue);
	m_ccColors->setColor(7, Qt::darkBlue);
	m_ccColors->setColor(8, Qt::cyan);
	m_ccColors->setColor(9, Qt::darkCyan);
	m_ccColors->setColor(10, Qt::magenta);
	m_ccColors->setColor(11, Qt::darkMagenta);
	m_ccColors->setColor(12, Qt::yellow);
	m_ccColors->setColor(13, Qt::darkYellow);
	m_ccColors->setColor(14, Qt::gray);
	m_ccColors->setColor(15, Qt::darkGray);

	m_pbCustom = new KPushButton(KIcon("kcolorchooser"), i18n("Select custom color..."), page);

	layout->addWidget(m_ccColors);
	layout->addWidget(m_pbCustom);

	QWidgetAction *widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(page);
	popupMenu()->addAction(widgetAction);

	connect(popupMenu(), SIGNAL(aboutToShow()),
	        this, SLOT(slotPopupAboutToShow()));
	connect(m_ccColors, SIGNAL(colorSelected(int, const QColor&)),
	        this, SLOT(slotColorSelected(int, const QColor&)));
	connect(m_pbCustom, SIGNAL(clicked()),
	        this, SLOT(slotCustomClicked()));
}

void SelectColorAction::slotPopupAboutToShow()
{
	m_ccColors->selectionModel()->clearSelection();
}

void SelectColorAction::slotColorSelected(int index, const QColor &color)
{
	Q_UNUSED(index);

	emit colorSelected(color);
	popupMenu()->hide();
}

void SelectColorAction::slotCustomClicked()
{
	QColor color;
	int result = KColorDialog::getColor(color);
	if (result == KColorDialog::Accepted) {
		emit colorSelected(color);
	}
	popupMenu()->hide();
}

}

#include "selectcoloraction.moc"
