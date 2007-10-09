/***************************************************************************
    date                 : Feb 12 2007
    version              : 0.30
    copyright            : (C) 2004-2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilehelp.h"

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include "kiledebug.h"
#include <klocale.h>
#include <kate/document.h>

#include "kiletool_enums.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "kileinfo.h"
#include "kilelogwidget.h"
#include "kilestdtools.h"
#include "kileconfig.h"


// tbraun 27.06.2007
// it _looks_ like texlive 2007 has the same layout than texlive 2005 so don't get confused about the variable names :)

namespace KileHelp
{

	Help::Help(KileDocument::EditorExtension *edit) : m_edit(edit), m_userhelp(0L)
	{
		readHelpList("latex-kile.lst",m_dictHelpKile);
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
		if ( dir.exists() )
		{
			// we found TexLive2005
			m_texVersion = TEXLIVE2005;
			m_texReference = "/english/tex-refs/";
			readHelpList("latex-texlive-3.9.lst",m_dictHelpTex);
		}
		else
		{
			// now we check for tetex3
			dir.setPath(m_texdocPath + "/latex/tex-refs");
			if ( dir.exists() ) 
			{
				m_texVersion = TETEX3;
				// check if this is buggy tetex3.0 or an updated version with subdirectory 'html'
				dir.setPath(m_texdocPath + "/latex/tex-refs/html");
				m_texReference = ( dir.exists() ) 
				               ? "/latex/tex-refs/html/" : "/latex/tex-refs/";
				readHelpList("latex-tetex3.lst",m_dictHelpTex);
			}
			else
			{
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
		if ( m_texdocPath != KileConfig::location() )
			initTexDocumentation();
	}

	////////////////////// set parameter/initialize user help //////////////////////
	
	void Help::setUserhelp(KileTool::Manager *manager, KMenuBar *menubar)
	{ 
		m_manager = manager;
		m_userhelp = new UserHelp(manager,menubar);
	}
	
	void Help::enableUserhelpEntries(bool state)
	{ 
		if ( m_userhelp )
			m_userhelp->enableUserHelpEntries(state);
	}
	////////////////////// show help //////////////////////
	
	void Help::showHelpFile(const QString &parameter)
	{
		KileTool::ViewHTML *tool = new KileTool::ViewHTML("ViewHTML", m_manager, false);
		tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
		//FIXME strip the #label part of the source (not the target),
		//somehow this is already done somewhere (by accident), 
		//bad to rely on it
		tool->setMsg(KileTool::NeedSourceExists, i18n("Sorry, could not find the teTeX documentation at %1; set the correct path in Settings->Configure Kile->Help."));
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

		switch ( m_texVersion )
		{
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
		
		if ( m_texVersion == TEXLIVE2005) 
		{
			switch ( type )
			{
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
		else if ( m_texVersion == TETEX3) 
		{
			switch ( type )
			{
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
		else
		{
			switch ( type )
			{
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
		if ( m_texVersion == TEXLIVE2005 )
			texversion = "TexLive 2005";
		else if ( m_texVersion == TETEX3 )
			texversion = "teTeX v3.x";
		else 
			texversion = "teTeX v2.x";
		KILE_DEBUG() << "TeX Version: "<< texversion << " link=" << link << endl;

		showHelpFile( m_texdocPath + m_texReference + link );
	}

	////////////////////// Help: Keyword //////////////////////

	// Context help: user either Kile LaTeX help or the help files shipped with teTeX,
	void Help::helpKeyword(Kate::View *view)                   // dani 04.08.2004
	{
		int type = (0 == KileConfig::use()) ? HelpKileRefs : HelpTexRefs;
		switch ( type )
		{
		case HelpTexRefs:
			helpTexRefsKeyword(view);
			break;
		case HelpKileRefs:
			helpKileRefsKeyword(view);
			break;
		}
	}

	void Help::helpTexRefsKeyword(Kate::View *view)
	{
		QString word = getKeyword(view);
		KILE_DEBUG() << "keyword: " << word << endl;
		if ( !word.isNull() && m_dictHelpTex.contains(word) )
		{
			KILE_DEBUG() << "about to show help for " << word << " (section " << m_dictHelpTex[word] << " )" << endl;
			showHelpFile( m_texdocPath + m_texReference + m_dictHelpTex[word] );
		}
		else
			noHelpAvailableFor(word);
	}

	void Help::helpKileRefsKeyword(Kate::View *view)
	{
		QString kilehelp = KGlobal::dirs()->findResource("html","en/kile/latexhelp.html");
		KILE_DEBUG() << "kilehelp = " << kilehelp << endl;
		if ( ! kilehelp.isEmpty() )
		{
			QString word = getKeyword(view);
			KILE_DEBUG() << "word = " << word << " " << m_dictHelpKile.contains(word) << endl;
			if ( !word.isNull() && m_dictHelpKile.contains(word) )
			{
				showHelpFile( kilehelp + '#' + m_dictHelpKile[word] );
			}
			else
				noHelpAvailableFor(word);
		}
	}

	void Help::noHelpAvailableFor(const QString &word)
	{
		m_manager->info()->logWidget()->printMsg(KileTool::Error, i18n("Sorry, no help available for %1.").arg(word), i18n("Help"));
	}
	
	QString Help::getKeyword(Kate::View *view)         
	{
		if ( !view )
			return QString::null;
		
		// get current position
		uint row,col,col1,col2;
		QString word;
		Kate::Document *doc = view->getDoc();
		view->cursorPositionReal(&row,&col);
		
		if ( m_edit->getCurrentWord(doc,row,col,KileDocument::EditorExtension::smTex,word,col1,col2) )
		   // There is no starred keyword in the references. So if     // dani 04.08.2004
			// we find one, we better try the unstarred keyword.
			if ( word.right(1) == "*" )
				return word.left( word.length()-1 );
			else
				return word;
		else
			return QString::null;
	}

//////////////////// read help lists ////////////////////

	void Help::readHelpList(const QString &filename,QMap<QString,QString> &map)
	{
		QString file = KGlobal::dirs()->findResource("appdata","help/" + filename);
		if ( file.isEmpty() )
		{
			KILE_DEBUG() << "   file not found: " << filename << endl;
			return;
		}

		KILE_DEBUG() << "   read file: " << filename << endl;
		QRegExp reg("\\s*(\\S+)\\s*=>\\s*(\\S+)");

		QFile f(file);
		if ( f.open(IO_ReadOnly) )
		{     // file opened successfully
			QTextStream t( &f );         // use a text stream
			while ( ! t.eof() )
			{        // until end of file...
			QString s = t.readLine().stripWhiteSpace();       // line of text excluding '\n'
			if ( ! (s.isEmpty() || s.at(0)=='#') )
			{
				int pos = reg.search(s);
				if ( pos != -1 )
				{
				map[reg.cap(1)] = reg.cap(2);
				}
			}
			}
			f.close();
		}
	}

}

#include "kilehelp.moc"
