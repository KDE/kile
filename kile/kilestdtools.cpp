/***************************************************************************
                          kilestdtools.cpp  -  description
                             -------------------
    begin                : Thu Nov 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#include <qfileinfo.h>
#include <qregexp.h>

#include <kconfig.h>
#include <klocale.h>
#include <kate/document.h>
#include <kstandarddirs.h>

#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kilestdtools.h"
#include "kileinfo.h"
#include "kilelistselector.h"

#include "latexoutputinfo.h"

namespace KileTool
{
	Base* Factory::create(const QString & tool)
	{
		if ( tool == "LaTeX")
			return new LaTeX(m_manager);

		if ( tool == "ForwardDVI" )
			return new ForwardDVI(m_manager);

		if ( tool == "ViewHTML" )
			return new ViewHTML(m_manager);

		if ( tool == "ViewBib" )
			return new ViewBib(m_manager);

		//perhaps we can find the tool in the config file
		if (m_config->hasGroup("Tool/"+tool))
		{
			m_config->setGroup("Tool/"+tool);
			QString toolClass = m_config->readEntry("class", "");

			if ( toolClass == "Base" )
				return new Base(tool, m_manager);

			if ( toolClass == "Compile" )
				return new Compile(tool, m_manager);

			if ( toolClass == "Convert" )
				return new Convert(tool, m_manager);

			if ( toolClass == "View" )
				return new View(tool, m_manager);

			if ( toolClass == "Sequence" )
				return new Sequence(tool, m_manager);
		}

		//unknown tool, return 0
		return 0L;
	}

	void Factory::writeStdConfig()
	{
		QString from_cfg = KGlobal::dirs()->findResource("appdata", "kilestdtools.rc");
		QString to_cfg = KGlobal::dirs()->saveLocation("config") + "/kilerc";
		KConfig *pCfg = new KConfig(from_cfg, false, false);
		//FIXME: use copyTo 
		//pCfg->copyTo(to_cfg, m_config);
		QStringList groups = pCfg->groupList();
		for(QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it)
		{
			QMap<QString, QString> map = pCfg->entryMap(*it);
			m_config->setGroup(*it);
			for (QMap<QString,QString>::Iterator it2  = map.begin(); it2 != map.end(); ++it2)
			{
				m_config->writeEntry(it2.key(), it2.data());
			}
		}
	}

	bool LaTeX::finish(int r)
	{
		QString finame = source();
		manager()->info()->outputFilter()->setSource( finame);
		finame.replace(QRegExp("\\..*$"), ".log");

		//manager()->info()->outputFilter()->Run( "/home/wijnhout/test.log" );
		manager()->info()->outputFilter()->Run(finame);

		int nErrors = 0, nWarnings = 0, nBadBoxes = 0;
		kdDebug() << "===LatexError()===================" << endl;
		kdDebug() << "Total: " << manager()->info()->outputInfo()->size() << " Infos reported" << endl;
		for (uint i =0; i<manager()->info()->outputInfo()->size();i++)
		{
			if ( (*manager()->info()->outputInfo())[i].type() == LatexOutputInfo::itmError ) nErrors++;
			if ( (*manager()->info()->outputInfo())[i].type() == LatexOutputInfo::itmWarning ) nWarnings++;
			if ( (*manager()->info()->outputInfo())[i].type() == LatexOutputInfo::itmBadBox ) nBadBoxes++;
			kdDebug() << (*manager()->info()->outputInfo())[i].type() << " in file <<" << (*manager()->info()->outputInfo())[i].source()
			<< ">> (line " << (*manager()->info()->outputInfo())[i].sourceLine()
			<< ") [Reported in line " << (*manager()->info()->outputInfo())[i].outputLine() << "]" << endl;
		}

		kdDebug() << "\terrors="<< nErrors<<" warnings="<< nWarnings<<" badboxes="<< nBadBoxes<<endl;
		sendMessage(Info, i18n("%1 errors, %2 warnings and %3 badboxes").arg(nErrors).arg(nWarnings).arg(nBadBoxes));

		return Compile::finish(r);
	}

	bool ForwardDVI::determineTarget()
	{
		if (!View::determineTarget())
			return false;

		int para = manager()->info()->lineNumber();
		Kate::Document *doc = manager()->info()->activeDocument();
		QString filepath;

		if (doc)
			filepath = doc->url().path();
		else
			return false;

		QString texfile = manager()->info()->relativePath(baseDir(),filepath);
		m_urlstr = "file:"+targetDir()+"/"+target()+"#src:"+QString::number(para+1)+texfile;
		addDict("%dir_target", QString::null);
		addDict("%target", m_urlstr);
		kdDebug() << "==KileTool::ForwardDVI::determineTarget()=============" << endl;
		kdDebug() << "\tusing  " << m_urlstr;

		return true;
	}

	bool ViewBib::determineSource()
	{
		if (!View::determineSource())
			return false;
			
		QString path = source(true);
		
		//get the bibliographies for this source
		const QStringList *bibs = manager()->info()->bibliographies(manager()->info()->infoFor(path));
		if (bibs->count() > 0)
		{
			QString bib = bibs->front();
			
			if (bibs->count() > 1)
			{
				//show dialog
				KileListSelector *dlg = new KileListSelector(*bibs, i18n("Select a bibliography"),i18n("a bibliography"));
				if (dlg->exec())
				{
					bib = (*bibs)[dlg->currentItem()];
					kdDebug() << "Bibliography selected : " << bib << endl;
				}
				else
				{
					sendMessage(Warning, i18n("No bibliography selected."));
					return false;
				}
				delete dlg;
			}
			
			QFileInfo info(path);
			setSource(info.dirPath()+"/"+bib+".bib");
		}
		else
		{
			sendMessage(Error, i18n("No bibliographies found."));
			return false;
		}
		
		return true;
	}

	bool ViewHTML::determineTarget()
	{
		if (target() == QString::null)
		{
			setRelativeBaseDir(S());
			setTarget("index.html");
		}

		return View::determineTarget();
	}
}

#include "kilestdtools.moc"

