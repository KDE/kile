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

#include "tabulartable.h"

#include <QEvent>
#include <QHeaderView>
#include <QKeyEvent>

#include <KDebug>

#include "tabularcell.h"
#include "tabularcelldelegate.h"

namespace KileDialog {

NewTabularTable::NewTabularTable(QWidget *parent)
	: QTableWidget(parent), m_ManualBorderPosition(QPoint(-1, -1)),
	  m_ManualBorderStart(QPoint(-1, -1)) {
	setItemDelegate(new TabularCellDelegate(this));
	setShowGrid(false);
	setAttribute(Qt::WA_Hover, true);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	installEventFilter(this);
	m_DefaultMode = selectionMode();
}

bool NewTabularTable::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == this) {
		if(event->type() == QEvent::KeyPress && selectedItems().count() == 1) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

			if(keyEvent->key() == Qt::Key_Return) {
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
		} else if(event->type() == QEvent::HoverMove) {
			QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
			QPoint pos = viewport()->mapFromGlobal(mapToGlobal(hoverEvent->pos()));
			QTableWidgetItem *itemAtPos = itemAt(pos);

			if(itemAtPos) {
				if(itemAtPos->row() == 0 && itemAtPos->column() == columnCount() - 1 &&
				   (visualItemRect(itemAtPos).topRight() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(columnCount());
					m_ManualBorderPosition.setY(0);
				} else if(itemAtPos->row() == 0 &&
				          (visualItemRect(itemAtPos).topLeft() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(itemAtPos->column());
					m_ManualBorderPosition.setY(0);
				} else if(itemAtPos->column() == columnCount() - 1 &&
				          (visualItemRect(itemAtPos).bottomRight() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(columnCount());
					m_ManualBorderPosition.setY(itemAtPos->row() + 1);
				} else if((visualItemRect(itemAtPos).bottomLeft() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
					m_ManualBorderPosition.setX(itemAtPos->column());
					m_ManualBorderPosition.setY(itemAtPos->row() + 1);
				} else {
					unsetCursor();
					m_ManualBorderPosition.setX(-1);
					m_ManualBorderPosition.setY(-1);
				}
			} else {
				unsetCursor();
				m_ManualBorderPosition.setX(-1);
				m_ManualBorderPosition.setY(-1);
			}

			return true;
		}
	}

	return QTableWidget::eventFilter(obj, event);
}

void NewTabularTable::mousePressEvent(QMouseEvent *event)
{
	m_ManualBorderStart = m_ManualBorderPosition;
	if(m_ManualBorderStart.x() > -1) {
		setSelectionMode(QAbstractItemView::NoSelection);
	}

	QTableWidget::mousePressEvent(event);
}

void NewTabularTable::mouseReleaseEvent(QMouseEvent *event)
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
				QWidget::repaint();
			}

			m_ManualBorderPosition.setX(-1);
			m_ManualBorderPosition.setY(-1);
		}

		m_ManualBorderStart.setX(-1);
		m_ManualBorderStart.setY(-1);
	}

	setSelectionMode(m_DefaultMode);
	QTableWidget::mouseReleaseEvent(event);
}

}

#include "tabulartable.moc"
