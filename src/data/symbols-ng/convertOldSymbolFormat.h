/***************************************************************************
begin                : Sat Mai 9 2009
copyright            : (C) 2009 by Thomas Braun
email                : thomas.braun@virtuell-zuhause.de
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef GESYMB_H
#define GESYMB_H

#include <QImage>
#include <QString>
#include <QRegExp>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <QStringList>

#include <stdlib.h>
#include <iostream>
#include "../../symbolviewclasses.h"

void usage();
void outputXML(const QString,const QString, QList< Package >& packages, bool mathMode);
void extractPackageString(const QString&string, QList<Package> &packages);

#endif
