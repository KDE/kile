/***************************************************************************
                          kileappIface.h  -  description
                             -------------------
    begin                : sam sep 28 2002
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
#ifndef KILEAPPDCOPIFACE_H
#define KILEAPPDCOPIFACE_H

#include <dcopobject.h>

namespace Kate {
class View;
}

class KURL;

class KileAppDCOPIface : virtual public DCOPObject
{
  K_DCOP

  k_dcop:
    virtual Kate::View* load( const KURL &url , const QString & encoding )=0;
    virtual void setLine( const QString &line )=0;
};

#endif
