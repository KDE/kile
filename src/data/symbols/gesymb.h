/***************************************************************************
    begin                : Mon April 17 2003
    copyright            : (C) 2006 by Thomas Braun
    email                : braun@physik.fu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QImage>
#include <QString>
#include <QRegExp>
#include <QFile>
#include <QTextStream>

#include <stdlib.h>
#include <iostream>

void usage();
void writeComment(QString cmd, QString pkgs, QString pkgsarg, QString type, int number);
void readComment(QString fname);

