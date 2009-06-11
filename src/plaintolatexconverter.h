/*
 * Copyright (c) 2004 Simon MARTIN <simartin@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PLAINTOLATEXCONVERTER_H_
#define _PLAINTOLATEXCONVERTER_H_

#include <qmap.h>
#include <QString>

/**
 * A class that replaces the selection in the document (plain text) by its
 * "LaTeX version" (ie. "_" -> "\_", "%" -> "\%"...).
 */
class PlainToLaTeXConverter
{
public:
	PlainToLaTeXConverter(void);
	~PlainToLaTeXConverter(void);

	QString ConvertToLaTeX(const QString&) const;

private:
	QMap<QChar, QString> m_replaceMap;
};

#endif /* _PLAINTOLATEXCONVERTER_H_ */
