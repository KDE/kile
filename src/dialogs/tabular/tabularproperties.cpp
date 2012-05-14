/********************************************************************************************
    Copyright (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
              (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tabularproperties.h"

namespace KileDialog {

TabularProperties::TabularProperties()
	: m_UseMultiColumn(false), m_ColorIndex(0),
	  m_TopBorder(false), m_LeftBorder(false) {}

void TabularProperties::setUseMultiColumn(bool useMultiColumn)
{
	m_UseMultiColumn = useMultiColumn;
}

bool TabularProperties::useMultiColumn() const
{
	return m_UseMultiColumn;
}

void TabularProperties::addRowColor(int row, const QColor &color)
{
	if(!color.isValid()) {
		return;
	}

	m_RowColors.insert(row, color);
}

void TabularProperties::addColor(const QColor &color)
{
	if(!color.isValid()) {
		return;
	}

	if(!m_ColorNames.contains(color.name())) {
		int index = m_ColorIndex;
		int value;
		QString colorName = "tc";

		do {
			value = index % 26;
			colorName += ('A' + value);
			index -= value;
		} while(index > 0);

		if(m_ColorNames.count() == 0) {
			m_RequiredPackages << "color" << "colortbl";
		}

		m_ColorNames.insert(color.name(), colorName);
		++m_ColorIndex;
	}
}

QColor TabularProperties::rowColor(int row) const
{
	if(m_RowColors.contains(row)) {
		return m_RowColors[row];
	}
	else {
		return QColor();
	}
}

QString TabularProperties::colorName(const QColor &color) const
{
	if(color.isValid() && m_ColorNames.contains(color.name())) {
		return m_ColorNames[color.name()];
	}
	else {
		return QString();
	}
}

const QHash<QString, QString>& TabularProperties::colorNames() const
{
	return m_ColorNames;
}

const QStringList& TabularProperties::requiredPackages() const
{
	return m_RequiredPackages;
}

void TabularProperties::setBullet(const QString &bullet)
{
	m_Bullet = bullet;
}

QString TabularProperties::bullet() const
{
	return m_Bullet;
}

void TabularProperties::addBorderUnderRow(int row)
{
	m_BorderUnderRow.append(row);
}

bool TabularProperties::hasBorderUnderRow(int row) const
{
	return m_BorderUnderRow.contains(row);
}

void TabularProperties::setHasTopBorder()
{
	m_TopBorder = true;
}

bool TabularProperties::hasTopBorder() const
{
	return m_TopBorder;
}

void TabularProperties::addBorderBesideColumn(int column)
{
	m_BorderBesideColumn.append(column);
}

bool TabularProperties::hasBorderBesideColumn(int column) const
{
	return m_BorderBesideColumn.contains(column);
}

void TabularProperties::setHasLeftBorder()
{
	m_LeftBorder = true;
}

bool TabularProperties::hasLeftBorder() const
{
	return m_LeftBorder;
}

}

