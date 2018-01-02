/***************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken
    email                : msoeken@informatik.uni-bremen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MULTICOLUMNBORDERHELPER_H
#define MULTICOLUMNBORDERHELPER_H

#include <utility>

#include <QString>
#include <QVector>

namespace KileDialog {

/**
 * @brief A helper class for managing multi column borders
 */
class MultiColumnBorderHelper {
public:
    MultiColumnBorderHelper();
    void addColumn(int column);
    void finish();
    QString toLaTeX() const;

private:
    QVector<std::pair<int,int> > m_SpanColumns;
    int m_FirstNumber;
    int m_LastNumber;
};

}

#endif
