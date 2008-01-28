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

#ifndef NEW_TABULARDIALOG_H
#define NEW_TABULARDIALOG_H

#include <KDialog>

class QSpinBox;
class QTableWidget;
class QToolBar;

class KAction;
class KIcon;

namespace KileDialog {

class NewTabularDialog : public KDialog {
	Q_OBJECT

	public:
		NewTabularDialog(QWidget *parent = 0);
		~NewTabularDialog();

	private:
		KAction* addAction(const KIcon &icon, const QString &text, const char *method, QObject *parent = 0);
		void alignItems(int alignment);
		QString iconForAlignment(int alignment) const;

	private Q_SLOTS:
		void updateColsAndRows();
		void slotAlignLeft();
		void slotAlignCenter();
		void slotAlignRight();
		void slotJoinCells();

	private:
		QToolBar *m_tbFormat;
		QTableWidget *m_Table;
		QSpinBox *m_sbRows, *m_sbCols;
};

}

#endif
