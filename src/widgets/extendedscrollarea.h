/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef EXTENDEDSCROLLAREA_H
#define EXTENDEDSCROLLAREA_H

#include <QColor>
#include <QScrollArea>

namespace KileWidget {

class ExtendedScrollArea : public QScrollArea
{
	Q_OBJECT

	public:
		ExtendedScrollArea(QWidget *parent = NULL);
		virtual ~ExtendedScrollArea();

		void setBackgroundColor(const QColor& color);

	protected:
		QColor m_backgroundColor;

		virtual void paintEvent(QPaintEvent *event);
};

}

#endif
