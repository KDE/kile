/********************************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newtabulardialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <KAction>
#include <KIcon>
#include <KLocale>

namespace KileDialog {

NewTabularDialog::NewTabularDialog(QWidget *parent)
	: KDialog(parent)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *pageLayout = new QVBoxLayout();
	pageLayout->setMargin(0);
	pageLayout->setSpacing(KDialog::spacingHint());
	page->setLayout(pageLayout);

	m_tbFormat = new QToolBar(page);
	m_tbFormat->setMovable(false);
	m_tbFormat->setFloatable(false);
	m_tbFormat->setOrientation(Qt::Horizontal);

	KAction *action = new KAction(KIcon("format-justify-left"), i18n("Align Left"), page);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAlignLeft()));
	m_tbFormat->addAction(action);
	action = new KAction(KIcon("format-justify-center"), i18n("Align Center"), page);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAlignCenter()));
	m_tbFormat->addAction(action);
	action = new KAction(KIcon("format-justify-right"), i18n("Align Right"), page);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAlignRight()));
	m_tbFormat->addAction(action);

	m_Table = new QTableWidget(page);

	QWidget *configPage = new QWidget(page);
	QHBoxLayout *configPageLayout = new QHBoxLayout();
	configPageLayout->setMargin(0);
	configPageLayout->setSpacing(KDialog::spacingHint());
	configPage->setLayout(configPageLayout);

	QLabel *label = new QLabel(i18n("Cols:"), configPage);
	m_sbCols = new QSpinBox(configPage);
	m_sbCols->setValue(3);
	label->setBuddy(m_sbCols);
	configPageLayout->addWidget(label);
	configPageLayout->addWidget(m_sbCols);
	label = new QLabel(i18n("Rows:"), configPage);
	m_sbRows = new QSpinBox(configPage);
	m_sbRows->setValue(3);
	label->setBuddy(m_sbRows);
	configPageLayout->addWidget(label);
	configPageLayout->addWidget(m_sbRows);

	pageLayout->addWidget(m_tbFormat);
	pageLayout->addWidget(m_Table);
	pageLayout->addWidget(configPage);

	setMainWidget(page);
	updateColsAndRows();

	connect(m_sbCols, SIGNAL(valueChanged(int)),
	        this, SLOT(updateColsAndRows()));
	connect(m_sbRows, SIGNAL(valueChanged(int)),
	        this, SLOT(updateColsAndRows()));
}

NewTabularDialog::~NewTabularDialog()
{
}

void NewTabularDialog::alignItems(int alignment)
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setTextAlignment(alignment | Qt::AlignVCenter);
	}
}

void NewTabularDialog::updateColsAndRows()
{
	m_Table->setColumnCount(m_sbCols->value());
	m_Table->setRowCount(m_sbRows->value());
}

void NewTabularDialog::slotAlignLeft()
{
	alignItems(Qt::AlignLeft);
}

void NewTabularDialog::slotAlignCenter()
{
	alignItems(Qt::AlignHCenter);
}

void NewTabularDialog::slotAlignRight()
{
	alignItems(Qt::AlignRight);
}

}

#include "newtabulardialog.moc"
