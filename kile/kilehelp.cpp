/***************************************************************************
                           kilehelp.cpp
----------------------------------------------------------------------------
    date                 : Aug 04 2004
    version              : 0.10.1
    copyright            : (C) 2004 by Holger Danielsson
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

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kate/document.h>

#include "kilehelp.h"
#include "kiletool_enums.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "kileinfo.h"
#include "kilestdtools.h"
#include "kileconfig.h"

namespace KileHelp
{

	Help::Help(KileDocument::EditorExtension *edit) : m_edit(edit)
	{
		readHelpList("latex-kile.lst",m_dictHelpKile);
		readHelpList("latex-tetex.lst",m_dictHelpTetex);
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
		QString subject = "";

		switch ( type )
		{
			case HelpLatexIndex:
				break;
			case HelpLatexCommand:
				subject = "#cmd";
				break;
			case HelpLatexSubject:
				subject = "#subj";
				break;
			case HelpLatexEnvironment:
				subject = "#env";
				break;
			default:
				break;
		}

		kdDebug() << "subject " << subject << endl;
		showHelpFile( KileConfig::location() + "/latex/latex2e-html/ltx-2.html" + subject );
	}

	////////////////////// Help: Keyword //////////////////////

	// Context help: user either Kile LaTeX help orthe help file shipped with teTeX,
	void Help::helpKeyword(Kate::View *view)                   // dani 04.08.2004
	{
		int type = (0 == KileConfig::use()) ? HelpLatex : HelpTetex;
		switch ( type )
		{
		case HelpTetex:
			helpTetexKeyword(view);
			break;
		case HelpLatex:
		default:
			kdDebug() << "ok" << endl;
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
			showHelpFile( KileConfig::location() + "/latex/latex2e-html/" + m_dictHelpTetex[word] );
		}
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
		}
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
