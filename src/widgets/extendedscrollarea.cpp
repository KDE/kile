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

#include "widgets/extendedscrollarea.h"

namespace KileWidget {

ExtendedScrollArea::ExtendedScrollArea(QWidget *parent) : QScrollArea(parent)
{
}

ExtendedScrollArea::~ExtendedScrollArea()
{
}

void ExtendedScrollArea::setBackgroundColor(const QColor& color)
{
	m_backgroundColor = color;
}

void ExtendedScrollArea::paintEvent(QPaintEvent *event)
{
	if(m_backgroundColor.isValid()) {
		QPalette p = this->palette();
		p.setColor(QPalette::Window, m_backgroundColor);
		setPalette(p);
	}
	QScrollArea::paintEvent(event);
}

}

#include "extendedscrollarea.moc"
