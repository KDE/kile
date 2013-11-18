/**************************************************************************
*   Copyright (C) 2006 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KILECONSTANTS_H
#define KILECONSTANTS_H

namespace KileDocument {
	enum Type {Undefined, Text, LaTeX, BibTeX, Script};

}

#ifdef Q_WS_WIN
 #define PATH_SEPARATOR ';'
#else
 #define PATH_SEPARATOR ':'
#endif

#endif
