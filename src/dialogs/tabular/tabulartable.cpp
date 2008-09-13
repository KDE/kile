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

#include "tabularcelldelegate.h"

namespace KileDialog {

NewTabularTable::NewTabularTable(QWidget *parent) : QTableWidget(parent) {
	setItemDelegate(new TabularCellDelegate(this));
	setShowGrid(false);
	setAttribute(Qt::WA_Hover, true);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	installEventFilter(this);
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
				} else if(itemAtPos->row() == 0 &&
				          (visualItemRect(itemAtPos).topLeft() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
				} else if(itemAtPos->column() == columnCount() - 1 &&
				          (visualItemRect(itemAtPos).bottomRight() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
				} else if((visualItemRect(itemAtPos).bottomLeft() - pos).manhattanLength() <= 3) {
					setCursor(Qt::CrossCursor);
				} else {
					unsetCursor();
				}
			} else {
				unsetCursor();
			}

			return true;
		}
	}

	return QTableWidget::eventFilter(obj, event);
}

}

//#include "tabulartable.moc"
