/*********************************************************************************************
    copyright            : (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                               2008-2011 by Michel Ludwig (michel.ludwig@kdemail.net)
                               2012      by Holger Danielsson (holger.danielsson@versanet.de)
 *********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EDITOREXTENSION_H
#define EDITOREXTENSION_H

#include <QObject>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include <KTextEditor/Document>

#include "widgets/structurewidget.h"
#include "latexcmd.h"

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

	enum EnvType {EnvNone, EnvList, EnvTab, EnvCrTab};

	enum SelectMode {smTex, smLetter, smWord, smNospace};

	void readConfig();

	void insertTag(const KileAction::TagData& data, KTextEditor::View *view);

	QString getTextLineReal(KTextEditor::Document *doc, int row);
	void gotoBullet(bool backwards, KTextEditor::View *view = NULL);
	void selectLine(int line,KTextEditor::View *view = NULL);
	bool replaceLine(int line, const QString &s, KTextEditor::View *view = NULL);

	void gotoEnvironment(bool backwards, KTextEditor::View *view = NULL);
	void matchEnvironment(KTextEditor::View *view = NULL);
	void closeEnvironment(KTextEditor::View *view = NULL);
	void closeAllEnvironments(KTextEditor::View *view = NULL);
	void selectEnvironment(bool inside, KTextEditor::View *view = NULL);
	void deleteEnvironment(bool inside, KTextEditor::View *view = NULL);
	QString autoIndentEnvironment() { return m_envAutoIndent; }

	bool hasTexgroup(KTextEditor::View *view = NULL);
	void gotoTexgroup(bool backwards, KTextEditor::View *view = NULL);
	void selectTexgroup(bool inside, KTextEditor::View *view = NULL);
	void deleteTexgroup(bool inside, KTextEditor::View *view = NULL);
	KTextEditor::Range texgroupRange(bool inside=true, KTextEditor::View *view = NULL);
	QString getTexgroupText(bool inside=true, KTextEditor::View *view = NULL);

	/**
	 * Returns a (potentially) translated list of options for inserting double quotes
	 */
	const QStringList doubleQuotesListI18N() { return m_quoteListI18N; }

	// get current word
	bool getCurrentWord(KTextEditor::Document *doc, int row, int col, SelectMode mode, QString &word, int &x1, int &x2);
	QString getEnvironmentText(int &row, int &col, QString &name, KTextEditor::View *view = NULL);
	bool hasEnvironment(KTextEditor::View *view = NULL);

	KTextEditor::Range environmentRange(bool inside=false, KTextEditor::View *view = NULL);
	QString environmentName(KTextEditor::View *view = NULL);
	QString environmentText(bool inside=false, KTextEditor::View *view = NULL);

	KTextEditor::Range wordRange(const KTextEditor::Cursor &cursor, bool latexCommand=false, KTextEditor::View *view = NULL);
	QString word(const KTextEditor::Cursor &cursor, bool latexCommand=false, KTextEditor::View *view = NULL);

	KTextEditor::Range findCurrentParagraphRange(KTextEditor::View *view);
	QString getParagraphText(KTextEditor::View *view);
	int prevNonEmptyLine(int line, KTextEditor::View *view = NULL);
	int nextNonEmptyLine(int line, KTextEditor::View *view = NULL);

	// complete environment
	bool eventInsertEnvironment(KTextEditor::View *view);

	// mathgroup
	QString getMathgroupText(uint &row, uint &col, KTextEditor::View *view = NULL);
	QString getMathgroupText(KTextEditor::View *view = NULL);
	bool hasMathgroup(KTextEditor::View *view = NULL);
	KTextEditor::Range  mathgroupRange(KTextEditor::View *view = NULL);

	bool moveCursorRight(KTextEditor::View *view = NULL);
	bool moveCursorLeft(KTextEditor::View *view = NULL);
	bool moveCursorUp(KTextEditor::View *view = NULL);
	bool moveCursorDown(KTextEditor::View *view = NULL);

public Q_SLOTS:
	void insertIntelligentNewline(KTextEditor::View *view = NULL);

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
	void matchTexgroup(KTextEditor::View *view = NULL);
	void closeTexgroup(KTextEditor::View *view = NULL);

	void selectParagraph(KTextEditor::View *view = NULL);
	void selectLine(KTextEditor::View *view = NULL);
	void selectLines(int from, int to, KTextEditor::View *view = NULL);
	void selectWord(SelectMode mode = smTex, KTextEditor::View *view = NULL);
	void deleteParagraph(KTextEditor::View *view = NULL);
	void deleteEndOfLine(KTextEditor::View *view = NULL);
	void deleteWord(SelectMode mode = smTex, KTextEditor::View *view = NULL);

	void selectMathgroup(KTextEditor::View *view = NULL);
	void deleteMathgroup(KTextEditor::View *view = NULL);

	void nextBullet(KTextEditor::View* view = NULL);
	void prevBullet(KTextEditor::View* view = NULL);
	void insertBullet(KTextEditor::View* view = NULL);

	void gotoNextParagraph(KTextEditor::View *view = NULL);
	void gotoPrevParagraph(KTextEditor::View *view = NULL);

	void gotoNextSectioning();
	void gotoPrevSectioning();
	void gotoSectioning(bool backwards, KTextEditor::View *view = NULL);
	void sectioningCommand(KileWidget::StructureViewItem *item, int id);

	bool insertDoubleQuotes(KTextEditor::View *view = NULL);
	void initDoubleQuotes();

	bool insertLatexFromUnicode(unsigned short rep, KTextEditor::View *view);
	bool insertSpecialCharacter(const QString& texString, KTextEditor::View *view = NULL, const QString& dep = "");

	void insertIntelligentTabulator(KTextEditor::View *view = NULL);

	void moveCursorToLastPositionInCurrentLine(KTextEditor::View *view = NULL);
	void keyReturn(KTextEditor::View *view = NULL);
	void commentLaTeX(KTextEditor::Document* document, const KTextEditor::Range &range);

	void goToLine(int line, KTextEditor::View *view = NULL);

private:

	enum EnvTag {EnvBegin, EnvEnd};

	enum EnvPos {EnvLeft, EnvInside, EnvRight};

	enum MathTag {mmNoMathMode, mmMathDollar, mmMathParen, mmDisplaymathParen, mmMathEnv, mmDisplaymathEnv};

	enum CursorMove {MoveCursorLeft, MoveCursorRight, MoveCursorUp, MoveCursorDown};

	struct EnvData {
		int row;
		int col;
		QString name;
		int len;
		EnvPos cpos;
		EnvTag tag;
		EnvType type;
	};

	struct MathData {
		int row;
		int col;
		int len;
		unsigned int numdollar;
		MathTag tag;
		QString envname;
	};

	struct BracketData {
		int row;
		int col;
		bool open;
	};

	QRegExp m_reg;
	bool m_overwritemode;
	QString m_envAutoIndent;

	KileInfo	*m_ki;

	// complete environments
	QRegExp m_regexpEnter;

	// double Quotes
	bool m_dblQuotes;
	QStringList m_quoteListI18N;
	QList<QPair<QString, QString> > m_quoteList;
	QString m_leftDblQuote, m_rightDblQuote;

	// special chars
	bool m_specialCharacters;

	// change cursor position
	bool increaseCursorPosition(KTextEditor::Document *doc, int &row, int &col);
	bool decreaseCursorPosition(KTextEditor::Document *doc, int &row, int &col);
	bool moveCursor(KTextEditor::View *view, CursorMove direction);

	// check position
	bool isValidBackslash(KTextEditor::Document *doc, int row, int col);
	bool isCommentPosition(KTextEditor::Document *doc, int row, int col);
	bool isEnvironmentPosition(KTextEditor::Document *doc, int row, int col,EnvData &env);

	// find environment tags
	bool findBeginEnvironment(KTextEditor::Document *doc, int row, int col, EnvData &env);
	bool findEndEnvironment(KTextEditor::Document *doc, int row, int col, EnvData &env);
	bool findEnvironmentTag(KTextEditor::Document *doc, int row, int col, EnvData &env, bool backwards = false);
	bool findOpenedEnvironment(int &row, int &col, QString &envname, KTextEditor::View *view);
	QStringList findOpenedEnvironmentList(KTextEditor::View *view, bool position = false);

	// get current environment
	bool getEnvironment(bool inside, EnvData &envbegin, EnvData &envend,KTextEditor::View *view);
	bool expandSelectionEnvironment(bool inside, KTextEditor::View *view);

	// find brackets
	bool isBracketPosition(KTextEditor::Document *doc, int row, int col, BracketData &bracket);
	bool findOpenBracket(KTextEditor::Document *doc, int row, int col, BracketData &bracket);
	bool findCloseBracket(KTextEditor::Document *doc, int row, int col, BracketData &bracket);
	bool findCloseBracketTag(KTextEditor::Document *doc, int row, int col,BracketData &bracket);
	bool findOpenBracketTag(KTextEditor::Document *doc, int row, int col, BracketData &bracket);

	// find math tags
	bool isOpeningMathTagPosition(KTextEditor::Document *doc, uint row, uint col, MathData &mathdata);
	bool isClosingMathTagPosition(KTextEditor::Document *doc, uint row, uint col, MathData &mathdata);
	bool findOpenMathTag(KTextEditor::Document *doc, int row, int col, MathData &mathdata);
	bool findCloseMathTag(KTextEditor::Document *doc, int row, int col, MathData &mathdata);
	bool checkMathtags(const MathData &begin,const MathData &end);

	// mathgroup
	bool getMathgroup(KTextEditor::View *view, int &row1, int &col1, int &row2, int &col2);

	// get current Texgroup
	bool getTexgroup(bool inside, BracketData &open, BracketData &close, KTextEditor::View *view);

	// find current paragraph
	bool findCurrentTexParagraph(int &startline, int &endline, KTextEditor::View *view);

	// sectioning commands
	bool findEndOfDocument(KTextEditor::Document *doc, int row, int col, int &rowFound, int &colFound);

	// check environment type
	KileDocument::LatexCommands *m_latexCommands;
	bool shouldCompleteEnv(const QString &envname, KTextEditor::View *view);
	QString getWhiteSpace(const QString &s);

	// verbatim text
	bool insideVerb(KTextEditor::View *view);
	bool insideVerbatim(KTextEditor::View *view);

	// help
	void readHelpList(QString const &filename);

	KTextEditor::View *determineView(KTextEditor::View *view);
	void deleteRange(KTextEditor::Range &range, KTextEditor::View *view);

	QString extractIndentationString(KTextEditor::View *view, int line);
};

}

#endif
