/***************************************************************************
                          kilelogwidget.h  -  description
                             -------------------
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

#include <ktextedit.h>

class KileInfo;
class KURL;
class QString;

namespace KileWidget
{
	class LogMsg : public KTextEdit  
	{
		Q_OBJECT

	public: 
		LogMsg(KileInfo *info, QWidget *parent, const char *name=0);
		~LogMsg();

		void setReadOnly(bool r) { QTextEdit::setReadOnly(r); } //we don't want the greyed background

	public slots:
		void highlight(); //FIXME for compatibility, should remove it asap
		void highlight(uint l, int direction = 1);
		void highlight(const QString & begin, int direction = 1);

		void printMsg(int type, const QString & message, const QString &tool = "Kile" );
		void printProblem(int type, const QString & problem);

		void slotClicked(int, int);

	signals:
		void fileOpen(const KURL &, const QString &);
		void setLine(const QString &);

	private:
		KileInfo	*m_info;
	};
}

#endif
