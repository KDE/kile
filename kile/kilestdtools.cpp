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

		/*if ( tool == "TeX" )
			return new TeX(m_manager);

		if ( tool == "ViewDVI" )
			return new ViewDVI(m_manager);
			
		if ( tool == "DVItoPS" )
			return new DVItoPS(m_manager);*/

		if ( tool == "ForwardDVI" )
			return new ForwardDVI(m_manager);

		if ( tool == "ViewHTML" )
			return new ViewHTML(m_manager);

		if ( tool == "ViewBib" )
			return new ViewBib(m_manager);

		if ( tool == "QuickBuild" )
			return new QuickBuild(m_manager);

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
		}

		//unknown tool, return 0
		return 0L;
	}

	void Factory::writeStdConfig()
	{
		m_config->setGroup("Tool/Archive");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","tar");
		m_config->writeEntry("options", "zcvf '%S.tar.gz' %F");
		m_config->writeEntry("from","kilepr");
		m_config->writeEntry("to","tar.gz");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/BibTeX");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","bibtex");
		m_config->writeEntry("options","'%S'");
		m_config->writeEntry("from","aux");
		m_config->writeEntry("to","bbl");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/DVItoPDF");
		m_config->writeEntry("class","Convert");
		m_config->writeEntry("command","dvipdfm");
		m_config->writeEntry("options","'%S.dvi'");
		m_config->writeEntry("from","dvi");
		m_config->writeEntry("to","pdf");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/DVItoPS");
		m_config->writeEntry("class","Convert");
		m_config->writeEntry("command","dvips");
		m_config->writeEntry("options","-o '%S.ps' '%S.dvi'");
		m_config->writeEntry("from","dvi");
		m_config->writeEntry("to","ps");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/LaTeX");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","latex");
		m_config->writeEntry("options","-interaction=nonstopmode '%source'");
		m_config->writeEntry("from","");
		m_config->writeEntry("to","dvi");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/LaTeX2HTML");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","latex2html");
		m_config->writeEntry("options"," '%source' %options");
		m_config->writeEntry("from","");
		m_config->writeEntry("to","");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/MakeIndex");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","makeindex");
		m_config->writeEntry("options","'%S.idx'");
		m_config->writeEntry("from","idx");
		m_config->writeEntry("to","idx");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/MetaPost");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","mpost");
		m_config->writeEntry("options","-interaction=nonstopmode '%source'");
		m_config->writeEntry("from","");
		m_config->writeEntry("to","");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/PDFLaTeX");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","pdflatex");
		m_config->writeEntry("options","-interaction=nonstopmode '%source'");
		m_config->writeEntry("from","");
		m_config->writeEntry("to","pdf");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/PStoPDF");
		m_config->writeEntry("class","Convert");
		m_config->writeEntry("command","ps2pdf");
		m_config->writeEntry("options","'%S.ps' '%S.pdf'");
		m_config->writeEntry("from","ps");
		m_config->writeEntry("to","pdf");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/QuickBuild");
		//m_config->writeEntry("type","Sequence");
		m_config->writeEntry("sequence","LaTeX,ViewDVI");

		m_config->setGroup("Tool/TeX");
		m_config->writeEntry("class","Compile");
		m_config->writeEntry("command","tex");
		m_config->writeEntry("options","-interaction=nonstopmode '%source'");
		m_config->writeEntry("from","");
		m_config->writeEntry("to","dvi");
		m_config->writeEntry("type","Process");

		m_config->setGroup("Tool/ViewBib");
		m_config->writeEntry("type","Process");
		m_config->writeEntry("command","pybliographic");
		m_config->writeEntry("options","'%source'");
		m_config->writeEntry("from","bib");
		m_config->writeEntry("to","bib");

		m_config->setGroup("Tool/ViewHTML");
		m_config->writeEntry("type","DocPart");
		m_config->writeEntry("state","HTMLpreview");

		m_config->setGroup("Tool/ViewDVI");
		m_config->writeEntry("class","View");
		m_config->writeEntry("type","Part");
		m_config->writeEntry("className","KViewPart");
		m_config->writeEntry("libName","kviewerpart");
		m_config->writeEntry("from","dvi");
		m_config->writeEntry("to","dvi");
		m_config->writeEntry("state","Viewer");
		m_config->writeEntry("libOptions","dvi");

		m_config->setGroup("Tool/ForwardDVI");
		m_config->writeEntry("type","Part");
		m_config->writeEntry("class","View");
		m_config->writeEntry("className","KViewPart");
		m_config->writeEntry("libName","kviewerpart");
		m_config->writeEntry("from","dvi");
		m_config->writeEntry("to","dvi");
		m_config->writeEntry("state","Viewer");
		m_config->writeEntry("libOptions","dvi");

		m_config->setGroup("Tool/ViewPDF");
		m_config->writeEntry("class","View");
		m_config->writeEntry("className","KGVPart");
		m_config->writeEntry("libName","libkghostviewpart");
		m_config->writeEntry("state","Viewer");
		m_config->writeEntry("from","pdf");
		m_config->writeEntry("to","pdf");
		m_config->writeEntry("type","Part");

		m_config->setGroup("Tool/ViewPS");
		m_config->writeEntry("class","View");
		m_config->writeEntry("className","KGVPart");
		m_config->writeEntry("libName","libkghostviewpart");
		m_config->writeEntry("state","Viewer");
		m_config->writeEntry("from","ps");
		m_config->writeEntry("to","ps");
		m_config->writeEntry("type","Part");

		m_config->setGroup("Tools");
		m_config->writeEntry("Quick Mode", 1);
		m_config->sync();
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
		setRelativeBaseDir(S());
		setTarget("index.html");
		
		return View::determineTarget();
	}

	int QuickBuild::run()
	{
		configure();

		QStringList tools = QStringList::split(',',readEntry("sequence"));
		Base *tool;
		for (uint i=0; i < tools.count(); i++)
		{
			tool = manager()->factory()->create(tools[i]);
			if (tool)
			{
				manager()->config()->setGroup("Tool/"+tool->name());
				if ( ! (manager()->info()->watchFile() && manager()->config()->readEntry("class") == "View") )
					append(tool);
			}
			else
			{
				sendMessage(Error, i18n("Unknown tool %1.").arg(tools[i]));
				emit(done(this, Failed));
				return ConfigureFailed;
			}
		}
		
		return Sequence::run();
	}
}

#include "kilestdtools.moc"

