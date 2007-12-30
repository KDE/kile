/***************************************************************************
    date                 : Feb 20 2007
    version              : 0.45
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

#ifndef KILEEDIT_H
#define KILEEDIT_H

#include <qobject.h>
#include <qregexp.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#include <ktexteditor/document.h>

#include "kilestructurewidget.h"
#include "codecompletion.h"
#include "latexcmd.h"         

/**
  *@author Holger Danielsson
  */

class KileInfo;
namespace KileAction { class TagData; }

namespace KileDocument
{
  
class EditorExtension : public QObject
{
	Q_OBJECT

public:
	EditorExtension(KileInfo *);
	~EditorExtension();

	enum EnvType { EnvNone, EnvList, EnvTab, EnvCrTab };

	enum SelectMode { smTex, smLetter, smWord, smNospace };

	void readConfig(void);

	void insertTag(const KileAction::TagData& data, KTextEditor::View *view);

	QString getTextLineReal(KTextEditor::Document *doc, uint row);
	void gotoBullet(bool backwards, KTextEditor::View *view = 0L);

	void gotoEnvironment(bool backwards, KTextEditor::View *view = 0L);
	void matchEnvironment(KTextEditor::View *view = 0L);
	void closeEnvironment(KTextEditor::View *view = 0L);
	void closeAllEnvironments(KTextEditor::View *view = 0L);
	void selectEnvironment(bool inside, KTextEditor::View *view = 0L);
	void deleteEnvironment(bool inside, KTextEditor::View *view = 0L);
	QString autoIndentEnvironment() { return m_envAutoIndent; }

	void gotoTexgroup(bool backwards, KTextEditor::View *view = 0L);
	void selectTexgroup(bool inside, KTextEditor::View *view = 0L);
	void deleteTexgroup(bool inside, KTextEditor::View *view = 0L);

	const QStringList doubleQuotesList() { return m_quoteList; }
	
	// get current word
	bool getCurrentWord(KTextEditor::Document *doc,uint row,uint col, SelectMode mode,QString &word,uint &x1,uint &x2);
	QString getEnvironmentText(uint &row, uint &col, QString &name, KTextEditor::View *view = 0L);
	bool hasEnvironment(KTextEditor::View *view = 0L);

	// complete environment
	bool eventInsertEnvironment(KTextEditor::View *view);

	// mathgroup
	QString getMathgroupText(uint &row, uint &col, KTextEditor::View *view = 0L);
	bool hasMathgroup(KTextEditor::View *view = 0L);
	
public Q_SLOTS:
	void insertIntelligentNewline(KTextEditor::View *view = 0L);

	void selectEnvInside() { selectEnvironment(true); }
	void selectEnvOutside() { selectEnvironment(false); }
	void deleteEnvInside() { deleteEnvironment(true); }
	void deleteEnvOutside() {deleteEnvironment(false); }
	void gotoBeginEnv() { gotoEnvironment(true); }
	void gotoEndEnv() { gotoEnvironment(false); }
	void matchEnv() { matchEnvironment(); }
	void closeEnv() {closeEnvironment(); }
	void closeAllEnv() {closeAllEnvironments(); }
	
	void selectTexgroupInside() { selectTexgroup(true); }
	void selectTexgroupOutside() { selectTexgroup(false); }
	void deleteTexgroupInside() { deleteTexgroup(true); }
	void deleteTexgroupOutside() { deleteTexgroup(false); }
	void gotoBeginTexgroup() { gotoTexgroup(true); }
	void gotoEndTexgroup() { gotoTexgroup(false); }
	void matchTexgroup(KTextEditor::View *view = 0L);
	void closeTexgroup(KTextEditor::View *view = 0L);

	void selectParagraph(KTextEditor::View *view = 0L);
	void selectLine(KTextEditor::View *view = 0L);
	void selectWord(SelectMode mode = smTex, KTextEditor::View *view = 0L);
	void deleteParagraph(KTextEditor::View *view = 0L);
	void deleteEndOfLine(KTextEditor::View *view = 0L);
	void deleteWord(SelectMode mode = smTex, KTextEditor::View *view = 0L);

	void selectMathgroup(KTextEditor::View *view = 0L);
	void deleteMathgroup(KTextEditor::View *view = 0L);

	void nextBullet(KTextEditor::View* view = 0L);
	void prevBullet(KTextEditor::View* view = 0L);
	void insertBullet(KTextEditor::View* view = 0L);

	void gotoLine(KTextEditor::View *view = 0L);
	void gotoNextParagraph(KTextEditor::View *view = 0L);
	void gotoPrevParagraph(KTextEditor::View *view = 0L);

	void gotoNextSectioning();
	void gotoPrevSectioning();
	void sectioningCommand(KileListViewItem *item, int id);

	bool insertDoubleQuotes();
	void initDoubleQuotes();

	void insertIntelligentTabulator();
private:

	enum EnvTag { EnvBegin, EnvEnd };

	enum EnvPos { EnvLeft, EnvInside, EnvRight };

	enum MathTag { mmNoMathMode, mmMathDollar, mmMathParen, mmDisplaymathParen, mmMathEnv, mmDisplaymathEnv };

	struct EnvData 
	{
		uint row;
		uint col;
		QString name;
		uint len;
		EnvPos cpos;
		EnvTag tag;
		EnvType type;
	};

	struct MathData 
	{
		uint row;
		uint col;
		uint len;
		uint numdollar;
		MathTag tag;
		QString envname;
	};

	struct BracketData
	{
		uint row;
		uint col;
		bool open;
	};

	QRegExp m_reg;
	bool m_overwritemode;
	QString m_envAutoIndent;

	// change cursor position
	bool increaseCursorPosition(KTextEditor::Document *doc, uint &row, uint &col);
	bool decreaseCursorPosition(KTextEditor::Document *doc, uint &row, uint &col);

	// check position
	bool isValidBackslash(KTextEditor::Document *doc, uint row, uint col);
	bool isCommentPosition(KTextEditor::Document *doc, uint row, uint col);
	bool isEnvironmentPosition(KTextEditor::Document *doc, uint row, uint col,EnvData &env);

	// find environment tags
	bool findBeginEnvironment(KTextEditor::Document *doc, uint row, uint col,EnvData &env);
	bool findEndEnvironment(KTextEditor::Document *doc, uint row, uint col,EnvData &env);
	bool findEnvironmentTag(KTextEditor::Document *doc, uint row, uint col,EnvData &env, bool backwards=false);
	bool findOpenedEnvironment(uint &row,uint &col, QString &envname, KTextEditor::View *view);
	QStringList findOpenedEnvironmentList(KTextEditor::View *view, bool position = false);

	// get current environment
	bool getEnvironment(bool inside, EnvData &envbegin, EnvData &envend,KTextEditor::View *view);
	bool expandSelectionEnvironment(bool inside, KTextEditor::View *view);

	// find brackets
	bool isBracketPosition(KTextEditor::Document *doc, uint row, uint col, BracketData &bracket);
	bool findOpenBracket(KTextEditor::Document *doc, uint row, uint col, BracketData &bracket);
	bool findCloseBracket(KTextEditor::Document *doc, uint row, uint col, BracketData &bracket);
	bool findCloseBracketTag(KTextEditor::Document *doc, uint row, uint col,BracketData &bracket);
	bool findOpenBracketTag(KTextEditor::Document *doc, uint row, uint col, BracketData &bracket);

	// find math tags
	bool isOpeningMathTagPosition(KTextEditor::Document *doc, uint row, uint col, MathData &mathdata);
	bool isClosingMathTagPosition(KTextEditor::Document *doc, uint row, uint col, MathData &mathdata);
	bool findOpenMathTag(KTextEditor::Document *doc, uint row, uint col, QRegExp &reg, MathData &mathdata);
	bool findCloseMathTag(KTextEditor::Document *doc, uint row, uint col, QRegExp &reg, MathData &mathdata);
	bool checkMathtags(const MathData &begin,const MathData &end);

	// mathgroup
	bool getMathgroup(KTextEditor::View *view, uint &row1, uint &col1, uint &row2, uint &col2);

	// get current Texgroup
	bool getTexgroup(bool inside, BracketData &open, BracketData &close, KTextEditor::View *view);

	// find current paragraph
	bool findCurrentTexParagraph(uint &startline, uint &endline, KTextEditor::View *view);

	// sectioning commands
	void gotoSectioning(bool backwards, KTextEditor::View *view = 0L);
	bool findEndOfDocument(KTextEditor::Document *doc, uint row,uint col, uint &rowFound, uint &colFound);

	// check environment type
	KileDocument::LatexCommands *m_latexCommands;	
	bool shouldCompleteEnv(const QString &envname, KTextEditor::View *view);
	QString getWhiteSpace(const QString &s);

	// verbatim text
	bool insideVerb(KTextEditor::View *view);
	bool insideVerbatim(KTextEditor::View *view);

	// complete environments
	QRegExp m_regexpEnter;
	
	// double Quotes
	bool m_dblQuotes;
	QStringList m_quoteList;
	QString m_leftDblQuote, m_rightDblQuote;
	
	// help
	void readHelpList(QString const &filename);

	KTextEditor::View *determineView(KTextEditor::View *);

	KileInfo	*m_ki;

//code completion
public Q_SLOTS:
	void completeWord();
	void completeEnvironment();
	void completeAbbreviation();

public:
	CodeCompletion* complete() const { return m_complete; }

private:
	CodeCompletion	*m_complete;
};

}

#endif
