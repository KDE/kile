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

#ifndef _LATEXCONFIG_H_
#define _LATEXCONFIG_H_

#include <qstring.h>

class KConfig;

/**
 * The class that holds the LaTeX specific configuration.
 */
class LatexConfig
{
public:
	// 'tors
	LatexConfig(KConfig*);
	~LatexConfig(void) {}

	// Accessors
	const QString& resolution(void) const;
	void resolution(const QString&);

	bool determineBoundingBox(void) const;
	void determineBoundingBox(bool);

	bool completeEnvironment(void) const;
	void completeEnvironment(bool);

	bool hasImageMagick(void) const;
	void hasImageMagick(bool);

	// Persistence
	void readConfig(void);
	void writeConfig(void);

private:
	QString m_resolution;
	KConfig *m_config;
	bool m_bbox, m_complete_env, m_has_imagick;
};

#endif /* _LATEXCONFIG_H_ */
