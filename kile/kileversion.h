/***************************************************************************
    begin                : Wednesday Jan 25 2006
    copyright            : (C) 2006 by Thomas Braun
    email                : braun@physik.fu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/


#ifndef KILEVERSION_H
#define KILEVERSION_H

#include <qstring.h>

const QString kileVersion="2.0";
const QString kileFullVersion="2.0.4svn";
const QString kilePrVersion="2";

/**
 * Compares two strings of the form "a.b(.c)" and "d.e(.f)" lexicographically,
 * where a, b, c, d, e and f are natural numbers.
 * @return 1 if s1 is bigger than s2, 0 if s1 == s2, -1 if s1 is smaller than s2
 **/
int compareVersionStrings(const QString& s1, const QString& s2);

#endif
