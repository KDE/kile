/***************************************************************************
                          kiletool.cpp  -  description
                             -------------------
    begin                : mon 3-11 20:40:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kiletool.h"
#include "kileinfo.h"

namespace KileTool
{
	Custom::Custom(const QString &from, const QString &to, KileInfo *ki, KConfig *config) :
		m_ki(ki),
		m_config(config),
		m_target(QString::null),
		m_from(from),
		m_to(to),
		m_basedir(QString::null),
		m_relativedir(QString::null)
	{
	}

	Custom::~Custom()
	{
	}

	int Custom::run()
	{
		if (determineTarget())
		{
			if (checkPrereqs())
			{
				if (!launch())
				{
					if (!selfCheck())
						return 3;
				}
			}
			else
				return 2;
		}
		else
			return 1;

		//everythin ok so far
		return 0;

	}

	bool Custom::determineTarget()
	{
		//determine the basedir
		QString name = m_ki->getCompileName();
		QFileInfo info(name);
		m_basedir = info.dirPath(true);

		if (m_target != QString::null)
		{
			m_target = info.fileName();
		}
	}
}

#include "kiletool.moc"
