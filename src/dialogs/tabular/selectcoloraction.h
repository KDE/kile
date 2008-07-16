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

#ifndef SELECTCOLORACTION_H
#define SELECTCOLORACTION_H

#include <KToolBarPopupAction>

class KColorCells;
class KPushButton;

namespace KileDialog {

class SelectColorAction : public KToolBarPopupAction {
	Q_OBJECT

	public:
		SelectColorAction(const KIcon &icon, const QString &text, QWidget *parent);

	private Q_SLOTS:
		void slotPopupAboutToShow();
		void slotColorSelected(int index, const QColor &color);
		void slotCustomClicked();

	Q_SIGNALS:
		void colorSelected(const QColor &color);

	private:
		KColorCells *m_ccColors;
		KPushButton *m_pbCustom;
};

}

#endif
