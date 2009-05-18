/**********************************************************************************************
    date                 : Feb 12 2007
    version              : 0.30
    copyright            : (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
 **********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilehelp.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>

#include <KApplication>
#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>
#include <KTextEditor/Document>

#include "kiledebug.h"
#include "kiletool_enums.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "kileinfo.h"
#include "widgets/logwidget.h"
#include "kilestdtools.h"
#include "kileconfig.h"

// tbraun 27.06.2007
// it _looks_ like texlive 2007 has the same layout than texlive 2005 so don't get confused about the variable names :)

namespace KileHelp
{

	Help::Help(KileDocument::EditorExtension *edit, QWidget *mainWindow) : m_mainWindow(mainWindow), m_edit(edit), m_userhelp(NULL)
	{
		readHelpList("latex-kile.lst", m_dictHelpKile);
		initTexDocumentation();
	}
	
	Help::~Help()
	{
		delete m_userhelp;
	}
	
	void Help::initTexDocumentation()
	{
		// use documentation for teTeX v2.x, v3.x or TexLive2005
		m_texdocPath = KileConfig::location();
		
		// first check for TexLive2005
		QString texref = m_texdocPath + "/english/tex-refs";
		QDir dir(texref);
		if(dir.exists()) {
			// we found TexLive2005
			m_texVersion = TEXLIVE2005;
			m_texReference = "/english/tex-refs/";
			readHelpList("latex-texlive-3.9.lst",m_dictHelpTex);
		}
		else {
			// now we check for tetex3
			dir.setPath(m_texdocPath + "/latex/tex-refs");
			if(dir.exists())  {
				m_texVersion = TETEX3;
				// check if this is buggy tetex3.0 or an updated version with subdirectory 'html'
				dir.setPath(m_texdocPath + "/latex/tex-refs/html");
				m_texReference = ( dir.exists() ) 
				               ? "/latex/tex-refs/html/" : "/latex/tex-refs/";
				readHelpList("latex-tetex3.lst",m_dictHelpTex);
			}
			else {
				// we set it to tetex2 (what else should it be?)
				m_texVersion = TETEX2;
				m_texReference = "/latex/latex2e-html/";
				readHelpList("latex-tetex.lst",m_dictHelpTex);
			}
		}
	}

	////////////////////// update path to TeX documentation  //////////////////////

	void Help::update()
	{
		if(m_texdocPath != KileConfig::location()) {
			initTexDocumentation();
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
		KileTool::ViewHTML *tool = new KileTool::ViewHTML("ViewHTML", m_manager, false);
		tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
		//FIXME strip the #label part of the source (not the target),
		//somehow this is already done somewhere (by accident), 
		//bad to rely on it
		tool->setMsg(KileTool::NeedSourceExists, ki18n("Could not find the teTeX documentation at %1; set the correct path in Settings->Configure Kile->Help."));
		tool->setSource(parameter);
		tool->setTargetPath(QFileInfo(parameter).fileName());
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
		QString filename;

		switch(m_texVersion) {
			case TEXLIVE2005:
				filename = "english/texlive-en/live.html";
				break;
			case TETEX3:
				filename = "index.html";
				break;
			case TETEX2:
				filename = "newhelpindex.html";
				break;
			default:
				return;
		}

		showHelpFile( m_texdocPath + '/' + filename );
	}

////////////////////// Help: LaTeX //////////////////////

	void Help::helpLatex(Type type)
	{
		QString link;
		
		if(m_texVersion == TEXLIVE2005) {
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
		}
		else if(m_texVersion == TETEX3) {
			switch(type) {
				case HelpLatexIndex:
					link = "latex.html#latex";
					break;
				case HelpLatexCommand:
					link = "appendices.html#tex-refs-idx";
					break;
				case HelpLatexSubject:
					link = "latex.html#commands";
					break;
				case HelpLatexEnvironment:
					link = "latex.html#environments";
					break;
				default:
					return;
			}
		}
		else {
			switch(type) {
				case HelpLatexCommand:
					link = "ltx-2.html#cmd";
					break;
				case HelpLatexSubject:
					link = "ltx-2.html#subj";
					break;
				case HelpLatexEnvironment:
					link = "ltx-2.html#env";
					break;
				case HelpLatexIndex:
					link = "ltx-2.html";
					break;
				default:
					return;
			}
		} 
		
		// show help file
		QString texversion;
		if(m_texVersion == TEXLIVE2005) {
			texversion = "TexLive 2005";
		}
		else if(m_texVersion == TETEX3) {
			texversion = "teTeX v3.x";
		}
		else { 
			texversion = "teTeX v2.x";
		}
		KILE_DEBUG() << "TeX Version: "<< texversion << " link=" << link << endl;

		showHelpFile( m_texdocPath + m_texReference + link );
	}

	////////////////////// Help: Keyword //////////////////////

	// Context help: user either Kile LaTeX help or the help files shipped with teTeX,
	void Help::helpKeyword(KTextEditor::View *view)                   // dani 04.08.2004
	{
		int type = ( KileConfig::kilerefs() ) ? HelpKileRefs : HelpTexRefs;
		switch(type) {
			case HelpTexRefs:
				helpTexRefsKeyword(view);
				break;
			case HelpKileRefs:
				helpKileRefsKeyword(view);
				break;
		}
	}

	void Help::helpTexRefsKeyword(KTextEditor::View *view)
	{
		QString word = getKeyword(view);
		KILE_DEBUG() << "keyword: " << word << endl;
		if(!word.isEmpty() && m_dictHelpTex.contains(word)) {
			KILE_DEBUG() << "about to show help for " << word << " (section " << m_dictHelpTex[word] << " )" << endl;
			showHelpFile( m_texdocPath + m_texReference + m_dictHelpTex[word] );
		}
		else
			noHelpAvailableFor(word);
	}

	void Help::helpKileRefsKeyword(KTextEditor::View *view)
	{
		QString kilehelp = KGlobal::dirs()->findResource("appdata","help/latexhelp.html");
		KILE_DEBUG() << "kilehelp = " << kilehelp << endl;
		if(!kilehelp.isEmpty()) {
			QString word = getKeyword(view);
			KILE_DEBUG() << "word = " << word << " " << m_dictHelpKile.contains(word) << endl;
			if(!word.isEmpty() && m_dictHelpKile.contains(word)) {
				showHelpFile(kilehelp + '#' + m_dictHelpKile[word]);
			}
			else {
				noHelpAvailableFor(word);
			}
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

//////////////////// read help lists ////////////////////

	void Help::readHelpList(const QString &filename,QMap<QString,QString> &map)
	{
		QString file = KGlobal::dirs()->findResource("appdata","help/" + filename);
		if(file.isEmpty()) {
			KILE_DEBUG() << "   file not found: " << filename << endl;
			return;
		}

		KILE_DEBUG() << "   read file: " << filename << endl;
		QRegExp reg("\\s*(\\S+)\\s*=>\\s*(\\S+)");

		QFile f(file);
		if(f.open(QIODevice::ReadOnly)) { // file opened successfully
			QTextStream t(&f);         // use a text stream
			while(!t.atEnd()) { // until end of file...
				QString s = t.readLine().trimmed();       // line of text excluding '\n'
				if(!(s.isEmpty() || s.at(0)=='#')) {
					int pos = reg.indexIn(s);
					if ( pos != -1 ) {
						map[reg.cap(1)] = reg.cap(2);
					}
				}
			}
			f.close();
		}
	}

}

#include "kilehelp.moc"
