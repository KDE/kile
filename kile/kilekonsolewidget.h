/***************************************************************************
                          kilekonsolewidget.h  -  description
                             -------------------
    begin                : Sat Dec 8 2001
    copyright            : (C) 2001 by Brachet Pascal
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEWIDGET_KONSOLE_H
#define KILEWIDGET_KONSOLE_H

/**
  *@author Brachet Pascal
  @author Jeroen Wijnhout
  */

#include <qvbox.h>

namespace KParts { class ReadOnlyPart; }
class KileInfo;

namespace KileWidget
{
	class Konsole : public QVBox
	{
		Q_OBJECT

		public:
			Konsole(KileInfo *, QWidget* parent, const char* name=0);
			~Konsole();

		public slots:
			void setDirectory(const QString & dir);
			void activate();
			void sync();

		private slots:
			void slotDestroyed();

		protected:
			void showEvent(QShowEvent *ev);
			void spawn();

		private:
			KParts::ReadOnlyPart	*m_part;
			bool					m_bPresent;
			KileInfo				*m_ki;
	};

}

#endif
