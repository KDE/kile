/***************************************************************************
                          kileapplication.h  -  description
                             -------------------
    begin                : Sun Apr 21 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEAPPLICATION_H
#define KILEAPPLICATION_H

#include <kapp.h>

/**
  *@author Pascal Brachet
  */

class KileApplication : public KApplication
{
public:
    KileApplication();
    ~KileApplication();
    static void closeSplash();
};

#endif
