/***************************************************************************
                           kilehelp.cpp
----------------------------------------------------------------------------
    date                 : Feb 09 2004
    version              : 0.10.0
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
		KileTool::ViewHTML *tool = new KileTool::ViewHTML("ViewHTML", m_manager);
		tool->setFlags(0);
		kdDebug() << "==Help::showHelpFile(" << parameter << ")============" << endl;
		tool->setSource(parameter);
		tool->setTarget(QFileInfo(parameter).fileName());
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

	//FIXME: the url passed to konqueror is ok, but konqueror doesn't jump to the label!?!?!
	void Help::helpKeyword(Kate::View *view)
	{
		int type = (0 == KileConfig::use()) ? HelpLatex : HelpTetex;
		switch ( type )
		{
		case HelpTetex:
			kdDebug() << "!ok" << endl;
			helpTetexKeyword(view);
			break;
		case HelpLatex:
			kdDebug() << "ok" << endl;
			helpLatexKeyword(view);
			break;
		default:
			kdDebug() << "!ok" << endl;
			break;
		}
		kdDebug() << "!!!ok" << endl;
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
