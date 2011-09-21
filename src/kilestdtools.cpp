/**************************************************************************************
    begin                : Thu Nov 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilestdtools.h"

#include <QFileInfo>
#include <QRegExp>

#include <KAction>
#include <KActionCollection>
#include <KConfig>
#include <KLocale>
#include <KStandardDirs>
#include <KProcess>

#include "kileconfig.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kileinfo.h"
#include "kilelistselector.h"
#include "kiledocmanager.h"
#include "documentinfo.h"
#include "outputinfo.h"

namespace KileTool
{
	Factory::Factory(Manager *mngr, KConfig *config, KActionCollection *actionCollection)
	: m_manager(mngr), m_config(config), m_actionCollection(actionCollection)
	{
		m_standardToolConfigurationFileName = KGlobal::dirs()->findResource("appdata", "kilestdtools.rc");
	}

	Factory::~Factory()
	{
	}

	static const QString shortcutGroupName = "Shortcuts";

	Base* Factory::create(const QString& tool, bool prepare /* = true */)
	{
		//perhaps we can find the tool in the config file
		if (m_config->hasGroup(groupFor(tool, m_config))) {
			KConfigGroup configGroup = m_config->group(groupFor(tool, m_config));
			QString toolClass = configGroup.readEntry("class", QString());

			if(toolClass == "LaTeX") {
				return new LaTeX(tool, m_manager, prepare);
			}

			if(toolClass == "LaTeXpreview") {
				return new PreviewLaTeX(tool, m_manager, prepare);
			}

			if(toolClass == "ForwardDVI") {
				return new ForwardDVI(tool, m_manager, prepare);
			}

			if(toolClass == "ViewHTML") {
				return new ViewHTML(tool, m_manager, prepare);
			}

			if(toolClass == "ViewBib") {
				return new ViewBib(tool, m_manager, prepare);
			}

			if(toolClass == "Base") {
				return new Base(tool, m_manager, prepare);
			}

			if(toolClass == "Compile") {
				return new Compile(tool, m_manager, prepare);
			}

			if(toolClass == "Convert") {
				return new Convert(tool, m_manager, prepare);
			}

			if(toolClass == "Archive") {
				return new Archive(tool, m_manager, prepare);
			}

			if(toolClass == "View") {
				return new View(tool, m_manager, prepare);
			}

			if(toolClass == "Sequence") {
				return new Sequence(tool, m_manager, prepare);
			}
		}

		//unknown tool, return 0
		return NULL;
	}

	void Factory::readStandardToolConfig()
	{
		KConfig stdToolConfig(m_standardToolConfigurationFileName, KConfig::NoGlobals);
		QStringList groupList = stdToolConfig.groupList();
		for(QStringList::iterator it = groupList.begin(); it != groupList.end(); ++it) {
			QString groupName = *it;
			if(groupName != shortcutGroupName) {
				KConfigGroup configGroup = stdToolConfig.group(groupName);
				m_config->deleteGroup(groupName);
				KConfigGroup newGroup = m_config->group(groupName);
				configGroup.copyTo(&newGroup, KConfigGroup::Persistent);
			}
		}
	}

	/////////////// LaTeX ////////////////

	LaTeX::LaTeX(const QString& tool, Manager *mngr, bool prepare) : Compile(tool, mngr, prepare)
	{
	}

	int LaTeX::m_reRun = 0;

	// FIXME don't hardcode bbl and ind suffix here.
	bool LaTeX::updateBibs()
	{
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if(docinfo) {
			if(manager()->info()->allBibliographies(docinfo).count() > 0) {
				return needsUpdate ( baseDir() + '/' + S() + ".bbl" , manager()->info()->lastModifiedFile(docinfo) );
			}
		}

		return false;
	}

	bool LaTeX::updateIndex()
	{
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if(docinfo) {
			QStringList pckgs = manager()->info()->allPackages(docinfo);
			if(pckgs.contains("makeidx")) {
				return needsUpdate ( baseDir() + '/' + S() + ".ind", manager()->info()->lastModifiedFile(docinfo) );
			}
		}

		return false;
	}
	
	bool LaTeX::updateAsy()
	{
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if(docinfo) {
			QStringList pckgs = manager()->info()->allPackages(docinfo);
			// As asymptote doesn't properly notify the user when it needs to be rerun, we run
			// it every time LaTeX is run (but only for m_reRun == 0 if LaTeX has to be rerun).
			if(pckgs.contains("asymptote")) {
				return true;
			}
		}
		return false;
	}

	bool LaTeX::finish(int r)
	{
		KILE_DEBUG() << "==bool LaTeX::finish(" << r << ")=====";
		
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
		// work around the 0 cases as the i18np call can cause some confusion when 0 is passed to it (#275700)
		QString es = (nErrors == 0 ? i18n("0 errors") : i18np("1 error", "%1 errors", nErrors));
		QString ws = (nWarnings == 0 ? i18n("0 warnings") : i18np("1 warning", "%1 warnings", nWarnings));
		QString bs = (nBadBoxes == 0 ? i18n("0 badboxes") : i18np("1 badbox", "%1 badboxes", nBadBoxes));

		sendMessage(Info, i18nc("String displayed in the log panel showing the number of errors/warnings/badboxes",
		                        "%1, %2, %3").arg(es).arg(ws).arg(bs));

		//jump to first error
		if(nErrors > 0 && (readEntry("jumpToFirstError") == "yes")) {
			connect(this, SIGNAL(jumpToFirstError()), manager(), SIGNAL(jumpToFirstError()));
			emit(jumpToFirstError());
		}
	}
	
	void LaTeX::checkAutoRun(int nErrors, int nWarnings)
	{
		KILE_DEBUG() << "check for autorun, m_reRun is " << m_reRun;
		if(m_reRun >= 2) {
			KILE_DEBUG() << "Already rerun twice, doing nothing.";
			m_reRun = 0;
			return;
		}
		if(nErrors > 0) {
			KILE_DEBUG() << "Errors found, not running again.";
			m_reRun = 0;
			return;
		}
		bool reRunWarningFound = false;
		//check for "rerun LaTeX" warnings
		if(nWarnings > 0) {
			int sz =  manager()->info()->outputInfo()->size();
			for(int i = 0; i < sz; ++i) {
				if ((*manager()->info()->outputInfo())[i].type() == LatexOutputInfo::itmWarning
				&&  (*manager()->info()->outputInfo())[i].message().contains("Rerun")) {
					reRunWarningFound = true;
					break;
				}
			}
		}

		bool asy = (m_reRun == 0) && updateAsy();
		bool bibs = updateBibs();
		bool index = updateIndex();
		KILE_DEBUG() << "asy:" << asy << "bibs:" << bibs << "index:" << index << "reRunWarningFound:" << reRunWarningFound;
		// Currently, we don't properly detect yet whether asymptote has to be run.
		// So, if asymtote figures are present, we run it each time after the first LaTeX run.
		bool reRun = (asy || bibs || index || reRunWarningFound);
		KILE_DEBUG() << "reRun:" << reRun;

		if(reRun) {
			KILE_DEBUG() << "rerunning LaTeX, m_reRun is now " << m_reRun;
			Base *tool = manager()->factory()->create(name());
			tool->setSource(source());
			manager()->runNext(tool);
		}

		if(bibs) {
			KILE_DEBUG() << "need to run BibTeX";
			Base *tool = manager()->factory()->create("BibTeX");
			tool->setSource(source());
			manager()->runNext(tool);
		}

		if(index) {
			KILE_DEBUG() << "need to run MakeIndex";
			Base *tool = manager()->factory()->create("MakeIndex");
			tool->setSource(source());
			manager()->runNext(tool);
		}

		if(asy) {
			KILE_DEBUG() << "need to run asymptote";
			int sz = manager()->info()->allAsyFigures().size();
			for(int i = sz -1; i >= 0; --i) {
			  Base *tool = manager()->factory()->create("Asymptote");
			  tool->setSource(baseDir() + '/' + S() + "-" + QString::number(i + 1) + ".asy");
			  KILE_DEBUG();
			  KILE_DEBUG() << "calling manager()->runNext(";
			  manager()->runNext(tool);
			}
		}

		if(reRun) {
			m_reRun++;
		}
		else {
			m_reRun = 0;
		}
	}
	
	
	/////////////// PreviewLaTeX (dani) ////////////////

	PreviewLaTeX::PreviewLaTeX(const QString& tool, Manager *mngr, bool prepare) : LaTeX(tool, mngr, prepare)
	{
	}

	// PreviewLatex makes three steps:
	// - filterLogfile()  : parse logfile and read info into InfoLists
	// - updateInfoLists(): change entries of temporary file into normal tex file
	// - checkErrors()    : count errors and warnings and emit signals   
	bool PreviewLaTeX::finish(int r)
	{
		KILE_DEBUG() << "==bool PreviewLaTeX::finish(" << r << ")=====";
		
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

	ForwardDVI::ForwardDVI(const QString& tool, Manager *mngr, bool prepare) : View(tool, mngr, prepare)
	{
	}

	bool ForwardDVI::checkPrereqs ()
	{
          KProcess okularVersionTester;
	  okularVersionTester.setOutputChannelMode(KProcess::MergedChannels);
	  okularVersionTester.setProgram("okular", QStringList("--version"));
	  okularVersionTester.start();
	  
	  if (okularVersionTester.waitForFinished()){
	    QString output = okularVersionTester.readAll();
	    QRegExp regExp = QRegExp("Okular: (\\d+).(\\d+).(\\d+)");

	    if(output.contains(regExp)){
     	      int majorVersion = regExp.cap(1).toInt();
      	      int minorVersion = regExp.cap(2).toInt();
	      int veryMinorVersion = regExp.cap(3).toInt();
		      
	      //  see http://mail.kde.org/pipermail/okular-devel/2009-May/003741.html
	      // 	the required okular version is > 0.8.5
	      if(  majorVersion > 0  ||
		( majorVersion == 0 && minorVersion > 8 ) ||
		( majorVersion == 0 && minorVersion == 8 && veryMinorVersion > 5 ) ){
	    	; // everything okay
	      }
	      else{
  	        sendMessage(Error,i18n("The version %1.%2.%3 of okular is too old for ForwardDVI. Please update okular to version 0.8.6 or higher",majorVersion,minorVersion,veryMinorVersion));
	      }
	    }
	  }
	    // don't return false here because we don't know for sure if okular is used
	  return true;
	}

	bool ForwardDVI::determineTarget()
	{
		if (!View::determineTarget()) {
			return false;
		}

		int para = manager()->info()->lineNumber();
		KTextEditor::Document *doc = manager()->info()->activeTextDocument();

		if (!doc) {
			return false;
		}

		QString filepath = doc->url().toLocalFile();

		QString texfile = KUrl::relativePath(baseDir(),filepath);
		QString relativeTarget = "file:" + targetDir() + '/' + target() + "#src:" + QString::number(para+1) + ' ' + texfile; // space added, for files starting with numbers
		QString absoluteTarget = "file:" + targetDir() + '/' + target() + "#src:" + QString::number(para+1) + filepath;
		addDict("%dir_target", QString());
		addDict("%target", relativeTarget);
		addDict("%absolute_target", absoluteTarget);
		KILE_DEBUG() << "==KileTool::ForwardDVI::determineTarget()=============\n";
		KILE_DEBUG() << "\tusing  (absolute)" << absoluteTarget;
		KILE_DEBUG() << "\tusing  (relative)" << relativeTarget;

		return true;
	}

	ViewBib::ViewBib(const QString& tool, Manager *mngr, bool prepare) : View(tool, mngr, prepare)
	{
	}

	bool ViewBib::determineSource()
	{
		KILE_DEBUG() << "==ViewBib::determineSource()=======";
		if (!View::determineSource()) {
			return false;
		}

		QString path = source(true);
		QFileInfo info(path);

		//get the bibliographies for this source
		QStringList bibs = manager()->info()->allBibliographies(manager()->info()->docManager()->textInfoFor(path));
		KILE_DEBUG() << "\tfound " << bibs.count() << " bibs";
		if(bibs.count() > 0) {
			QString bib = bibs.front();
			if (bibs.count() > 1) {
				//show dialog
				bool bib_selected = false;
				KileListSelector *dlg = new KileListSelector(bibs, i18n("Select Bibliography"),i18n("Select a bibliography"));
				if (dlg->exec()) {
					bib = bibs[dlg->currentItem()];
					bib_selected = true;
					KILE_DEBUG() << "Bibliography selected : " << bib;
				}
				delete dlg;
				
				if(!bib_selected) {
					sendMessage(Warning, i18n("No bibliography selected."));
					return false;
				}
			}
			KILE_DEBUG() << "filename before: " << info.path();
			setSource(manager()->info()->checkOtherPaths(info.path(),bib + ".bib",KileInfo::bibinputs));	
		}
		else if(info.exists()) { //active doc is a bib file
			KILE_DEBUG() << "filename before: " << info.path();
			setSource(manager()->info()->checkOtherPaths(info.path(),info.fileName(),KileInfo::bibinputs));
		}
		else {
			sendMessage(Error, i18n("No bibliographies found."));
			return false;
		}
		return true;
	}

	ViewHTML::ViewHTML(const QString& tool, Manager *mngr, bool prepare) : View(tool, mngr, prepare)
	{
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

