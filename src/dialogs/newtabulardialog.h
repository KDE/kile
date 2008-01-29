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

class QCheckBox;
class QSpinBox;
class QTableWidget;
class QToolBar;

class KAction;
class KComboBox;
class KIcon;

namespace KileDocument {
	class LatexCommands;
}

namespace KileDialog {

class NewTabularDialog : public KDialog {
	Q_OBJECT

	public:
		NewTabularDialog(KileDocument::LatexCommands *commands, QWidget *parent = 0);
		~NewTabularDialog();

	private:
		void initEnvironments();
		KAction* addAction(const KIcon &icon, const QString &text, const char *method, QObject *parent = 0);
		void alignItems(int alignment);
		QString iconForAlignment(int alignment) const;

	private Q_SLOTS:
		void updateColsAndRows();
		void slotEnvironmentChanged(const QString &environment);
		void slotAlignLeft();
		void slotAlignCenter();
		void slotAlignRight();
		void slotJoinCells();

	private:
		KileDocument::LatexCommands *m_latexCommands;

		QToolBar *m_tbFormat;
		QTableWidget *m_Table;
		KComboBox *m_cmbName, *m_cmbParameter;
		QSpinBox *m_sbRows, *m_sbCols;
		QCheckBox *m_cbStarred, *m_cbCenter, *m_cbBooktabs, *m_cbBullets;
};

}

#endif
