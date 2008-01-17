/***************************************************************************
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

#include "kilestdtools.h"

#include <qfileinfo.h>
#include <qregexp.h>

#include <kconfig.h>
#include <klocale.h>
#include <ktexteditor/document.h>
#include <kstandarddirs.h>

#include "kileconfig.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kileinfo.h"
#include "kilelistselector.h"
#include "kiledocmanager.h"
#include "documentinfo.h"
#include "latexoutputinfo.h"

namespace KileTool
{
	Base* Factory::create(const QString & tool, bool prepare /* = true */)
	{
		//perhaps we can find the tool in the config file
		if (m_config->hasGroup(groupFor(tool, m_config))) {
			KConfigGroup configGroup = m_config->group(groupFor(tool, m_config));
			QString toolClass = configGroup.readEntry("class", QString());

			if ( toolClass == "LaTeX")
				return new LaTeX(tool, m_manager, prepare);

			if ( toolClass == "LaTeXpreview")
				return new PreviewLaTeX(tool, m_manager, prepare);
				
			if ( toolClass == "ForwardDVI" )
				return new ForwardDVI(tool, m_manager, prepare);

			if ( toolClass == "ViewHTML" )
				return new ViewHTML(tool, m_manager, prepare);

			if ( toolClass == "ViewBib" )
				return new ViewBib(tool, m_manager, prepare);

			if ( toolClass == "Base" )
				return new Base(tool, m_manager, prepare);

			if ( toolClass == "Compile" )
				return new Compile(tool, m_manager, prepare);

			if ( toolClass == "Convert" )
				return new Convert(tool, m_manager, prepare);

			if ( toolClass == "Archive" )
				return new Archive(tool, m_manager, prepare);

			if ( toolClass == "View" )
				return new View(tool, m_manager, prepare);

			if ( toolClass == "Sequence" )
				return new Sequence(tool, m_manager, prepare);
		}

		//unknown tool, return 0
		return NULL;
	}

	void Factory::writeStdConfig()
	{
		QString from_cfg = KGlobal::dirs()->findResource("appdata", "kilestdtools.rc");
		QString to_cfg = KGlobal::dirs()->saveLocation("config") + "/kilerc";
		KConfig *pCfg = new KConfig(from_cfg, false, false);
		pCfg->copyTo(to_cfg, m_config);
	}

	/////////////// LaTeX ////////////////

	int LaTeX::m_reRun = 0;

	bool LaTeX::updateBibs()
	{
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if(docinfo) {
			if(manager()->info()->allBibliographies()->count() > 0) {
				return needsUpdate ( baseDir() + '/' + S() + ".bbl" , manager()->info()->lastModifiedFile(docinfo) );
			}
		}

		return false;
	}

	bool LaTeX::updateIndex()
	{
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if(docinfo) {
			const QStringList *pckgs = manager()->info()->allPackages();
			if(pckgs->contains("makeidx")) {
				return needsUpdate ( baseDir() + '/' + S() + ".ind", manager()->info()->lastModifiedFile(docinfo) );
			}
		}

		return false;
	}
	
	bool LaTeX::updateAsy()
	{
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if(docinfo) {
			const QStringList *pckgs = manager()->info()->allPackages();
			if(pckgs->contains("asymptote")) {
				static QRegExp msg("File " + QRegExp::escape(S()) + "_?\\d+_?.(?:eps|pdf|tex) does not exist");
				int sz =  manager()->info()->outputInfo()->size();
				for(int i = 0; i < sz; ++i) {
					if((*manager()->info()->outputInfo())[i].message().contains(msg)) {
						return true;
					}
				}
			}
		}
		return false;
	}

	bool LaTeX::finish(int r)
	{
		KILE_DEBUG() << "==bool LaTeX::finish(" << r << ")=====" << endl;
		
		int nErrors = 0, nWarnings = 0;
		if(filterLogfile()) {
			checkErrors(nErrors,nWarnings);
		}
		
		if(readEntry("autoRun") == "yes") {
			checkAutoRun(nErrors, nWarnings);
		}

		return Compile::finish(r);
	}

	bool LaTeX::filterLogfile()
	{
		manager()->info()->outputFilter()->setSource(source());
		QString log = baseDir() + '/' + S() + ".log";

		return manager()->info()->outputFilter()->Run(log);
	}

	void LaTeX::checkErrors(int &nErrors, int &nWarnings)
	{
		int nBadBoxes = 0;
		
		manager()->info()->outputFilter()->sendProblems();
		manager()->info()->outputFilter()->getErrorCount(&nErrors, &nWarnings, &nBadBoxes);
		QString es = i18np("1 error", "%1 errors", nErrors);
		QString ws = i18np("1 warning", "%1 warnings", nWarnings);
		QString bs = i18np("1 badbox", "%1 badboxes", nBadBoxes);

		sendMessage(Info, es +", " + ws + ", " + bs);
	
		//jump to first error
		if(nErrors > 0 && (readEntry("jumpToFirstError") == "yes")) {
			connect(this, SIGNAL(jumpToFirstError()), manager(), SIGNAL(jumpToFirstError()));
			emit(jumpToFirstError());
		}
	}
	
	void LaTeX::checkAutoRun(int nErrors, int nWarnings)
	{
		KILE_DEBUG() << "check for autorun, m_reRun is " << m_reRun << endl;
		//check for "rerun LaTeX" warnings
		bool reRan = false;
		if((m_reRun < 2) && (nErrors == 0) && (nWarnings > 0)) {
			int sz =  manager()->info()->outputInfo()->size();
			for(int i = 0; i < sz; ++i) {
				if ((*manager()->info()->outputInfo())[i].type() == LatexOutputInfo::itmWarning
				&&  (*manager()->info()->outputInfo())[i].message().contains("Rerun")) {
					reRan = true;
					break;
				}
			}
		}

		
		if(reRan) {
			m_reRun++;
		}
		else {
			m_reRun = 0;
		}

		bool bibs = updateBibs();
		bool index = updateIndex();
		bool asy = updateAsy();
		
		if(reRan) {
			KILE_DEBUG() << "rerunning LaTeX " << m_reRun << endl;
			Base *tool = manager()->factory()->create(name());
			tool->setSource(source());
			manager()->runNext(tool);
		}
		
		if(bibs || index || asy) {
			Base *tool = manager()->factory()->create(name());
			tool->setSource(source());
			manager()->runNext(tool);

			if(bibs) {
				KILE_DEBUG() << "need to run BibTeX" << endl;
				tool = manager()->factory()->create("BibTeX");
				tool->setSource(source());
				manager()->runNext(tool);
			}

			if(index) {
				KILE_DEBUG() << "need to run MakeIndex" << endl;
				tool = manager()->factory()->create("MakeIndex");
				tool->setSource(source());
				manager()->runNext(tool);
			}
			
			if(asy) {
				KILE_DEBUG() << "need to run asymptote" << endl;
				tool = manager()->factory()->create("Asymptote");
				tool->setSource(source());
				manager()->runNext(tool);
			}	
		}
	}
	
	
	/////////////// PreviewLaTeX (dani) ////////////////

	// PreviewLatex makes three steps:
	// - filterLogfile()  : parse logfile and read info into InfoLists
	// - updateInfoLists(): change entries of temporary file into normal tex file
	// - checkErrors()    : count errors and warnings and emit signals   
	bool PreviewLaTeX::finish(int r)
	{
		KILE_DEBUG() << "==bool PreviewLaTeX::finish(" << r << ")=====" << endl;
		
		int nErrors = 0, nWarnings = 0;
		if(filterLogfile()) {
			manager()->info()->outputFilter()->updateInfoLists(m_filename,m_selrow,m_docrow);
			checkErrors(nErrors,nWarnings);
		}
		
		return Compile::finish(r);
	}

	void PreviewLaTeX::setPreviewInfo(const QString &filename, int selrow,int docrow)
	{
		m_filename = filename;
		m_selrow = selrow;
		m_docrow = docrow;
	}
	
	bool ForwardDVI::determineTarget()
	{
		if (!View::determineTarget()) {
			return false;
		}

		int para = manager()->info()->lineNumber();
		KTextEditor::Document *doc = manager()->info()->activeTextDocument();
		QString filepath;

		if (doc) {
			filepath = doc->url().path();
		}
		else {
			return false;
		}

		QString texfile = manager()->info()->relativePath(baseDir(),filepath);
		m_urlstr = "file:" + targetDir() + '/' + target() + "#src:" + QString::number(para+1) + ' ' + texfile; // space added, for files starting with numbers
		addDict("%dir_target", QString());
		addDict("%target", m_urlstr);
		KILE_DEBUG() << "==KileTool::ForwardDVI::determineTarget()=============\n" << endl;
		KILE_DEBUG() << "\tusing  " << m_urlstr << endl;

		return true;
	}

	bool ViewBib::determineSource()
	{
		KILE_DEBUG() << "==ViewBib::determineSource()=======" << endl;
		if (!View::determineSource()) {
			return false;
		}

		QString path = source(true);
		QFileInfo info(path);

		//get the bibliographies for this source
		const QStringList *bibs = manager()->info()->allBibliographies(manager()->info()->docManager()->textInfoFor(path));
		KILE_DEBUG() << "\tfound " << bibs->count() << " bibs" << endl;
		if(bibs->count() > 0) {
			QString bib = bibs->front();
			if (bibs->count() > 1) {
				//show dialog
				bool bib_selected = false;
				KileListSelector *dlg = new KileListSelector(*bibs, i18n("Select Bibliography"),i18n("Select a bibliography"));
				if (dlg->exec()) {
					bib = (*bibs)[dlg->currentItem()];
					bib_selected = true;
					KILE_DEBUG() << "Bibliography selected : " << bib << endl;
				}
				delete dlg;
				
				if(!bib_selected) {
					sendMessage(Warning, i18n("No bibliography selected."));
					return false;
				}
			}
			KILE_DEBUG() << "filename before: " << info.path() << endl;
			setSource(manager()->info()->checkOtherPaths(info.path(),bib + ".bib",KileInfo::bibinputs));	
		}
		else if(info.exists()) { //active doc is a bib file
			KILE_DEBUG() << "filename before: " << info.path() << endl;
			setSource(manager()->info()->checkOtherPaths(info.path(),info.fileName(),KileInfo::bibinputs));
		}
		else {
			sendMessage(Error, i18n("No bibliographies found."));
			return false;
		}
		return true;
	}

	bool ViewHTML::determineTarget()
	{
		if (target().isNull()) {
			//setRelativeBaseDir(S());
			QString dir = readEntry("relDir");
			QString trg = readEntry("target");

			if(!dir.isEmpty()) {
				translate(dir);
				setRelativeBaseDir(dir);
			}

			if(!trg.isEmpty()) {
				translate(trg);
				setTarget(trg);
			}

			//auto-detect the file to view
			if(dir.isEmpty() && trg.isEmpty()) {
				QFileInfo file1 = QFileInfo(baseDir() + '/' + S() + "/index.html");
				QFileInfo file2 = QFileInfo(baseDir() + '/' + S() + ".html");

				bool read1 = file1.isReadable();
				bool read2 = file2.isReadable();

				if(!read1 && !read2) {
					sendMessage(Error, i18n("Unable to find %1 or %2; if you are trying to view some other HTML file, go to Settings->Configure Kile->Tools->ViewHTML->Advanced.", file1.absoluteFilePath(), file2.absoluteFilePath()));
					return false;
				}

				//both exist, take most recent
				if(read1 && read2) {
					read1 = file1.lastModified() > file2.lastModified();
					read2 = !read1;
				}

				if(read1) {
					dir = S();
					trg = "index.html";

					translate(dir);
					setRelativeBaseDir(dir);
					translate(trg);
					setTarget(trg);
				}
			}
		}

		return View::determineTarget();
	}
}

#include "kilestdtools.moc"

