/*****************************************************************************
*   Copyright (C) 2007 by Thomas Braun (braun@physik.fu-berlin.de)           *
*             (C) 2014-2017 by Michel Ludwig (michel.ludwig@kdemail.net)     *
******************************************************************************/

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

#include <QLoggingCategory>

#include "kile_main_debug.h"
#include "kile_parser_debug.h"
#include "kile_codecompletion_debug.h"

#define KILE_DEBUG_MAIN qCDebug(LOG_KILE_MAIN)
#define KILE_WARNING_MAIN qCWarning(LOG_KILE_MAIN)
#define KILE_DEBUG_CODECOMPLETION qCDebug(LOG_KILE_CODECOMPLETION)

#endif
