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

#include <qfileinfo.h>

#include <klocale.h>
#include <kconfig.h>
#include <kurl.h>

namespace KileTool
{
	Custom::Custom(const QString &name, const QString &from, const QString &to, KileInfo *ki, KConfig *config) :
		m_ki(ki),
		m_config(config),
		m_name(name),
		m_from(from),
		m_to(to),
		m_target(QString::null),
		m_basedir(QString::null),
		m_relativedir(QString::null),
		m_targetdir(QString::null)
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

		if (m_target == QString::null)
		{
			m_target = info.fileName();
		}

		KURL url = KURL::fromPathOrURL(m_basedir);
		url.addPath(m_relativedir);
		m_targetdir = url.path();

		info.setFile(m_targetdir);

		if (! info.isExecutable())
		{
			QString warning = i18n("WARNING: It is not possible to change to the directory %1, this could cause problems.").arg(m_targetdir);
			emit(message(warning));
		}

		if (! info.isWritable())
		{
			QString warning = i18n("ERROR: It appear that the directory %1 is not writable, therefore %2 will not be able to save its results.").arg(m_targetdir).arg(m_name);
			emit(message(warning));
		}

		return true;
	}

	bool Custom::selfCheck()
	{
		return true;
	}

	bool Custom::checkPrereqs()
	{
		return true;
	}

	bool Custom::launch()
	{
		return true;
	}

	void Custom::stop()
	{
	}

	bool Custom::finish()
	{
		return true;
	}


	void Custom::readConfig()
	{
	}

	void Custom::writeConfig()
	{
	}


}

#include "kiletool.moc"


