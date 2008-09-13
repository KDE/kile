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

// TODO rename to TabularTable and delete old TabularTable
#ifndef NEWTABULARTABLE_H
#define NEWTABULARTABLE_H

#include <QTableWidget>

class QEvent;

namespace KileDialog {

class NewTabularTable : public QTableWidget {
	Q_OBJECT

	public:
		NewTabularTable(QWidget *parent = 0);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	Q_SIGNALS:
		void rowAppended();
};

}

#endif
