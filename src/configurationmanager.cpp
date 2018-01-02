/**************************************************************************
*   Copyright (C) 2008 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "configurationmanager.h"

#include "kileconfig.h"

#include "kileinfo.h"

namespace KileConfiguration {

Manager::Manager(KileInfo *kileInfo, QObject *parent, const char *name)  : QObject(parent), m_kileInfo(kileInfo)
{
    setObjectName(name);
}

Manager::~Manager() {

}

void Manager::emitConfigChanged()
{
    emit configChanged();
}
}


