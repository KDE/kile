/***************************************************************************
                          helpconfig.h  -  description
                             -------------------
    begin                : Sat Mar 27 2004
    copyright            : (C) 2004 by Jeroen Wijnout
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
 
#include "helpconfig.h"

void HelpConfig::readConfig()
{
	m_config->setGroup("Help");
	setLocation(m_config->readPathEntry("location", "/usr/share/texmf/doc"));
	setUseKileRefForContext(m_config->readBoolEntry("use", true));
}

void HelpConfig::writeConfig()
{
	m_config->setGroup("Help");
	m_config->writePathEntry("location", location());
	m_config->writeEntry("use", useKileRefForContext());
}
