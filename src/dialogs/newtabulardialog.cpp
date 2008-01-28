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
#include <QList>
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

	addAction(KIcon("format-justify-left"), i18n("Align Left"), SLOT(slotAlignLeft()), page);
	addAction(KIcon("format-justify-center"), i18n("Align Center"), SLOT(slotAlignCenter()), page);
	addAction(KIcon("format-justify-right"), i18n("Align Right"), SLOT(slotAlignRight()), page);
	addAction(KIcon("table-join-cells"), i18n("Join Cells"), SLOT(slotJoinCells()), page); // FIXME icon

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

KAction* NewTabularDialog::addAction(const KIcon &icon, const QString &text, const char *method, QObject *parent)
{
	KAction *action = new KAction(icon, text, parent);
	connect(action, SIGNAL(triggered(bool)), this, method);
	m_tbFormat->addAction(action);

	return action;
}

void NewTabularDialog::alignItems(int alignment)
{
	QList<int> checkedColumns;

	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setTextAlignment(alignment | Qt::AlignVCenter);

		int column = item->column();
		if(!checkedColumns.contains(column)) {
			bool allItemsInColumnAreSelected = true;

			for(int row = 0; row < m_Table->rowCount(); ++row) {
				if(!(m_Table->item(row, column)->isSelected())) {
					allItemsInColumnAreSelected = false;
					break;
				}
			}

			if(allItemsInColumnAreSelected) {
				m_Table->horizontalHeaderItem(column)->setIcon(KIcon(iconForAlignment(alignment)));
			}

			checkedColumns.append(column);
		}
	}
}

inline QString NewTabularDialog::iconForAlignment(int alignment) const
{
	switch(alignment) {
		case Qt::AlignLeft:
			return "format-justify-left";
		case Qt::AlignHCenter:
			return "format-justify-center";
		case Qt::AlignRight:
			return "format-justify-right";
		default:
			return "";
	}
}

void NewTabularDialog::updateColsAndRows()
{
	int addedCols = m_sbCols->value() - m_Table->columnCount();
	int addedRows = m_sbRows->value() - m_Table->rowCount();

	m_Table->setColumnCount(m_sbCols->value());
	m_Table->setRowCount(m_sbRows->value());

	if(addedCols > 0) {
		for(int i = m_Table->columnCount() - addedCols; i < m_Table->columnCount(); ++i) {
			m_Table->setHorizontalHeaderItem(i, new QTableWidgetItem(KIcon("format-justify-left"), QString::number(i + 1)));

			// each cell should be an item. This is necessary for selection checking
			for(int row = m_Table->rowCount() - addedRows; row < m_Table->rowCount(); ++row) {
				m_Table->setItem(row, i, new QTableWidgetItem(QString()));
			}
		}
	}

	if(addedRows > 0) {
		for(int i = m_Table->rowCount() - addedRows; i < m_Table->rowCount(); ++i) {
			m_Table->resizeRowToContents(i);
		}
	}
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

// FIXME joining cells does only work the first time
void NewTabularDialog::slotJoinCells()
{
	QList<QTableWidgetSelectionRange> selectedRanges = m_Table->selectedRanges();

	if(selectedRanges.count() == 0) return;

	foreach(QTableWidgetSelectionRange range, selectedRanges) {
		m_Table->setSpan(range.topRow(), range.leftColumn(),
		                 range.rowCount(), range.columnCount());
	}
}

}

#include "newtabulardialog.moc"
