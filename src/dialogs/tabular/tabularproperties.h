/***************************************************************************
  Copyright (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TABULARPROPERTIES_H
#define TABULARPROPERTIES_H

#include <QColor>
#include <QList>
#include <QHash>
#include <QString>
#include <QStringList>

namespace KileDocument {
class LatexCommands;
}

namespace KileDialog {

/**
 * @brief This class stores data while generating LaTeX output.
 *
 * This class saves information like whether the \multicolumn command
 * and/or other commands have been used.
 */
class TabularProperties {
public:
    TabularProperties();

    void setUseMultiColumn(bool useMultiColumn = true);
    bool useMultiColumn() const;

    void addRowColor(int row, const QColor &color);
    void addColor(const QColor &color);
    QColor rowColor(int row) const;
    QString colorName(const QColor &color) const;
    const QHash<QString, QString>& colorNames() const;

    const QStringList& requiredPackages() const;

    void setBullet(const QString &bullet);
    QString bullet() const;

    void addBorderUnderRow(int row);
    bool hasBorderUnderRow(int row) const;
    void setHasTopBorder();
    bool hasTopBorder() const;

    void addBorderBesideColumn(int column);
    bool hasBorderBesideColumn(int column) const;
    void setHasLeftBorder();
    bool hasLeftBorder() const;

private:
    bool m_UseMultiColumn;
    QHash<int, QColor> m_RowColors;
    QHash<QString, QString> m_ColorNames;
    int m_ColorIndex;
    QStringList m_RequiredPackages;
    QString m_Bullet;
    QList<int> m_BorderUnderRow;
    bool m_TopBorder;
    QList<int> m_BorderBesideColumn;
    bool m_LeftBorder;
};

}

#endif
