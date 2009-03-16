/***************************************************************************
* begin                : 26.09.2007
* copyright            : (C) Thomas Braun
* email                : braun _aeht_ physik.fu-berlin.de
****************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef KILEDEBUG_H
#define KILEDEBUG_H

#include <kdebug.h>

#ifdef NDEBUG
	#define KILE_DEBUG if (true); else kDebug
#else
	#define KILE_DEBUG kDebug
#endif

#endif
