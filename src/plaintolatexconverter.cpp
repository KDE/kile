/*
 *
 * Copyright (C) 2004  Simon Martin <simartin@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "plaintolatexconverter.h"

PlainToLaTeXConverter::PlainToLaTeXConverter()
{
    // Fill the replacement map
    //TODO Do it only once!
    m_replaceMap.insert('$', "\\$");
    m_replaceMap.insert('%', "\\%");
    m_replaceMap.insert('^', "\\^");
    m_replaceMap.insert('&', "\\&");
    m_replaceMap.insert('_', "\\_");
    m_replaceMap.insert('#', "\\#");
    m_replaceMap.insert('{', "\\{");
    m_replaceMap.insert('}', "\\}");
    m_replaceMap.insert('~', "$\\sim$");
}

PlainToLaTeXConverter::~PlainToLaTeXConverter() {}

/**
 * Converts plain text to LaTeX.
 * @param toConv The string to convert
 * @return The conversion's result
 */
QString PlainToLaTeXConverter::ConvertToLaTeX(const QString& toConv) const
{
    QString result(toConv);

    // Replacing what must be...
    uint sSize = result.length();
    QMap<QChar, QString>::const_iterator mapEnd = m_replaceMap.end();
    for(uint i = 0 ; i < sSize ; ++i)
    {
        QMap<QChar, QString>::const_iterator it = m_replaceMap.find(result.at(i));

        if(it != mapEnd) { // The character must be replaced
            result.replace(i, 1, *it);
            uint len = (*it).length();
            if(1 < len) {
                i += len - 1;
                sSize += len - 1;
            }
        }
    }

    return result;
}
