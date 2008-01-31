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
#include <QPainter>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <KAction>
#include <KColorCells>
#include <KColorDialog>
#include <KComboBox>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KPushButton>

#include "kiledebug.h"
#include "latexcmd.h"

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
	m_ccColors->setSelectionMode(QAbstractItemView::NoSelection);
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

	m_pbCustom = new KPushButton(i18n("Select custom color"), page);

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

NewTabularDialog::NewTabularDialog(KileDocument::LatexCommands *commands, QWidget *parent)
	: KDialog(parent),
	  m_latexCommands(commands),
	  m_clCurrentBackground(Qt::white),
	  m_clCurrentForeground(Qt::black)
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

	m_acLeft = addAction(KIcon("format-justify-left"), i18n("Align Left"), SLOT(slotAlignLeft()), page);
	m_acCenter = addAction(KIcon("format-justify-center"), i18n("Align Center"), SLOT(slotAlignCenter()), page);
	m_acRight = addAction(KIcon("format-justify-right"), i18n("Align Right"), SLOT(slotAlignRight()), page);
	m_tbFormat->addSeparator();
	m_acBold = addAction(KIcon("format-text-bold"), i18n("Bold"), SLOT(slotBold()), page);
	m_acItalic = addAction(KIcon("format-text-italic"), i18n("Italic"), SLOT(slotItalic()), page);
	m_acUnderline = addAction(KIcon("format-text-underline"), i18n("Underline"), SLOT(slotUnderline()), page);
	m_tbFormat->addSeparator();
	m_acJoin = addAction(KIcon("table-join-cells"), i18n("Join Cells"), SLOT(slotJoinCells()), page); // FIXME icon
	m_acSplit = addAction(KIcon("table-split-cells"), i18n("Split Cells"), SLOT(slotSplitCells()), page); // FIXME icon
	m_tbFormat->addSeparator();

	m_acBackground = new SelectColorAction(KIcon("format-fill-color"), i18n("Background"), page);
	m_acBackground->setIcon(generateColorIcon(true));
	connect(m_acBackground, SIGNAL(triggered(bool)), this, SLOT(slotCurrentBackground()));
	connect(m_acBackground, SIGNAL(colorSelected(const QColor&)), this, SLOT(slotBackground(const QColor&)));
	m_tbFormat->addAction(m_acBackground);
	m_acForeground = new SelectColorAction(KIcon("format-stroke-color"), i18n("Foreground"), page);
	m_acForeground->setIcon(generateColorIcon(false));
	connect(m_acForeground, SIGNAL(colorSelected(const QColor&)), this, SLOT(slotForeground(const QColor&)));
	connect(m_acForeground, SIGNAL(triggered(bool)), this, SLOT(slotCurrentForeground()));
	m_tbFormat->addAction(m_acForeground);

	m_tbFormat->addSeparator();
	m_acClearText = addAction(KIcon("edit-clear"), i18n("Clear Text"), SLOT(slotClearText()), page); // FIXME icon
	m_acClearAttributes = addAction(KIcon("edit-clear"), i18n("Clear Attributes"), SLOT(slotClearAttributes()), page); // FIXME icon
	m_acClearAll = addAction(KIcon("edit-clear"), i18n("Clear All"), SLOT(slotClearAll()), page);

	/* checkable items */
	m_acLeft->setCheckable(true);
	m_acCenter->setCheckable(true);
	m_acRight->setCheckable(true);
	m_acBold->setCheckable(true);
	m_acItalic->setCheckable(true);
	m_acUnderline->setCheckable(true);

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

	connect(m_Table, SIGNAL(itemSelectionChanged()),
	        this, SLOT(slotItemSelectionChanged()));
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

	slotItemSelectionChanged();
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

QIcon NewTabularDialog::generateColorIcon(bool background) const
{
	QString iconName = background ? "format-fill-color" : "format-stroke-color";
	QPixmap pixmap = KIconLoader().loadIcon(iconName, KIconLoader::Toolbar);

	QPainter painter(&pixmap);
	QColor color = background ? m_clCurrentBackground : m_clCurrentForeground;
	painter.fillRect(1, pixmap.height() - 7, pixmap.width() - 2, 6, color);
	painter.end();

	return QIcon(pixmap);
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
				QTableWidgetItem *item = new QTableWidgetItem(QString());
				item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				m_Table->setItem(row, i, item);
			}
		}
	}

	if(addedRows > 0) {
		for(int i = m_Table->rowCount() - addedRows; i < m_Table->rowCount(); ++i) {
			m_Table->resizeRowToContents(i);

			// each cell should be an item. This is necessary for selection checking
			for(int column = 0; column < m_Table->columnCount(); ++column) {
				QTableWidgetItem *item = new QTableWidgetItem(QString());
				item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				m_Table->setItem(i, column, item);
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

void NewTabularDialog::slotItemSelectionChanged()
{
	/* unset some items */
	m_acLeft->setChecked(false);
	m_acCenter->setChecked(false);
	m_acRight->setChecked(false);

	/* set all font format items and eventually unset them later */
	m_acBold->setChecked(true);
	m_acItalic->setChecked(true);
	m_acUnderline->setChecked(true);

	/* nothing selected, nothing to do! */
	QList<QTableWidgetItem*> selectedItems = m_Table->selectedItems();
	if(selectedItems.count() == 0) return;

	/* check for alignment */
	int alignment = selectedItems[0]->textAlignment();
	bool sameAlignment = true;
	for(int i = 1; i < selectedItems.count(); ++i) {
		if(selectedItems[i]->textAlignment() != alignment) {
			sameAlignment = false;
			break;
		}
	}
	if(sameAlignment) {
		m_acLeft->setChecked(alignment & Qt::AlignLeft);
		m_acCenter->setChecked(alignment & Qt::AlignHCenter);
		m_acRight->setChecked(alignment & Qt::AlignRight);
	}

	/* check for font format */
	bool unsetBold = false;
	bool unsetItalic = false;
	bool unsetUnderline = false;
	foreach(QTableWidgetItem *item, selectedItems) {
		if(!unsetBold && !item->font().bold()) {
			m_acBold->setChecked(false);
			unsetBold = true;
		}
		if(!unsetItalic && !item->font().italic()) {
			m_acItalic->setChecked(false);
			unsetItalic = true;
		}
		if(!unsetUnderline && !item->font().underline()) {
			m_acUnderline->setChecked(false);
			unsetUnderline = true;
		}
		if(unsetBold && unsetItalic && unsetUnderline) {
			break;
		}
	}

	// TODO set/unset join/split actions
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

void NewTabularDialog::slotBold()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		QFont font = item->font();
		font.setBold(!font.bold());
		item->setFont(font);
	}
	slotItemSelectionChanged();
}

void NewTabularDialog::slotItalic()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		QFont font = item->font();
		font.setItalic(!font.italic());
		item->setFont(font);
	}
	slotItemSelectionChanged();
}

void NewTabularDialog::slotUnderline()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		QFont font = item->font();
		font.setUnderline(!font.underline());
		item->setFont(font);
	}
	slotItemSelectionChanged();
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

void NewTabularDialog::slotBackground(const QColor &color)
{
	m_clCurrentBackground = color;
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setBackground(color);
	}
	m_acBackground->setIcon(generateColorIcon(true));
	m_acForeground->setIcon(generateColorIcon(false));
}

void NewTabularDialog::slotForeground(const QColor &color)
{
	m_clCurrentForeground = color;
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setForeground(color);
	}
	m_acBackground->setIcon(generateColorIcon(true));
	m_acForeground->setIcon(generateColorIcon(false));
}

void NewTabularDialog::slotCurrentBackground()
{
	slotBackground(m_clCurrentBackground);
}

void NewTabularDialog::slotCurrentForeground()
{
	slotForeground(m_clCurrentForeground);
}

void NewTabularDialog::slotClearText()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setText(QString());
	}
}

void NewTabularDialog::slotClearAttributes()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		QFont font = item->font();
		font.setBold(false);
		font.setItalic(false);
		font.setUnderline(false);
		item->setFont(font);
		item->setBackground(Qt::white);
		item->setForeground(Qt::black);
	}
}

void NewTabularDialog::slotClearAll()
{
	slotClearText();
	slotClearAttributes();
}

}

#include "newtabulardialog.moc"
