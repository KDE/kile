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

#include "multicolumnborderhelper.h"

namespace KileDialog {

MultiColumnBorderHelper::MultiColumnBorderHelper()
    : m_FirstNumber(-2), m_LastNumber(-2)
{
}

void MultiColumnBorderHelper::addColumn(int column)
{
    if(column == m_LastNumber + 1) { // enlarge range
        m_LastNumber = column;
    } else {
        if(m_LastNumber != -2) {
            m_SpanColumns.append(std::make_pair(m_FirstNumber, m_LastNumber));
        }
        m_FirstNumber = m_LastNumber = column;
    }
}

void MultiColumnBorderHelper::finish()
{
    if(m_LastNumber != -2) {
        m_SpanColumns.append(std::make_pair(m_FirstNumber, m_LastNumber));
    }
}

QString MultiColumnBorderHelper::toLaTeX() const
{
    QString result;
    QVector<std::pair<int,int> >::const_iterator it;
    for(it = m_SpanColumns.constBegin(); it != m_SpanColumns.constEnd(); ++it) {
        result += "\\cline{" + QString::number(it->first + 1) + '-' +
                  QString::number(it->second + 1) + '}';
    }
    return result;
}

}
