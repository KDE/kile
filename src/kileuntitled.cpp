/**************************************************************************
*   Copyright (C) 2005 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)   *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kileuntitled.h"

#include <QRegExp>
#include <klocale.h>

QString KileUntitled::m_untitled = i18n("Untitled");
int KileUntitled::m_last = -1;

bool KileUntitled::isUntitled(const QString &str)
{
    static QRegExp reUntitled(m_untitled + " [0-9]+.*");
    return reUntitled.exactMatch(str);
}

QString KileUntitled::next()
{
    ++m_last;
    return current();
}

QString KileUntitled::current()
{
    return m_untitled + ' ' + QString::number(m_last);
}

