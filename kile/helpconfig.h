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

#ifndef HELPCONFIG_H
#define HELPCONFIG_H 
#include <kconfig.h>

class HelpConfig
{
public:
 	HelpConfig(KConfig *config) : m_config(config) {}

	void setLocation(const QString & location) { m_location = location; }
	QString & location() { return m_location; }

	void setUseKileRefForContext(bool use) { m_use = use; }
	bool useKileRefForContext() { return m_use; }

	void readConfig();
	void writeConfig();

private:
	KConfig		*m_config;
	QString		m_location;
	bool			m_use;
};

#endif
