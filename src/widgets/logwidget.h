/*************************************************************************************
    begin                : Sat Dec 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/
#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <Q3PopupMenu>
#include <KTextEdit>

class QString;
class Q3PopupMenu;
class QPoint;

class KileInfo;
class KUrl;

namespace KileWidget {
	class LogWidget : public KTextEdit
	{
		Q_OBJECT

	public:
		LogWidget(KileInfo *info, QWidget *parent, const char *name=0);
		~LogWidget();

		bool isShowingOutput() const;

		void scrollToBottom();

	public Q_SLOTS:
		void highlight(); //FIXME for compatibility, should remove it asap
		void highlight(uint l, int direction = 1);
		void highlightByIndex(int index, int size, int direction = 1);

		void printMsg(int type, const QString & message, const QString &tool = "Kile");
		void printProblem(int type, const QString & problem);

		void slotClicked(int, int);

	Q_SIGNALS:
		void fileOpen(const KUrl &, const QString &);
		void setLine(const QString &);
		void showingErrorMessage(QWidget *);

	protected:
// 		Q3PopupMenu* createPopupMenu (const QPoint & pos);

	protected Q_SLOTS:
		void handlePopup(int);

	private:
		KileInfo	*m_info;
		int		m_idWarning, m_idBadBox;
	};
}

#endif
