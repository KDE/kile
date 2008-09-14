/***************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken
    email                : msoeken@informatik.uni-bremen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TABULARTABLE_H
#define TABULARTABLE_H

#include <QTableWidget>

class QEvent;
class QMouseEvent;

namespace KileDialog {

class TabularTable : public QTableWidget {
	Q_OBJECT

	public:
		TabularTable(QWidget *parent = 0);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

	Q_SIGNALS:
		void rowAppended();

	private:
		SelectionMode m_DefaultMode;
		QPoint m_ManualBorderPosition;
		QPoint m_ManualBorderStart;
};

}

#endif
