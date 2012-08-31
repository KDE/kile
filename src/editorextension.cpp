/***********************************************************************************************
  Copyright (C) 2004-2012 by Holger Danielsson (holger.danielsson@versanet.de)
                2008-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editorextension.h"

#include <QFileInfo>
#include <QClipboard>

#include <KApplication>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Range>
#include <KTextEditor/Cursor>
#include <KTextEditor/SearchInterface>
#include <KLocale>
#include <KStandardDirs>

#include "errorhandler.h"
#include "codecompletion.h"
#include "errorhandler.h"
#include "kile.h"
#include "kileactions.h"
#include "kileconfig.h"
#include "kileextensions.h"
#include "kileinfo.h"
#include "kiletool_enums.h"
#include "kileviewmanager.h"
#include "quickpreview.h"
#include "widgets/konsolewidget.h"

/*
 * FIXME: The code in this file should be reworked completely. Once we've got a better parser
 *        most of the code in here should also be superfluous.
 */

namespace KileDocument
{

EditorExtension::EditorExtension(KileInfo *info) : m_ki(info)
{
	m_latexCommands = m_ki->latexCommands();

	// init regexp
	m_reg.setPattern("(\\\\(begin|end)\\s*\\{([A-Za-z]+\\*?)\\})|(\\\\\\[|\\\\\\])");
	//                1    2                 3                   4
	m_regexpEnter.setPattern("^(.*)((\\\\begin\\s*\\{([^\\{\\}]*)\\})|(\\\\\\[))");
	//                         1   23                 4               5

	// init double quotes
	m_quoteListI18N // this is shown in the configuration dialog
		<< i18n("English quotes:   ``   &apos;&apos;")
		<< i18n("French quotes:   &quot;&lt;   &quot;&gt;")
		<< i18n("German quotes:   &quot;`   &quot;&apos;")
		<< i18n("French quotes (long):   \\flqq   \\frqq")
		<< i18n("German quotes (long):   \\glqq   \\grqq")
		<< i18n("Icelandic quotes (v1):   \\ilqq   \\irqq")
		<< i18n("Icelandic quotes (v2):   \\iflqq   \\ifrqq")
		<< i18n("Czech quotes:   \\uv{}")
		<< i18n("csquotes package:   \\enquote{}");


	m_quoteList
		<< QPair<QString, QString>("``", "''")
		<< QPair<QString, QString>("\"<", "\">")
		<< QPair<QString, QString>("\"`", "\"'")
		<< QPair<QString, QString>("\\flqq", "\\frqq")
		<< QPair<QString, QString>("\\glqq", "\\grqq")
		<< QPair<QString, QString>("\\ilqq", "\\irqq")
		<< QPair<QString, QString>("\\iflqq", "\\ifrqq")
		<< QPair<QString, QString>("\\uv{", "}")
		<< QPair<QString, QString>("\\enquote{", "}");

	readConfig();
}

EditorExtension::~EditorExtension()
{
}

//////////////////// read configuration ////////////////////

void EditorExtension::readConfig(void)
{
	// init insertion of double quotes
	initDoubleQuotes();

	// allow special chars?
	m_specialCharacters = KileConfig::insertSpecialCharacters();

	// calculate indent for autoindent of environments
	m_envAutoIndent.clear();
	if(KileConfig::envIndentation()) {
		if(KileConfig::envIndentSpaces()) {
			int num = KileConfig::envIndentNumSpaces();
			if(num < 1 || num > 9) {
				num = 1;
			}
			m_envAutoIndent.fill(' ', num);
		}
		else {
			m_envAutoIndent = "\t";
		}
	}
}

void EditorExtension::insertTag(const KileAction::TagData& data, KTextEditor::View *view)
{
	KTextEditor::Document *doc = view->document();
	if(!doc) {
		return;
	}

	//whether or not to wrap tag around selection
	bool wrap = !data.tagEnd.isNull() && view->selection();

	//%C before or after the selection
	bool before = data.tagBegin.count("%C");
	bool after = data.tagEnd.count("%C");

	//save current cursor position
	KTextEditor::Cursor cursor = view->cursorPosition();
	KTextEditor::Cursor virtualCursor = view->cursorPositionVirtual();
	int para = cursor.line();
	int para_begin = para;
	int index = cursor.column();
	int index_begin = index;
	int para_end = 0;
	int index_cursor = index;
	int para_cursor = index;
	// offset for autoindentation of environments
	int dxIndentEnv = 0;

	// environment tag
	bool envtag = data.tagBegin.count("%E") || data.tagEnd.count("%E");
	QString whitespace = getWhiteSpace( doc->line(para).left(index) );

	//if there is a selection act as if cursor is at the beginning of selection
	if (wrap) {
		KTextEditor::Range selectionRange = view->selectionRange();
		index = selectionRange.start().column();
		para  = selectionRange.start().line();
		para_end = selectionRange.end().line();
	}

	QString ins = data.tagBegin;
	QString tagEnd = data.tagEnd;

	//start an atomic editing sequence
	doc->startEditing();

	//cut the selected text
	QString trailing;
	if(wrap) {
		QString sel = view->selectionText();
		view->removeSelectionText();

		// no autoindentation of environments, when text is selected
		if(envtag) {
			ins.remove("%E");
			tagEnd.remove("%E\n");
		}

		// strip one of two consecutive line ends
		int len = sel.length();
		if(tagEnd.at(0)=='\n' && len > 0 && sel.indexOf('\n',-1) == len - 1) {
			sel.truncate( len-1 );
		}

		// now add the selection
		ins += sel;

		// place the cursor behind this tag, if there is no other wish
		if(!before && !after) {
			trailing = "%C";
			after = true;
		}
	}
	else if(envtag) {
		ins.replace("%E",whitespace+m_envAutoIndent);
		tagEnd.replace("%E",whitespace+m_envAutoIndent);
		if(data.dy > 0) {
			dxIndentEnv = whitespace.length() + m_envAutoIndent.length();
		}
	}

	tagEnd.replace("\\end{",whitespace+"\\end{");
	ins += tagEnd + trailing;

	//do some replacements
	QFileInfo fi( doc->url().toLocalFile());
	ins.replace("%S", fi.completeBaseName());
	ins.replace("%B", s_bullet);

	//insert first part of tag at cursor position
	doc->insertText(KTextEditor::Cursor(para, index), ins);

	//move cursor to the new position
	if(before || after) {
		int n = data.tagBegin.count("\n")+ data.tagEnd.count("\n");
		if(wrap) {
			n += para_end > para ? para_end-para : para-para_end;
		}
		for (int line = para_begin; line <= para_begin+n; ++line) {
			if(doc->line(line).count("%C")) {
				int i=doc->line(line).indexOf("%C");
				para_cursor = line; index_cursor = i;
				doc->removeText(KTextEditor::Range(line, i, line, i+2));
				break;
			}
			index_cursor=index;
			para_cursor=line;
		}
	}
	else {
		int py = para_begin, px = index_begin;
		if(wrap) { //act as if cursor was at beginning of selected text (which is the point where the tagBegin is inserted)
			py = para;
			px = index;
		}
		para_cursor = py+data.dy; index_cursor = px+data.dx+dxIndentEnv;
	}

	//end the atomic editing sequence
	doc->endEditing();

	//set the cursor position (it is important that this is done outside of the atomic editing sequence)
	view->setCursorPosition(KTextEditor::Cursor(para_cursor, index_cursor));

	view->removeSelection();
}

//////////////////// goto environment tag (begin or end) ////////////////////

// goto the next non-nested environment tag

KTextEditor::View* EditorExtension::determineView(KTextEditor::View *view)
{
	if (!view) {
		view = m_ki->viewManager()->currentTextView();
	}

	m_overwritemode = (!view) ? false : (view->viewEditMode() == KTextEditor::View::EditOverwrite);

	return view;
}

void EditorExtension::gotoEnvironment(bool backwards, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	uint row,col;
	EnvData env;
	bool found;

	// get current position
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();


	// start searching
	if(backwards) {
		found = findBeginEnvironment(doc,row,col,env);
		//KILE_DEBUG() << "   goto begin env:  " << env.row << "/" << env.col;

	}
	else {
		found = findEndEnvironment(doc,row,col,env);
		env.col += env.len;
	}

	if(found) {
		view->setCursorPosition(KTextEditor::Cursor(env.row, env.col));
	}
}

// match the opposite environment tag

void EditorExtension::matchEnvironment(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	uint row,col;
	EnvData env;

	// get current position
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	// we only start, when we are at an environment tag
	if(!isEnvironmentPosition(doc, row, col, env)) {
		return;
	}

	gotoEnvironment(env.tag != EnvBegin, view);
}

//////////////////// close opened environments  ////////////////////

// search for the last opened environment and close it

void EditorExtension::closeEnvironment(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int row, col, currentRow, currentCol;
	QString name;

	KTextEditor::Cursor cursor = view->cursorPosition();
	currentRow = cursor.line();
	currentCol = cursor.column();

	if(findOpenedEnvironment(row, col, name, view)) {
		if(name == "\\[") {
			view->document()->insertText(KTextEditor::Cursor(currentRow, currentCol), "\\]");
		}
		else {
			view->document()->insertText(KTextEditor::Cursor(currentRow, currentCol), "\\end{" + name + '}');
		}
// 		view->setCursorPosition(KTextEditor::Cursor(row + 1, 0));
	}
}

// close all opened environments

void EditorExtension::closeAllEnvironments(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	QStringList envlist = findOpenedEnvironmentList(view, true);
	if(envlist.count() == 0) {
		return;
	}

	int currentRow, currentCol, outputCol;
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	currentRow = cursor.line();
	currentCol = cursor.column();

	bool indent = !m_envAutoIndent.isEmpty();
	if(indent && currentCol > 0) {
		doc->insertText(KTextEditor::Cursor(currentRow, currentCol),"\n");
		currentRow++;
		currentCol = 0;
	}

	bool ok1,ok2;
	for(QStringList::Iterator it = envlist.begin(); it != envlist.end(); ++it) {
		QStringList entry = (*it).split(',');
		if(entry[0] == "document") {
			break;
		}

		int row = entry[1].toInt(&ok1);
		int col = entry[2].toInt(&ok2);
		if(!ok1 || !ok2) {
			continue;
		}

		outputCol = currentCol;
		if(indent) {
			QString whitespace = getWhiteSpace( doc->line(row).left(col) );
			doc->insertText(KTextEditor::Cursor(currentRow, outputCol), whitespace);
			outputCol += whitespace.length();
		}
		QString endtag = ( entry[0] == "\\[" ) ? "\\]\n" : "\\end{"+entry[0]+"}\n";
		doc->insertText(KTextEditor::Cursor(currentRow, outputCol), endtag);
		++currentRow;
	}
}

//////////////////// mathgroup ////////////////////

void EditorExtension::selectMathgroup(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = mathgroupRange(view);
	if(range.isValid()) {
		view->setSelection(range);
	}
}

void EditorExtension::deleteMathgroup(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = mathgroupRange(view);
	if(range.isValid()) {
		deleteRange(range,view);
	}
}

bool EditorExtension::hasMathgroup(KTextEditor::View *view)
{
	// view will be checked in mathgroupRange()
	KTextEditor::Range range = mathgroupRange(view);
	return (range.isValid()) ? true : false;
}

QString EditorExtension::getMathgroupText(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return QString();
	}

	KTextEditor::Range range = mathgroupRange(view);
	return (range.isValid()) ? view->document()->text(range) : QString();
}

QString EditorExtension::getMathgroupText(uint &row, uint &col, KTextEditor::View *view)
{
	int row1, col1, row2, col2;

	view = determineView(view);
	if(view && getMathgroup(view, row1, col1, row2, col2)) {
		row = row1;
		col = col1;
		return view->document()->text(KTextEditor::Range(row1, col1, row2, col2));
	}
	else {
		return QString();
	}
}

KTextEditor::Range  EditorExtension::mathgroupRange(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return KTextEditor::Range::invalid();
	}

	int row1, col1, row2, col2;
	if(getMathgroup(view, row1, col1, row2, col2)) {
		return KTextEditor::Range(row1, col1, row2, col2);
	}
	else {
		return KTextEditor::Range::invalid();
	}
}

bool EditorExtension::getMathgroup(KTextEditor::View *view, int &row1, int &col1, int &row2, int &col2)
{
	int row, col, r, c;
	MathData begin, end;

	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	QString textline = getTextLineReal(doc,row);

	// check for '\ensuremath{...}'
	QString word;
	int x1, x2;
	if(getCurrentWord(doc, row, col, smTex, word, x1, x2) && word == "\\ensuremath") {
		view->setCursorPosition(KTextEditor::Cursor(row, x2));
	}

	BracketData open,close;
	if(getTexgroup(false, open, close, view)) {
		QString s = getTextLineReal(doc,open.row);
		if(open.col >= 11 && s.mid(open.col - 11, 11) == "\\ensuremath") {
			view->setCursorPosition(KTextEditor::Cursor(row, col));
			row1 = open.row;
			col1 = open.col-11;
			row2 = close.row;
			col2 = close.col;
			return true;
		}
	}

	// do we need to restore the cursor position
	view->setCursorPosition(KTextEditor::Cursor(row, col));

	// '$' is difficult, because it is used as opening and closing tag
	int mode = 0;
	if(textline[col] == '$') {
		mode = 1;
	}
	else if(col > 0 && textline[col - 1] == '$') {
		mode = 2;
	}

	if(mode > 0) {
		// first look, if this is a closing '$'
		r = row;
		c = (mode == 1) ? col : col - 1;
		if(decreaseCursorPosition(doc, r, c) && findOpenMathTag(doc, r, c, begin)) {
			if(begin.tag == mmMathDollar && (begin.numdollar & 1)) {
				row1 = begin.row;
				col1 = begin.col;
				row2 = row;
				col2 = (mode == 1) ? col + 1 : col;
				return true;
			}
		}

		// perhaps an opening '$'
		r = row;
		c = (mode == 1) ? col+1 : col;
		if(findCloseMathTag(doc, r, c, end)) {
			if(end.tag == mmMathDollar) {
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
	bool openingtag = isOpeningMathTagPosition(doc, row, col, begin);
	if(openingtag) {
		// try to find the corresponding closing tag at the right
		bool closetag = findCloseMathTag(doc, row, col + 1, end);
		if(closetag && checkMathtags(begin, end)) {
			row1 = begin.row;
			col1 = begin.col;
			row2 = end.row;
			col2 = end.col + end.len;
			return true;
		}
	}

	r = row;
	c = col;
	bool closingtag = isClosingMathTagPosition(doc, row, col, end);
	if(closingtag) {
		c = end.col;
		if(!decreaseCursorPosition(doc, r, c)) {
			return false;
		}
	}

	// now try to search to opening tag of the math group
	if(!findOpenMathTag(doc, r, c, begin)) {
		return false;
	}

	if(begin.tag == mmMathDollar && !(begin.numdollar & 1)) {
		//KILE_DEBUG() << "error: even number of '$' --> no math mode" ;
		return false;
	}

	// and now the closing tag
	if(!findCloseMathTag(doc, r, c, end)) {
		return false;
	}

	// both tags were found, but they must be of the same type
	if(checkMathtags(begin, end)) {
		row1 = begin.row;
		col1 = begin.col;
		row2 = end.row;
		col2 = end.col + end.len;
		return true;
	}
	else {
		return false;
	}
}

//////////////////// mathgroup tags ////////////////////

bool EditorExtension::checkMathtags(const MathData &begin,const MathData &end)
{
	// both tags were found, but they must be of the same type
	if(begin.tag != end.tag) {
		//KILE_DEBUG() << "error: opening and closing tag of mathmode don't match: " << begin.tag << " - " << end.tag;
		return false;
	}

	// and additionally: if it is a math env, both tags must have the same name
	if(begin.tag == mmDisplaymathEnv && begin.envname != end.envname) {
		//KILE_DEBUG() << "error: opening and closing env tags have different names: " << begin.envname << " - " << end.envname;
		return false;
	}

	return true;
}

bool EditorExtension::isOpeningMathTagPosition(KTextEditor::Document *doc, uint row, uint col, MathData &mathdata)
{
	QString textline = getTextLineReal(doc,row);

	QRegExp reg("\\\\begin\\s*\\{([A-Za-z]+\\*?)\\}|\\\\\\[|\\\\\\(");
	if((int)col != reg.indexIn(textline, col)) {
		return false;
	}

	QChar id = reg.cap(0)[1];
	QString envname = reg.cap(1);

	mathdata.row = row;
	mathdata.col = col;
	mathdata.len = reg.cap(0).length();

	switch(id.toAscii()) {
		case 'b':
			if(!(m_latexCommands->isMathEnv(envname) || envname=="math") || m_latexCommands->needsMathMode(envname)) {
				return false;
			}
			mathdata.tag = ( envname=="math" ) ? mmMathEnv : mmDisplaymathEnv;
			mathdata.envname = envname;
		break;
		case '[':
			mathdata.tag = mmDisplaymathParen;
		break;
		case '(':
			mathdata.tag = mmMathParen;
		break;
	}

	return true;
}

bool EditorExtension::isClosingMathTagPosition(KTextEditor::Document *doc, uint row, uint col,MathData &mathdata)
{
	QString textline = doc->line(row);

	QRegExp reg("\\\\end\\s*\\{([A-Za-z]+\\*?)\\}|\\\\\\]|\\\\\\)");
	int pos = reg.lastIndexIn(textline, col);
	if(pos < 0 || (int)col > pos + reg.matchedLength()) {
		return false;
	}

	QChar id = reg.cap(0)[1];
	QString envname = reg.cap(1);

	mathdata.row = row;
	mathdata.col = pos;
	mathdata.len = reg.cap(0).length();

	switch(id.toAscii()) {
		case 'e':
			if(!(m_latexCommands->isMathEnv(envname) || envname=="math") || m_latexCommands->needsMathMode(envname)) {
				return false;
			}
			mathdata.tag = ( envname=="math" ) ? mmMathEnv : mmDisplaymathEnv;
			mathdata.envname = envname;
		break;
		case ']':
			mathdata.tag = mmDisplaymathParen;
		break;
		case ')':
			mathdata.tag = mmMathParen;
		break;
	}

	return true;
}

bool EditorExtension::findOpenMathTag(KTextEditor::Document *doc, int row, int col, MathData &mathdata)
{
	const QString regExpString = "\\$"
		 "|\\\\begin\\s*\\{([A-Za-z]+\\*?)\\}"
		 "|\\\\end\\s*\\{([A-Za-z]+\\*?)\\}"
		 "|\\\\\\[|\\\\\\]"
		 "|\\\\\\(|\\\\\\)";

	QRegExp reg(regExpString);
	int lastrow = -1, lastcol = -1;
	QString mathname;

	bool foundDollar = false;
	uint numDollar = 0;

	QString textline = getTextLineReal(doc, row);
	int column = col;

	bool continueSearch = true;
	while(continueSearch) {
		while((column = reg.lastIndexIn(textline, col)) != -1) {
			col = column;

			mathdata.row = row;
			mathdata.col = col;
			mathdata.len = reg.cap(0).length();
			mathname = reg.cap(0).left(2);

			// should be better called 'isValidChar()', because it checks for comments
			// and escaped chars like backslash and dollar in '\\' and '\$'
			if(mathname == "$") {
				// count and continue search
				++numDollar;

				// but remember the first dollar found backwards
				if(!foundDollar) {
					lastrow = row;
					lastcol = col;
					foundDollar = true;
				}
			}
			else if(mathname=="\\[" || mathname=="\\(") {
				// found start of mathmode
				if(numDollar == 0) {
					mathdata.tag = ( mathname == "\\[" ) ? mmDisplaymathParen : mmMathParen;
					mathdata.numdollar = 0;
					return true;
				}
				else {
					//KILE_DEBUG() << "error: dollar not allowed in \\[ or \\( mode";
					return false;
				}
			}
			else if(mathname=="\\]" || mathname=="\\)") {
				continueSearch = false;
				break;
			}
			else  if(mathname=="\\b") {
				// save name of environment
				QString envname = reg.cap(1);

				// if we found the opening tag of a math env
				if(m_latexCommands->isMathEnv(envname) || envname=="math") {
					if(numDollar > 0) {
						//KILE_DEBUG() << "error: dollar not allowed in math env   numdollar=" << numDollar;
						return false;
					}

					// if this is a math env with its own mathmode, we have found the starting position
					if(envname == "math") {
						mathdata.tag = mmMathEnv;
						mathdata.envname = envname;
						return true;
					}

					if(!m_latexCommands->needsMathMode(envname)) {
						mathdata.tag = mmDisplaymathEnv;
						mathdata.envname = envname;
						return true;
					}
				}
				// no math env, we found the opening tag of a normal env
				else {
					continueSearch = false;
					break;
				}
			}
			else if(mathname == "\\e") {
				QString envname = reg.cap(2);

				// if we found the closing tag of a math env
				if(m_latexCommands->isMathEnv(envname) || envname == "math") {
					// if this is a math env with its own mathmode
					if(!m_latexCommands->needsMathMode(envname) || envname == "math") {
						continueSearch = false;
						break;
					}

					// if this is a math env which needs $..$
					if(m_latexCommands->isMathModeEnv(envname)) {
						if(numDollar >= 1) {
							--numDollar;
							continueSearch = false;
							break;
						}
						// else continue search
					}
				}
				// if we found the closing tag of a normal env
				else {
					continueSearch = false;
					break;
				}
			}
			else {
				//KILE_DEBUG() << "error: unknown match";
				return false;
			}

			// continue search one column left of the last match (if this is possible)
			if(col == 0) {
				break;
			}

			--col;
		}

		if(row > 0) {
	 		textline = getTextLineReal(doc,--row);
			col = textline.length();
		}
		else if(column == -1) {
			continueSearch = false;
			break;
		}
	}

	// nothing else found, so math mode starts a the last dollar (the first one found backwards)
	mathdata.row = lastrow;
	mathdata.col = lastcol;
	mathdata.len = 1;
	mathdata.numdollar = numDollar;

	mathdata.tag = (numDollar > 0) ? mmMathDollar : mmNoMathMode;

	return true;
}

bool EditorExtension::findCloseMathTag(KTextEditor::Document *doc, int row, int col, MathData &mathdata)
{
	const QString regExpString = "\\$"
		 "|\\\\begin\\s*\\{([A-Za-z]+\\*?)\\}"
		 "|\\\\end\\s*\\{([A-Za-z]+\\*?)\\}"
		 "|\\\\\\[|\\\\\\]"
		 "|\\\\\\(|\\\\\\)";

	KTextEditor::SearchInterface *iface = dynamic_cast<KTextEditor::SearchInterface*>(doc);
	if(!iface) {
		return false;
	}

	int rowFound, colFound;
	QRegExp reg(regExpString);

	KTextEditor::Range searchRange = KTextEditor::Range(KTextEditor::Cursor(row, col), doc->documentEnd());

	while(true) {
		QVector<KTextEditor::Range> foundRanges = iface->searchText(searchRange, regExpString, KTextEditor::Search::Regex | KTextEditor::Search::CaseInsensitive);
		if(foundRanges.isEmpty() || (foundRanges.size() == 1 && !foundRanges.first().isValid())) {
			break;
		}

		//KILE_DEBUG() << "number of ranges " << foundRanges.count();
		if(foundRanges.size() < 3) {
			break;
		}

		KTextEditor::Range range = foundRanges.first();
		//KILE_DEBUG() << "found math tag: " << doc->text(range);
		if(!range.isValid()) {
			break;
		}

		rowFound = range.start().line();
		colFound = range.start().column();
		QString textFound = doc->text(range);

		// should be better called 'isValidChar()', because it checks for comments
		// and escaped chars like backslash and dollar in '\\' and '\$'
		if(isValidBackslash(doc, rowFound, colFound)) {
			QString mathname = textFound.left(2);

			// always remember behind the last match
			mathdata.row = rowFound;
			mathdata.col = colFound;
			mathdata.len = textFound.length();

			if(mathname=="$") {
				mathdata.tag = mmMathDollar;
				return true;
			}
			else if(mathname=="\\]") {
				mathdata.tag = mmDisplaymathParen;
				return true;
			}
			else if(mathname=="\\)") {
				mathdata.tag = mmMathParen;
				return true;
			}
			else if(mathname=="\\[" || mathname=="\\(") {
				//KILE_DEBUG() << "error: current mathgroup was not closed";
				return false;
			}
			else if(mathname=="\\b") {
				QString envname = doc->text(foundRanges[1]);
				if(!(m_latexCommands->isMathEnv(envname) || envname=="math")) {
					//KILE_DEBUG() << "error: only math env are allowed in mathmode (found begin tag)";
					return false;
				}

				if(!m_latexCommands->needsMathMode(envname) || envname=="math") {
					//KILE_DEBUG() << "error: mathenv with its own mathmode are not allowed in mathmode ";
					return false;
				}

				// else continue search
			}
			else if(mathname == "\\e") {
				QString envname = doc->text(foundRanges[2]);
				if(!(m_latexCommands->isMathEnv(envname) || envname=="math")) {
					//KILE_DEBUG() << "error: only math env are allowed in mathmode (found end tag)";
					return false;
				}

				if(envname == "math") {
					mathdata.tag = mmMathEnv;
					mathdata.envname = envname;
					return true;
				}

				if(!m_latexCommands->needsMathMode(envname)) {
					mathdata.tag = mmDisplaymathEnv;
					mathdata.envname = envname;
					return true;
				}

				// else continue search
			}
		}

		// go ahead
		searchRange = KTextEditor::Range(foundRanges.first().end(), doc->documentEnd());
	}

	//KILE_DEBUG() << "not found anything";
	return false;
}



//////////////////// insert newlines inside an environment ////////////////////

// intelligent newlines: look for the last opened environment
// and decide what to insert
// or continue the comment

void EditorExtension::insertIntelligentNewline(KTextEditor::View *view)
{
	KILE_DEBUG() << view;

	view = determineView(view);

	if(!view) {
		return;
	}

	KTextEditor::Document* doc = view->document();

	if(!doc) {
		return;
	}

	QString name;
	KTextEditor::Cursor cursor = view->cursorPosition();
	int row = cursor.line();
	int col = cursor.column();

	QString newLineAndIndentationString = '\n' + extractIndentationString(view, row);

	if(isCommentPosition(doc, row, col)) {
		KILE_DEBUG() << "found comment";
		view->insertText(newLineAndIndentationString + "% ");
		moveCursorToLastPositionInCurrentLine(view);
		return;
	}
	else if(findOpenedEnvironment(row, col, name, view)) {
		if(m_latexCommands->isListEnv(name)) {
			if ( name == "description" ) {
				view->insertText(newLineAndIndentationString + "\\item[]");
			}
			else {
				view->insertText(newLineAndIndentationString + "\\item ");
			}
			moveCursorToLastPositionInCurrentLine(view);
			return;
		}
		else if(m_latexCommands->isTabularEnv(name) || m_latexCommands->isMathEnv(name)) {
			view->insertText(newLineAndIndentationString + "\\\\");
			moveCursorToLastPositionInCurrentLine(view);
			return;
		}
	}
	// - no comment position
	// - found no opened environment
	// - unknown environment
	// - finish tabular or math environment
	view->insertText(newLineAndIndentationString);
	moveCursorToLastPositionInCurrentLine(view);
}

bool EditorExtension::findOpenedEnvironment(int &row, int &col, QString &envname, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	// get current cursor position
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	EnvData env;
	int startrow = row;
	int startcol = col;

	//KILE_DEBUG() << "   close - start ";
	// accept a starting place outside an environment
	bool env_position = isEnvironmentPosition(doc, row, col, env);

	// We can also accept a column, if we are on the left side of an environment.
	// But we should decrease the current cursor position for the search.
	if(env_position && env.cpos != EnvInside) {
		if(env.cpos == EnvLeft && !decreaseCursorPosition(doc, startrow, startcol)) {
			return false;
		}
		env_position = false;
	}

	if(!env_position && findEnvironmentTag(doc, startrow, startcol, env, true)) {
		//KILE_DEBUG() << "   close - found begin env at:  " << env.row << "/" << env.col << " " << env.name;
		row = env.row;
		col = env.col;
		envname = env.name;
		return true;
	}
	else {
		return false;
	}
}

QStringList EditorExtension::findOpenedEnvironmentList(KTextEditor::View *view, bool position)
{
	QStringList envlist;

	view = determineView(view);
	if(view) {
		int currentRow, currentCol;
		KTextEditor::Document *doc = view->document();
		KTextEditor::Cursor cursor = view->cursorPosition();
		currentRow = cursor.line();
		currentCol = cursor.column();


		int row = currentRow;
		int col = currentCol;
		EnvData env;

		// check the starting position
		bool env_position = isEnvironmentPosition(doc, row, col, env);
		if(env_position) {
			// we are inside an environment tag: bad to complete
			if(env.cpos == EnvInside) {
				return envlist;
			}
			// we are left of an environment tag: go one position to the left
			if(env.cpos == EnvLeft) {
				if (!decreaseCursorPosition(doc, row, col)) {
					return envlist;
				}
			}
		}

		while (findEnvironmentTag(doc, row, col, env, true)) {
			row = env.row;
			col = env.col;

			if(position) {
				envlist << env.name + QString(",%1,%2").arg(row).arg(col);
			}
			else {
				envlist << env.name;
			}

			if(col == 0) {
				if (!decreaseCursorPosition(doc, row, col)) {
					break;
				}
			}
			view->setCursorPosition(KTextEditor::Cursor(row, col));
		}

		// reset cursor original position
		view->setCursorPosition(KTextEditor::Cursor(currentRow, currentCol));
	}

	return envlist;
}

//////////////////// select an environment  ////////////////////

void EditorExtension::selectEnvironment(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	if (!view->selection() || !expandSelectionEnvironment(inside,view)) {
		KTextEditor::Range range = environmentRange(inside,view);
		if(range.isValid()) {
			view->setSelection(range);
		}
	}
}

void EditorExtension::deleteEnvironment(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = environmentRange(inside,view);
	if(range.isValid()) {
		deleteRange(range,view);
	}
}

void EditorExtension::deleteRange(KTextEditor::Range &range, KTextEditor::View *view)
{
	view->removeSelection();
	view->document()->removeText(range);
	view->setCursorPosition(range.start());
}

// calculate start and end of an environment

bool EditorExtension::getEnvironment(bool inside, EnvData &envbegin, EnvData &envend, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	int row, col;

	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();
	if(!findBeginEnvironment(doc, row, col, envbegin)) {
		return false;
	}
	if(!findEndEnvironment(doc, row, col, envend)) {
		return false;
	}

	if(inside) {
		envbegin.col += envbegin.len;
	}
	else {
		envend.col += envend.len;
	}

	return true;
}

KTextEditor::Range EditorExtension::environmentRange(bool inside, KTextEditor::View *view)
{
	// view will be checked in getEnvironment()
	EnvData envbegin, envend;
	return (getEnvironment(inside, envbegin, envend, view))
	         ? KTextEditor::Range(envbegin.row, envbegin.col, envend.row, envend.col)
	         : KTextEditor::Range::invalid();
}

QString EditorExtension::environmentText(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return QString();
	}

	KTextEditor::Range range = environmentRange(inside,view);
	return (range.isValid()) ? view->document()->text(range) : QString();
}

QString EditorExtension::environmentName(KTextEditor::View *view)
{
	// view will be checked in getEnvironment()
	EnvData envbegin, envend;
	return (getEnvironment(false, envbegin, envend, view)) ? envbegin.name : QString();
}

// determine text, startrow and startcol of current environment

QString EditorExtension::getEnvironmentText(int &row, int &col, QString &name, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return QString();
	}

	EnvData envbegin, envend;

	if(getEnvironment(false, envbegin, envend, view) && envbegin.name != "document") {
		row = envbegin.row;
		col = envbegin.col;
		name = envbegin.name;
		return view->document()->text(KTextEditor::Range(envbegin.row, envbegin.col, envend.row, envend.col));
	}
	else {
		return QString();
	}
}

bool EditorExtension::hasEnvironment(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	EnvData envbegin,envend;
	return (getEnvironment(false, envbegin, envend, view) && envbegin.name != "document");
}

// when an environment is selected (inside or outside),
// the selection is expanded to the surrounding environment

bool EditorExtension::expandSelectionEnvironment(bool inside, KTextEditor::View *view)
{
	KTextEditor::Document *doc = view->document();
	if (!view->selection()) {
		return false;
	}

	// get current position
	int row, col;
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	// get current selection
	KTextEditor::Range selectionRange = view->selectionRange();
	int row1 = selectionRange.start().line();
	int col1 = selectionRange.start().column();
	int row2 = selectionRange.end().line();
	int col2 = selectionRange.end().column();

	// determine current environment outside
	EnvData oenvbegin,oenvend;
	if(!getEnvironment(false, oenvbegin, oenvend, view)) {
		return false;
	}

	bool newselection = false;
	// first look, if this environment is selected outside
	if(row1 == oenvbegin.row && col1 == oenvbegin.col && row2 == oenvend.row && col2 == oenvend.col) {

		if(!decreaseCursorPosition(doc, oenvbegin.row, oenvbegin.col) ) {
			return newselection;
		}
		view->setCursorPosition(KTextEditor::Cursor(oenvbegin.row, oenvbegin.col));
		// search the surrounding environment and select it
		if(getEnvironment(inside, oenvbegin, oenvend, view)) {
			view->setSelection(KTextEditor::Range(oenvbegin.row, oenvbegin.col, oenvend.row, oenvend.col));
			newselection = true;

		}
	}
	else {
	// then determine current environment inside
		EnvData ienvbegin, ienvend;
		getEnvironment(true, ienvbegin, ienvend, view);
		// and look, if this environment is selected inside
		if(row1 == ienvbegin.row && col1 == ienvbegin.col && row2 == ienvend.row && col2 == ienvend.col) {
			if(!decreaseCursorPosition(doc, oenvbegin.row, oenvbegin.col) ) {
				return newselection;
			}
			view->setCursorPosition(KTextEditor::Cursor(oenvbegin.row, oenvbegin.col));
			// search the surrounding environment and select it
			if(getEnvironment(inside, ienvbegin, ienvend, view)) {
				view->setSelection(KTextEditor::Range(ienvbegin.row, ienvbegin.col, ienvend.row, ienvend.col));
				newselection = true;
			}

		}
	}

	// restore old cursor position
	view->setCursorPosition(KTextEditor::Cursor(row, col));
	return newselection;
}

//////////////////// search for \begin{env}  ////////////////////

// Find the last \begin{env} tag. If the current cursor is over
//  - \begin{env} tag: we will stop immediately
//  - \end{env} tag: we will start before this tag

bool EditorExtension::findBeginEnvironment(KTextEditor::Document *doc, int row, int col, EnvData &env)
{
	// KILE_DEBUG() << "   find begin:  ";
	if(isEnvironmentPosition(doc, row, col, env)) {
		// already found position?
		//KILE_DEBUG() << "   found env at:  " << env.row << "/" << env.col << " " << env.name;
		if(env.tag == EnvBegin) {
		//KILE_DEBUG() << "   is begin env at:  " << env.row << "/" << env.col << " " << env.name;
			return true;
		}

		// go one position back
		//KILE_DEBUG() << "   is end env at:  " << env.row << "/" << env.col << " " << env.name;
		row = env.row;
		col = env.col;
		if(!decreaseCursorPosition(doc, row, col)) {
			return false;
		}
	}

	// looking back for last environment
	//KILE_DEBUG() << "   looking back from pos:  " << row << "/" << col << " " << env.name;
	return findEnvironmentTag(doc, row, col, env, true);
}

//////////////////// search for \end{env}  ////////////////////

// Find the last \end{env} tag. If the current cursor is over
//  - \end{env} tag: we will stop immediately
//  - \begin{env} tag: we will start behind this tag

bool EditorExtension::findEndEnvironment(KTextEditor::Document *doc, int row, int col, EnvData &env)
{
	if(isEnvironmentPosition(doc, row, col, env)) {
		// already found position?
		if(env.tag == EnvEnd ) {
			return true;
		}

		// go one position forward
		row = env.row;
		col = env.col + 1;
	}

	// looking forward for the next environment
	return findEnvironmentTag(doc, row, col, env, false);
}

//////////////////// search for an environment tag ////////////////////
// find the last/next non-nested environment tag
bool EditorExtension::findEnvironmentTag(KTextEditor::Document *doc, int row, int col, EnvData &env, bool backwards)
{
	KTextEditor::SearchInterface *iface = dynamic_cast<KTextEditor::SearchInterface*>(doc);
	if(!iface) {
		return false;
	}

	unsigned int envcount = 0;

	KTextEditor::Range searchRange;
	if(backwards) {
		searchRange = KTextEditor::Range(KTextEditor::Cursor(0, 0), KTextEditor::Cursor(row, col));
	}
	else {
		searchRange = KTextEditor::Range(KTextEditor::Cursor(row, col), doc->documentEnd());
	}

	KTextEditor::Search::SearchOptions searchOptions = (backwards) ? KTextEditor::Search::Regex | KTextEditor::Search::Backwards : KTextEditor::Search::Regex;

	while(true) {
		QVector<KTextEditor::Range> foundRanges = iface->searchText(searchRange, m_reg.pattern(), searchOptions);
		if(foundRanges.isEmpty() || (foundRanges.size() == 1 && !foundRanges.first().isValid())) {
			break;
		}

		//KILE_DEBUG() << "number of ranges " << foundRanges.count();

		EnvTag wrong_env = (backwards) ? EnvEnd : EnvBegin;

		if(foundRanges.size() < 5) {
			break;
		}

		KTextEditor::Range range = foundRanges.first();

		if(!range.isValid()) {
			//KILE_DEBUG() << "invalid range found";
			break;
		}
		env.row = range.start().line();
		env.col = range.start().column();
		env.len = doc->text(range).length();

		if(isValidBackslash(doc, env.row, env.col)) {
			// index 0 is the fullmatch, 1 first cap and so on
			QString cap2 = (foundRanges[2].isValid() ? doc->text(foundRanges[2]) : "");
			QString cap3 = (foundRanges[3].isValid() ? doc->text(foundRanges[3]) : "");
			QString cap4 = (foundRanges[4].isValid() ? doc->text(foundRanges[4]) : "");
			EnvTag found_env = (cap2 == "begin" || cap4 == "\\[") ? EnvBegin : EnvEnd;
			if(found_env == wrong_env) {
				++envcount;
			}
			else {
				if(envcount > 0) {
					--envcount;
				}
				else {
					if(found_env == EnvBegin) {
						env.name = (cap2 == "begin") ? cap3 : "\\[";
					}
					else {
						env.name = (cap2 == "end") ? cap3 : "\\]";
					}
					env.tag = found_env;
					//KILE_DEBUG() << "found " << env.name;
					return true;
				}
			}
		}

		// finally, prepare the range for the next search
		if(backwards) {
			searchRange = KTextEditor::Range(KTextEditor::Cursor(0, 0), foundRanges.first().start());
		}
		else {
			searchRange = KTextEditor::Range(foundRanges.first().end(), doc->documentEnd());
		}
	}

	//KILE_DEBUG() << "not found anything";
	return false;
}

//////////////////// check for an environment position ////////////////////

// Check if the current position belongs to an environment. The result is set
// to the beginning backslash of the environment tag. The same algorithms as
// matching brackets is used.
//
// insert mode:    if there is a full tag on the left, always take it
//                 if not, look to the right
// overwrite mode: always take the tag, which begins at the cursor position
//
// test it with {a}{{b}}{c}

bool EditorExtension::isEnvironmentPosition(KTextEditor::Document *doc, int row, int col, EnvData &env)
{
	// get real textline without comments, quoted characters and pairs of backslashes
	QString textline = getTextLineReal(doc, row);

	if(col > textline.length()) {
		return false;
	}

	bool left = false;

   //KILE_DEBUG() << "col=" << col;
	//KTextEditor::SearchInterface *iface;
	//iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);

	// check if there is a match in this line from the current position to the left
	int startcol = (textline[col] == '\\') ? col - 1 : col;
	if(startcol >= 1) {
		//KILE_DEBUG() << "search to the left ";
		int pos = textline.lastIndexOf(m_reg, startcol);
		env.len = m_reg.matchedLength();
		if(pos != -1 && pos < col && col <= pos + env.len) {
		   //KILE_DEBUG() << "search to the left: found";
			env.row = row;
			env.col = pos;
			QChar ch = textline.at(pos + 1);
			if(ch=='b' || ch=='e') {
				env.tag = (ch == 'b') ? EnvBegin : EnvEnd;
				env.name = m_reg.cap(3);
			}
			else {
				env.tag = (ch == '[') ? EnvBegin : EnvEnd;
				env.name = m_reg.cap(4);
			}

			if ( !m_overwritemode || (m_overwritemode && col<pos+env.len) ) {
				// insert mode:    position is inside the tag or behind the tag, which also belongs to the tag
				// overwrit emode: position is inside the tag) {
				//KILE_DEBUG() << "search to the left: stop";
				return true;
			}
			// overwritemode: position is behind the tag
			left = true;
			//KILE_DEBUG() << "search to the left: left=true, but also look to the right";
		}
	}

	// check if there is a match in this line from the current position to the right
	//KILE_DEBUG() << "search to the right " ;
	if (textline[col] == '\\' && col == textline.indexOf(m_reg, col)) {
		//KILE_DEBUG() << "search to the right: found";
		env.row = row;
		env.col = col;
		env.len = m_reg.matchedLength();
		QChar ch = textline.at(col+1);
		if(ch == 'b' || ch == 'e') { // found "\begin" or "\end"
			env.tag = ( ch == 'b' ) ? EnvBegin : EnvEnd;
			env.name = m_reg.cap(3);
		}
		else { // found "\[" or "\\]"
			env.tag = (ch == '[') ? EnvBegin : EnvEnd;
			env.name = m_reg.cap(4);
		}
		//KILE_DEBUG() << "search to the right: stop";
		return true;
	}

	return left;
}

//////////////////// check for a comment ////////////////////

// check if the current position is within a comment

bool EditorExtension::isCommentPosition(KTextEditor::Document *doc, int row, int col)
{
	QString textline = doc->line(row);

	bool backslash = false;
	for(int i = 0; i < col; ++i) {
		if(textline[i] == '%') {
			if(!backslash) { // found a comment sign
				return true;
			}
			else {
				backslash = false;
			}
		}
		else if(textline[i] == '\\') { // count number of backslashes
			backslash = !backslash;
		}
		else {
			backslash = false;               // no backslash
		}
	}

	return false;
}

// check if the character at text[col] is a valid backslash:
//  - there is no comment sign in this line before
//  - there is not a odd number of backslashes directly before

bool EditorExtension::isValidBackslash(KTextEditor::Document *doc, int row, int col)
{
	QString textline = doc->line(row);

	bool backslash = false;
	for(int i = 0; i < col; ++i) {
		if(textline[i] == '%') {
			if(!backslash) {
				return false;                 // found a comment sign
			}
			else {
				backslash = false;
			}
		}
		else if(textline[i] == '\\') {  // count number of backslashes
			backslash = !backslash;
		}
		else {
			backslash = false;               // no backslash
		}
	}

	return !backslash;
}

//////////////////// goto next bullet ////////////////////

void EditorExtension::gotoBullet(bool backwards, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int row, col;

	// get current position
	KTextEditor::Document *doc = view->document();

	KTextEditor::SearchInterface *iface = dynamic_cast<KTextEditor::SearchInterface*>(doc);
	if(!iface) {
		return;
	}

	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	KTextEditor::Search::SearchOptions searchOptions = (backwards) ? KTextEditor::Search::Backwards : KTextEditor::Search::Default;

	KTextEditor::Range searchRange;
	if(backwards) {
		searchRange = KTextEditor::Range(KTextEditor::Cursor(0, 0), KTextEditor::Cursor(row, col));
	}
	else {
		searchRange = KTextEditor::Range(KTextEditor::Cursor(row, col), doc->documentEnd());
	}

	QVector<KTextEditor::Range> foundRanges = iface->searchText(searchRange, s_bullet, searchOptions);
	if(foundRanges.size() >= 1) {
		KTextEditor::Range range = foundRanges.first();
		if(range.isValid()) {
			int line = range.start().line();
			int column = range.start().column();
			view->setCursorPosition(KTextEditor::Cursor(line, column));
			view->setSelection(KTextEditor::Range(line, column, line, column + 1));
		}
	}
}

//////////////////// increase/decrease cursor position ////////////////////

bool EditorExtension::moveCursorRight(KTextEditor::View *view)
{
	return moveCursor(view, MoveCursorRight);
}

bool EditorExtension::moveCursorLeft(KTextEditor::View *view)
{
	return moveCursor(view, MoveCursorLeft);
}

bool EditorExtension::moveCursorUp(KTextEditor::View *view)
{
	return moveCursor(view, MoveCursorUp);
}

bool EditorExtension::moveCursorDown(KTextEditor::View *view)
{
	return moveCursor(view, MoveCursorDown);
}

bool EditorExtension::moveCursor(KTextEditor::View *view, CursorMove direction)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	KTextEditor::Document *doc = view->document();

	KTextEditor::Cursor cursor = view->cursorPosition();
	int row = cursor.line();
	int col = cursor.column();

	bool ok = false;
	switch (direction)  {
		case MoveCursorLeft:  ok = decreaseCursorPosition(doc,row,col);
		                      break;
		case MoveCursorRight: ok = increaseCursorPosition(doc,row,col);
		                      break;
		case MoveCursorUp:    if(row > 0) {
		                         row--;
		                         ok = true;
		                      }
		                      break;
		case MoveCursorDown:  if(row < doc->lines() - 1) {
		                          row++;
		                          ok = true;
		                      }
		                      break;
	}

	if(ok) {
		return view->setCursorPosition(KTextEditor::Cursor(row,col));
	}
	else {
		return false;
	}
}

bool EditorExtension::increaseCursorPosition(KTextEditor::Document *doc, int &row, int &col)
{
	bool ok = true;

	if(col < doc->lineLength(row) - 1) {
		++col;
	}
	else if(row < doc->lines() - 1) {
		++row;
		col = 0;
	}
	else {
		ok = false;
	}

	return ok;
}

bool EditorExtension::decreaseCursorPosition(KTextEditor::Document *doc, int &row, int &col)
{
	bool ok = true;

	if(col > 0) {
		--col;
	}
	else if(row > 0) {
		--row;
		col = doc->lineLength(row);
	}
	else {
		ok = false;
	}

	return ok;
}

//////////////////// texgroups ////////////////////

// goto the next non-nested bracket

void EditorExtension::gotoTexgroup(bool backwards, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) return;

	uint row,col;
	bool found;
	BracketData bracket;

	// get current position
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();
	m_overwritemode = (view->viewEditMode() == KTextEditor::View::EditOverwrite);

	// start searching
	if(backwards) {
		found = findOpenBracket(doc, row, col, bracket);
	}
	else {
		found = findCloseBracket(doc, row, col, bracket);
		// go behind the bracket
		if(!m_overwritemode) {
			++bracket.col;
		}
	}

	if(found) {
		view->setCursorPosition(KTextEditor::Cursor(bracket.row, bracket.col));
	}
}

// match the opposite bracket

void EditorExtension::matchTexgroup(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int row, col;
	BracketData bracket;

	// get current position
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();
	m_overwritemode = (view->viewEditMode() == KTextEditor::View::EditOverwrite);

	// this operation is only allowed at a bracket position
	if(!isBracketPosition(doc, row, col, bracket)) {
		return;
	}

	// start searching
	bool found = false;
	if(bracket.open) {
		found = findCloseBracketTag(doc, bracket.row, bracket.col + 1, bracket);
		// go behind the bracket
		if(!m_overwritemode) {
			++bracket.col;
		}
	}
	else {
		if(!decreaseCursorPosition(doc, bracket.row, bracket.col)) {
			return;
		}
		found = findOpenBracketTag(doc, bracket.row, bracket.col, bracket);
	}

	if(found) {
		view->setCursorPosition(KTextEditor::Cursor(bracket.row, bracket.col));
	}
}

//////////////////// close an open texgroup  ////////////////////

// search for the last opened texgroup and close it

void EditorExtension::closeTexgroup(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int row, col;
	BracketData bracket;

	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	int rowtemp = row;
	int coltemp = col;
	if(!decreaseCursorPosition(doc, rowtemp, coltemp)) {
		return;
	}

	if(findOpenBracketTag(doc, rowtemp, coltemp, bracket)) {
		doc->insertText(KTextEditor::Cursor(row, col), "}");
		view->setCursorPosition(KTextEditor::Cursor(row, col + 1));
	}
}

//////////////////// select a texgroup  ////////////////////

void EditorExtension::selectTexgroup(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = texgroupRange(inside,view);
	if(range.isValid()) {
		view->setSelection(range);
	}
}

void EditorExtension::deleteTexgroup(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range =texgroupRange(inside,view);
	if(range.isValid()) {
		deleteRange(range, view);
	}
}

// calculate start and end of a Texgroup

KTextEditor::Range EditorExtension::texgroupRange(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return KTextEditor::Range::invalid();
	}

	BracketData open, close;
	if(getTexgroup(inside, open, close, view)) {
		return KTextEditor::Range(open.row, open.col, close.row, close.col);
	}
	else {
		return KTextEditor::Range::invalid();
	}
}

bool EditorExtension::hasTexgroup(KTextEditor::View *view)
{
	// view will be checked in texgroupRange()
	KTextEditor::Range range = texgroupRange(true, view);
	return (range.isValid()) ? true : false;
}

QString EditorExtension::getTexgroupText(bool inside, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return QString();
	}

	KTextEditor::Range range = texgroupRange(inside,view);
	return (range.isValid()) ? view->document()->text(range) : QString();
}

bool EditorExtension::getTexgroup(bool inside, BracketData &open, BracketData &close, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	int row, col;

	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	if(!findOpenBracket(doc, row, col, open))  {
		//KILE_DEBUG() << "no open bracket";
		return false;
	}
	if(!findCloseBracket(doc, row, col, close)) {
		//KILE_DEBUG() << "no close bracket";
		return false;
	}

	if(inside) {
		++open.col;
	}
	else {
		++close.col;
	}

	return true;
}

//////////////////// search for a bracket position  ////////////////////

// Find the last opening bracket. If the current cursor is over
//  - '{': we will stop immediately
//  - '}': we will start before this character

bool EditorExtension::findOpenBracket(KTextEditor::Document *doc, int row, int col, BracketData &bracket)
{
	if(isBracketPosition(doc, row, col, bracket)) {
		// already found position?
		if(bracket.open) {
			return true;
		}

		// go one position back
		row = bracket.row;
		col = bracket.col;
		if(!decreaseCursorPosition(doc, row, col)) {
			return false;
		}
	}

	// looking back for last bracket
	return findOpenBracketTag(doc, row, col, bracket);
}

// Find the last closing bracket. If the current cursor is over
//  - '}': we will stop immediately
//  - '{': we will start behind this character

bool EditorExtension::findCloseBracket(KTextEditor::Document *doc, int row, int col, BracketData &bracket)
{
	if (isBracketPosition(doc, row, col, bracket)) {
		// already found position?
		if(!bracket.open) {
			return true;
		}

		// go one position forward
		row = bracket.row;
		col = bracket.col + 1;
	}

	// looking forward for next bracket
	return findCloseBracketTag(doc, row, col, bracket);
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

bool EditorExtension::isBracketPosition(KTextEditor::Document *doc, int row, int col, BracketData &bracket)
{
	// default results
	bracket.row = row;
	bracket.col = col;

	QString textline = getTextLineReal(doc, row);
	QChar right = textline[col];
	QChar left  = (col > 0) ? textline[col-1] : QChar(' ');

	if (m_overwritemode) {
		if(right == '{') {
			bracket.open = true;
		}
		else if(left == '}') {
			bracket.open = false;
		}
		else {
			return false;
		}
	}
	else if(left == '}') {
		bracket.open = false;
		--bracket.col;
	}
	else if(right == '{') {
		bracket.open = true;
	}
	else if(left == '{') {
		bracket.open = true;
		--bracket.col;
	}
	else if(right == '}') {
		bracket.open = false;
	}
	else {
		return false;
	}

	return true;
}

// find next non-nested closing bracket

bool EditorExtension::findCloseBracketTag(KTextEditor::Document *doc, int row, int col, BracketData &bracket)
{
	uint brackets = 0;
	for(int line = row; line < doc->lines(); ++line) {
		uint start = (line == row) ? col : 0;
		QString textline = getTextLineReal(doc,line);
		for(int i = start; i < textline.length(); ++i) {
			if(textline[i] == '{') {
				++brackets;
			}
			else if(textline[i] == '}') {
				if(brackets > 0) {
					--brackets;
				}
				else {
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

bool EditorExtension::findOpenBracketTag(KTextEditor::Document *doc, int row, int col, BracketData &bracket)
{
	uint brackets = 0;
	for(int line = row; line >= 0; --line) {
		QString textline = getTextLineReal(doc, line);
		int start = (line == row) ? col : textline.length() - 1;
		for (int i = start; i >= 0; --i) {
			//KILE_DEBUG() << "findOpenBracketTag: (" << line << "," << i << ") = " << textline[i].toLatin1();
			if(textline[i] == '{') {
				if(brackets > 0) {
					--brackets;
				}
				else {
					bracket.row = line;
					bracket.col = i;
					bracket.open = true;
					return true;
				}
			}
			else if(textline[i] == '}') {
				++brackets;
			}
		}
	}

	//KILE_DEBUG() << "nothting found";
	return false;
}

//////////////////// get real text ////////////////////

// get current textline and remove
//  - all pairs of backslashes: '\\'
//  - all quoted comment signs: '\%'
//  - all quoted brackets: '\{' and '\}'
//  - all comments
// replace these characters one one, which never will be looked for

QString EditorExtension::getTextLineReal(KTextEditor::Document *doc, int row)
{
	QString textline = doc->line(row);
	int len = textline.length();
	if(len == 0) {
		return QString();
	}

	bool backslash = false;
	for(int i = 0; i < len; ++i) {
		if (textline[i]=='{' || textline[i]=='}' || textline[i]=='$') {
			if(backslash) {
				textline[i-1] = '&';
				textline[i] = '&';
			}
			backslash = false;
		}
		else if(textline[i] == '\\') {
			if(backslash) {
				textline[i-1] = '&';
				textline[i] = '&';
				backslash = false;
			}
			else {
				backslash = true;
			}
		}
		else if(textline[i]=='%') {
			if (backslash) {
				textline[i-1] = '&';
				textline[i] = '&';
			}
			else {
				len = i;
				break;
			}
			backslash = false;
		}
		else {
			backslash = false;
		}
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

bool EditorExtension::getCurrentWord(KTextEditor::Document *doc, int row, int col, EditorExtension::SelectMode mode, QString &word, int &x1, int &x2)
{
	// get real textline without comments, quoted characters and pairs of backslashes
	QString textline = getTextLineReal(doc, row);
	if (col > textline.length()) {
		return false;
	}

	QRegExp reg;
	QString pattern1, pattern2;
	switch(mode) {
		case smLetter:
			pattern1 = "[^a-zA-Z]+";
			pattern2 = "[a-zA-Z]+";
		break;
		case smWord:
			pattern1 = "[^a-zA-Z0-9]";
			pattern2 = "[a-zA-Z0-9]+";
		break;
		case smNospace:
			pattern1 = "\\s";
			pattern2 = "\\S+";
		break;
		default:
			pattern1 = "[^a-zA-Z]";
			pattern2 = "\\\\?[a-zA-Z]+\\*?";
		break;
	}
	x1 = x2 = col;

	int pos;
	// search to the left side
	if(col > 0) {
		reg.setPattern(pattern1);
		pos = textline.lastIndexOf(reg, col - 1);
		if(pos != -1) {        // found an illegal character
			x1 = pos + 1;
			if(mode == smTex) {
				if(textline[pos] == '\\') {
					x1 = pos;
				}
				col = x1;
			}
		}
		else {
			x1 = 0;               // pattern matches from beginning of line
		}
	}

	// search at the current position
	reg.setPattern(pattern2);
	pos = textline.indexOf(reg, col);
	if(pos != -1 && pos == col) {
		x2 = pos + reg.matchedLength();
	}

	// get all characters
	if(x1 != x2) {
		word = textline.mid(x1, x2 - x1);
		return true;
	}
	else {
		return false;
	}
}

KTextEditor::Range EditorExtension::wordRange(const KTextEditor::Cursor &cursor, bool latexCommand, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return KTextEditor::Range::invalid();
	}

	int col1, col2;
	QString word;
	EditorExtension::SelectMode mode = ( latexCommand ) ? EditorExtension::smTex : EditorExtension::smLetter;
	int line = cursor.line();

	return (getCurrentWord(view->document(), line, cursor.column(), mode, word, col1, col2))
	       ? KTextEditor::Range(line,col1,line,col2)
	       : KTextEditor::Range::invalid();
}

QString EditorExtension::word(const KTextEditor::Cursor &cursor, bool latexCommand, KTextEditor::View *view)
{
	KTextEditor::Range range = EditorExtension::wordRange(cursor,latexCommand,view);
	return ( range.isValid() ) ? view->document()->text(range) : QString();
}

//////////////////// paragraph ////////////////////

void EditorExtension::selectParagraph(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = findCurrentParagraphRange(view);
	if ( range.isValid() ) {
		view->setSelection(range);
	}
}

void EditorExtension::deleteParagraph(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}
	int startline, endline;

	if(findCurrentTexParagraph(startline, endline, view)) {
		KTextEditor::Document *doc = view->document();
		view->removeSelection();
		if(startline > 0) {
			--startline;
		}
		else if(endline < doc->lines() - 1) {
			++endline;
		}
		doc->removeText(KTextEditor::Range(startline, 0, endline+1, 0));
		view->setCursorPosition(KTextEditor::Cursor(startline, 0));
	}
}

// get the range of the current paragraph
KTextEditor::Range EditorExtension::findCurrentParagraphRange(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return KTextEditor::Range::invalid();
	}

	int startline, endline;
	return (findCurrentTexParagraph(startline, endline, view))
	       ? KTextEditor::Range(startline, 0, endline + 1, 0)
	       : KTextEditor::Range::invalid();
}

QString  EditorExtension::getParagraphText(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return QString();
	}

	KTextEditor::Range range = findCurrentParagraphRange(view);
	return (range.isValid()) ? view->document()->text(range) : QString();
}

bool EditorExtension::findCurrentTexParagraph(int &startline, int &endline, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	int row;

	// get current position
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();

	// don't accept an empty line as part of a paragraph
	if(doc->line(row).trimmed().isEmpty()) {
		return false;
	}

	// settings default results
	startline = row;
	endline = row;

	// find the previous empty line
	for(int line = row - 1; line >= 0; --line) {
		if(doc->line(line).trimmed().isEmpty()) {
			break;
		}
		startline = line;
	}

	// find the next empty line
	for(int line = row + 1; line < doc->lines(); ++line) {
		if(doc->line(line).trimmed().isEmpty()) {
			break;
		}
		endline = line;
	}

	// settings result
	return true;
}

void EditorExtension::gotoNextParagraph(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	bool found;
	int startline, endline;
	KTextEditor::Document *doc = view->document();

	endline = view->cursorPosition().line();
	if(doc->line(endline).trimmed().isEmpty()) {
		found = true;
	}
	else {
		found = findCurrentTexParagraph(startline, endline, view);
	}

	// we are in an empty line or in the last line of a paragraph
	if (found) {
		// find the next non empty line
		for(int line = endline + 1; line < doc->lines(); ++line) {
			if(!doc->line(line).trimmed().isEmpty()) {
				view->setCursorPosition(KTextEditor::Cursor(line, 0));
				return;
			}
		}
	}
}

void EditorExtension::gotoPrevParagraph(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	bool found;
	int startline,endline;
	KTextEditor::Document *doc = view->document();

	startline = view->cursorPosition().line();
	if(doc->line(startline).trimmed().isEmpty()) {
		startline++;
		found = true;
	}
	else {
		found = findCurrentTexParagraph(startline,endline,view);
	}
	// we are in an empty line or in the first line of a paragraph
	if(found) {
		// find the last line of the previous paragraph
		int foundline = -1;
		for (int line = startline - 1; line >= 0; --line) {
			if(!doc->line(line).trimmed().isEmpty()) {
				break;
			}
			foundline = line;
		}
		if(foundline < 0) {
			return;
		}

		// and finally the first line of this paragraph
		int prevstartline = -1;
		for(int line = foundline - 1; line >= 0; --line) {
			if(doc->line(line).trimmed().isEmpty()) {
				break;
			}
			prevstartline = line;
		}

		if(prevstartline >= 0) {
			view->setCursorPosition(KTextEditor::Cursor(prevstartline, 0));
		}
	}
}

int EditorExtension::prevNonEmptyLine(int line, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return -1;
	}

	KTextEditor::Document *doc = view->document();
	for(int i = line - 1; i >= 0; --i) {
		if(!doc->line(i).trimmed().isEmpty()) {
			return i;
		}
	}
	return -1;
}

int EditorExtension::nextNonEmptyLine(int line, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return -1;
	}

	KTextEditor::Document *doc = view->document();
	int lines = doc->lines();
	for(int i = line + 1; i < lines; ++i) {
		if(!doc->line(i).trimmed().isEmpty()) {
			return i;
		}
	}
	return -1;
}

//////////////////// one line of text////////////////////

void EditorExtension::selectLine(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	// get current position
	int row;
	QString word;
	KTextEditor::Document *doc = view->document();
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();

	if(doc->lineLength(row) > 0) {
		view->setSelection(KTextEditor::Range(row, 0, row + 1, 0));
	}
}

void EditorExtension::selectLine(int line, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	if(view->document()->lineLength(line) > 0) {
		view->setSelection(KTextEditor::Range(line, 0, line + 1, 0));
	}
}

void EditorExtension::selectLines(int from, int to, KTextEditor::View *view)
{
	view = determineView(view);
	if(view && from <= to) {
		view->setSelection(KTextEditor::Range(from, 0, to + 1, 0));
	}
}

bool EditorExtension::replaceLine(int line, const QString &s, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	KTextEditor::Document *doc = view->document();
	doc->startEditing();
	doc->removeLine(line);
	bool result = doc->insertLine(line, s);
	doc->endEditing();
	return result;
}

void EditorExtension::deleteEndOfLine(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int row, col;
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	KTextEditor::Document *doc = view->document();
	view->removeSelection();
	doc->removeText(KTextEditor::Range(row, col, row, doc->lineLength(row)));
}

//////////////////// LaTeX command ////////////////////

void EditorExtension::selectWord(EditorExtension::SelectMode mode, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = wordRange(view->cursorPosition(),mode,view);
	if ( range.isValid() ) {
		view->setSelection(range);
	}
}

void EditorExtension::deleteWord(EditorExtension::SelectMode mode, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Range range = wordRange(view->cursorPosition(),mode,view);
	if(range.isValid()) {
		deleteRange(range,view);
	}
}

void EditorExtension::nextBullet(KTextEditor::View* view)
{
	gotoBullet(false, view);
}

void EditorExtension::prevBullet(KTextEditor::View* view)
{
	gotoBullet(true, view);
}

void EditorExtension::insertBullet(KTextEditor::View* view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	view->document()->insertText(view->cursorPosition(), s_bullet);
}

///////////////////// Special Functions ///////////////
/*
void EditorExtension::insertNewLine(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int newLineNumber = view->cursorPosition().line() + 1;
	view->document()->insertLine(newLineNumber, QString());
}
*/
void EditorExtension::moveCursorToLastPositionInCurrentLine(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	const KTextEditor::Cursor currentPosition = view->cursorPosition();
	view->setCursorPosition(KTextEditor::Cursor(currentPosition.line(),
	                                            view->document()->lineLength(currentPosition.line())));
}

void EditorExtension::keyReturn(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int newLineNumber = view->cursorPosition().line() + 1;
	view->document()->insertLine(newLineNumber, QString());
	view->setCursorPosition(KTextEditor::Cursor(newLineNumber, 0));
}

void EditorExtension::commentLaTeX(KTextEditor::Document* document, const KTextEditor::Range& range)
{
	int startLine = range.start().line(), endLine = range.end().line();
	for(int i = startLine; i <= endLine; ++i) {
		document->insertText(KTextEditor::Cursor(i, 0), "% ");
	}
}

void EditorExtension::goToLine(int line, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	KTextEditor::Cursor cursor(line, 0);
	view->setCursorPosition(cursor);
}

//////////////////// double quotes ////////////////////

void EditorExtension::initDoubleQuotes()
{
	m_dblQuotes = KileConfig::insertDoubleQuotes();

	int index = KileConfig::doubleQuotes();
	if(index < 0 || index >= m_quoteList.count()) {
		index = 0;
	}

	m_leftDblQuote = m_quoteList[index].first;
	m_rightDblQuote = m_quoteList[index].second;
	KILE_DEBUG() << "new quotes: " << m_dblQuotes << " left=" << m_leftDblQuote << " right=" << m_rightDblQuote<< endl;
}

bool EditorExtension::insertDoubleQuotes(KTextEditor::View *view)
{
	// don't insert double quotes, if konsole has focus
	// return false, because if this is called from an event
	// handler, because this event has to be passed on
	if(m_ki->texKonsole()->hasFocus()) {
		return false;
	}

	// insert double quotes, normal mode or autocompletion mode
	// always return true for event handler
	view = determineView(view);
	if(!view) {
		return true;
	}

	KTextEditor::Document *doc = view->document();

	if(!doc) {
		return false;
	}

	view->removeSelectionText();

	int row, col;
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	// simply insert, if we are inside a verb command
	if(insideVerb(view) || insideVerbatim(view)) {
		return false;
	}

	// simply insert, if autoinsert mode is not active or the char bevor is \ (typically for \"a useful)
	if (!m_dblQuotes || (col > 0 && doc->text(KTextEditor::Range(row, col - 1, row, col)) == "\\")) {
		return false;
	}

	// insert with auto mode
	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);
	if(!iface) {
		return false;
	}

	QString pattern1 = QRegExp::escape(m_leftDblQuote);
	if(m_leftDblQuote.at(m_leftDblQuote.length()-1).isLetter()) {
		pattern1 += "(\\b|(\\{\\}))";
	}
	QString pattern2 = QRegExp::escape(m_rightDblQuote);
	if(m_rightDblQuote.at(m_rightDblQuote.length()-1).isLetter()) {
		pattern2 += "(\\b|(\\{\\}))";
	}

	bool openFound = false;
	KTextEditor::Range searchRange = KTextEditor::Range(KTextEditor::Cursor(0, 0), KTextEditor::Cursor(row, col));
	QVector<KTextEditor::Range> foundRanges = iface->searchText(searchRange, '(' + pattern1 + ")|(" + pattern2 + ')',
		KTextEditor::Search::Regex | KTextEditor::Search::Backwards);
	// KTextEditor::SearchInterface#searchText always returns at least one range, even
	// if no occurrences have been found. Thus, we have to check if the range is valid.
	KTextEditor::Range range = foundRanges.first();
	if(range.isValid()) {
		int lineFound = range.start().line();
		int columnFound = range.start().column();
		openFound = (doc->line(lineFound).indexOf(m_leftDblQuote, columnFound) == columnFound);
	}

	QString textline = doc->line(row);
	//KILE_DEBUG() << "text=" << textline << " open=" << openfound;
	if(openFound) {
		// If we last inserted a language specific doublequote open,
		// we have to change it to a normal doublequote. If not we
		// insert a language specific doublequote close
		int startcol = col - m_leftDblQuote.length();
		//KILE_DEBUG() << "startcol=" << startcol << " col=" << col ;
		if (startcol>=0 && textline.indexOf(m_leftDblQuote, startcol) == startcol) {
				doc->removeText(KTextEditor::Range(row, startcol, row, startcol + m_leftDblQuote.length()));
				doc->insertText(KTextEditor::Cursor(row, startcol), "\"");
		}
		else {
			doc->insertText(KTextEditor::Cursor(row, col), m_rightDblQuote);
		}
	}
	else {
		// If we last inserted a language specific doublequote close,
		// we have to change it to a normal doublequote. If not we
		// insert a language specific doublequote open
		int startcol = col - m_rightDblQuote.length();
		//KILE_DEBUG() << "startcol=" << startcol << " col=" << col ;
		if (startcol >= 0 && textline.indexOf(m_rightDblQuote, startcol) == startcol) {
			doc->removeText(KTextEditor::Range(row, startcol, row, startcol + m_rightDblQuote.length()));
			doc->insertText(KTextEditor::Cursor(row,startcol), "\"");
		}
		else {
			doc->insertText(KTextEditor::Cursor(row, col), m_leftDblQuote);
		}
	}
	return true;
}

// Takes an unicode unsigned short and calls insertSpecialCharacter to
// insert the proper LaTeX sequence and warn user of any dependencies.
//FIXME: there should be one central place to convert unicode chars to LaTeX;
//       also see 'LaTeXEventFilter::eventFilter'.
bool EditorExtension::insertLatexFromUnicode(unsigned short rep, KTextEditor::View *view)
{
	switch(rep)
		{ // Find the unicode decimal representation
		case 160:	return insertSpecialCharacter("~", view);
		case 161:	return insertSpecialCharacter("!`", view);
		case 162:	return insertSpecialCharacter("\\textcent", view, "textcomp");
		case 163:	return insertSpecialCharacter("\\pounds", view);
		case 164:	return insertSpecialCharacter("\\textcurrency", view, "textcomp");
		case 165:	return insertSpecialCharacter("\\textyen", view, "textcomp");
		case 166:	return insertSpecialCharacter("\\textbrokenbar", view, "textcomp");
		case 167:	return insertSpecialCharacter("\\S", view);
		case 168:	return insertSpecialCharacter("\"", view);
		case 169:	return insertSpecialCharacter("\\copyright", view);
		case 170:	return insertSpecialCharacter("\\textordfeminine", view, "textcomp");
		case 171:	return insertSpecialCharacter("\\guillemotleft", view);
		case 172:	return insertSpecialCharacter("\\neg", view); // TODO: Check for math
		case 173:	return insertSpecialCharacter("-", view); // TODO: Check for math
		case 174:	return insertSpecialCharacter("\\textregistered", view, "textcomp");
		case 176:	return insertSpecialCharacter("^\\circ", view); // TODO: Check for math
		case 177:	return insertSpecialCharacter("\\pm", view); // TODO: Check for math
		case 178:	return insertSpecialCharacter("^2", view); // TODO: Check for math
		case 179:	return insertSpecialCharacter("^3", view); // TODO: Check for math
		case 180:	return insertSpecialCharacter("'", view);
		case 181:	return insertSpecialCharacter("\\mu", view); // TODO: Check for math
		case 182:	return insertSpecialCharacter("\\P", view);
		case 185:	return insertSpecialCharacter("^1", view); // TODO: Check for math
		case 186:	return insertSpecialCharacter("\\textordmasculine", view, "textcomp");
		case 187:	return insertSpecialCharacter("\\guillemotright", view);
		case 191:	return insertSpecialCharacter("?`", view);
		case 192:	return insertSpecialCharacter("\\`A", view);
		case 193:	return insertSpecialCharacter("\\'A", view);
		case 194:	return insertSpecialCharacter("\\^A", view);
		case 195:	return insertSpecialCharacter("\\~A", view);
		case 196:	return insertSpecialCharacter("\\\"A", view);
		case 197:	return insertSpecialCharacter("\\AA", view);
		case 198:	return insertSpecialCharacter("\\AE", view);
		case 199:	return insertSpecialCharacter("\\c{C}", view);
		case 200:	return insertSpecialCharacter("\\`E", view);
		case 201:	return insertSpecialCharacter("\\'E", view);
		case 202:	return insertSpecialCharacter("\\^E", view);
		case 203:	return insertSpecialCharacter("\\\"E", view);
		case 204:	return insertSpecialCharacter("\\`I", view);
		case 205:	return insertSpecialCharacter("\\'I", view);
		case 206:	return insertSpecialCharacter("\\^I", view);
		case 207:	return insertSpecialCharacter("\\\"I", view);
		case 209:	return insertSpecialCharacter("\\~N", view);
		case 210:	return insertSpecialCharacter("\\`O", view);
		case 211:	return insertSpecialCharacter("\\'O", view);
		case 212:	return insertSpecialCharacter("\\^O", view);
		case 213:	return insertSpecialCharacter("\\~O", view);
		case 214:	return insertSpecialCharacter("\\\"O", view);
		case 215:	return insertSpecialCharacter("\\times", view); //TODO: Check for math
		case 216:	return insertSpecialCharacter("\\Oslash", view);
		case 217:	return insertSpecialCharacter("\\`U", view);
		case 218:	return insertSpecialCharacter("\\'U", view);
		case 219:	return insertSpecialCharacter("\\^U", view);
		case 220:	return insertSpecialCharacter("\\\"U", view);
		case 221:	return insertSpecialCharacter("\\'Y", view);
		case 223:	return insertSpecialCharacter("\\ss{}", view);
		case 224:	return insertSpecialCharacter("\\`a", view);
		case 225:	return insertSpecialCharacter("\\'a", view);
		case 226:	return insertSpecialCharacter("\\^a", view);
		case 227:	return insertSpecialCharacter("\\~a", view);
		case 228:	return insertSpecialCharacter("\\\"a", view);
		case 229:	return insertSpecialCharacter("\\aa", view);
		case 230:	return insertSpecialCharacter("\\ae", view);
		case 231:	return insertSpecialCharacter("\\c{c}", view);
		case 232:	return insertSpecialCharacter("\\`e", view);
		case 233:	return insertSpecialCharacter("\\'e", view);
		case 234:	return insertSpecialCharacter("\\^e", view);
		case 235:	return insertSpecialCharacter("\\\"e", view);
		case 236:	return insertSpecialCharacter("\\`i", view);
		case 237:	return insertSpecialCharacter("\\'i", view);
		case 238:	return insertSpecialCharacter("\\^i", view);
		case 239:	return insertSpecialCharacter("\\\"i", view);
		case 240:	return insertSpecialCharacter("\\~o", view);
		case 241:	return insertSpecialCharacter("\\~n", view);
		case 242:	return insertSpecialCharacter("\\`o", view);
		case 243:	return insertSpecialCharacter("\\'o", view);
		case 244:	return insertSpecialCharacter("\\^o", view);
		case 245:	return insertSpecialCharacter("\\~o", view);
		case 246:	return insertSpecialCharacter("\\\"o", view);
		case 247:	return insertSpecialCharacter("\\div", view);
		case 248:	return insertSpecialCharacter("\\oslash", view);
		case 249:	return insertSpecialCharacter("\\`u", view);
		case 250:	return insertSpecialCharacter("\\'u", view);
		case 251:	return insertSpecialCharacter("\\^u", view);
		case 252:	return insertSpecialCharacter("\\\"u", view);
		case 253:	return insertSpecialCharacter("\\'y", view);
		case 255:	return insertSpecialCharacter("\\\"y", view);
		case 256:	return insertSpecialCharacter("\\=A", view);
		case 257:	return insertSpecialCharacter("\\=a", view);
		case 258:	return insertSpecialCharacter("\\uA", view);
		case 259:	return insertSpecialCharacter("\\ua", view);
		case 262:	return insertSpecialCharacter("\\'C", view);
		case 263:	return insertSpecialCharacter("\\'c", view);
		case 264:	return insertSpecialCharacter("\\^C", view);
		case 265:	return insertSpecialCharacter("\\^c", view);
		case 266:	return insertSpecialCharacter("\\.C", view);
		case 267:	return insertSpecialCharacter("\\.c", view);
		case 268:	return insertSpecialCharacter("\\vC", view);
		case 269:	return insertSpecialCharacter("\\vc", view);
		case 270:	return insertSpecialCharacter("\\vD", view);
		case 271:	return insertSpecialCharacter("\\vd", view);
		case 272:	return insertSpecialCharacter("\\=D", view);
		case 273:	return insertSpecialCharacter("\\=d", view);
		case 274:	return insertSpecialCharacter("\\=E", view);
		case 275:	return insertSpecialCharacter("\\=e", view);
		case 276:	return insertSpecialCharacter("\\uE", view);
		case 277:	return insertSpecialCharacter("\\ue", view);
		case 278:	return insertSpecialCharacter("\\.E", view);
		case 279:	return insertSpecialCharacter("\\.e", view);
		case 282:	return insertSpecialCharacter("\\vE", view);
		case 283:	return insertSpecialCharacter("\\ve", view);
		case 284:	return insertSpecialCharacter("\\^G", view);
		case 285:	return insertSpecialCharacter("\\^g", view);
		case 286:	return insertSpecialCharacter("\\uG", view);
		case 287:	return insertSpecialCharacter("\\ug", view);
		case 288:	return insertSpecialCharacter("\\.G", view);
		case 289:	return insertSpecialCharacter("\\.g", view);
		case 290:	return insertSpecialCharacter("\\cG", view);
		case 291:	return insertSpecialCharacter("\\'g", view);
		case 292:	return insertSpecialCharacter("\\^H", view);
		case 293:	return insertSpecialCharacter("\\^h", view);
		case 294:	return insertSpecialCharacter("\\=H", view);
		case 295:	return insertSpecialCharacter("\\=h", view);
		case 296:	return insertSpecialCharacter("\\~I", view);
		case 297:	return insertSpecialCharacter("\\~i", view);
		case 298:	return insertSpecialCharacter("\\=I", view);
		case 299:	return insertSpecialCharacter("\\=i", view);
		case 300:	return insertSpecialCharacter("\\uI", view);
		case 301:	return insertSpecialCharacter("\\ui", view);
		case 304:	return insertSpecialCharacter("\\.I", view);
		case 305:	return insertSpecialCharacter("\\i", view);
		case 308:	return insertSpecialCharacter("\\^J", view);
		case 309:	return insertSpecialCharacter("\\^j", view);
		case 310:	return insertSpecialCharacter("\\cK", view);
		case 311:	return insertSpecialCharacter("\\ck", view);
		case 313:	return insertSpecialCharacter("\\'L", view);
		case 314:	return insertSpecialCharacter("\\'l", view);
		case 315:	return insertSpecialCharacter("\\cL", view);
		case 316:	return insertSpecialCharacter("\\cl", view);
		case 317:	return insertSpecialCharacter("\\vL", view);
		case 318:	return insertSpecialCharacter("\\vl", view);
		case 323:	return insertSpecialCharacter("\\'N", view);
		case 324:	return insertSpecialCharacter("\\'n", view);
		case 325:	return insertSpecialCharacter("\\cN", view);
		case 326:	return insertSpecialCharacter("\\cn", view);
		case 327:	return insertSpecialCharacter("\\vN", view);
		case 328:	return insertSpecialCharacter("\\vn", view);
		case 332:	return insertSpecialCharacter("\\=O", view);
		case 333:	return insertSpecialCharacter("\\=o", view);
		case 334:	return insertSpecialCharacter("\\uO", view);
		case 335:	return insertSpecialCharacter("\\uo", view);
		case 336:	return insertSpecialCharacter("\\HO", view);
		case 337:	return insertSpecialCharacter("\\Ho", view);
		case 338:	return insertSpecialCharacter("\\OE", view);
		case 339:	return insertSpecialCharacter("\\oe", view);
		case 340:	return insertSpecialCharacter("\\'R", view);
		case 341:	return insertSpecialCharacter("\\'r", view);
		case 342:	return insertSpecialCharacter("\\cR", view);
		case 343:	return insertSpecialCharacter("\\cr", view);
		case 344:	return insertSpecialCharacter("\\vR", view);
		case 345:	return insertSpecialCharacter("\\vr", view);
		case 346:	return insertSpecialCharacter("\\'S", view);
		case 347:	return insertSpecialCharacter("\\'s", view);
		case 348:	return insertSpecialCharacter("\\^S", view);
		case 349:	return insertSpecialCharacter("\\^s", view);
		case 350:	return insertSpecialCharacter("\\cS", view);
		case 351:	return insertSpecialCharacter("\\cs", view);
		case 352:	return insertSpecialCharacter("\\vS", view);
		case 353:	return insertSpecialCharacter("\\vs", view);
		case 354:	return insertSpecialCharacter("\\cT", view);
		case 355:	return insertSpecialCharacter("\\ct", view);
		case 356:	return insertSpecialCharacter("\\vT", view);
		case 357:	return insertSpecialCharacter("\\vt", view);
		case 358:	return insertSpecialCharacter("\\=T", view);
		case 359:	return insertSpecialCharacter("\\=t", view);
		case 360:	return insertSpecialCharacter("\\~U", view);
		case 361:	return insertSpecialCharacter("\\~u", view);
		case 362:	return insertSpecialCharacter("\\=U", view);
		case 363:	return insertSpecialCharacter("\\=u", view);
		case 364:	return insertSpecialCharacter("\\uU", view);
		case 365:	return insertSpecialCharacter("\\uu", view);
		case 366:	return insertSpecialCharacter("\\AU", view);
		case 367:	return insertSpecialCharacter("\\Au", view);
		case 368:	return insertSpecialCharacter("\\HU", view);
		case 369:	return insertSpecialCharacter("\\Hu", view);
		case 370:	return insertSpecialCharacter("\\cU", view);
		case 371:	return insertSpecialCharacter("\\cu", view);
		case 372:	return insertSpecialCharacter("\\^W", view);
		case 373:	return insertSpecialCharacter("\\^w", view);
		case 374:	return insertSpecialCharacter("\\^Y", view);
		case 375:	return insertSpecialCharacter("\\^y", view);
		case 376:	return insertSpecialCharacter("\\\"Y", view);
		case 377:	return insertSpecialCharacter("\\'Z", view);
		case 378:	return insertSpecialCharacter("\\'z", view);
		case 379:	return insertSpecialCharacter("\\.Z", view);
		case 380:	return insertSpecialCharacter("\\.z", view);
		case 381:	return insertSpecialCharacter("\\vZ", view);
		case 382:	return insertSpecialCharacter("\\vz", view);
		default:	return false;
		}
}

// If allowed, inserts texString at current cursor postition. Startlingly similar to insertDoubleQuotes.
bool EditorExtension::insertSpecialCharacter(const QString& texString, KTextEditor::View *view, const QString& dep)
{
	// stop if special character replacement is disabled
	if (!m_specialCharacters) {
		return false;
	}

	// return false if konsole has focus
	if(m_ki->texKonsole()->hasFocus()) {
		return false;
	}

	// always return true for event handler
	view = determineView(view);
	if(!view) {
		return true;
	}

	KTextEditor::Document *doc = view->document();

	// Only change if we have a tex document
	if(!doc || !m_ki->extensions()->isTexFile(doc->url())) {
		return false;
	}

	// In case of replace
	view->removeSelectionText();

	int row, col;
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	// insert texString
	doc->insertText(KTextEditor::Cursor(row, col), texString);

	KILE_DEBUG() << "Replacing with "<<texString;

	// Check dependency
	if (!dep.isEmpty()) {
		QStringList packagelist = m_ki->allPackages();
		if(!packagelist.contains(dep)) {
			m_ki->errorHandler()->printMessage(KileTool::Error, i18n("You have to include the package %1 to use %2.", dep, texString), i18n("Missing Package"));
			KILE_DEBUG() << "Need package "<< dep;
		}
	}

	return true;
}

//////////////////// insert tabulator ////////////////////

void EditorExtension::insertIntelligentTabulator(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int row, col, currentRow, currentCol;
	QString envname,tab;
	QString prefix = " ";

	KTextEditor::Cursor cursor = view->cursorPosition();
	currentRow = cursor.line();
	currentCol = cursor.column();
	if(findOpenedEnvironment(row, col, envname, view)) {
		// look if this is an environment with tabs
		tab = m_latexCommands->getTabulator(envname);

		// try to align tabulator with textline above
		if(currentRow >= 1) {
			int tabpos = view->document()->line(currentRow - 1).indexOf('&', currentCol);
			if(tabpos >= 0) {
				currentCol = tabpos;
				prefix.clear();
			}
		}
	}

	if(tab.isEmpty()) {
		tab = '&';
	}
	tab = prefix + tab + ' ';

	view->document()->insertText(KTextEditor::Cursor(currentRow, currentCol), tab);
	view->setCursorPosition(KTextEditor::Cursor(currentRow, currentCol + tab.length()));
}

//////////////////// autocomplete environment ////////////////////

// should we complete the current environment (call from LaTeXEventFilter)

bool EditorExtension::eventInsertEnvironment(KTextEditor::View *view)
{
	if(!view) {
		return false;
	}

	// don't complete environment, if we are
	// still working inside the completion box
	KTextEditor::CodeCompletionInterface *codeCompletionInterface
	                                      = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(codeCompletionInterface && codeCompletionInterface->isCompletionActive()) {
		return false;
	}

	int row = view->cursorPosition().line();
	int col = view->cursorPositionVirtual().column();
	QString line = view->document()->line(row).left(col);

	int pos = m_regexpEnter.indexIn(line);
	if (pos != -1) {
		line = m_regexpEnter.cap(1);
		for(int i = 0; i < line.length(); ++i) {
			if(!line[i].isSpace()) {
				line[i] = ' ';
			}
		}

		QString envname, endenv;
		if(m_regexpEnter.cap(2) == "\\[") {
			envname = m_regexpEnter.cap(2);
			endenv = "\\]\n";
		}
		else {
			envname = m_regexpEnter.cap(4);
			endenv = m_regexpEnter.cap(2).replace("\\begin","\\end") + '\n';
		}

		if(shouldCompleteEnv(envname, view)) {
			QString item =  m_latexCommands->isListEnv(envname) ? "\\item " : QString();
			view->document()->insertText(KTextEditor::Cursor(row, col), '\n' + line + m_envAutoIndent + item + '\n' + line + endenv);
			view->setCursorPosition(KTextEditor::Cursor(row + 1, line.length() + m_envAutoIndent.length() + item.length()));
			return true;
		}
	}
	return false;
}

bool EditorExtension::shouldCompleteEnv(const QString &env, KTextEditor::View *view)
{
	KILE_DEBUG() << "===EditorExtension::shouldCompleteEnv( " << env << " )===";
	QRegExp reTestBegin,reTestEnd;
	if(env == "\\[") {
		KILE_DEBUG() << "display style";
		reTestBegin.setPattern("(?:[^\\\\]|^)\\\\\\[");
		// the first part is a non-capturing bracket (?:...) and we check if we don't have a backslash in front,
		//  or that we are at the begin of the line
		reTestEnd.setPattern("(?:[^\\\\]|^)\\\\\\]");
	}
	else {
		reTestBegin.setPattern("(?:[^\\\\]|^)\\\\begin\\s*\\{" + QRegExp::escape(env) + "\\}");
		reTestEnd.setPattern("(?:[^\\\\]|^)\\\\end\\s*\\{" + QRegExp::escape(env) + "\\}");
	}

	int num = view->document()->lines();
	int numBeginsFound = 0;
	int numEndsFound = 0;
	KTextEditor::Cursor cursor = view->cursorPosition();
	int realLine = cursor.line();

	for(int i = realLine; i < num; ++i) {
		numBeginsFound += view->document()->line(i).count(reTestBegin);
		numEndsFound += view->document()->line(i).count(reTestEnd);
		KILE_DEBUG() << "line is " << i <<  " numBeginsFound = " << numBeginsFound <<  " , " << "numEndsFound = " << numEndsFound;
		if(numEndsFound >= numBeginsFound) {
			return false;
		}
		else if(numEndsFound == 0 && numBeginsFound > 1) {
			return true;
		}
		else if(numBeginsFound > 2 || numEndsFound > 1) {
			return true; // terminate the search
		}
	}

	return true;
}

QString EditorExtension::getWhiteSpace(const QString &s)
{
	QString whitespace = s;

	for(int i = 0; i < whitespace.length(); ++i) {
		if(!whitespace[i].isSpace()) {
			whitespace[i] = ' ';
		}
	}
	return whitespace;
}

//////////////////// inside verbatim commands ////////////////////

bool EditorExtension::insideVerbatim(KTextEditor::View *view)
{
	int rowEnv, colEnv;
	QString nameEnv;

	if(findOpenedEnvironment(rowEnv, colEnv, nameEnv, view)) {
		if(m_latexCommands->isVerbatimEnv(nameEnv)) {
			return true;
		}
	}

	return false;
}

bool EditorExtension::insideVerb(KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return false;
	}

	// get current position
	int row, col;
	KTextEditor::Cursor cursor = view->cursorPosition();
	row = cursor.line();
	col = cursor.column();

	int startpos = 0;
	QString textline = getTextLineReal(view->document(),row);
	QRegExp reg("\\\\verb(\\*?)(.)");
	while(true) {
		int pos = textline.indexOf(reg,startpos);
		if(pos < 0 || col < pos + 6 + reg.cap(1).length()) {
			return false;
		}
		pos = textline.indexOf(reg.cap(2), pos + 6 + reg.cap(1).length());
		if(pos < 0 || col <= pos) {
			return true;
		}

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

void EditorExtension::gotoSectioning(bool backwards, KTextEditor::View *view)
{
	view = determineView(view);
	if(!view) {
		return;
	}

	int rowFound, colFound;
	if( view && view->document()->isModified() ){ // after saving, the document structure is the current one, so in this case we don't need to update it
		m_ki->viewManager()->updateStructure(true);
	}
	if(m_ki->structureWidget()->findSectioning(NULL,view->document(), view->cursorPosition().line(), view->cursorPosition().column(), backwards, false, rowFound, colFound)) {
		view->setCursorPosition(KTextEditor::Cursor(rowFound, colFound));
	}
}

//////////////////// sectioning popup ////////////////////

void EditorExtension::sectioningCommand(KileWidget::StructureViewItem *item, int id)
{
	KTextEditor::View *view = determineView(NULL);
	if(!view) {
		return;
	}

	if(!item) {
		return;
	}
	KTextEditor::Document *doc = view->document();

	// try to determine the whole secting
	// get the start auf the selected sectioning
	int row, col, row1, col1, row2, col2;
	row = row1 = item->startline() - 1;
	col = col1 = item->startcol() - 1;

        // FIXME tbraun make this more clever, introdoce in kiledocinfo a flag which can be easily queried for that, so that we don'
	// check, if the document was changed in the meantime
	QRegExp reg( "\\\\(part|chapter|section|subsection|subsubsection|paragraph|subparagraph)\\*?\\s*(\\{|\\[)" );
	QString textline = getTextLineReal(doc,row1);
	if(reg.indexIn(textline, col1) != col1) {
		m_ki->errorHandler()->clearMessages();
		m_ki->errorHandler()->printMessage(KileTool::Error,
		       i18n("The document was modified and the structure view should be updated, before starting such an operation."),
		       i18n("Structure View Error") );
		return;
	}

	// increase cursor position and find the following sectioning command
	if(!increaseCursorPosition(doc, row, col)) {
		return;
	}
	if (!m_ki->structureWidget()->findSectioning(item, doc, row, col, false, true, row2, col2)){
		// or the end of the document
		// if there is a '\end{document} command, we have to exclude it
		if (!findEndOfDocument(doc, row, col, row2, col2)) {
			row2 = doc->lines() - 1;
			col2 = 0;
		}
	}

	// clear selection and make cursor position visible
	view->removeSelection();
 	view->setCursorPosition(KTextEditor::Cursor(row1, col1));

	QString text;
	doc->startEditing();
	switch (id) {
		case KileWidget::StructureWidget::SectioningCut:
			KApplication::clipboard()->setText(doc->text(KTextEditor::Range(row1, col1, row2, col2)));  // copy -> clipboard
			doc->removeText(KTextEditor::Range(row1, col1, row2, col2));                                  // delete
		break;
		case KileWidget::StructureWidget::SectioningCopy:
			KApplication::clipboard()->setText(doc->text(KTextEditor::Range(row1, col1, row2, col2)));  // copy -> clipboard
		break;
		case KileWidget::StructureWidget::SectioningPaste:
			text = KApplication::clipboard()->text();                              // clipboard -> text
			if(!text.isEmpty()) {
				view->setCursorPosition(KTextEditor::Cursor(row2, col2));                             // insert
				view->insertText(text + '\n');
			}
		break;
		case KileWidget::StructureWidget::SectioningSelect:
			view->setSelection(KTextEditor::Range(row1, col1, row2, col2));                                // select
		break;
		case KileWidget::StructureWidget::SectioningDelete:
			doc->removeText(KTextEditor::Range(row1, col1, row2, col2));                                  // delete
		break;
		case KileWidget::StructureWidget::SectioningComment:
			commentLaTeX(doc, KTextEditor::Range(row1, col1, row2, col2));
		break;
		case KileWidget::StructureWidget::SectioningPreview:
			view->setSelection(KTextEditor::Range(row1, col1, row2, col2));                               // quick preview
			m_ki->quickPreview()->previewSelection(view, false);
			view->removeSelection();
		break;
	}
	doc->endEditing();

	// update structure view, because it has changed
	if(id == KileWidget::StructureWidget::SectioningDelete || id == KileWidget::StructureWidget::SectioningComment) {
		m_ki->viewManager()->updateStructure(true);
	}

}

bool EditorExtension::findEndOfDocument(KTextEditor::Document *doc, int row, int col, int &rowFound, int &colFound)
{

	KTextEditor::SearchInterface *iface;
	iface = dynamic_cast<KTextEditor::SearchInterface*>(doc);
	if(!iface) {
		return false;
	}

	KTextEditor::Range documentRange(KTextEditor::Cursor(row, col), doc->documentEnd());
	QVector<KTextEditor::Range> foundRanges = iface->searchText(documentRange, "\\end{document}");

	if(foundRanges.size() >= 1) {
		KTextEditor::Range range = foundRanges.first();
		if(range.isValid()) {
			rowFound = range.start().line();
			colFound = range.start().column();
			return true;
		}
	}

	return false;
}

QString EditorExtension::extractIndentationString(KTextEditor::View *view, int line)
{
	KTextEditor::Document* doc = view->document();

	if(!doc) {
		return QString();
	}

	const QString lineString = doc->line(line);
	int lastWhiteSpaceCharIndex = -1;

	for(int i = 0; i < lineString.length(); ++i) {
		if(!lineString[i].isSpace()) {
			break;
		}
		++lastWhiteSpaceCharIndex;
	}
	return lineString.left(lastWhiteSpaceCharIndex + 1);
}

}

#include "editorextension.moc"
