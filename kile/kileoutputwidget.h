/***************************************************************************
    begin                : Sun Dec 21 2003
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
#ifndef KILEWIDGET_OUTPUTMSG_H
#define KILEWIDGET_OUTPUTMSG_H

#include <q3textedit.h>
#include <k3textedit.h>

namespace KileWidget
{
	class Output : public K3TextEdit  
	{
		Q_OBJECT

	public: 
		Output(QWidget *parent, const char *name=0);
		~Output();

		void setReadOnly(bool r) { Q3TextEdit::setReadOnly(r); } //we don't want the greyed background

	public slots:
		void receive(const QString &);
	};
}

#endif
