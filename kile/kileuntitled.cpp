//
// C++ Implementation: kileuntitled
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qregexp.h>
#include <klocale.h>

#include "kileuntitled.h"

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

