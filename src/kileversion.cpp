/**************************************************************************
*   Copyright (C) 2006 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kileversion.h"

#include <QStringList>

int compareVersionStrings(const QString& s1, const QString& s2) {
    QStringList l1 = s1.split('.');
    QStringList l2 = s2.split('.');
    while(l1.size() < 3) {
        l1.push_back("0");
    }
    while(l2.size() < 3) {
        l2.push_back("0");
    }
    QStringList::iterator i1 = l1.begin();
    QStringList::iterator i2 = l2.begin();
    for(unsigned int i = 0; i < 3; ++i) {
        unsigned int c1 = (*i1).toUInt();
        unsigned int c2 = (*i2).toUInt();
        if(c1 < c2) {
            return -1;
        }
        else if(c1 > c2) {
            return 1;
        }
        ++i1;
        ++i2;
    }
    return 0;
}
