/************************************************************************************************
    begin                : Sat Dec 8 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 ************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KONSOLEWIDGET_H
#define KONSOLEWIDGET_H

#include <QShowEvent>
#include <QWidget>

namespace KParts { class ReadOnlyPart; }

class KileInfo;

namespace KileWidget
{
	class Konsole : public QWidget
	{
		Q_OBJECT

		public:
			Konsole(KileInfo *, QWidget* parent);
			~Konsole();

		public slots:
			void setDirectory(const QString& dir);
			void activate();
			void sync();

		private slots:
			void slotDestroyed();

		protected:
			void showEvent(QShowEvent *ev);
			void spawn();

		private:
			KParts::ReadOnlyPart	*m_part;
			bool			m_bPresent;
			KileInfo		*m_ki;
	};
}

#endif
