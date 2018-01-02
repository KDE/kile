/***************************************************************************
date                 : Dec 23 2007
version              : 0.22
copyright            : (C) 2004 by Mathias Soeken
email                : msoeken@tzi.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CATEGORYCOMBOBOX_H
#define CATEGORYCOMBOBOX_H

#include <KComboBox>

namespace KileWidget {

class CategoryComboBox : public KComboBox
{
public:
    enum {
        Category = Qt::UserRole + 1
    };

public:
    explicit CategoryComboBox(QWidget *parent = 0);
    explicit CategoryComboBox(bool rw, QWidget *parent = 0);

    virtual ~CategoryComboBox();

public:
    void addCategoryItem(const QString &text);
};

}
#endif
