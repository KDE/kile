//
// C++ Interface: kileuntitled
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

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

