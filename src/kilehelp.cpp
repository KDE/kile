/**********************************************************************************************
    begin     : 2004
    copyright : (C) 2004-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 **********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDir>
#include <QFileInfo>
#include <QTextStream>

#include <KGlobal>
#include <KStandardDirs>

#include "kilehelp.h"
#include "kiledebug.h"
#include "kiletool_enums.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "kileinfo.h"
#include "widgets/logwidget.h"
#include "dialogs/texdocumentationdialog.h"
#include "kileconfig.h"

// tbraun 27.06.2007
// it _looks_ like texlive 2007 has the same layout than texlive 2005 so don't get confused about the variable names :)

namespace KileHelp
{

	Help::Help(KileDocument::EditorExtension *edit, QWidget *mainWindow) : m_mainWindow(mainWindow), m_edit(edit), m_userhelp(NULL)
	{
		m_helpDir = KGlobal::dirs()->findResource("appdata","help/");
		KILE_DEBUG() << "help dir: " << m_helpDir;

		m_kileReference = m_helpDir + "latexhelp.html";
		m_latex2eReference =  m_helpDir + "latex2e-texlive.html";

		m_contextHelpType = contextHelpType();
		initTexDocumentation();
		initContextHelp();
	}

	Help::~Help()
	{
		delete m_userhelp;
	}

	void Help::initTexDocumentation()
	{
		// use documentation for teTeX v3.x, TexLive 2005-2007, TexLive 2009, TexLive2 010-2011 (TUG)
		m_texdocPath = KileConfig::location();

		// first check for TexLive 2010-2011 (TUG)
		m_texlivePath = locateTexLive201x();
		if ( !m_texlivePath.isEmpty() ) {
			KILE_DEBUG() << "found TexLive 2010-2011 (TUG): " << m_texlivePath;
			m_texVersion = TEXLIVE_201x_TUG;
			m_texVersionText = "TexLive " + m_texlivePath.right(4) + " (TUG)";
			m_texrefsReference = "/generic/tex-refs/";
			return;
		}

		//  then check for TexLive 2009 (as found with Debian, Ubuntu, ...)
		QDir dir(m_texdocPath + "/generic/tex-refs/");
		if ( dir.exists() )  {
			KILE_DEBUG() << "found TexLive 2009: " << m_texdocPath;
			m_texVersion = TEXLIVE2009;
			m_texVersionText = "TexLive 2009";
			m_texrefsReference = "/generic/tex-refs/";
			return;
		}

		// then check for older versions of TexLive 2005-2007
		dir.setPath(m_texdocPath + "/english/tex-refs");
		if ( dir.exists() ) {
			KILE_DEBUG() << "found TexLive 2005-2007: " << m_texdocPath;
			m_texVersion = TEXLIVE2005;
			m_texVersionText = "TexLive 2005-2007";
			m_texrefsReference = "/english/tex-refs/";
			return;
		}

		// finally we check for tetex3
		dir.setPath(m_texdocPath + "/latex/tex-refs");
		if ( dir.exists() )  {
			m_texVersion = TETEX3;
			m_texVersionText = "teTeX v3.x";
			// check if this is buggy tetex3.0 or an updated version with subdirectory 'html'
			dir.setPath(m_texdocPath + "/latex/tex-refs/html");
			m_texrefsReference = ( dir.exists() ) ? "/latex/tex-refs/html/" : "/latex/tex-refs/";
			return;
		}

		// found no tex documents for LaTeX help
		m_texVersion = TEX_UNKNOWN;
	}

	void Help::initContextHelp()
	{
		// read a list with keywords for context help
		if ( m_contextHelpType == HelpKileRefs ) {
			readHelpList("latex-kile.lst");
		}
		else if ( m_contextHelpType == HelpLatex2eRefs ) {
			readHelpList("latex2e-texlive.lst");
		}
		else if ( m_contextHelpType == HelpTexRefs ) {
			QString keyfile = ( m_texVersion != TETEX3 ) ? "latex-texlive-3.9.lst" : "latex-tetex3.lst";
			readHelpList(keyfile);
		}
	}

	QString Help::locateTexLivePath(const QStringList &paths)
	{
		QString sep = QDir::separator();
		QRegExp re( sep + "texlive" + sep + "(201\\d)" + sep );

		for (QStringList::ConstIterator it = paths.begin(); it != paths.end(); ++it) {
			// Remove any leading or trailing ", this is commonly used in the environment variables
			QString path = (*it);
			if (path.startsWith("\""))
				path = path.right(path.length() - 1);
			if (path.endsWith("\""))
				path = path.left(path.length() - 1);

			if ( re.indexIn(path) > 0 ) {
				return path.left(re.pos(1)+4);
			}
		}
		return QString();
	}

	QString Help::locateTexLive201x()
	{
#if defined(Q_OS_WIN32)
		QRegExp splitReg("[;,]");
#else
		QRegExp splitReg("[:]");
#endif
		QStringList paths = QString::fromLocal8Bit(getenv("PATH")).split(splitReg, QString::SkipEmptyParts);
		return locateTexLivePath(paths);
}

	////////////////////// update paths and context help of TeX documentation  //////////////////////

	void Help::update()
	{
		if ( m_texdocPath != KileConfig::location() ) {
			initTexDocumentation();
		}

		HelpType contextHelp = contextHelpType();
		if ( m_contextHelpType != contextHelp ) {
			m_contextHelpType = contextHelp;
			initContextHelp();
		}
	}

	////////////////////// set parameter/initialize user help //////////////////////

	void Help::setUserhelp(KileTool::Manager *manager, KActionMenu *userHelpActionMenu)
	{
		m_manager = manager;
		m_userhelp = new UserHelp(manager, userHelpActionMenu, m_mainWindow);
	}

	void Help::enableUserhelpEntries(bool state)
	{
		if(m_userhelp) {
			m_userhelp->enableUserHelpEntries(state);
		}
	}
	////////////////////// show help //////////////////////

	void Help::showHelpFile(const QString &parameter)
	{
		KILE_DEBUG() << "--------------------------------------------> help file: " << parameter;
		KileTool::Base *tool = m_manager->createTool("ViewHTML", QString(), false);
		if(!tool) {
			return;
		}
		tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
		//FIXME strip the #label part of the source (not the target),
		//somehow this is already done somewhere (by accident),
		//bad to rely on it
		tool->setMsg(KileTool::NeedSourceExists, ki18n("Could not find the LaTeX documentation at %1; please set the correct path in Settings->Configure Kile->Help."));
		tool->setSource(parameter);
		tool->setTargetPath(parameter);
		tool->prepareToRun();
		m_manager->run(tool);
	}

	void Help::helpKeyword()
	{
		//FIXME: we should have a better way to access the current view
		helpKeyword(m_manager->info()->viewManager()->currentTextView());
	}

////////////////////// Help: TexDoc //////////////////////

	void Help::helpDocBrowser()
	{
		KileDialog::TexDocDialog *dlg = new KileDialog::TexDocDialog();
		dlg->exec();
		delete dlg;
	}

////////////////////// Help: TeTeX //////////////////////

	void Help::helpTexGuide()
	{
		QString filename = m_texdocPath;

		switch(m_texVersion) {
			case TEXLIVE_201x_TUG:
				filename = filename.replace("texmf-dist","texmf");
				filename += "/texlive/texlive-en/texlive-en.html";
				break;
			case TEXLIVE2009:
				filename += "/texlive/texlive-en/texlive-en.html";
				break;
			case TEXLIVE2005:
				filename += "/english/texlive-en/live.html";
				break;
			case TETEX3:
				filename += "/index.html";
				break;
			default:
				return;
		}

		KILE_DEBUG() << "show TeX Guide: " <<  m_texVersionText << " file=" << filename;
		showHelpFile( filename );
	}

////////////////////// Help: LaTeX //////////////////////

	void Help::helpLatex(HelpType type)
	{
		QString filename;
		// use older 'tex-refs' documentation, if this document is present and explicitly wanted
		// in all other cases use current TexLive documentation (latex2e-texlive.html)
		if  ( m_contextHelpType==HelpTexRefs && m_texVersion!=TETEX3 ) {
			QString link;
			switch(type) {
				case HelpLatexIndex:
					link = "tex-refs.html#latex";
					break;
				case HelpLatexCommand:
					link = "tex-refs.html#tex-refs-idx";
					break;
				case HelpLatexSubject:
					link = "tex-refs.html#commands";
					break;
				case HelpLatexEnvironment:
					link = "tex-refs.html#env-latex";
					break;
				default:
					return;
			}
			filename =  m_texdocPath + m_texrefsReference + link;
		}
		else {
			QString link;
			switch(type) {
				case HelpLatexIndex:
					link = "LaTeX2e";
					break;
				case HelpLatexCommand:
					link = "Command-Index";
					break;
				case HelpLatexSubject:
					link = "SEC_Overview";
					break;
				case HelpLatexEnvironment:
					link = "Environments";
					break;
				default:
					return;
			}
			filename = m_latex2eReference + "#" + link;
		}

		// show help file
		KILE_DEBUG() << "show LaTeX help: " << m_texVersionText << " file=" << filename;
		showHelpFile( filename );
	}

	////////////////////// Help: Keyword //////////////////////

	// Context help: user either current TexLive's Latex2e help, TexLive's older tex-refs help or Kile LaTeX help
	void Help::helpKeyword(KTextEditor::View *view)
	{
		QString word = getKeyword(view);
		KILE_DEBUG() << "keyword: " << word;

		if ( !m_helpDir.isEmpty() && !word.isEmpty() && m_dictHelpTex.contains(word) ) {
			KILE_DEBUG() << "about to show help for '" << word << "' (section " << m_dictHelpTex[word] << " )";

			if ( m_contextHelpType == HelpLatex2eRefs ) {
				showHelpFile( m_latex2eReference + "#" + m_dictHelpTex[word] );
			}
			else if ( m_contextHelpType == HelpTexRefs ) {
				showHelpFile( m_texdocPath + m_texrefsReference + m_dictHelpTex[word] );
			}
			else if ( m_contextHelpType == HelpKileRefs ) {
				showHelpFile(m_kileReference + '#' + m_dictHelpTex[word]);
			}
		}
		else {
			noHelpAvailableFor(word);
		}
	}

	void Help::noHelpAvailableFor(const QString &word)
	{
		m_manager->info()->logWidget()->printMessage(KileTool::Error, i18n("No help available for %1.", word), i18n("Help"));
	}

	QString Help::getKeyword(KTextEditor::View *view)
	{
		if(!view) {
			return QString();
		}

		// get current position
		int row, col, col1, col2;
		QString word;
		KTextEditor::Document *doc = view->document();
		KTextEditor::Cursor cursor = view->cursorPosition();
		row = cursor.line();
		col = cursor.column();

		if (m_edit->getCurrentWord(doc, row, col, KileDocument::EditorExtension::smTex, word, col1, col2)) {
		   // There is no starred keyword in the references. So if     // dani 04.08.2004
			// we find one, we better try the unstarred keyword.
			if(word.right(1) == "*") {
				return word.left(word.length() - 1);
			}
			else {
				return word;
			}
		}
		else {
			return QString();
		}
	}

	HelpType Help::contextHelpType()
	{
		if ( KileConfig::latex2erefs() ) {
			return HelpLatex2eRefs;
		}
		else if ( KileConfig::texrefs() ) {
			return HelpTexRefs;
		}
		else {
			return HelpKileRefs;
		}
	}

//////////////////// read help lists ////////////////////

	void Help::readHelpList(const QString &filename)
	{
		// clear old map
		m_dictHelpTex.clear();

		QString file = m_helpDir + filename;
		if(file.isEmpty()) {
			KILE_DEBUG() << "   file not found: " << filename << endl;
			return;
		}

//		KILE_DEBUG() << "   read file: " << filename << endl;
		KILE_DEBUG() << "read keyword file: " << file;
		QRegExp reg("\\s*(\\S+)\\s*=>\\s*(\\S+)");

		QFile f(file);
		if(f.open(QIODevice::ReadOnly)) { // file opened successfully
			QTextStream t(&f);         // use a text stream
			while(!t.atEnd()) { // until end of file...
				QString s = t.readLine().trimmed();       // line of text excluding '\n'
				if(!(s.isEmpty() || s.at(0)=='#')) {
					int pos = reg.indexIn(s);
					if ( pos != -1 ) {
						m_dictHelpTex[reg.cap(1)] = reg.cap(2);
					}
				}
			}
			f.close();
		}
	}

}

#include "kilehelp.moc"
