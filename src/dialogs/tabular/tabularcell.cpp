/********************************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
    copyright            : (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tabularcell.h"

#include "tabularheaderitem.h"
#include "tabularproperties.h"

namespace KileDialog {

//BEGIN TabularCell
TabularCell::TabularCell()
	: QTableWidgetItem(),
	  m_Border(None)
{
}

TabularCell::TabularCell(const QString &text)
	: QTableWidgetItem(text),
	  m_Border(None)
{
}

void TabularCell::setBorder(int border)
{
	m_Border = border;
	tableWidget()->update();
}

int TabularCell::border() const
{
	return m_Border;
}

QString TabularCell::toLaTeX( TabularProperties &properties ) const
{
	QString prefix;
	QString suffix;

	int alignment = textAlignment() & ~Qt::AlignVCenter;
	TabularHeaderItem *headerItem = static_cast<TabularHeaderItem*>(tableWidget()->horizontalHeaderItem(column()));

	QString colorCommand;
	if(backgroundColor().isValid()
	    && backgroundColor() != Qt::white
	    && !properties.rowColor(row()).isValid()) {
		colorCommand = ">{\\columncolor{" + properties.colorName(backgroundColor()) + "}}";
	}

	QString leftBorder, rightBorder;
	if(column() == 0 &&  !properties.hasLeftBorder() &&
		 (border() & TabularCell::Left)) {
		leftBorder = '|';
		if(properties.hasBorderBesideColumn(1)) {
			rightBorder = '|';
		}
	}
	if(!properties.hasBorderBesideColumn(column()) && (border() & TabularCell::Right)) {
		rightBorder = '|';
		if(column() == 0 && properties.hasLeftBorder()) {
			leftBorder = '|';
		}
	}
	bool adjustBorder = !leftBorder.isEmpty() || !rightBorder.isEmpty();

	int columnSpan = tableWidget()->columnSpan(row(), column());

	if(headerItem->alignment() != alignment || !colorCommand.isEmpty() ||
		 adjustBorder || columnSpan > 1 ) {

		switch(alignment) {
			case Qt::AlignLeft: // TODO consider AlignP etc.
				properties.setUseMultiColumn();
				prefix += "\\mc{" + QString::number(columnSpan) + "}{" +
				          leftBorder + colorCommand + 'l' + rightBorder + "}{";
				suffix = '}' + suffix;
				break;

			case Qt::AlignHCenter:
				properties.setUseMultiColumn();
				prefix += "\\mc{" + QString::number(columnSpan) + "}{" +
				          leftBorder + colorCommand + 'c' + rightBorder + "}{";
				suffix = '}' + suffix;
				break;

			case Qt::AlignRight:
				properties.setUseMultiColumn();
				prefix += "\\mc{" + QString::number(columnSpan) + "}{" +
				          leftBorder + colorCommand + 'r' + rightBorder + "}{";
				suffix = '}' + suffix;
				break;
		};
	}

	/* format */
	if (font().bold()) {
		prefix += "\\textbf{";
		suffix = '}' + suffix;
	}
	if (font().italic()) {
		prefix += "\\textit{";
		suffix = '}' + suffix;
	}

	/* prefix */
	if (font().underline()) {
		prefix += "\\underline{";
		suffix = '}' + suffix;
	}

	/* content */
	QString content = "";
	QString incontent = text().trimmed();
	if(incontent.isEmpty()) {
		incontent = properties.bullet();
	}
	content += prefix + incontent + suffix;
	return content;
}
//END

}
