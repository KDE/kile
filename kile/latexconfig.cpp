/***************************************************************************
                        latexconfig.h  -  description
                             -------------------
    begin                : Fri Apr 23 2004
    copyright            : (C) 2004 Simon MARTIN
    email                : simartin@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "latexconfig.h"
#include <kconfig.h>

LatexConfig::LatexConfig(KConfig *conf) : m_config(conf) {}

const QString& LatexConfig::resolution(void) const
{
	return m_resolution;
}

void LatexConfig::resolution(const QString& newRes)
{
	m_resolution = newRes;
}

bool LatexConfig::determineBoundingBox(void) const
{
	return m_bbox;
}

void LatexConfig::determineBoundingBox(bool newBBox)
{
	m_bbox = newBBox;
}

bool LatexConfig::completeEnvironment(void) const
{
	return m_complete_env;
}

void LatexConfig::completeEnvironment(bool newComp)
{
	m_complete_env = newComp;
}

bool LatexConfig::hasImageMagick(void) const
{
	return m_has_imagick;
}

void LatexConfig::hasImageMagick(bool newHasIM)
{
	m_has_imagick = newHasIM;
}

void LatexConfig::readConfig(void)
{
	if(NULL != m_config) {
		m_config->setGroup("Editor Ext");
		completeEnvironment(m_config->readBoolEntry("Complete Environment", true));

		m_config->setGroup("IncludeGraphics");
		hasImageMagick(m_config->readBoolEntry("imagemagick", true));
		determineBoundingBox(m_config->readBoolEntry("boundingbox", true));
		resolution(m_config->readEntry("resolution", "300"));
	}
}

void LatexConfig::writeConfig(void)
{
	if(NULL != m_config) {
		m_config->setGroup("Editor Ext");
		m_config->writeEntry("Complete Environment", m_complete_env);

		m_config->setGroup("IncludeGraphics");
		m_config->writeEntry("boundingbox", m_bbox);
		m_config->writeEntry("resolution", m_resolution);
	}
}
