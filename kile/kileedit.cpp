/***************************************************************************
    date                 : Mar 12 2007
    version              : 0.46
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

#include <qfileinfo.h>
#include <qvaluestack.h>
#include <qclipboard.h>

#include <kate/view.h>
#include <kate/document.h>
#include <ktexteditor/searchinterface.h>
#include <ktexteditor/editinterfaceext.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kstandarddirs.h>

#include "kilekonsolewidget.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kileconfig.h"
#include "kileactions.h"

#include "kiletool_enums.h"
#include "kilelogwidget.h"
#include "kileextensions.h"
#include "quickpreview.h"
#include "kileedit.h"

namespace KileDocument
{

EditorExtension::EditorExtension(KileInfo *info) : m_ki(info)
{
	m_complete = new KileDocument::CodeCompletion(m_ki);
	m_latexCommands = m_ki->latexCommands();

	// init regexp
	m_reg.setPattern("(\\\\(begin|end)\\s*\\{([A-Za-z]+\\*?)\\})|(\\\\\\[|\\\\\\])");
	//                1    2                 3                   4         
	m_regexpEnter.setPattern("^(.*)((\\\\begin\\s*\\{([^\\{\\}]*)\\})|(\\\\\\[))");
	//                         1   23                 4               5 

	// init double quotes
	m_quoteList 
		<< "English quotes:   ``   ''"
		<< "French quotes:   \"<   \">"
		<< "German quotes:   \"`   \"'"
		<< "French quotes (long):   \\flqq   \\frqq"
		<< "German quotes (long):   \\glqq   \\grqq"
		<< "Icelandic quotes (v1):   \\ilqq   \\irqq"
		<< "Icelandic quotes (v2):   \\iflqq   \\ifrqq"
		<< "Czech quotes:   \\uv{   }"
		;

	readConfig();
}

EditorExtension::~EditorExtension()
{
	delete m_complete;
}

//////////////////// read configuration ////////////////////

void EditorExtension::readConfig(void)
{
	// init insertion of double quotes
	initDoubleQuotes();

	// calculate indent for autoindent of environments
	m_envAutoIndent = QString::null;
	if ( KileConfig::envIndentation() )
	{
		if ( KileConfig::envIndentSpaces() )
		{
			int num = KileConfig::envIndentNumSpaces();
			if ( num<1 || num>9 )
				num = 1;
			m_envAutoIndent.fill(' ',num);
		}
		else
		{
			m_envAutoIndent = "\t";
		}
	}
}

void EditorExtension::insertTag(const KileAction::TagData& data, Kate::View *view)
{
	Kate::Document *doc = view->getDoc();
	if ( !doc) return;

	//whether or not to wrap tag around selection
	bool wrap = ( (!data.tagEnd.isNull()) && doc->hasSelection());

	//%C before or after the selection
	bool before = data.tagBegin.contains("%C");
	bool after = data.tagEnd.contains("%C");

	//save current cursor position
	int para=view->cursorLine();
	int para_begin=para;
	int index=view->cursorColumnReal();
	int index_begin=index;
	int para_end=0;
	int index_cursor=index;
	int para_cursor=index;
	// offset for autoindentation of environments
	int dxIndentEnv = 0;

	// environment tag
	bool envtag = data.tagBegin.contains("%E") || data.tagEnd.contains("%E");
	QString whitespace = getWhiteSpace( doc->textLine(para).left(index) );

	//if there is a selection act as if cursor is at the beginning of selection
	if (wrap)
	{
		index = doc->selStartCol();
		para  = doc->selStartLine();
		para_end = doc->selEndLine();
	}

	QString ins = data.tagBegin;
	QString tagEnd = data.tagEnd;

	//start an atomic editing sequence
	KTextEditor::EditInterfaceExt *editInterfaceExt = KTextEditor::editInterfaceExt( doc );
	if ( editInterfaceExt ) editInterfaceExt->editBegin();

	//cut the selected text
	QString trailing;
	if (wrap)
	{
		QString sel = doc->selection();
		doc->removeSelectedText();
		
		// no autoindentation of environments, when text is selected
		if ( envtag )
		{
			ins.remove("%E");
			tagEnd.remove("%E\n");
		}

		// strip one of two consecutive line ends
		int len = sel.length();
		if ( tagEnd.at(0)=='\n' && len>0 && sel.find('\n',-1)==len-1 )
			sel.truncate( len-1 );
			
		// now add the selection
		ins += sel;
		
		// place the cursor behind this tag, if there is no other wish
		if ( !before && !after )
		{
			trailing = "%C";
			after = true;
		}
	}
	else if ( envtag  )
	{
		ins.replace("%E",whitespace+m_envAutoIndent);
		tagEnd.replace("%E",whitespace+m_envAutoIndent);
		if ( data.dy > 0 )
			dxIndentEnv = whitespace.length() + m_envAutoIndent.length();
	}

	tagEnd.replace("\\end{",whitespace+"\\end{");
	ins += tagEnd + trailing;

	//do some replacements
	QFileInfo fi( doc->url().path());
	ins.replace("%S", fi.baseName(true));
	ins.replace("%B", s_bullet);
	
	//insert first part of tag at cursor position
	doc->insertText(para,index,ins);

	//move cursor to the new position
	if ( before || after )
	{
		int n = data.tagBegin.contains("\n")+ data.tagEnd.contains("\n");
		if (wrap) n += para_end > para ? para_end-para : para-para_end;
		for (int line = para_begin; line <= para_begin+n; ++line)
		{
			if (doc->textLine(line).contains("%C"))
			{
				int i=doc->textLine(line).find("%C");
				para_cursor = line; index_cursor = i;
				doc->removeText(line,i,line,i+2);
				break;
			}
			index_cursor=index;
			para_cursor=line;
		}
	}
	else
	{
		int py = para_begin, px = index_begin;
		if (wrap) //act as if cursor was at beginning of selected text (which is the point where the tagBegin is inserted)
		{
			py = para;
			px = index;
		}
		para_cursor = py+data.dy; index_cursor = px+data.dx+dxIndentEnv;
	}

	//end the atomic editing sequence
	if ( editInterfaceExt ) editInterfaceExt->editEnd();

	//set the cursor position (it is important that this is done outside of the atomic editing sequence)
	view->setCursorPositionReal(para_cursor, index_cursor);

	doc->clearSelection();
}

//////////////////// goto environment tag (begin or end) ////////////////////

// goto the next non-nested environment tag

Kate::View* EditorExtension::determineView(Kate::View *view)
{
	if (view == 0L)
		view = m_ki->viewManager()->currentTextView();

	m_overwritemode = (view == 0L) ? false : view->isOverwriteMode();

	return view;
}

void EditorExtension::gotoEnvironment(bool backwards, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	uint row,col;
	EnvData env;
	bool found;
	
	// get current position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	// start searching
	if ( backwards )
	{
		found = findBeginEnvironment(doc,row,col,env);
		//kdDebug() << "   goto begin env:  " << env.row << "/" << env.col << endl;
	
	}
	else
	{
		found = findEndEnvironment(doc,row,col,env);
		if ( !m_overwritemode )
		env.col += env.len;
	}
	
	if ( found )
		view->setCursorPositionReal(env.row,env.col);
}

// match the opposite environment tag

void EditorExtension::matchEnvironment(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	uint row,col;
	EnvData env;
	
	// get current position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	// we only start, when we are at an environment tag
	if ( !isEnvironmentPosition(doc,row,col,env) )
		return;
	
	gotoEnvironment( env.tag != EnvBegin,view);
}

//////////////////// close opened environments  ////////////////////

// search for the last opened environment and close it

void EditorExtension::closeEnvironment(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint row,col,currentRow,currentCol;
	QString name;
	
	view->cursorPositionReal(&currentRow,&currentCol);
	if ( findOpenedEnvironment(row,col,name,view) )
	{
		if ( name == "\\[" )
			view->getDoc()->insertText( currentRow,currentCol, "\\]" );
		else
			view->getDoc()->insertText( currentRow,currentCol, "\\end{" + name + '}' );
// 		view->setCursorPositionReal(row+1,0);
	}
}

// close all opened environments

void EditorExtension::closeAllEnvironments(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	QStringList envlist = findOpenedEnvironmentList(view,true);
	if ( envlist.count() == 0 )
		return;

	uint currentRow,currentCol,outputCol;
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&currentRow,&currentCol);

	bool indent = ! m_envAutoIndent.isEmpty();
	if ( indent && currentCol > 0 )
	{
		doc->insertText(currentRow,currentCol,"\n");
		currentRow++;
		currentCol = 0;
	}
		
	bool ok1,ok2;
	for ( QStringList::Iterator it=envlist.begin(); it!=envlist.end(); ++it ) 
	{
		QStringList entry = QStringList::split(',',*it);
		if ( entry[0] == "document" )
			break;

		uint row = entry[1].toUInt(&ok1);
		uint col = entry[2].toUInt(&ok2);
		if ( !ok1 || !ok2 )
			continue;

		outputCol = currentCol;
		if ( indent )
		{
			QString whitespace = getWhiteSpace( doc->textLine(row).left(col) );
			doc->insertText(currentRow,outputCol,whitespace);
			outputCol += whitespace.length();
		}
		QString endtag = ( entry[0] == "\\[" ) ? "\\]\n" : "\\end{"+entry[0]+"}\n";
		doc->insertText(currentRow,outputCol,endtag);
		currentRow++;
	}
}

//////////////////// mathgroup ////////////////////

void EditorExtension::selectMathgroup(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint row1,col1,row2,col2;
	if ( getMathgroup(view,row1,col1,row2,col2) )
		view->getDoc()->setSelection(row1,col1,row2,col2);
}

void EditorExtension::deleteMathgroup(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint row1,col1,row2,col2;
	if ( getMathgroup(view,row1,col1,row2,col2) )
	{
		view->getDoc()->clearSelection();
		view->getDoc()->removeText(row1,col1,row2,col2);
		view->setCursorPosition(row1,0);
	}
}

bool EditorExtension::hasMathgroup(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) 
		return false;

	uint row1,col1,row2,col2;
	return getMathgroup(view,row1,col1,row2,col2);
}

QString EditorExtension::getMathgroupText(uint &row, uint &col, Kate::View *view)
{
	uint row1,col1,row2,col2;

	view = determineView(view);
	if ( view && getMathgroup(view,row1,col1,row2,col2) )
	{
		row = row1;
		col = col1;
		return view->getDoc()->text(row1,col1,row2,col2);
	}
	else
		return QString::null;
}


bool EditorExtension::getMathgroup(Kate::View *view, uint &row1, uint &col1, uint &row2, uint &col2)
{
	QRegExp reg( QString("\\$")
		+ "|\\\\begin\\s*\\{([A-Za-z]+\\*?)\\}" 
		+ "|\\\\end\\s*\\{([A-Za-z]+\\*?)\\}"
		+ "|\\\\\\[|\\\\\\]"
		+ "|\\\\\\(|\\\\\\)"
		);
	
	uint row,col,r,c;
	MathData begin,end;

	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);

	QString textline = getTextLineReal(doc,row);

	// check for '\ensuremath{...}'
	QString word;
	uint x1,x2;
	if ( getCurrentWord(doc,row,col,smTex,word,x1,x2) && word=="\\ensuremath" )
		view->setCursorPositionReal(row,x2);

	BracketData open,close;
	if ( getTexgroup(false,open,close,view) )
	{
		QString s = getTextLineReal(doc,open.row);
		if ( open.col>=11 && s.mid(open.col-11,11)=="\\ensuremath" )
		{
			view->setCursorPositionReal(row,col);
			row1 = open.row;
			col1 = open.col-11;
			row2 = close.row;
			col2 = close.col;
			return true;
		}
	}

	// do we need to restore the cursor position
	view->setCursorPositionReal(row,col);

	// '$' is difficult, because it is used as opening and closing tag
	int mode = 0;
	if ( textline[col] == '$' )
		mode = 1;
	else if ( col>0 && textline[col-1]=='$' )
		mode = 2;

	if ( mode > 0 )
	{
		// first look, if this is a closing '$'
		r = row;
		c = ( mode == 1 ) ? col : col-1;		
		if ( decreaseCursorPosition(doc,r,c) && findOpenMathTag(doc,r,c,reg,begin) )
		{
			if ( begin.tag==mmMathDollar && (begin.numdollar & 1) )
			{
				row1 = begin.row;
				col1 = begin.col;
				row2 = row;
				col2 = ( mode == 1 ) ? col+1 : col;
				return true;
			}
		}

		// perhaps an opening '$'
		r = row;
		c = ( mode == 1 ) ? col+1 : col;
		if ( findCloseMathTag(doc,r,c,reg,end) )
		{
			if ( end.tag==mmMathDollar )
			{
				row1 = row;
				col1 = ( mode == 1 ) ? col : col-1;
				row2 = end.row;
				col2 = end.col + end.len;
				return true;
			}
		}

		// found no mathgroup with '$'
		return false;
	}	

	// now let's search for all other math tags:
	// if a mathgroup tag starts in the current column, we save this 
	// position and move the cursor one column to the right
	bool openingtag = isOpeningMathTagPosition(doc,row,col,begin);
	if ( openingtag )
	{
		// try to find the corresponding closing tag at the right
		bool closetag = findCloseMathTag(doc,row,col+1,reg,end);
		if ( closetag && checkMathtags(begin,end) )
		{
			row1 = begin.row;
			col1 = begin.col;
			row2 = end.row;
			col2 = end.col + end.len;
			return true;
		}	
	}

	r = row;
	c = col;
	bool closingtag = isClosingMathTagPosition(doc,row,col,end);
	if ( closingtag )
	{
			c = end.col;
			if ( ! decreaseCursorPosition(doc,r,c) )
				return false;
	}
		
	// now try to search to opening tag of the math group
	if ( ! findOpenMathTag(doc,r,c,reg,begin) )
		return false;

	if ( begin.tag==mmMathDollar && !(begin.numdollar & 1) )
	{
		//kdDebug() << "error: even number of '$' --> no math mode"  << endl;
		return false;
	}

	// and now the closing tag
	if ( ! findCloseMathTag(doc,r,c,reg,end) )
		return false;

	// both tags were found, but they must be of the same type
	if ( checkMathtags(begin,end) )
	{
		row1 = begin.row;
		col1 = begin.col;
		row2 = end.row;
		col2 = end.col + end.len;
		return true;
	}
	else
		return false;
}

//////////////////// mathgroup tags ////////////////////

bool EditorExtension::checkMathtags(const MathData &begin,const MathData &end)
{
	// both tags were found, but they must be of the same type
	if ( begin.tag != end.tag )
	{
		//kdDebug() << "error: opening and closing tag of mathmode don't match"  << endl;
		return false;
	}

	// and additionally: if it is a math env, both tags must have the same name
	if ( begin.tag==mmDisplaymathEnv && begin.envname!=end.envname )
	{
		//kdDebug() << "error: opening and closing env tags have different names"  << endl;
		return false;
	}

	return true;
}

bool EditorExtension::isOpeningMathTagPosition(Kate::Document *doc, uint row, uint col, MathData &mathdata)
{
	QString textline = getTextLineReal(doc,row);

	QRegExp reg("\\\\begin\\s*\\{([A-Za-z]+\\*?)\\}|\\\\\\[|\\\\\\(");
	if ( (int)col != reg.search(textline,col) )
		return false;

	QChar id = reg.cap(0)[1];
	QString envname = reg.cap(1);

	mathdata.row = row;
	mathdata.col = col;
	mathdata.len = reg.cap(0).length();

	switch ( id )
	{
		case 'b' : if ( !(m_latexCommands->isMathEnv(envname) || envname=="math") || m_latexCommands->needsMathMode(envname) )
				        return false;
				     mathdata.tag = ( envname=="math" ) ? mmMathEnv : mmDisplaymathEnv;
				     mathdata.envname = envname;
			        break;
		case '[' : mathdata.tag = mmDisplaymathParen; 
			        break;
		case '(' : mathdata.tag = mmMathParen; 
			        break;
	}

	return true;
}

bool EditorExtension::isClosingMathTagPosition(Kate::Document *doc, uint row, uint col,MathData &mathdata)
{
	QString textline = doc->textLine(row);

	QRegExp reg("\\\\end\\s*\\{([A-Za-z]+\\*?)\\}|\\\\\\]|\\\\\\)");
	int pos = reg.searchRev(textline,col);
	if ( pos<0 || (int)col>pos+reg.matchedLength() )
		return false;

	QChar id = reg.cap(0)[1];
	QString envname = reg.cap(1);

	mathdata.row = row;
	mathdata.col = pos;
	mathdata.len = reg.cap(0).length();

	switch ( id )
	{
		case 'e' : if ( !(m_latexCommands->isMathEnv(envname) || envname=="math") || m_latexCommands->needsMathMode(envname) )
				        return false;
				     mathdata.tag = ( envname=="math" ) ? mmMathEnv : mmDisplaymathEnv;
				     mathdata.envname = envname;
			        break;
		case ']' : mathdata.tag = mmDisplaymathParen; 
			        break;
		case ')' : mathdata.tag = mmMathParen; 
			        break;
	}

	return true;
}

bool EditorExtension::findOpenMathTag(Kate::Document *doc, uint row, uint col, QRegExp &reg, MathData &mathdata)
{
	uint lastrow,lastcol;
	QString mathname;

	bool foundDollar= false; 
	uint numDollar = 0;
	
	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);	

	QString textline = getTextLineReal(doc,row);
	int column = (int)col;

	bool continueSearch = true;
	while ( continueSearch )
	{
		while ( (column = reg.searchRev(textline,col)) != -1 )
		{
			col = column;

			mathdata.row = row;
			mathdata.col = col;
			mathdata.len = reg.cap(0).length();
			mathname = reg.cap(0).left(2);
		
			// should be better called 'isValidChar()', because it checks for comments
			// and escaped chars like backslash and dollar in '\\' and '\$'
			if ( mathname == "$" )
			{
				// count and continue search
				numDollar++;

				// but remember the first dollar found backwards 
				if ( ! foundDollar )
				{
					lastrow = row;
					lastcol = col;
					foundDollar = true;
				}
			}
			else if ( mathname=="\\[" || mathname=="\\(" )
			{
				// found start of mathmode
				if ( numDollar == 0 )
				{
					mathdata.tag = ( mathname == "\\[" ) ? mmDisplaymathParen : mmMathParen;
					mathdata.numdollar = 0;
					return true;
				}
				else
				{
					//kdDebug() << "error: dollar not allowed in \\[ or \\( mode" << endl;
					return false;
				}
			}
			else if ( mathname=="\\]" || mathname=="\\)" )
			{
				continueSearch = false;
				break;
			}
			else  if ( mathname=="\\b" )
			{
				// save name of environment
				QString envname = reg.cap(1);

				// if we found the opening tag of a math env
				if ( m_latexCommands->isMathEnv(envname) || envname=="math" )
				{
					if ( numDollar > 0 )
					{
						//kdDebug() << "error: dollar not allowed in math env   numdollar=" << numDollar  << endl;
						return false;
					}

					// if this is a math env with its own mathmode, we have found the starting position
					if ( envname == "math" )
					{
						mathdata.tag = mmMathEnv;
						mathdata.envname = envname;
						return true;
					}

					if ( ! m_latexCommands->needsMathMode(envname) )
					{
						mathdata.tag = mmDisplaymathEnv;
						mathdata.envname = envname;
						return true;
					}
				} 
				// no math env, we found the opening tag of a normal env
				else
				{
					continueSearch = false;
					break;
				}
			}
			else if ( mathname == "\\e" ) 
			{
				QString envname = reg.cap(2);

				// if we found the closing tag of a math env
				if ( m_latexCommands->isMathEnv(envname) || envname=="math" )
				{
					// if this is a math env with its own mathmode
					if ( ! m_latexCommands->needsMathMode(envname) || envname=="math" )
					{
						continueSearch = false;
						break;
					}

					// if this is a math env which needs $..$
					if ( m_latexCommands->isMathModeEnv(envname) )
					{
						if ( numDollar >= 1 )
						{
							numDollar--;
							continueSearch = false;
							break;
						}
						// else continue search
					}
				}
				// if we found the closing tag of a normal env
				else
				{
					continueSearch = false;
					break;
				}
			}
			else
			{
				//kdDebug() << "error: unknown match" << endl;
				return false;
			}

		// continue search one column left of the last match (if this is possible)
		if ( col == 0 )
			break;

		col--;
		}

		if ( row > 0 )
		{
	 		textline = getTextLineReal(doc,--row);
			col = textline.length();
		}

		if ( column == -1 )
		{
			continueSearch = false;
			break;
		}
	}
	
	// nothing else found, so math mode starts a the last dollar (the first one found backwards)
	mathdata.row = lastrow;
	mathdata.col = lastcol;
	mathdata.len = 1;
	mathdata.numdollar = numDollar;
	
	mathdata.tag = ( numDollar > 0 ) ? mmMathDollar : mmNoMathMode;
	
	return true;
}

bool EditorExtension::findCloseMathTag(Kate::Document *doc, uint row, uint col, QRegExp &reg, MathData &mathdata)
{
	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);	

	uint rowFound,colFound,lenFound;
	while ( iface->searchText(row,col,reg,&rowFound,&colFound,&lenFound,false) )
	{
		// should be better called 'isValidChar()', because it checks for comments
		// and escaped chars like backslash and dollar in '\\' and '\$'
		if ( isValidBackslash(doc,rowFound,colFound) )
		{
			QString mathname = reg.cap(0).left(2);

			// always remember behind the last match
			mathdata.row = rowFound;
			mathdata.col = colFound;
			mathdata.len = lenFound;
		
			if ( mathname=="$" )
			{
				mathdata.tag = mmMathDollar;
				return true;
			}
			else if ( mathname=="\\]" )
			{
				mathdata.tag = mmDisplaymathParen;
				return true;
			}
			else if ( mathname=="\\)" )
			{
				mathdata.tag = mmMathParen;
				return true;
			}
			else if ( mathname=="\\[" || mathname=="\\(" )
			{
				//kdDebug() << "error: current mathgroup was not closed" << endl;
				return false;
			}
			else if ( mathname=="\\b" )
			{
				QString envname = reg.cap(1);
				
				if ( ! (m_latexCommands->isMathEnv(envname) || envname=="math") )
				{
					//kdDebug() << "error: only math env are allowed in mathmode (found begin tag)" << endl;
					return false;
				}

				if ( !m_latexCommands->needsMathMode(envname) || envname=="math" )
				{
					//kdDebug() << "error: mathenv with its own mathmode are not allowed in mathmode " << endl;
					return false;
				}
				// else continue search
			}
			else if ( mathname=="\\e" )
			{
				QString envname = reg.cap(2);
				if ( ! (m_latexCommands->isMathEnv(envname) || envname=="math") )
				{
					//kdDebug() << "error: only math env are allowed in mathmode (found end tag)" << endl;
					return false;
				}

				if ( envname == "math" )
				{
					mathdata.tag = mmMathEnv;
					mathdata.envname = envname;
					return true;
				}

				if ( ! m_latexCommands->needsMathMode(envname) )
				{
					mathdata.tag = mmDisplaymathEnv;
					mathdata.envname = envname;
					return true;
				}

				// else continue search
			}
		}

		// continue search one column right of the last match (if this is possible)
		row = rowFound;
		col = colFound;
		if ( ! increaseCursorPosition(doc,row,col) )
			break;
	}
	
	return false;
}

//////////////////// insert newlines inside an environment ////////////////////

// intelligent newlines: look for the last opened environment
// and decide what to insert

void EditorExtension::insertIntelligentNewline(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint row,col;
	QString name;

	if ( findOpenedEnvironment(row,col,name,view) )
	{
		if ( m_latexCommands->isListEnv(name) )
		{
			view->keyReturn();
			view->insertText("\\item " );
			return;
		}
		else if ( m_latexCommands->isTabularEnv(name) || m_latexCommands->isMathEnv(name) )
		{
			view->insertText(" \\\\");
		}
	}
	
	// - found no opened environment
	// - unknown environment
	// - finish tabular or math environment
	view->keyReturn();
}

bool EditorExtension::findOpenedEnvironment(uint &row,uint &col, QString &envname, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return false;
	
	// get current cursor position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	EnvData env;
	uint startrow = row;
	uint startcol = col;
	
	//kdDebug() << "   close - start " << endl;
	// accept a starting place outside an environment
	bool env_position = isEnvironmentPosition(doc,row,col,env);
	
	// We can also accept a column, if we are on the left side of an environment.
	// But we should decrease the current cursor position for the search.
	if ( env_position && env.cpos!=EnvInside )
	{
		if ( env.cpos==EnvLeft && !decreaseCursorPosition(doc,startrow,startcol) )
		return false;
		env_position = false;
	}
	
	if ( !env_position && findEnvironmentTag(doc,startrow,startcol,env,true) )
	{
		//kdDebug() << "   close - found begin env at:  " << env.row << "/" << env.col << " " << env.name << endl;
		row = env.row;
		col = env.col;
		envname = env.name;
		return true;
	}
	else
		return false;
}

QStringList EditorExtension::findOpenedEnvironmentList(Kate::View *view, bool position)
{
	QStringList envlist;

	view = determineView(view);
	if ( view )
	{
		uint currentRow,currentCol;
		Kate::Document *doc = view->getDoc();
		view->cursorPositionReal(&currentRow,&currentCol);

		uint row = currentRow;
		uint col = currentCol;
		EnvData env;

		// check the starting position
		bool env_position = isEnvironmentPosition(doc,row,col,env);
		if ( env_position )
		{
			// we are inside an environment tag: bad to complete
			if ( env.cpos == EnvInside )
				return envlist;	
			// we are left of an environment tag: go one position to the left
			if ( env.cpos == EnvLeft )
			{
				if ( ! decreaseCursorPosition(doc,row,col) )
					return envlist;	
			}
		}
	
		while ( findEnvironmentTag(doc,row,col,env,true) )
		{
			row = env.row;
			col = env.col;

			if ( position )
				envlist << env.name + QString(",%1,%2").arg(row).arg(col);
			else
				envlist << env.name;

			if ( col == 0 )
			{
				if ( ! decreaseCursorPosition(doc,row,col) )
					break;
			}
			view->setCursorPositionReal(row,col);
		}

		// reset cursor original position
		view->setCursorPositionReal(currentRow,currentCol);
	}
	
	return envlist;
}

//////////////////// select an environment  ////////////////////

void EditorExtension::selectEnvironment(bool inside, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	EnvData envbegin,envend;
	
	if ( !view->getDoc()->hasSelection() || !expandSelectionEnvironment(inside,view) ) 
	{
		if ( getEnvironment(inside,envbegin,envend,view) )
			view->getDoc()->setSelection(envbegin.row,envbegin.col,envend.row,envend.col);
	}
}

void EditorExtension::deleteEnvironment(bool inside, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	EnvData envbegin,envend;
	
	if ( getEnvironment(inside,envbegin,envend,view) )
	{
		Kate::Document *doc = view->getDoc();
		doc->clearSelection();
		doc->removeText(envbegin.row,envbegin.col,envend.row,envend.col);
		view->setCursorPosition(envbegin.row,0);
	}
}

// calculate start and end of an environment

bool EditorExtension::getEnvironment(bool inside, EnvData &envbegin, EnvData &envend, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return false;
	
	uint row,col;
	
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	if ( !findBeginEnvironment(doc,row,col,envbegin) )
		return false;
	if ( !findEndEnvironment(doc,row,col,envend) )
		return false;
	
	if ( inside )
	{
		// check first line
		envbegin.col += envbegin.len;
		if ( envbegin.col >= (uint)doc->lineLength(envbegin.row) )
		{
		++envbegin.row;
		envbegin.col = 0;
		}
	}
	else
	{
		envend.col += envend.len;
		// check last line
		if ( envbegin.col==0 && envend.col==(uint)doc->lineLength(envend.row) )
		{
		++envend.row;
		envend.col = 0;
		}
	}
	
	return true;
}

// determine text, startrow and startcol of current environment

QString EditorExtension::getEnvironmentText(uint &row, uint &col, QString &name, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return QString::null;

	EnvData envbegin,envend;
	
	if ( getEnvironment(false,envbegin,envend,view) && envbegin.name!="document" ) 
	{
		row = envbegin.row;
		col = envbegin.col;
		name = envbegin.name;
		return view->getDoc()->text(envbegin.row,envbegin.col,envend.row,envend.col);
	}
	else
	{
		return QString::null;
	}
}

bool EditorExtension::hasEnvironment(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) 
		return false;

	EnvData envbegin,envend;
	return ( getEnvironment(false,envbegin,envend,view) && envbegin.name!="document" );
}

// when an environment is selected (inside or outside), 
// the selection is expanded to the surrounding environment

bool EditorExtension::expandSelectionEnvironment(bool inside, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return false;
	
	Kate::Document *doc = view->getDoc();
	if ( ! doc->hasSelection() )
		return false;
		
	// get current position
	uint row,col;
	view->cursorPositionReal(&row,&col);
	
	// get current selection
	uint row1 = doc->selStartLine();
	uint col1 = doc->selStartCol();
	uint row2 = doc->selEndLine();
	uint col2 = doc->selEndCol();

	// determine current environment outside 
	EnvData oenvbegin,oenvend;
	if ( ! getEnvironment(false,oenvbegin,oenvend,view)  )
		return false;
	
	bool newselection = false;
	// first look, if this environment is selected outside
	if ( row1==oenvbegin.row && col1==oenvbegin.col && row2==oenvend.row && col2==oenvend.col ) {
		if ( ! decreaseCursorPosition(doc,oenvbegin.row,oenvbegin.col) )
			return newselection;
		view->setCursorPositionReal(oenvbegin.row,oenvbegin.col);
		// search the surrounding environment and select it
		if ( getEnvironment(inside,oenvbegin,oenvend,view) ) {
			doc->setSelection(oenvbegin.row,oenvbegin.col,oenvend.row,oenvend.col);
			newselection = true;
		}
	} else { 
		// then determine current environment inside 
		EnvData ienvbegin,ienvend;
		getEnvironment(true,ienvbegin,ienvend,view);
		// and look, if this environment is selected inside
		if ( row1==ienvbegin.row && col1==ienvbegin.col && row2==ienvend.row && col2==ienvend.col ) {
			if ( ! decreaseCursorPosition(doc,oenvbegin.row,oenvbegin.col) )
				return newselection;
			view->setCursorPositionReal(oenvbegin.row,oenvbegin.col);
			// search the surrounding environment and select it
			if ( getEnvironment(inside,ienvbegin,ienvend,view) ) {
				doc->setSelection(ienvbegin.row,ienvbegin.col,ienvend.row,ienvend.col);
				newselection = true;
			}
		}
	} 
	
	// restore old cursor position
	view->setCursorPositionReal(row,col);
	return newselection;
}

//////////////////// search for \begin{env}  ////////////////////

// Find the last \begin{env} tag. If the current cursor is over
//  - \begin{env} tag: we will stop immediately
//  - \end{env} tag: we will start before this tag

bool EditorExtension::findBeginEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env)
{
	// kdDebug() << "   find begin:  " << endl;
	if ( isEnvironmentPosition(doc,row,col,env) )
	{
		// already found position?
		//kdDebug() << "   found env at:  " << env.row << "/" << env.col << " " << env.name << endl;
		if ( env.tag == EnvBegin )
		{
		//kdDebug() << "   is begin env at:  " << env.row << "/" << env.col << " " << env.name << endl;
		return true;
		}
	
		// go one position back
		//kdDebug() << "   is end env at:  " << env.row << "/" << env.col << " " << env.name << endl;
		row = env.row;
		col = env.col;
		if ( ! decreaseCursorPosition(doc,row,col) )
		return false;
	}
	
	// looking back for last environment
	//kdDebug() << "   looking back from pos:  " << row << "/" << col << " " << env.name << endl;
	return findEnvironmentTag(doc,row,col,env,true);
}

//////////////////// search for \end{env}  ////////////////////

// Find the last \end{env} tag. If the current cursor is over
//  - \end{env} tag: we will stop immediately
//  - \begin{env} tag: we will start behind this tag

bool EditorExtension::findEndEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env)
{
	if ( isEnvironmentPosition(doc,row,col,env) )
	{
		// already found position?
		if ( env.tag == EnvEnd )
			return true;

		// go one position forward
		row = env.row;
		col = env.col + 1;
	}

	// looking forward for the next environment
	return findEnvironmentTag(doc,row,col,env,false);
}

//////////////////// search for an environment tag ////////////////////

// find the last/next non-nested environment tag

bool EditorExtension::findEnvironmentTag(Kate::Document *doc, uint row, uint col,
                                  EnvData &env,bool backwards)
{
	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);
	
	uint envcount = 0;
	
	EnvTag wrong_env = ( backwards ) ? EnvEnd : EnvBegin;
	while ( iface->searchText(row,col,m_reg,&env.row,&env.col,&env.len,backwards) )
	{
		if ( isValidBackslash(doc,env.row,env.col) )
		{
			EnvTag found_env = ( m_reg.cap(2)=="begin" || m_reg.cap(4)=="\\[" ) ? EnvBegin : EnvEnd; 
			if ( found_env == wrong_env )
			{
				++envcount;
			}
			else
			{
				if ( envcount > 0 )
					--envcount;
				else
				{
					if ( found_env == EnvBegin ) 
					{
						env.name = ( m_reg.cap(2)=="begin" ) ? m_reg.cap(3) : "\\[";
					}
					else
					{
						env.name = ( m_reg.cap(2)=="end" ) ? m_reg.cap(3) : "\\]";
					}	
					env.tag = found_env;
					return true;
				}
			}
		}
	
		// new start position
		if ( !backwards )
		{
			row = env.row;
			col = env.col + 1;
		}
		else
		{
			row = env.row;
			col = env.col;
			if ( ! decreaseCursorPosition(doc,row,col) )
				return false;
		}
	}

	return false;
}

//////////////////// check for an environment position ////////////////////

// Check if the current position belongs to an environment. The result is set
// to the beginning backslash of the environment tag. The same algorithms as
// matching brackets is used.

bool EditorExtension::isEnvironmentPosition(Kate::Document *doc, uint row, uint col, EnvData &env)
{
	// get real textline without comments, quoted characters and pairs of backslashes
	QString textline = getTextLineReal(doc,row);
	
	if ( col > textline.length() )
		return false;
	
	EnvData envright;
	bool left = false;
	bool right = false;
	
	//KTextEditor::SearchInterface *iface;
	//iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);
	
	// check if there is a match in this line from the current position to the left
	int startcol = ( textline[col] == '\\' ) ? col - 1 : col;
	if ( startcol >= 1 )
	{
		int pos = textline.findRev(m_reg,startcol);
		env.len = m_reg.matchedLength();
		if ( pos!=-1 && (uint)pos<col && col<=(uint)pos+env.len )
		{
			env.row = row;
			env.col = pos;
			QChar ch = textline.at(pos+1);
			if ( ch=='b' || ch=='e' ) 
			{
				env.tag = ( ch == 'b' ) ? EnvBegin : EnvEnd;
				env.name = m_reg.cap(3);
			} 
			else
			{
				env.tag = ( ch == '[' ) ? EnvBegin : EnvEnd;
				env.name = m_reg.cap(4);
			}
			env.cpos =  ( col < (uint)pos+env.len ) ? EnvInside : EnvRight;
			// we have already found a tag, if the cursor is inside, but not behind this tag
			if ( env.cpos == EnvInside )
				return true;
			left = true;
		//kdDebug() << "   is - found left:  pos=" << pos << " " << env.name << " " << QString(textline.at(pos+1)) << endl;
		}
	}
	
	// check if there is a match in this line from the current position to the right
	if ( textline[col]=='\\' && col==(uint)textline.find(m_reg,col) )
	{
		envright.row = row;
		envright.col = col;
		envright.len = m_reg.matchedLength();
		QChar ch = textline.at(col+1);
		if ( ch=='b' || ch=='e' )        // found "\begin" or "\end"
		{
			envright.tag = ( ch == 'b' ) ? EnvBegin : EnvEnd;
			envright.name = m_reg.cap(3);
		} 
		else                             // found "\[" or "\\]"
		{
			envright.tag = ( ch == '[' ) ? EnvBegin : EnvEnd;
			envright.name = m_reg.cap(4);
		}
		envright.cpos = EnvLeft;
		right = true;
		//kdDebug() << "   is - found right:  pos=" <<col << " " << envright.name << " " << QString(textline.at(col+1)) << endl;
	}
	
	//kdDebug() << "found left/right: " << left << "/" << right << endl;
	// did we find a tag?
	if ( ! (left || right) )
		return false;
	
	// now check, which tag we should be taken (algorithm like matching brackets)
	
	if ( m_overwritemode )
	{
		if ( right && envright.tag==EnvBegin )
		{
		env = envright;
		return true;
		}
		else if ( left && env.tag==EnvEnd )
		return true;
		else
		return false;
	}
	else if ( left && env.tag==EnvEnd )
	{
		//kdDebug() << "   1: accept left end:  " << env.name << endl;
		return true;
	}
	else if ( right && envright.tag==EnvBegin )
	{
		//kdDebug() << "   2: accept right begin:  " << envright.name << endl;
		env = envright;
	}
	else if ( left && env.tag==EnvBegin )
	{
		// kdDebug() << "   3: accept left begin:  " << env.name << endl;
		return true;
	}
	else if ( right && envright.tag==EnvEnd )
	{
		//kdDebug() << "   4: accept right end:  " << envright.name << endl;
		env = envright;
	}
	else
		return false;
	
	return true;
}

//////////////////// check for a comment ////////////////////

// check if the current position is within a comment

bool EditorExtension::isCommentPosition(Kate::Document *doc, uint row, uint col)
{
	QString textline = doc->textLine(row);
	
	bool backslash = false;
	for ( uint i=0; i<col; ++i )
	{
		if ( textline[i] == '%' )
		{
		if ( !backslash )
			return true;                  // found a comment sign
		else
			backslash = false;
		}
		else if ( textline[i] == '\\' )   // count number of backslashes
		backslash = !backslash;
		else
		backslash = false;               // no backslash
	}
	
	return false;
}

// check if the character at text[col] is a valid backslash:
//  - there is no comment sign in this line before
//  - there is not a odd number of backslashes directly before

bool EditorExtension::isValidBackslash(Kate::Document *doc, uint row, uint col)
{
	QString textline = doc->textLine(row);
	
	bool backslash = false;
	for ( uint i=0; i<col; ++i )
	{
		if ( textline[i] == '%' )
		{
		if ( !backslash )
			return false;                 // found a comment sign
		else
			backslash = false;
		}
		else if ( textline[i] == '\\' )   // count number of backslashes
		backslash = !backslash;
		else
		backslash = false;               // no backslash
	}
	
	return !backslash;
}

//////////////////// goto next bullet ////////////////////

void EditorExtension::gotoBullet(bool backwards, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	uint row,col,ypos,xpos,len;
	
	// get current position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	// change the start position or we will stay at this place
	if ( backwards )
	{
		if ( ! decreaseCursorPosition(doc,row,col) )
		return;
	}
	else
	{
		if ( ! increaseCursorPosition(doc,row,col) )
		return;
	}
	
	if ( doc->searchText(row,col,s_bullet,&ypos,&xpos,&len,true,backwards) )
	{
		doc->setSelection(ypos,xpos,ypos,xpos+1);
		view->setCursorPositionReal(ypos,xpos);
	}
}

//////////////////// increase/decrease cursor position ////////////////////

bool EditorExtension::increaseCursorPosition(Kate::Document *doc, uint &row, uint &col)
{
	bool ok = true;
	
	if ( (int)col < doc->lineLength(row)-1 )
		++col;
	else if ( row < doc->numLines() - 1 )
	{
		++row;
		col=0;
	}
	else
		ok = false;
	
	return ok;
}

bool EditorExtension::decreaseCursorPosition(Kate::Document *doc, uint &row, uint &col)
{
	bool ok = true;
	
	if (col > 0)
		--col;
	else if ( row > 0 )
	{
		--row;
		col = doc->lineLength(row);
	}
	else
		ok = false;
	
	return ok;
}

//////////////////// texgroups ////////////////////

// goto the next non-nested bracket

void EditorExtension::gotoTexgroup(bool backwards, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	uint row,col;
	bool found;
	BracketData bracket;
	
	// get current position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	m_overwritemode = view->isOverwriteMode();
	
	// start searching
	if ( backwards )
		found = findOpenBracket(doc,row,col,bracket);
	else
	{
		found = findCloseBracket(doc,row,col,bracket);
		// go behind the bracket
		if ( ! m_overwritemode )
		++bracket.col;
	}
	
	if ( found )
		view->setCursorPositionReal(bracket.row,bracket.col);
}

// match the opposite bracket

void EditorExtension::matchTexgroup(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	uint row,col;
	BracketData bracket;
	
	// get current position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	m_overwritemode = view->isOverwriteMode();
	
	// this operation is only allowed at a bracket position
	if ( !isBracketPosition(doc,row,col,bracket) )
		return;
	
	// start searching
	bool found = false;
	if ( bracket.open )
	{
		found = findCloseBracketTag(doc,bracket.row,bracket.col+1,bracket);
		// go behind the bracket
		if ( ! m_overwritemode )
		++bracket.col;
	}
	else
	{
		if ( !decreaseCursorPosition(doc,bracket.row,bracket.col) )
		return;
		found = findOpenBracketTag(doc,bracket.row,bracket.col,bracket);
	}
	
	if ( found )
		view->setCursorPositionReal(bracket.row,bracket.col);
}

//////////////////// close an open texgroup  ////////////////////

// search for the last opened texgroup and close it

void EditorExtension::closeTexgroup(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	uint row,col;
	BracketData bracket;
	
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	uint rowtemp = row;
	uint coltemp = col;
	if ( !decreaseCursorPosition(doc,rowtemp,coltemp) )
		return;
	
	if ( findOpenBracketTag(doc,rowtemp,coltemp,bracket) )
	{
		doc->insertText( row,col,"}" );
		view->setCursorPositionReal(row,col+1);
	}
}

//////////////////// select a texgroup  ////////////////////

void EditorExtension::selectTexgroup(bool inside, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	BracketData open,close;
	
	if ( getTexgroup(inside,open,close,view) )
	{
		Kate::Document *doc = view->getDoc();
		doc->setSelection(open.row,open.col,close.row,close.col);
	}
}

void EditorExtension::deleteTexgroup(bool inside, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	BracketData open,close;
	
	if ( getTexgroup(inside,open,close,view) )
	{
		Kate::Document *doc = view->getDoc();
		doc->clearSelection();
		doc->removeText(open.row,open.col,close.row,close.col);
		view->setCursorPositionReal(open.row,open.col+1);
	}
}

// calculate start and end of an environment

bool EditorExtension::getTexgroup(bool inside, BracketData &open, BracketData &close, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return false;
	
	uint row,col;
	
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	if ( !findOpenBracket(doc,row,col,open) ) 
	{ 
		//kdDebug() << "no open bracket" << endl; 
		return false;
	}
	if ( !findCloseBracket(doc,row,col,close) ) 
	{ 
		//kdDebug() << "no close bracket" << endl; 
		return false;
	}
	
	if ( inside )
		++open.col;
	else
		++close.col;

	return true;
}

//////////////////// search for a bracket position  ////////////////////

// Find the last opening bracket. If the current cursor is over
//  - '{': we will stop immediately
//  - '}': we will start before this character

bool EditorExtension::findOpenBracket(Kate::Document *doc, uint row, uint col, BracketData &bracket)
{
	if ( isBracketPosition(doc,row,col,bracket) )
	{
		// already found position?
		if ( bracket.open ) return true;
		
		
		// go one position back
		row = bracket.row;
		col = bracket.col;
		if ( ! decreaseCursorPosition(doc,row,col) ) return false;
	}
	
	// looking back for last bracket
	return findOpenBracketTag(doc,row,col,bracket);
}

// Find the last closing bracket. If the current cursor is over
//  - '}': we will stop immediately
//  - '{': we will start behind this character

bool EditorExtension::findCloseBracket(Kate::Document *doc, uint row, uint col, BracketData &bracket)
{
	if ( isBracketPosition(doc,row,col,bracket) )
	{
		// already found position?
		if ( ! bracket.open ) return true;
	
		// go one position forward
		row = bracket.row;
		col = bracket.col + 1;
	}
	
	// looking forward for next bracket
	return findCloseBracketTag(doc,row,col,bracket);
}

/*
   Bracket matching uses the following algorithm (taken from Kate):
   1) If in overwrite mode, match the bracket currently underneath the cursor.
   2) Otherwise, if the character to the left of the cursor is an ending bracket,
      match it.
   3) Otherwise if the character to the right of the cursor is a
      starting bracket, match it.
   4) Otherwise, if the the character to the left of the cursor is an
      starting bracket, match it.
   5) Otherwise, if the character to the right of the cursor is an
      ending bracket, match it.
   6) Otherwise, don't match anything.
*/

bool EditorExtension::isBracketPosition(Kate::Document *doc, uint row, uint col, BracketData &bracket)
{
	// default results
	bracket.row = row;
	bracket.col = col;
	
	QString textline = getTextLineReal(doc,row);
	QChar right = textline[col];
	QChar left  = ( col > 0 ) ? textline[col-1] : QChar(' ');
	
	if ( m_overwritemode )
	{
		if ( right == '{' )
		{
		bracket.open = true;
		}
		else if ( left == '}' )
		{
		bracket.open = false;
		}
		else
		return false;
	}
	else if ( left == '}' )
	{
		bracket.open = false;
		--bracket.col;
	}
	else if ( right == '{' )
	{
		bracket.open = true;
	}
	else if ( left == '{' )
	{
		bracket.open = true;
		--bracket.col;
	}
	else if ( right == '}' )
	{
		bracket.open = false;
	}
	else
		return false;
	
	return true;
}

// find next non-nested closing bracket

bool EditorExtension::findCloseBracketTag(Kate::Document *doc, uint row, uint col,BracketData &bracket)
{
	uint brackets = 0;
	for ( uint line=row; line<doc->numLines(); ++line )
	{
		uint start = ( line == row ) ? col : 0;
		QString textline = getTextLineReal(doc,line);
		for ( uint i=start; i<textline.length(); ++i )
		{
		if ( textline[i] == '{' )
		{
			++brackets;
		}
		else if ( textline[i] == '}' )
		{
			if ( brackets > 0 )
				--brackets;
			else
			{
			bracket.row = line;
			bracket.col = i;
			bracket.open = false;
			return true;
			}
		}
		}
	}
	
	return false;
}

// find next non-nested opening bracket

bool EditorExtension::findOpenBracketTag(Kate::Document *doc, uint row, uint col, BracketData &bracket)
{
	uint brackets = 0;
	for ( int line=row; line>=0; --line )
	{
		QString textline = getTextLineReal(doc,line);
		int start = ( line == (int)row ) ? col : textline.length()-1;
		for ( int i=start; i>=0; --i )
		{
			//kdDebug() << "findOpenBracketTag: (" << line << "," << i << ") = " << textline[i].latin1() << endl;
			if ( textline[i] == '{' )
			{
				if ( brackets > 0 )
					--brackets;
				else
				{
					bracket.row = line;
					bracket.col = i;
					bracket.open = true;
					return true;
				}
			}
			else if ( textline[i] == '}' )
			{
				++brackets;
			}
		}
	}
	
	//kdDebug() << "nothting found" << endl;
	return false;
}

//////////////////// get real text ////////////////////

// get current textline and remove
//  - all pairs of backslashes: '\\'
//  - all quoted comment signs: '\%'
//  - all quoted brackets: '\{' and '\}'
//  - all comments
// replace these characters one one, which never will be looked for

QString EditorExtension::getTextLineReal(Kate::Document *doc, uint row)
{
	QString textline = doc->textLine(row);
	uint len = textline.length();
	if ( len == 0)
		return QString::null;
	
	bool backslash = false;
	for (uint i=0; i<len; ++i )
	{
		if ( textline[i]=='{' || textline[i]=='}' || textline[i]=='$')
		{
		if ( backslash )
		{
			textline[i-1] = '&';
			textline[i] = '&';
		}
		backslash = false;
		}
		else if ( textline[i]=='\\' )
		{
		if ( backslash )
		{
			textline[i-1] = '&';
			textline[i] = '&';
			backslash = false;
		}
		else
			backslash = true;
		}
		else if ( textline[i]=='%' )
		{
		if ( backslash )
		{
			textline[i-1] = '&';
			textline[i] = '&';
		}
		else
		{
			len = i;
			break;
		}
		backslash = false;
		}
		else
		backslash = false;
	
	}
	
	// return real text
	return textline.left(len);
}

//////////////////// capture the current word ////////////////////

// Capture the current word from the cursor position to the left and right.
// The result depens on the given search mode;
// - smTex       only letters, except backslash as first and star as last  character
// - smLetter:   only letters
// - smWord:     letters and digits
// - smNospace:  everything except white space

bool EditorExtension::getCurrentWord(Kate::Document *doc, uint row, uint col, EditorExtension::SelectMode mode, QString &word,uint &x1,uint &x2)
{
    // get real textline without comments, quoted characters and pairs of backslashes
	QString textline = getTextLineReal(doc,row);
	if ( col > textline.length() )
		return false;
	
	QRegExp reg;
	QString pattern1,pattern2;
	switch ( mode )
	{
	case smLetter :
		pattern1 = "[^a-zA-Z]+";
		pattern2 = "[a-zA-Z]+";
		break;
	case smWord   :
		pattern1 = "[^a-zA-Z0-9]";
		pattern2 = "[a-zA-Z0-9]+";
		break;
	case smNospace:
		pattern1 = "\\s";
		pattern2 = "\\S+";
		break;
	default       :
		pattern1 = "[^a-zA-Z]";
		pattern2 = "\\\\?[a-zA-Z]+\\*?";
		break;
	}
	x1 = x2 = col;
	
	int pos;
	// search to the left side
	if ( col > 0 )
	{
		reg.setPattern(pattern1);
		pos = textline.findRev(reg,col-1);
		if ( pos != -1 ) {        // found an illegal character
			x1 = pos + 1;
			if ( mode == smTex ) {
				if ( textline[pos] == '\\' )
					x1 = pos;
				col = x1;
			}
		} else {
			x1 = 0;               // pattern matches from beginning of line
		} 
	}
	
	// search at the current position
	reg.setPattern(pattern2);
	pos = textline.find(reg,col);
	if ( pos!=-1 && (uint)pos==col )
	{
		x2 = pos + reg.matchedLength();
	}
	
	// get all characters
	if ( x1 != x2 )
	{
		word = textline.mid(x1,x2-x1);
		return true;
	}
	else
		return false;
}


//////////////////// paragraph ////////////////////

void EditorExtension::selectParagraph(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint startline,endline;
	
	if ( findCurrentTexParagraph(startline,endline,view) )
	{
		view->getDoc()->setSelection(startline,0,endline+1,0);
	}
}

void EditorExtension::deleteParagraph(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint startline,endline;
	
	if ( findCurrentTexParagraph(startline,endline,view) )
	{
		Kate::Document *doc = view->getDoc();
		doc->clearSelection();
		if ( startline > 0 )
		--startline;
		else if ( endline < doc->numLines()-1 )
		++endline;
		doc->removeText(startline,0,endline+1,0);
		view->setCursorPosition(startline,0);
	}
}

// get the range of the current paragraph

bool EditorExtension::findCurrentTexParagraph(uint &startline, uint &endline, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return false;
	
	uint row,col;
	
	// get current position
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	// don't accept an empty line as part of a paragraph
	if ( doc->textLine(row).stripWhiteSpace().isEmpty() )
		return false;
	
	// settings default results
	startline = row;
	endline = row;
	
	// find the previous empty line
	for ( int line=row-1; line>=0; --line )
	{
		if ( doc->textLine(line).stripWhiteSpace().isEmpty() )
		break;
		startline = line;
	}
	
	// find the next empty line
	for ( uint line=row+1; line<doc->numLines(); ++line )
	{
		if ( doc->textLine(line).stripWhiteSpace().isEmpty() )
		break;
		endline = line;
	}
	
	// settings result
	return true;
}

void EditorExtension::gotoNextParagraph(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	bool found;
	uint startline,endline;
	Kate::Document *doc = view->getDoc();

	endline = view->cursorLine();
	if ( doc->textLine(endline).stripWhiteSpace().isEmpty() )
		found = true;
	else
		found = findCurrentTexParagraph(startline,endline,view);

	// we are in an empty line or in the last line of a paragraph
	if ( found )
	{
		// find the next non empty line
		for ( uint line=endline+1; line<doc->numLines(); ++line )
		{
			if ( ! doc->textLine(line).stripWhiteSpace().isEmpty() )
			{
				view->setCursorPositionReal(line,0);
				return;
			}
		}
	}
}

void EditorExtension::gotoPrevParagraph(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	bool found;
	uint startline,endline;
	Kate::Document *doc = view->getDoc();

	startline = view->cursorLine();
	if ( doc->textLine(startline).stripWhiteSpace().isEmpty() )
	{
		startline++;
		found = true;
	}
	else
		found = findCurrentTexParagraph(startline,endline,view);

	// we are in an empty line or in the first line of a paragraph
	if ( found  )
	{
		// find the last line of the previous paragraph
		int foundline = -1;
		for ( int line=startline-1; line>=0; --line )
		{
			if ( ! doc->textLine(line).stripWhiteSpace().isEmpty() )
				break;
			foundline = line;
		}
		if ( foundline < 0 )
			return;

		// and finally the first line of this paragraph
		int prevstartline = -1;
		for ( int line=foundline-1; line>=0; --line )
		{
			if ( doc->textLine(line).stripWhiteSpace().isEmpty() )
				break;
			prevstartline = line;
		}

		if ( prevstartline >= 0 )
			view->setCursorPositionReal((uint)prevstartline,0);
	}
}

//////////////////// gotoLine ////////////////////

void EditorExtension::gotoLine(Kate::View *view)
{
	view = determineView(view);
	if ( view ) 
		view->gotoLine();
}

//////////////////// one line of text////////////////////

void EditorExtension::selectLine(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	// get current position
	uint row,col;
	QString word;
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	if ( doc->lineLength(row) > 0 )
	{
		doc->setSelection(row,0,row+1,0);
	}
}

void EditorExtension::deleteEndOfLine(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint row,col;
	view->cursorPositionReal(&row,&col);

	Kate::Document *doc = view->getDoc();
	doc->clearSelection();
	doc->removeText(row,col,row,doc->lineLength(row));
}

//////////////////// LaTeX command ////////////////////

void EditorExtension::selectWord(EditorExtension::SelectMode mode, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	// get current position
	uint row,col,col1,col2;
	QString word;
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	if ( getCurrentWord(doc,row,col,mode,word,col1,col2) )
	{
		doc->setSelection(row,col1,row,col2);
	}
}

void EditorExtension::deleteWord(EditorExtension::SelectMode mode, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;
	
	// get current position
	uint row,col,col1,col2;
	QString word;
	Kate::Document *doc = view->getDoc();
	view->cursorPositionReal(&row,&col);
	
	if ( getCurrentWord(doc,row,col,mode,word,col1,col2) )
	{
		doc->removeText(row,col1,row,col2);
	}
}

void EditorExtension::nextBullet(Kate::View* view)
{
	gotoBullet(false, view);
}

void EditorExtension::prevBullet(Kate::View* view)
{
	gotoBullet(true, view);
}

void EditorExtension::insertBullet(Kate::View* view)
{
	uint col, pos;
	view = determineView(view);
	if(!view)
	{
		return;
	}
	view->cursorPositionReal(&col, &pos);
	view->getDoc()->insertText(col, pos, s_bullet);
}

void EditorExtension::completeWord()
{
	complete()->editComplete(m_ki->viewManager()->currentTextView(), KileDocument::CodeCompletion::cmLatex);
}

void EditorExtension::completeEnvironment()
{
	complete()->editComplete(m_ki->viewManager()->currentTextView(), KileDocument::CodeCompletion::cmEnvironment);
}

void EditorExtension::completeAbbreviation()
{
	complete()->editComplete(m_ki->viewManager()->currentTextView(), KileDocument::CodeCompletion::cmAbbreviation);
}

//////////////////// double quotes ////////////////////

void EditorExtension::initDoubleQuotes()
{
	m_dblQuotes = KileConfig::insertDoubleQuotes();
	
	int index = KileConfig::doubleQuotes();
	if ( index<0 && index>=(int)m_quoteList.count() )
		index = 0;
	
	QStringList quotes = QStringList::split(QRegExp("\\s{2,}"), m_quoteList[index] ); 
	m_leftDblQuote=  quotes[1];
	m_rightDblQuote = quotes[2];
	kdDebug() << "new quotes: " << m_dblQuotes << " left=" << m_leftDblQuote << " right=" << m_rightDblQuote<< endl;
}

bool EditorExtension::insertDoubleQuotes()
{
	// don't insert double quotes, if konsole has focus 
	// return false, because if this is called from an event
	// handler, because this event has to be passed on
	if ( m_ki->texKonsole()->hasFocus() )
		return false;

	// insert double quotes, normal mode or autocompletion mode
	// always return true for event handler
	Kate::View *view = determineView(0L);
	if ( !view ) return true;
	
	uint row,col;
	view->cursorPositionReal(&row,&col);
	Kate::Document *doc = view->getDoc();

	if( doc && m_ki->extensions()->isTexFile(doc->url()) )
		doc->removeSelectedText();
	else
		return false;

	// simply insert, if we are inside a verb command
	if ( insideVerb(view) || insideVerbatim(view) )
		return false;

	// simply insert, if autoinsert mode is not active or the char bevor is \ (typically for \"a useful)
	if ( !m_dblQuotes || ( col > 0 && doc->text(row,col-1,row,col) == QString("\\") ) )
		return false;

	// insert with auto mode
	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);
		
	QString pattern1 = QRegExp::escape(m_leftDblQuote);
	if ( m_leftDblQuote.at(m_leftDblQuote.length()-1).isLetter() )
		pattern1 += "(\\b|(\\{\\}))";
	QString pattern2 = QRegExp::escape(m_rightDblQuote);
	if ( m_rightDblQuote.at(m_rightDblQuote.length()-1).isLetter() )
		pattern2 += "(\\b|(\\{\\}))";

	QRegExp reg('(' + pattern1 + ")|(" + pattern2 + ')');

	uint r,c,l;
	bool openfound = false;
	if ( iface->searchText(row,col,reg,&r,&c,&l,true) )  
	{
		openfound = ( doc->textLine(r).find(m_leftDblQuote,c) == (int)c );
		//kdDebug() << "pattern=" << reg.pattern() << " " << reg.cap(1) << " r=" << r << " c=" << c << " open=" << openfound<< endl;
	}
	
	QString textline = doc->textLine(row);
	//kdDebug() << "text=" << textline << " open=" << openfound << endl;
	if ( openfound ) 
	{
		// If we last inserted a language specific doublequote open,  
		// we have to change it to a normal doublequote. If not we 
		// insert a language specific doublequote close
		int startcol = col - m_leftDblQuote.length();
		//kdDebug() << "startcol=" << startcol << " col=" << col  << endl;
		if ( startcol>=0 && textline.find(m_leftDblQuote,startcol) == (int)startcol ) 
		{
				doc->removeText(row,startcol,row,startcol+m_leftDblQuote.length());
				doc->insertText(row,startcol,"\"");
		}
		else
		{
			doc->insertText(row,col,m_rightDblQuote);
		}
	}
	else 
	{
		// If we last inserted a language specific doublequote close,  
		// we have to change it to a normal doublequote. If not we 
		// insert a language specific doublequote open
		int startcol = col - m_rightDblQuote.length();
		//kdDebug() << "startcol=" << startcol << " col=" << col  << endl;
		if ( startcol>=0 && textline.find(m_rightDblQuote,startcol) == (int)startcol ) 
		{
			doc->removeText(row,startcol,row,startcol+m_rightDblQuote.length());
			doc->insertText(row,startcol,"\"");
		} 
		else 
		{
			doc->insertText(row,col,m_leftDblQuote);
		}
	}
	return true;
}

//////////////////// insert tabulator ////////////////////
	
void EditorExtension::insertIntelligentTabulator()
{
	Kate::View *view = determineView(0L);
	if ( !view ) return;
	
	uint row,col,currentRow,currentCol;
	QString envname,tab;
	QString prefix = " ";
	
	view->cursorPositionReal(&currentRow,&currentCol);
	if ( findOpenedEnvironment(row,col,envname,view) ) 
	{
		// look if this is an environment with tabs
		tab = m_latexCommands->getTabulator(envname); 
		
		// try to align tabulator with textline above
		if ( currentRow >= 1 ) 
		{
			int tabpos = view->getDoc()->textLine(currentRow-1).find('&',currentCol);
			if ( tabpos >= 0 ) 
			{
				currentCol = tabpos;
				prefix = QString::null;
			}
		}
	}
	
	if ( tab == QString::null ) 
		tab = "&";
	tab = prefix + tab + ' ';
	
	view->getDoc()->insertText(currentRow,currentCol,tab);
	view->setCursorPositionReal(currentRow,currentCol+tab.length());
}

//////////////////// autocomplete environment ////////////////////

// should we complete the current environment (call from KileEventFilter)

bool EditorExtension::eventInsertEnvironment(Kate::View *view)
{
	// don't complete environment, if we are
	// still working inside the completion box
	if ( m_complete->inProgress() )
		return false;

	int row = view->cursorLine();
	int col = view->cursorColumnReal();
	QString line = view->getDoc()->textLine(row).left(col);

	int pos = m_regexpEnter.search(line);
	if (pos != -1 )
	{
		line = m_regexpEnter.cap(1);
		for (uint i=0; i < line.length(); ++i)
			if ( ! line[i].isSpace() ) line[i] = ' ';
		
		QString envname, endenv;
		if ( m_regexpEnter.cap(2) == "\\[" ) 
		{
			envname = m_regexpEnter.cap(2);
			endenv = "\\]\n";
		}
		else
		{
			envname = m_regexpEnter.cap(4);
			endenv = m_regexpEnter.cap(2).replace("\\begin","\\end") + '\n';
		}
		
		if ( shouldCompleteEnv(envname, view) )
		{
			QString item =  m_latexCommands->isListEnv(envname) ? "\\item " : QString::null;
			view->getDoc()->insertText(row,col, '\n'+line+m_envAutoIndent+item +'\n'+line+endenv);
			view->setCursorPositionReal(row+1, line.length()+m_envAutoIndent.length()+item.length());
			return true;
		}
	}
	return false;
}

bool EditorExtension::shouldCompleteEnv(const QString &env, Kate::View *view)
{
	kdDebug() << "===EditorExtension::shouldCompleteEnv( " << env << " )===" << endl;
	QRegExp reTestBegin,reTestEnd;
	if ( env == "\\[" )
	{
		kdDebug() << "display style" << endl;
		reTestBegin.setPattern("(?:[^\\\\]|^)\\\\\\["); 
		// the first part is a non-capturing bracket (?:...) and we check if we don't have a backslash in front,
		//  or that we are at the begin of the line
		reTestEnd.setPattern("(?:[^\\\\]|^)\\\\\\]");
	}
	else
	{
		reTestBegin.setPattern("(?:[^\\\\]|^)\\\\begin\\s*\\{" + QRegExp::escape(env) + "\\}");
		reTestEnd.setPattern("(?:[^\\\\]|^)\\\\end\\s*\\{" + QRegExp::escape(env) + "\\}");
	}

	int num = view->getDoc()->numLines();
	int numBeginsFound = 0;
	int numEndsFound = 0;
	uint realLine, realColumn;
	view->cursorPositionReal(&realLine, &realColumn);	
	for ( int i = realLine; i < num; ++i)
	{
		numBeginsFound += view->getDoc()->textLine(i).contains(reTestBegin);
		numEndsFound += view->getDoc()->textLine(i).contains(reTestEnd);
		kdDebug() << "line is " << i <<  " numBeginsFound = " << numBeginsFound <<  " , " << "numEndsFound = " << numEndsFound << endl;
		if ( numEndsFound >= numBeginsFound )
			return false;
		else if ( numEndsFound == 0 && numBeginsFound > 1 )
			return true;
		else if ( numBeginsFound > 2 || numEndsFound > 1 )
			return true; // terminate the search
	}

	return true;
}

QString EditorExtension::getWhiteSpace(const QString &s)
{
	QString whitespace = s;
	for ( uint i=0; i<whitespace.length(); ++i )
	{
		if ( ! whitespace[i].isSpace() ) 
			whitespace[i] = ' ';
	}
	return whitespace;
}

//////////////////// inside verbatim commands ////////////////////

bool EditorExtension::insideVerbatim(Kate::View *view)
{
	uint rowEnv,colEnv;
	QString nameEnv;

	if ( findOpenedEnvironment(rowEnv,colEnv,nameEnv,view) )
	{
		if ( m_latexCommands->isVerbatimEnv(nameEnv) )
			return true;
	}

	return false;
}

bool EditorExtension::insideVerb(Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return false;
	
	// get current position
	uint row,col;
	view->cursorPositionReal(&row,&col);

	int startpos = 0;
	QString textline = getTextLineReal(view->getDoc(),row);
	QRegExp reg("\\\\verb(\\*?)(.)");
	while ( true )
	{
		int pos = textline.find(reg,startpos);
		if ( pos<0 || col<(uint)pos+6+reg.cap(1).length() ) 
			return false; 

		pos = textline.find(reg.cap(2),pos+6+reg.cap(1).length());
		if ( pos<0 || col<=(uint)pos )
			return true;

		startpos = pos + 1;
	}
}

//////////////////// goto sectioning command ////////////////////

void EditorExtension::gotoNextSectioning()
{
	gotoSectioning(false);
}

void EditorExtension::gotoPrevSectioning()
{
	gotoSectioning(true);
}

void EditorExtension::gotoSectioning(bool backwards, Kate::View *view)
{
	view = determineView(view);
	if ( !view ) return;

	uint rowFound,colFound;
	m_ki->viewManager()->updateStructure(true);
	if ( m_ki->structureWidget()->findSectioning(view->getDoc(),view->cursorLine(),view->cursorColumn(),
	                                             backwards,rowFound,colFound) )
	{
		view->setCursorPositionReal(rowFound,colFound);
	}
}

//////////////////// sectioning popup ////////////////////

void EditorExtension::sectioningCommand(KileListViewItem *item, int id)
{
	Kate::View *view = determineView(0L);
	if ( !view ) return;

	if ( ! item ) 
		return;
	Kate::Document *doc = view->getDoc();

	// try to determine the whole secting
	// get the start auf the selected sectioning
	uint row,col,row1,col1,row2,col2;
	row = row1 = item->startline() - 1;
	col = col1 = item->startcol() - 1;

	// check, if the document was changed in the meantime 
	QRegExp reg( "\\\\(part|chapter|section|subsection|subsubsection|paragraph|subparagraph)\\*?\\s*(\\{|\\[)" );
	QString textline = getTextLineReal(doc,row1);
	if ( reg.search(textline,col1) != (int)col1 )
	{
		m_ki->logWidget()->clear();
		m_ki->logWidget()->printMsg(KileTool::Error,
		       i18n("The document was modified and the structure view should be updated, before starting such an operation."), 
		       i18n("Structure View Error") );
		return;
	}

	// increase cursor position and find the following sectioning command
	if ( ! increaseCursorPosition(doc,row,col) )
		return;
	if ( ! m_ki->structureWidget()->findSectioning(doc,row,col,false,row2,col2) )
	{
		// or the end of the document
		// if there is a '\end{document} command, we have to exclude it
		if ( ! findEndOfDocument(doc,row,col,row2,col2) )
		{
			row2 = doc->numLines() - 1;
			col2 = 0;
		}
	}

	// clear selection and make cursor position visible
	doc->clearSelection();
 	view->setCursorPositionReal(row1,col1);

	QString text;
	KTextEditor::EditInterfaceExt *iface = KTextEditor::editInterfaceExt( doc );
	if ( iface ) iface->editBegin();
	switch ( id )
	{
		case KileWidget::Structure::SectioningCut:
			QApplication::clipboard()->setText( doc->text(row1,col1,row2,col2) );  // copy -> clipboard
			doc->removeText(row1,col1,row2,col2);                                  // delete
			break;
		case KileWidget::Structure::SectioningCopy:
			QApplication::clipboard()->setText( doc->text(row1,col1,row2,col2) );  // copy -> clipboard
			break;
		case KileWidget::Structure::SectioningPaste: 
			text = QApplication::clipboard()->text();                              // clipboard -> text
			if ( ! text.isEmpty() )
			{
				view->setCursorPositionReal(row2,col2);                             // insert
				view->insertText(text + "\n");
			}
			break;
		case KileWidget::Structure::SectioningSelect:
			doc->setSelection(row1,col1,row2,col2);                                // select
			break;
		case KileWidget::Structure::SectioningDelete:
			doc->removeText(row1,col1,row2,col2);                                  // delete
			break;
		case KileWidget::Structure::SectioningComment:
			doc->setSelection(row1,col1,row2,col2);
			view->comment();
			doc->clearSelection();
			break;
		case KileWidget::Structure::SectioningPreview: 
			doc->setSelection(row1,col1,row2,col2);                               // quick preview
			m_ki->quickPreview()->previewSelection(doc,false);
			doc->clearSelection();
			break;
	}
	if ( iface ) iface->editEnd();

	// update structure view, because it has changed
	if ( id==KileWidget::Structure::SectioningDelete || id==KileWidget::Structure::SectioningComment )
		m_ki->viewManager()->updateStructure(true);

}

bool EditorExtension::findEndOfDocument(Kate::Document *doc, uint row, uint col, uint &rowFound, uint &colFound)
{
	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);	

	uint lenFound;
	QString textline;
	while ( iface->searchText(row,col,"\\end{document}",&rowFound,&colFound,&lenFound) )
	{
		textline = getTextLineReal(doc,rowFound);
		if ( textline.find("\\end{document}",colFound) == (int)colFound )
			return true;

		row = rowFound;
		col = colFound;
		if ( ! increaseCursorPosition(doc,row,col) )
			break;
	}

	return false;
}

}

#include "kileedit.moc"
