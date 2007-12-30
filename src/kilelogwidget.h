/***************************************************************************
    begin                : Sat Dec 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/
#ifndef KILEWIDGET_LOGMSG_H
#define KILEWIDGET_LOGMSG_H

#include <k3textedit.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <Q3TextEdit>

class QString;
class Q3PopupMenu;
class QPoint;

class KileInfo;
class KUrl;

namespace KileWidget
{
	class LogMsg : public K3TextEdit  
	{
		Q_OBJECT

	public: 
		LogMsg(KileInfo *info, QWidget *parent, const char *name=0);
		~LogMsg();

		void setReadOnly(bool r) { Q3TextEdit::setReadOnly(r); } //we don't want the greyed background

	public Q_SLOTS:
		void highlight(); //FIXME for compatibility, should remove it asap
		void highlight(uint l, int direction = 1);
		void highlightByIndex(int index, int size, int direction = 1);

		void printMsg(int type, const QString & message, const QString &tool = "Kile" );
		void printProblem(int type, const QString & problem);

		void slotClicked(int, int);

	Q_SIGNALS:
		void fileOpen(const KUrl &, const QString &);
		void setLine(const QString &);
		void showingErrorMessage(QWidget *);

	protected:
		Q3PopupMenu* createPopupMenu (const QPoint & pos);

	protected Q_SLOTS:
		void handlePopup(int);

	private:
		KileInfo	*m_info;
		int		m_idWarning, m_idBadBox;
	};
}

#endif
