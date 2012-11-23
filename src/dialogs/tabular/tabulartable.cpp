/********************************************************************************************
    begin                : Sunday Jun 27 2008
    Copyright (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
              (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
              (C) 2011 by Felix Mauch (felix_mauch@web.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tabulartable.h"

#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include "tabularcell.h"
#include "tabularcelldelegate.h"

namespace KileDialog {

TabularTable::TabularTable(QWidget *parent)
	: QTableWidget(parent), m_ManualBorderPosition(QPoint(-1, -1)),
	  m_ManualBorderStart(QPoint(-1, -1)),m_LastItem(NULL) {
	setItemDelegate(new TabularCellDelegate(this));
	setShowGrid(false);
	setAttribute(Qt::WA_Hover, true);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	installEventFilter(this);
	m_DefaultMode = selectionMode();
}

bool TabularTable::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == this) {
		if(event->type() == QEvent::KeyPress) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

			if(keyEvent->key() == Qt::Key_Return  && selectedItems().count() == 1) {
				QTableWidgetItem *selectedItem = selectedItems()[0];
				int row = selectedItem->row();
				int column = selectedItem->column();
				if(column < (columnCount() - 1)) {
					selectedItem->setSelected(false);
					item(row, column + 1)->setSelected(true);
					setCurrentItem(item(row, column + 1));
				} else {
					if(row == (rowCount() - 1)) {
						emit rowAppended();
					}
					selectedItem->setSelected(false);
					item(row + 1, 0)->setSelected(true);
					setCurrentItem(item(row + 1, 0));
				}
				return true;
			}
			else if(keyEvent->matches(QKeySequence::Paste)) {
				paste();
			}
		}
		else if(event->type() == QEvent::HoverMove) {
			QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
			QPoint pos = viewport()->mapFromGlobal(mapToGlobal(hoverEvent->pos()));
			QTableWidgetItem *itemAtPos = itemAt(pos);

			if(itemAtPos) {
				if(itemAtPos->row() == 0 && itemAtPos->column() == columnCount() - 1 &&
				   (visualItemRect(itemAtPos).topRight() - pos).manhattanLength() <= 8) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(columnCount());
					m_ManualBorderPosition.setY(0);
				}
				else if(itemAtPos->row() == 0 &&
				        (visualItemRect(itemAtPos).topLeft() - pos).manhattanLength() <= 8) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(itemAtPos->column());
					m_ManualBorderPosition.setY(0);
				}
				else if(itemAtPos->column() == columnCount() - 1 &&
				        (visualItemRect(itemAtPos).bottomRight() - pos).manhattanLength() <= 8) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(columnCount());
					m_ManualBorderPosition.setY(itemAtPos->row() + 1);
				}
				else if((visualItemRect(itemAtPos).bottomLeft() - pos).manhattanLength() <= 8) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(itemAtPos->column());
					m_ManualBorderPosition.setY(itemAtPos->row() + 1);
				}
				else {
					unsetCursor();
					m_ManualBorderPosition.setX(-1);
					m_ManualBorderPosition.setY(-1);
				}
			}
			else {
				unsetCursor();
				m_ManualBorderPosition.setX(-1);
				m_ManualBorderPosition.setY(-1);
			}

			m_HoverPosition = pos;
			viewport()->update();

			return true;
		}
	}

	return QTableWidget::eventFilter(obj, event);
}

void TabularTable::mousePressEvent(QMouseEvent *event)
{
	m_ManualBorderStart = m_ManualBorderPosition;
	if(m_ManualBorderStart.x() > -1) {
		setSelectionMode(QAbstractItemView::NoSelection);
		if(currentItem()) {
			m_LastItem = currentItem();
			currentItem()->setSelected(false);
		}
	}

	QTableWidget::mousePressEvent(event);
}

void TabularTable::mouseReleaseEvent(QMouseEvent *event)
{
	if(m_ManualBorderStart.x() > -1) {
		if(m_ManualBorderPosition.x() > -1) {
			if(m_ManualBorderStart != m_ManualBorderPosition) {
				if(m_ManualBorderStart.x() == m_ManualBorderPosition.x()) {
					int column = (m_ManualBorderStart.x() == columnCount()) ? m_ManualBorderStart.x() - 1 : m_ManualBorderStart.x();
					for(int row = qMin(m_ManualBorderStart.y(), m_ManualBorderPosition.y());
						  row < qMax(m_ManualBorderStart.y(), m_ManualBorderPosition.y()); ++row) {
						TabularCell *cell = static_cast<TabularCell*>(item(row, column));
						int border = cell->border() | ((m_ManualBorderStart.x() == columnCount()) ? TabularCell::Right : TabularCell::Left);
						cell->setBorder(border);
					}
				} else if(m_ManualBorderStart.y() == m_ManualBorderPosition.y()) {
					int row = (m_ManualBorderStart.y() == rowCount()) ? m_ManualBorderStart.y() - 1 : m_ManualBorderStart.y();
					for(int column = qMin(m_ManualBorderStart.x(), m_ManualBorderPosition.x());
						  column < qMax(m_ManualBorderStart.x(), m_ManualBorderPosition.x()); ++column) {
						TabularCell *cell = static_cast<TabularCell*>(item(row, column));
						int border = cell->border() | ((m_ManualBorderStart.y() == rowCount()) ? TabularCell::Bottom : TabularCell::Top);
						cell->setBorder(border);
					}
				}
				viewport()->update();
			}

			m_ManualBorderPosition.setX(-1);
			m_ManualBorderPosition.setY(-1);
		}

		m_ManualBorderStart.setX(-1);
		m_ManualBorderStart.setY(-1);
	}

	setSelectionMode(m_DefaultMode);
	if(m_LastItem) {
		setCurrentItem(m_LastItem);
		currentItem()->setSelected(true);
		m_LastItem = 0;
	}
	QTableWidget::mouseReleaseEvent(event);
}

void TabularTable::paintEvent(QPaintEvent *event)
{
	QPainter painter(viewport());
	if(m_ManualBorderStart.x() > -1) {
		QTableWidgetItem *startItem = item(
			(m_ManualBorderStart.y() == rowCount() ? m_ManualBorderStart.y() - 1 : m_ManualBorderStart.y()),
			(m_ManualBorderStart.x() == columnCount() ? m_ManualBorderStart.x() - 1 : m_ManualBorderStart.x())); 

		int xStart = (m_ManualBorderStart.x() == columnCount() ? visualItemRect(startItem).right() : visualItemRect(startItem).left());
		int yStart = (m_ManualBorderStart.y() == rowCount() ? visualItemRect(startItem).bottom() : visualItemRect(startItem).top());

		QColor color =
			((m_ManualBorderStart != m_ManualBorderPosition) &&
			 ((m_ManualBorderStart.x() == m_ManualBorderPosition.x()) ||
			 (m_ManualBorderStart.y() == m_ManualBorderPosition.y())) ? Qt::darkGreen : Qt::darkRed);
		painter.setPen(QPen(color, 2));
		painter.drawLine(xStart, yStart, m_HoverPosition.x(), m_HoverPosition.y());
	}

	QTableWidget::paintEvent(event);
}

}

void KileDialog::TabularTable::paste()
{
	// Maybe we want to insert content at a certain point in the table
	int selectedRow = 0;
	int selectedCol = 0;
	if(!selectedItems().isEmpty()) {
		selectedRow = selectedItems().first()->row();
		selectedCol = selectedItems().first()->column();
	}

	//Clipboard->QStringlist
	QString selectedText = qApp->clipboard()->text();
	selectedText = selectedText.remove('\r');
	if(selectedText.isEmpty()) {
		KMessageBox::information(this, i18n("There is no content to insert into the table as the clipboard is empty."), i18n("Empty Clipboard"));
		return;
	}
	if(!selectedText.endsWith('\n')) {
		selectedText += '\n';
	}
	QStringList cells = selectedText.split(QRegExp(QLatin1String("\\n|\\t")));
	while(!cells.empty() && cells.back().size() == 0) {
		cells.pop_back(); // strip empty trailing tokens
	}
	int rows = selectedText.count(QLatin1Char('\n'));
	int cols = cells.size() / rows;

	// Fill everything in the tableWidget
	// If needed, new rows and cols get generated on the fly
	int cell = 0;
	for(int row = 0; row < rows; ++row) {
		if(selectedRow + row > (rowCount() - 1)) {
			emit rowAppended();
		}
		for(int col = 0; col < cols; ++col, ++cell) {
			if(selectedCol + col > (columnCount() - 1)) {
				emit colAppended();
			}
			item(selectedRow + row, selectedCol + col)->setText(cells[cell]);
		}
	}
}


#include "tabulartable.moc"
