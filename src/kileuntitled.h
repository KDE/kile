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

#ifndef KILEUNTITLED_H
#define KILEUNTITLED_H

#include <qstring.h>

class KileUntitled
{
public:
    static bool isUntitled(const QString &str);
    static QString next();
    static QString current();

private:
    static QString m_untitled;
    static int     m_last;
};

#endif // KILEUNTITLED_H

