/***************************************************************************
                          kileapplication.h  -  description
                             -------------------
    begin                : Sun Apr 21 2002
    copyright            : (C) 2002 by Pascal Brachet
    email                : 
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
