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
namespace KileAction {
class TagData;
}

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
    void gotoBullet(bool backwards, KTextEditor::View *view = Q_NULLPTR);
    void selectLine(int line,KTextEditor::View *view = Q_NULLPTR);
    bool replaceLine(int line, const QString &s, KTextEditor::View *view = Q_NULLPTR);

    void gotoEnvironment(bool backwards, KTextEditor::View *view = Q_NULLPTR);
    void matchEnvironment(KTextEditor::View *view = Q_NULLPTR);
    void closeEnvironment(KTextEditor::View *view = Q_NULLPTR);
    void closeAllEnvironments(KTextEditor::View *view = Q_NULLPTR);
    void selectEnvironment(bool inside, KTextEditor::View *view = Q_NULLPTR);
    void deleteEnvironment(bool inside, KTextEditor::View *view = Q_NULLPTR);
    QString autoIndentEnvironment() {
        return m_envAutoIndent;
    }

    bool hasTexgroup(KTextEditor::View *view = Q_NULLPTR);
    void gotoTexgroup(bool backwards, KTextEditor::View *view = Q_NULLPTR);
    void selectTexgroup(bool inside, KTextEditor::View *view = Q_NULLPTR);
    void deleteTexgroup(bool inside, KTextEditor::View *view = Q_NULLPTR);
    KTextEditor::Range texgroupRange(bool inside=true, KTextEditor::View *view = Q_NULLPTR);
    QString getTexgroupText(bool inside=true, KTextEditor::View *view = Q_NULLPTR);

    /**
     * Returns a (potentially) translated list of options for inserting double quotes
     */
    const QStringList doubleQuotesListI18N() {
        return m_quoteListI18N;
    }

    // get current word
    bool getCurrentWord(KTextEditor::Document *doc, int row, int col, SelectMode mode, QString &word, int &x1, int &x2);
    QString getEnvironmentText(int &row, int &col, QString &name, KTextEditor::View *view = Q_NULLPTR);
    bool hasEnvironment(KTextEditor::View *view = Q_NULLPTR);

    KTextEditor::Range environmentRange(bool inside=false, KTextEditor::View *view = Q_NULLPTR);
    QString environmentName(KTextEditor::View *view = Q_NULLPTR);
    QString environmentText(bool inside=false, KTextEditor::View *view = Q_NULLPTR);

    KTextEditor::Range wordRange(const KTextEditor::Cursor &cursor, bool latexCommand=false, KTextEditor::View *view = Q_NULLPTR);
    QString word(const KTextEditor::Cursor &cursor, bool latexCommand=false, KTextEditor::View *view = Q_NULLPTR);

    KTextEditor::Range findCurrentParagraphRange(KTextEditor::View *view, bool wholeLines = true);
    QString getParagraphText(KTextEditor::View *view);
    int prevNonEmptyLine(int line, KTextEditor::View *view = Q_NULLPTR);
    int nextNonEmptyLine(int line, KTextEditor::View *view = Q_NULLPTR);

    // complete environment
    bool eventInsertEnvironment(KTextEditor::View *view);

    // mathgroup
    QString getMathgroupText(uint &row, uint &col, KTextEditor::View *view = Q_NULLPTR);
    QString getMathgroupText(KTextEditor::View *view = Q_NULLPTR);
    bool hasMathgroup(KTextEditor::View *view = Q_NULLPTR);
    KTextEditor::Range  mathgroupRange(KTextEditor::View *view = Q_NULLPTR);

    bool moveCursorRight(KTextEditor::View *view = Q_NULLPTR);
    bool moveCursorLeft(KTextEditor::View *view = Q_NULLPTR);
    bool moveCursorUp(KTextEditor::View *view = Q_NULLPTR);
    bool moveCursorDown(KTextEditor::View *view = Q_NULLPTR);

public Q_SLOTS:
    void insertIntelligentNewline(KTextEditor::View *view = Q_NULLPTR);

    void selectEnvInside() {
        selectEnvironment(true);
    }
    void selectEnvOutside() {
        selectEnvironment(false);
    }
    void deleteEnvInside() {
        deleteEnvironment(true);
    }
    void deleteEnvOutside() {
        deleteEnvironment(false);
    }
    void gotoBeginEnv() {
        gotoEnvironment(true);
    }
    void gotoEndEnv() {
        gotoEnvironment(false);
    }
    void matchEnv() {
        matchEnvironment();
    }
    void closeEnv() {
        closeEnvironment();
    }
    void closeAllEnv() {
        closeAllEnvironments();
    }

    void selectTexgroupInside() {
        selectTexgroup(true);
    }
    void selectTexgroupOutside() {
        selectTexgroup(false);
    }
    void deleteTexgroupInside() {
        deleteTexgroup(true);
    }
    void deleteTexgroupOutside() {
        deleteTexgroup(false);
    }
    void gotoBeginTexgroup() {
        gotoTexgroup(true);
    }
    void gotoEndTexgroup() {
        gotoTexgroup(false);
    }
    void matchTexgroup(KTextEditor::View *view = Q_NULLPTR);
    void closeTexgroup(KTextEditor::View *view = Q_NULLPTR);

    void selectParagraph(KTextEditor::View *view = Q_NULLPTR, bool wholeLines = true);
    void selectLine(KTextEditor::View *view = Q_NULLPTR);
    void selectLines(int from, int to, KTextEditor::View *view = Q_NULLPTR);
    void selectWord(SelectMode mode = smTex, KTextEditor::View *view = Q_NULLPTR);
    void deleteParagraph(KTextEditor::View *view = Q_NULLPTR);
    void deleteEndOfLine(KTextEditor::View *view = Q_NULLPTR);
    void deleteWord(SelectMode mode = smTex, KTextEditor::View *view = Q_NULLPTR);

    void selectMathgroup(KTextEditor::View *view = Q_NULLPTR);
    void deleteMathgroup(KTextEditor::View *view = Q_NULLPTR);

    void nextBullet(KTextEditor::View* view = Q_NULLPTR);
    void prevBullet(KTextEditor::View* view = Q_NULLPTR);
    void insertBullet(KTextEditor::View* view = Q_NULLPTR);

    void gotoNextParagraph(KTextEditor::View *view = Q_NULLPTR);
    void gotoPrevParagraph(KTextEditor::View *view = Q_NULLPTR);

    void gotoNextSectioning();
    void gotoPrevSectioning();
    void gotoSectioning(bool backwards, KTextEditor::View *view = Q_NULLPTR);
    void sectioningCommand(KileWidget::StructureViewItem *item, int id);

    bool insertDoubleQuotes(KTextEditor::View *view = Q_NULLPTR);
    void initDoubleQuotes();

    bool insertLatexFromUnicode(unsigned short rep, KTextEditor::View *view);
    bool insertSpecialCharacter(const QString& texString, KTextEditor::View *view = Q_NULLPTR, const QString& dep = "");

    void insertIntelligentTabulator(KTextEditor::View *view = Q_NULLPTR);

    void moveCursorToLastPositionInCurrentLine(KTextEditor::View *view = Q_NULLPTR);
    void keyReturn(KTextEditor::View *view = Q_NULLPTR);
    void commentLaTeX(KTextEditor::Document* document, const KTextEditor::Range &range);

    void goToLine(int line, KTextEditor::View *view = Q_NULLPTR);

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
    bool findCurrentTexParagraph(int &startline, int& startcolumn, int &endline, int& endcolumn, KTextEditor::View *view);

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
