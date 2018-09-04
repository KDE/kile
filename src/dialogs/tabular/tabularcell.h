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

#ifndef TABULARCELL_H
#define TABULARCELL_H

#include <QTableWidgetItem>

namespace KileDocument {
class LatexCommands;
}

namespace KileDialog {

class TabularProperties;

class TabularCell : public QTableWidgetItem {
public:
    enum { None = 0, Left = 1, Top = 2, Right = 4, Bottom = 8 };

    TabularCell();
    explicit TabularCell(const QString &text);

    void setBorder(int border);
    int border() const;

    QString toLaTeX( TabularProperties &properties ) const;

private:
    int m_Border;
};

}

#endif
