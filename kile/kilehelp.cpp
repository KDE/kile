/***************************************************************************
    date                 : Apr 04 2005
    version              : 0.11
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kate/document.h>

#include "kilehelp.h"
#include "kiletool_enums.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "kileinfo.h"
#include "kilelogwidget.h"
#include "kilestdtools.h"
#include "kileconfig.h"

namespace KileHelp
{

	Help::Help(KileDocument::EditorExtension *edit) : m_edit(edit)
	{
		readHelpList("latex-kile.lst",m_dictHelpKile);
		
		// use documentation for teTeX v2.x or v3.x
		QString texref = KileConfig::location() + "/latex/tex-refs";
		QDir dir(texref);
		if ( dir.exists() ) {
			m_tetexVersion = 3;
			// check if this is buggy tetex3.0 or an updated version with subdirectory 'html'
			dir.setPath(texref + "/html");
			m_tetexLatexReference = ( dir.exists() ) ? "/latex/tex-refs/html/" : "/latex/tex-refs/";
			readHelpList("latex-tetex3.lst",m_dictHelpTetex);
		} else {
			m_tetexVersion = 2;
			m_tetexLatexReference = "/latex/latex2e-html/";
			readHelpList("latex-tetex.lst",m_dictHelpTetex);
		}
	}
	
	Help::~Help() 
	{
		delete m_userhelp;
	}
	
	////////////////////// set parameter/initialize user help //////////////////////
	
	void Help::setUserhelp(KileTool::Manager *manager, KMenuBar *menubar)
	{ 
		m_manager = manager;
		m_userhelp = new UserHelp(manager,menubar);
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
		helpKeyword(m_manager->info()->viewManager()->currentView());
	}

////////////////////// Help: TeTeX //////////////////////

	void Help::helpTetex(KileHelp::Type type)
	{
		QString filename;

		switch ( type )
		{
			case HelpTetexGuide:
				filename = "newhelpindex.html";
				break;
			case HelpTetexDoc:
				filename = "helpindex.html";
				break;
			default:
				return;
		}

		showHelpFile( KileConfig::location() + "/" + filename );
	}

////////////////////// Help: LaTeX //////////////////////

	void Help::helpLatex(Type type)
	{
		QString link;
		
		if ( m_tetexVersion == 2) {
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
		} else {
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
		
		// show help file
		kdDebug() << "teTeX v"<< m_tetexVersion << " link=" << link << endl;
		showHelpFile( KileConfig::location() + m_tetexLatexReference + link );
	}

	////////////////////// Help: Keyword //////////////////////

	// Context help: user either Kile LaTeX help or the help files shipped with teTeX,
	void Help::helpKeyword(Kate::View *view)                   // dani 04.08.2004
	{
		int type = (0 == KileConfig::use()) ? HelpLatex : HelpTetex;
		switch ( type )
		{
		case HelpTetex:
			helpTetexKeyword(view);
			break;
		case HelpLatex:
			helpLatexKeyword(view);
			break;
		}
	}

	void Help::helpTetexKeyword(Kate::View *view)
	{
		QString word = getKeyword(view);
		kdDebug() << "keyword: " << word << endl;
		if ( !word.isNull() && m_dictHelpTetex.contains(word) )
		{
			kdDebug() << "about to show help for " << word << " (section " << m_dictHelpTetex[word] << " )" << endl;
			showHelpFile( KileConfig::location() + m_tetexLatexReference + m_dictHelpTetex[word] );
		}
		else
			noHelpAvailableFor(word);
	}

	void Help::helpLatexKeyword(Kate::View *view)
	{
		QString kilehelp = KGlobal::dirs()->findResource("html","en/kile/latexhelp.html");
		kdDebug() << "kilehelp = " << kilehelp << endl;
		if ( ! kilehelp.isEmpty() )
		{
			QString word = getKeyword(view);
			kdDebug() << "word = " << word << " " << m_dictHelpKile.contains(word) << endl;
			if ( !word.isNull() && m_dictHelpKile.contains(word) )
			{
				showHelpFile( kilehelp + "#" + m_dictHelpKile[word] );
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
			kdDebug() << "   file not found: " << filename << endl;
			return;
		}

		kdDebug() << "   read file: " << filename << endl;
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
