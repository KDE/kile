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

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <KAction>
#include <KComboBox>
#include <KIcon>
#include <KLocale>
#include <KMessageBox>

#include "kiledebug.h"
#include "latexcmd.h"

namespace KileDialog {

NewTabularDialog::NewTabularDialog(KileDocument::LatexCommands *commands, QWidget *parent)
	: KDialog(parent), m_latexCommands(commands)
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
	addAction(KIcon("table-split-cells"), i18n("Split Cells"), SLOT(slotSplitCells()), page); // FIXME icon

	m_Table = new QTableWidget(page);

	QGroupBox *configPage = new QGroupBox(i18n("Environment"), page);
	QGridLayout *configPageLayout = new QGridLayout();
	configPageLayout->setMargin(KDialog::marginHint());
	configPageLayout->setSpacing(KDialog::spacingHint());
	configPage->setLayout(configPageLayout);

	QLabel *label = new QLabel(i18n("Name:"), configPage);
	m_cmbName = new KComboBox(configPage);
	label->setBuddy(m_cmbName);
	configPageLayout->addWidget(label, 0, 0);
	configPageLayout->addWidget(m_cmbName, 0, 1);
	label = new QLabel(i18n("Parameter:"), configPage);
	m_cmbParameter = new KComboBox(configPage);
	label->setBuddy(m_cmbParameter);
	configPageLayout->addWidget(label, 1, 0);
	configPageLayout->addWidget(m_cmbParameter, 1, 1);

	label = new QLabel(i18n("Number of rows:"), configPage);
	m_sbRows = new QSpinBox(configPage);
	m_sbRows->setValue(3);
	label->setBuddy(m_sbRows);
	configPageLayout->addWidget(label, 0, 2);
	configPageLayout->addWidget(m_sbRows, 0, 3);
	label = new QLabel(i18n("Number of cols:"), configPage);
	m_sbCols = new QSpinBox(configPage);
	m_sbCols->setValue(3);
	label->setBuddy(m_sbCols);
	configPageLayout->addWidget(label, 1, 2);
	configPageLayout->addWidget(m_sbCols, 1, 3);

	m_cbStarred = new QCheckBox(i18n("Use starred version"), configPage);
	m_cbCenter = new QCheckBox(i18n("Center"), configPage);
	m_cbCenter->setChecked(true);
	m_cbBooktabs = new QCheckBox(i18n("Use booktabs package"), configPage);
	m_cbBullets = new QCheckBox(i18n("Insert bullets"), configPage);
	m_cbBullets->setChecked(true);
	configPageLayout->addWidget(m_cbStarred, 2, 0, 1, 2);
	configPageLayout->addWidget(m_cbCenter, 2, 2, 1, 2);
	configPageLayout->addWidget(m_cbBooktabs, 3, 0, 1, 2);
	configPageLayout->addWidget(m_cbBullets, 3, 2, 1, 2);

	pageLayout->addWidget(m_tbFormat);
	pageLayout->addWidget(m_Table);
	pageLayout->addWidget(configPage);

	// whats this texts
	// NOTE add texts for table and table header
	m_cmbName->setWhatsThis(i18n("Choose an environment."));
	m_cmbParameter->setWhatsThis(i18n("Optional parameter for the chosen environment."));
	m_sbRows->setWhatsThis(i18n("Choose the number of table rows."));
	m_sbCols->setWhatsThis(i18n("Choose the number of table columns."));
	m_cbCenter->setWhatsThis(i18n("The tabular will be centered."));
	m_cbBooktabs->setWhatsThis(i18n("Use line commands of the booktabs package."));
	m_cbStarred->setWhatsThis(i18n("Use the starred version of this environment."));
	m_cbBullets->setWhatsThis(i18n("Insert bullets in each cell. Alt+Ctrl+Right and Alt+Ctrl+Left will move very quick from one cell to another."));

	setMainWidget(page);
	initEnvironments();
	updateColsAndRows();

	connect(m_cmbName, SIGNAL(activated(const QString&)),
	        this, SLOT(slotEnvironmentChanged(const QString&)));
	connect(m_sbCols, SIGNAL(valueChanged(int)),
	        this, SLOT(updateColsAndRows()));
	connect(m_sbRows, SIGNAL(valueChanged(int)),
	        this, SLOT(updateColsAndRows()));
}

NewTabularDialog::~NewTabularDialog()
{
}

void NewTabularDialog::initEnvironments()
{
	/* read all tabular environments and insert them into the combobox */
	QStringList list;
	QStringList::ConstIterator it;
	m_latexCommands->commandList(list, KileDocument::CmdAttrTabular, false);
	m_cmbName->addItems(list);
	
	// FIXME differ between array and tabular environment

	// refresh other gui elements regarding environment combo box
	slotEnvironmentChanged(m_cmbName->currentText());
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

	// check whether content could be deleted when shrinking the table
	if(addedCols < 0) {
		bool hasContent = false;
		for(int column = m_Table->columnCount() + addedCols; column < m_Table->columnCount(); ++column) {
			for(int row = 0; row < m_Table->rowCount(); ++row) {
				if(m_Table->item(row, column) && !(m_Table->item(row, column)->text().isEmpty())) {
					hasContent = true;
					break;
				}
			}
			if(hasContent) break;
		}

		if(hasContent) {
			if(KMessageBox::questionYesNo(m_Table, i18n("Setting the new size for the table will delete content. Are you sure to set the new size?"), i18n("Resizing table")) == KMessageBox::No) {
				m_sbCols->setValue(m_Table->columnCount());
				return;
			}
		}
	}

	// check whether content could be deleted when shrinking the table
	if(addedRows < 0) {
		bool hasContent = false;
		for(int row = m_Table->rowCount() + addedRows; row < m_Table->rowCount(); ++row) {
			for(int column = 0; column < m_Table->columnCount(); ++column) {
				if(m_Table->item(row, column) && !(m_Table->item(row, column)->text().isEmpty())) {
					hasContent = true;
					break;
				}
			}
			if(hasContent) break;
		}

		if(hasContent) {
			if(KMessageBox::questionYesNo(m_Table, i18n("Setting the new size for the table will delete content. Are you sure to set the new size?"), i18n("Resizing table")) == KMessageBox::No) {
				m_sbRows->setValue(m_Table->rowCount());
				return;
			}
		}
	}

	m_Table->setColumnCount(m_sbCols->value());
	m_Table->setRowCount(m_sbRows->value());

	if(addedCols > 0) {
		for(int i = m_Table->columnCount() - addedCols; i < m_Table->columnCount(); ++i) {
			m_Table->setHorizontalHeaderItem(i, new QTableWidgetItem(KIcon("format-justify-left"), QString::number(i + 1)));

			// each cell should be an item. This is necessary for selection checking
			for(int row = 0; row < m_Table->rowCount(); ++row) {
				m_Table->setItem(row, i, new QTableWidgetItem(QString()));
			}
		}
	}

	if(addedRows > 0) {
		for(int i = m_Table->rowCount() - addedRows; i < m_Table->rowCount(); ++i) {
			m_Table->resizeRowToContents(i);

			// each cell should be an item. This is necessary for selection checking
			for(int column = 0; column < m_Table->columnCount(); ++column) {
				m_Table->setItem(i, column, new QTableWidgetItem(QString()));
			}
		}
	}
}

void NewTabularDialog::slotEnvironmentChanged(const QString &environment)
{
	// clear parameter combobox
	m_cmbParameter->clear();
	m_cmbParameter->setEnabled(false);

	// look for environment parameter in dictionary
	KileDocument::LatexCmdAttributes attr;
	if(m_latexCommands->commandAttributes(environment, attr)) {
		// starred version
		m_cbStarred->setEnabled(attr.starred);

		// option
		if(attr.option.indexOf('[') == 0) {
			QStringList optionlist = attr.option.split("");
			if(optionlist.count() > 2) {
				// ok, let's enable it
				m_cmbParameter->setEnabled(true);
				m_cmbParameter->insertItem(QString());
				// insert some options
				for(int i = 1; i < optionlist.count() - 1; ++i) {
					m_cmbParameter->insertItem(optionlist[i]);
				}
			}
		}
	}

	// NOTE do not forget the align list
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

void NewTabularDialog::slotJoinCells()
{
	QList<QTableWidgetItem*> selectedItems = m_Table->selectedItems();
	if(selectedItems.count() < 2) {
		KILE_DEBUG() << "cannot join cells, because selectedItems.count() < 2";
		return;
	}

	/* check whether all selected items are in the same row */
	int row = selectedItems[0]->row();
	for(int i = 1; i < selectedItems.count(); ++i) {
		if(selectedItems[i]->row() != row) {
			KILE_DEBUG() << "cannot join cells, because of different rows";
			return;
		}
	}

	/* check whether all selected items are adjacent */
	QList<int> columns;
	foreach(QTableWidgetItem* item, selectedItems) {
		columns.append(item->column());
	}
	qSort(columns);
	if((columns.last() - columns.first()) != (columns.size() - 1)) {
		KILE_DEBUG() << "cannot join cells, because not all cells are adjacent";
		return;
	}

	int newColumnSpan = columns.size();

	/* check for already joined cells in range */
	foreach(int column, columns) {
		int thisColumnSpan = m_Table->columnSpan(row, column);
		if(thisColumnSpan > 1) {
			newColumnSpan = qMax(newColumnSpan, thisColumnSpan + column - columns.first());
			m_Table->setSpan(row, column, 1, 1);
		}
	}

	/* everything's fine -> join the cells */
	m_Table->setSpan(row, columns.first(), 1, newColumnSpan);
}

void NewTabularDialog::slotSplitCells()
{
	/* one item has to be selected */
	if(m_Table->selectedItems().count() != 1) return;

	QTableWidgetItem *selectedItem = m_Table->selectedItems()[0];

	if(m_Table->columnSpan(selectedItem->row(), selectedItem->column()) > 1) {
		m_Table->setSpan(selectedItem->row(), selectedItem->column(), 1, 1);
	}
}

}

#include "newtabulardialog.moc"
