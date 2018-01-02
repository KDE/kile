/**********************************************************************************************
  Copyright (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                2008-2016 by Michel Ludwig (michel.ludwig@kdemail.net)
***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "codecompletion.h"

#include <algorithm>

#include <QFile>
#include <QList>
#include <QRegExp>
#include <QStandardPaths>
#include <QTimer>

#include <KConfig>
#include <KLocalizedString>
#include <KTextEditor/Cursor>

#include "kiledebug.h"
#include "abbreviationmanager.h"
#include "documentinfo.h"
#include "editorextension.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kileconfig.h"

namespace KileCodeCompletion {

LaTeXCompletionModel::LaTeXCompletionModel(QObject *parent, KileCodeCompletion::Manager *manager,
        KileDocument::EditorExtension *editorExtension)
    : KTextEditor::CodeCompletionModel(parent), m_codeCompletionManager(manager), m_editorExtension(editorExtension), m_currentView(Q_NULLPTR)
{
    setHasGroups(false);
}

LaTeXCompletionModel::~LaTeXCompletionModel()
{
}

void LaTeXCompletionModel::completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range,
        InvocationType invocationType)
{
    if(!range.isValid()
            || (invocationType == AutomaticInvocation && !KileConfig::completeAuto())) {
        beginResetModel();
        m_completionList.clear();
        endResetModel();
        return;
    }
    Q_UNUSED(invocationType);
    m_currentView = view;
    KILE_DEBUG_CODECOMPLETION << "building model...";
    buildModel(view, range);
}

KTextEditor::Range LaTeXCompletionModel::updateCompletionRange(KTextEditor::View *view,
        const KTextEditor::Range &range)
{
    KILE_DEBUG_CODECOMPLETION << "updating model..." << view << range;
    KTextEditor::Range newRange = completionRange(view, view->cursorPosition());
    if(newRange.isValid()) {
        buildModel(view, newRange);
    }
    return newRange;
}

static inline bool isSpecialLaTeXCommandCharacter(const QChar& c) {
    return (c == '{' || c == '[' || c == '*' || c == ']' || c == '}');
}

static inline int specialLaTeXCommandCharacterOrdering(const QChar& c)
{
    switch(c.unicode()) {
    case '{':
        return 1;
    case '[':
        return 2;
    case ']':
        return 3;
    case '}':
        return 4;
    case '*':
        return 5;
    default: // does nothing
        break;
    }
    return 4; // must be 'isLetterOrNumber()' now
}

static bool laTeXCommandLessThan(const QString& s1, const QString& s2)
{
    for(int i = 0; i < s1.length(); ++i) {
        if(i >= s2.length()) {
            return false;
        }
        const QChar c1 = s1.at(i);
        const QChar c2 = s2.at(i);

        if(c1 == c2) {
            continue;
        }
        if(c1.isLetterOrNumber()) {
            if(isSpecialLaTeXCommandCharacter(c2)) {
                return false;
            }
            else {
                return (c1 < c2);
            }
        }
        else if(isSpecialLaTeXCommandCharacter(c1)) {
            if(isSpecialLaTeXCommandCharacter(c2)) {
                return (specialLaTeXCommandCharacterOrdering(c1)
                        < specialLaTeXCommandCharacterOrdering(c2));
            }
            else if(c2.isLetterOrNumber()) {
                return true;
            }
            else {
                return (c1 < c2);
            }
        }
    }
    return true;
}

void LaTeXCompletionModel::buildModel(KTextEditor::View *view, const KTextEditor::Range &range)
{
    QString completionString = view->document()->text(range);
    KILE_DEBUG_CODECOMPLETION << "Text in completion range: " << completionString;
    m_completionList.clear();

    if(completionString.startsWith('\\')) {
        m_completionList = m_codeCompletionManager->getLaTeXCommands();
        m_completionList += m_codeCompletionManager->getLocallyDefinedLaTeXCommands(view);
    }
    else {
        KTextEditor::Cursor latexCommandStart = determineLaTeXCommandStart(view->document(),
                                                view->cursorPosition());
        if(!latexCommandStart.isValid()) {
            return;
        }
        QString leftSubstring = view->document()->text(KTextEditor::Range(latexCommandStart,
                                view->cursorPosition()));
        // check whether we are supposed to build a model for reference or citation completion
        int citationIndex = leftSubstring.indexOf(m_codeCompletionManager->m_citeRegExp);
        int referenceIndex = leftSubstring.indexOf(m_codeCompletionManager->m_referencesRegExp);
        if(referenceIndex != -1) {
            //FIXME: the retrieval of labels and BibTeX entries has to be revised!
            m_completionList = m_codeCompletionManager->m_ki->allLabels();
        }
        else if(citationIndex != -1) {
            m_completionList = m_codeCompletionManager->m_ki->allBibItems();
        }
    }
    beginResetModel();
    filterModel(completionString);
    std::sort(m_completionList.begin(), m_completionList.end(), laTeXCommandLessThan);
    endResetModel();
}

KTextEditor::Cursor LaTeXCompletionModel::determineLaTeXCommandStart(KTextEditor::Document *doc,
        const KTextEditor::Cursor& position) const
{
    QString line = doc->line(position.line());
// 	QRegExp completionStartRegExp("((\\s|^)?)((\\\\\\w*)|(\\w+))$");
// 	QRegExp completionStartRegExp("((\\\\\\w*)|([^\\\\]\\b\\w+))$");
// 	QRegExp completionStartRegExp("(\\\\\\w*)[^\\\\]*$");

    // TeX allows '.' characters inside citation labels (bug 266670)
    QRegExp completionStartRegExp("(\\\\([\\s\\{\\}\\[\\]\\w,.=\"'~:]|(\\&)|(\\$)|(\\%)(\\#)(\\_)|(\\{)|(\\})|(\\backslash)|(\\^)|(\\[)|(\\]))*)$");
    completionStartRegExp.setMinimal(true);
    QString leftSubstring = line.left(position.column());
    KILE_DEBUG_CODECOMPLETION << "leftSubstring: " << leftSubstring;
    int startPos = completionStartRegExp.lastIndexIn(leftSubstring);
    if(startPos >= 0) {
        return KTextEditor::Cursor(position.line(), startPos);
    }
    else {
        return KTextEditor::Cursor::invalid();
    }
}

bool LaTeXCompletionModel::isWithinLaTeXCommand(KTextEditor::Document *doc, const KTextEditor::Cursor& commandStart,
        const KTextEditor::Cursor& cursorPosition) const
{
    QString commandText = doc->text(KTextEditor::Range(commandStart, cursorPosition));
    int numOpenSquareBrackets = commandText.count(QRegExp("[^\\\\]\\["));
    int numClosedSquareBrackets = commandText.count(QRegExp("[^\\\\]\\]"));
    int numOpenCurlyBrackets = commandText.count(QRegExp("[^\\\\]\\{"));
    int numClosedCurlyBrackets = commandText.count(QRegExp("[^\\\\]\\}"));
    if(numOpenSquareBrackets != numClosedSquareBrackets || numOpenCurlyBrackets != numClosedCurlyBrackets) {
        return true;
    }
    if(numOpenSquareBrackets == 0 && numOpenCurlyBrackets == 0 && commandText.count(' ') == 0) {
        return true;
    }
    return false;
}

KTextEditor::Range LaTeXCompletionModel::completionRange(KTextEditor::View *view, const KTextEditor::Cursor &position)
{
    bool latexCompletion = true;
    QString line = view->document()->line(position.line());
    KTextEditor::Cursor startCursor = position;
    KTextEditor::Cursor endCursor = position;

    QRegExp completionEndRegExp("\\W|\\b|\\\\");

    int cursorPos = position.column();

    KTextEditor::Cursor latexCommandStart = determineLaTeXCommandStart(view->document(), position);
    KILE_DEBUG_CODECOMPLETION << "LaTeX command start " << latexCommandStart;
    if(!latexCommandStart.isValid() || !isWithinLaTeXCommand(view->document(), latexCommandStart, position)) {
        return KTextEditor::Range::invalid();
    }
    QString completionString = view->document()->text(KTextEditor::Range(latexCommandStart,
                               position));
    KILE_DEBUG_CODECOMPLETION << "completionString " << completionString;
    //check whether we are completing a citation of reference
    if(completionString.indexOf(m_codeCompletionManager->m_citeRegExp) != -1
            || completionString.indexOf(m_codeCompletionManager->m_referencesRegExp) != -1) {
        KILE_DEBUG_CODECOMPLETION << "found citation or reference!";
        int openBracketIndex = completionString.indexOf('{');
        if(openBracketIndex != -1) {
            // TeX allows '.' characters inside citation labels (bug 266670)
            QRegExp labelListRegExp("\\s*(([:.\\w]+)|([:.\\w]+(\\s*,\\s*[:.\\w]*)+))");
            labelListRegExp.setMinimal(false);
            int column = openBracketIndex + 1;
            KILE_DEBUG_CODECOMPLETION << "open bracket column + 1: " << column;
            KILE_DEBUG_CODECOMPLETION << labelListRegExp.indexIn(completionString, openBracketIndex + 1);
            if(labelListRegExp.indexIn(completionString, openBracketIndex + 1) == openBracketIndex + 1
                    && labelListRegExp.matchedLength() + openBracketIndex + 1 == completionString.length()) {
                QRegExp lastCommaRegExp(",\\s*");
                int lastCommaIndex = lastCommaRegExp.lastIndexIn(completionString);
                if(lastCommaIndex >= 0) {
                    KILE_DEBUG_CODECOMPLETION << "last comma found at: " << lastCommaIndex;
                    column =  lastCommaIndex + lastCommaRegExp.matchedLength();
                }
            }
            KILE_DEBUG_CODECOMPLETION << labelListRegExp.errorString();
            startCursor.setColumn(latexCommandStart.column() + column);
            latexCompletion = false;
        }
        else {
            startCursor = latexCommandStart;
        }
    }
    else {
        startCursor = latexCommandStart;
    }

    int endPos = line.indexOf(completionEndRegExp, cursorPos);
    KILE_DEBUG_CODECOMPLETION << "endPos" << endPos;
    if(endPos >= 0) {
        endCursor.setColumn(endPos);
    }
    KTextEditor::Range completionRange(startCursor, endCursor);
    int rangeLength = endCursor.column() - startCursor.column();

    if(latexCompletion && KileConfig::completeAuto() && rangeLength < KileConfig::completeAutoThreshold() + 1) { // + 1 for the command backslash
        KILE_DEBUG_CODECOMPLETION << "not reached the completion threshold yet";
        return KTextEditor::Range::invalid();
    }
    KILE_DEBUG_CODECOMPLETION << "returning completion range: " << completionRange;
    return completionRange;
}

bool LaTeXCompletionModel::shouldStartCompletion(KTextEditor::View *view, const QString &insertedText,
        bool userInsertion, const KTextEditor::Cursor &position)
{
    Q_UNUSED(view);
    Q_UNUSED(position);
    if(!KileConfig::completeAuto()) {
        return false;
    }

    if(insertedText.isEmpty()) {
        return false;
    }

    if(insertedText.endsWith('{')) {
        return true;
    }
    else {
        return CodeCompletionModelControllerInterface::shouldStartCompletion(view, insertedText, userInsertion, position);
    }
}

bool LaTeXCompletionModel::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range,
        const QString &currentCompletion)
{
    Q_UNUSED(currentCompletion);
    if(view->cursorPosition() < range.start() || view->cursorPosition() > range.end()
            || m_completionList.size() == 0) {
        return true;
    }
    return false;
}

QString LaTeXCompletionModel::filterString(KTextEditor::View *view, const KTextEditor::Range &range,
        const KTextEditor::Cursor &position)
{
    Q_UNUSED(position);
    KILE_DEBUG_CODECOMPLETION << "range: " << range;
    KILE_DEBUG_CODECOMPLETION << "text: " << (range.isValid() ? view->document()->text(range)
                              : "(invalid range)");

    return "";
}

QVariant LaTeXCompletionModel::data(const QModelIndex& index, int role) const
{
    switch(role) {
    case Qt::DisplayRole:
        if(index.column() != KTextEditor::CodeCompletionModel::Name) {
            return QVariant();
        }
        return m_completionList.at(index.row());
    case InheritanceDepth:
        return index.row();
    }

    return QVariant();
}

QModelIndex LaTeXCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= m_completionList.count() || column < 0 || column >= ColumnCount || parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

int LaTeXCompletionModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return m_completionList.size();
}

void LaTeXCompletionModel::filterModel(const QString& text)
{
    QMutableStringListIterator it(m_completionList);
    while(it.hasNext()) {
        QString string = it.next();
        if(!string.startsWith(text)) {
            it.remove();
        }
    }
}

void LaTeXCompletionModel::executeCompletionItem(KTextEditor::View *view,
        const KTextEditor::Range& word, const QModelIndex &index) const
{
    KTextEditor::Document *document = view->document();
    KTextEditor::Cursor startCursor = word.start();
    const static QRegExp reEnv = QRegExp("^\\\\(begin|end)[^a-zA-Z]+");

    int cursorXPos = -1, cursorYPos = -1;
    QString completionText = data(index.sibling(index.row(), Name), Qt::DisplayRole).toString();

    QString textToInsert;
    int envIndex = reEnv.indexIn(completionText);
    if(completionText != "\\begin{}" && envIndex != -1) { // we are completing an environment
        QString prefix;
        prefix = document->text(KTextEditor::Range(startCursor.line(), 0,
                                startCursor.line(), word.start().column()));
        textToInsert = buildEnvironmentCompletedText(completionText, prefix, cursorYPos, cursorXPos);
        KILE_DEBUG_CODECOMPLETION << cursorYPos << ", " << cursorXPos;
    }
    else {
        textToInsert = buildRegularCompletedText(stripParameters(completionText), cursorYPos, cursorXPos, true);
    }
    // if there are brackets present immediately after 'word' (for example, due to auto-bracketing of
    // the editor), we still have to remove them
    QString replaceText = document->text(word);
    const int numberOfOpenSimpleBrackets = replaceText.count('(');
    const int numberOfOpenSquareBrackets = replaceText.count('[');
    const int numberOfOpenCurlyBrackets = replaceText.count('{');
    const int numberOfClosedSimpleBrackets = replaceText.count(')');
    const int numberOfClosedSquareBrackets = replaceText.count(']');
    const int numberOfClosedCurlyBrackets = replaceText.count('}');
    const int numberOfClosedBracketsLeft = (numberOfOpenSimpleBrackets - numberOfClosedSimpleBrackets)
                                           + (numberOfOpenSquareBrackets - numberOfClosedSquareBrackets)
                                           + (numberOfOpenCurlyBrackets - numberOfClosedCurlyBrackets);
    if(numberOfOpenSimpleBrackets >= numberOfClosedSimpleBrackets
            && numberOfOpenSquareBrackets >= numberOfClosedSquareBrackets
            && numberOfOpenCurlyBrackets >= numberOfClosedCurlyBrackets
            && document->lineLength(word.end().line()) >= word.end().column() + numberOfClosedBracketsLeft) {
        KTextEditor::Range bracketRange = KTextEditor::Range(word.end(), numberOfClosedBracketsLeft);

        QString bracketText = document->text(bracketRange);
        if(bracketText.count(")") == (numberOfOpenSimpleBrackets - numberOfClosedSimpleBrackets)
                && bracketText.count("]") == (numberOfOpenSquareBrackets - numberOfClosedSquareBrackets)
                && bracketText.count("}") == (numberOfOpenCurlyBrackets - numberOfClosedCurlyBrackets)) {
            document->removeText(bracketRange);
        }
    }

    // now do the real completion
    document->replaceText(word, textToInsert);
    //HACK, but it's impossible to do this otherwise
    if(KileConfig::completeCursor() && (cursorXPos > 0 || cursorYPos > 0)
            && m_currentView && document->views().contains(m_currentView)) {
        m_currentView->setCursorPosition(KTextEditor::Cursor(startCursor.line() + (cursorYPos >= 0 ? cursorYPos : 0),
                                         startCursor.column() + (cursorXPos >= 0 ? cursorXPos : 0)));
    }
}

QString LaTeXCompletionModel::filterLatexCommand(const QString &text, int &cursorYPos, int &cursorXPos)
{
    const static QRegExp reEnv = QRegExp("^\\\\(begin|end)[^a-zA-Z]+");

    cursorXPos = -1, cursorYPos = -1;
    QString textToInsert;
    int envIndex = reEnv.indexIn(text);
    if(text != "\\begin{}" && envIndex != -1) {
        textToInsert = buildEnvironmentCompletedText(text, QString(), cursorYPos, cursorXPos);
    }
    else {
        textToInsert = buildRegularCompletedText(stripParameters(text), cursorYPos, cursorXPos, true);
    }
    return textToInsert;
}

// strip all names enclosed in braces
// consider also beamer like stuff [<...>] and <...>
QString LaTeXCompletionModel::stripParameters(const QString &text) const
{
    QString s;
    bool ignore = false;

    for(int i = 0; i < text.length(); ++i) {
        QChar c = text[i];
        switch(c.unicode()) {
        case '[':
        case '{':
        case '(':
        case '<':
            s += c;
            ignore = true;
            break;
        case ']':
        case '}':
        case ')':
        case '>':
            s += c;
            ignore = false;
            break;
        case ',':
            s += c;
            break;
        default:
            if(!ignore) {
                s += c;
            }
            break;
        }
    }
    return s;
}

QString LaTeXCompletionModel::buildRegularCompletedText(const QString &text, int &cursorYPos, int &cursorXPos, bool checkGroup) const
{
    bool setCursor = true, setBullets = true;
    bool foundgroup = false;
    QString s;

    cursorXPos = -1;
    cursorYPos = -1;
    for(int i = 0; i < text.length(); ++i) {
        QChar c = text[i];
        switch(c.unicode()) {
        case '<':
        case '{':
        case '(':
        case '[': // insert character
            s += c;
            if(cursorXPos < 0) {
                // remember position after first brace
                if(c == '[' && (i + 1) < text.length() &&  text[i + 1] == '<') {
                    cursorXPos = i + 2;
                    s += text[i + 1];
                    i++;
                }// special handling for '[<'
                else {
                    cursorXPos = i + 1;
                }
                // insert bullet, if this is no cursorposition
                if((!setCursor) && setBullets && !(c == '[' && (i + 1) < text.length() &&  text[i + 1] == '<')) {
                    s += s_bullet;
                }
            }
            // insert bullets after following braces
            else if(setBullets && !(c == '[' && (i + 1) < text.length() &&  text[i + 1] == '<')) {
                s += s_bullet;
            }
            break;
        case '>':
        case '}':
        case ')':
        case ']':                    // insert character
            s += c;
            break;
        case ',':                    // insert character
            s += c;
            // insert bullet?
            if(setBullets) {
                s += s_bullet;
            }
            break;
        case '.': // if the last character is a point of a range operator,
            // it will be replaced by a space or a bullet surrounded by spaces
            if(checkGroup && (s.right(1) == ".")) {
                foundgroup = true;
                s.truncate(s.length() - 1);
                if(setBullets) {
                    s += ' ' + s_bullet + ' ';
                }
                else {
                    s += ' ';
                }
            }
            else {
                s += c;
            }
            break;
        default:                      // insert all other characters
            s += c;
            break;
        }
    }

    // some more work with groups and bullets
    if(s.length() >= 2 && checkGroup && foundgroup && (setBullets | setCursor)) {
        int pos = 0;

        // search for braces, brackets and parens
        switch(s[1].unicode()) {
        case 'l':
            if(s.left(6) == "\\left ") {
                pos = 5;
            }
            break;
        case 'b':
            if(s.left(6) == "\\bigl ") {
                pos = 5;
            }
            else if(s.left(7) == "\\biggl ") {
                pos = 6;
            }
            break;
        case 'B' :
            if(s.left(6) == "\\Bigl ") {
                pos = 5;
            }
            else if(s.left(7) == "\\Biggl ") {
                pos = 6;
            }
            break;
        }

        // update cursorposition and set bullet
        if(pos > 0) {
            if(setCursor) {
                cursorXPos = pos;
            }
            if(setBullets) {
                if(!setCursor) {
                    s.insert(pos, s_bullet);
                }
                s.append(s_bullet);
            }
        }
    }

    return s;
}

QString LaTeXCompletionModel::buildEnvironmentCompletedText(const QString &text, const QString &prefix,
        int &ypos, int &xpos) const
{
    static QRegExp reEnv = QRegExp("^\\\\(begin|end)\\{([^\\}]*)\\}([^\\\\]*)(.*)");

    if(reEnv.indexIn(text) == -1) {
        return text;
    }

    QString parameter = stripParameters(reEnv.cap(3));
    QString start = reEnv.cap(1);
    QString envname = reEnv.cap(2);
    QString remainder = reEnv.cap(4);
    QString whitespace = buildWhiteSpaceString(prefix);
    QString envIndent = m_editorExtension->autoIndentEnvironment();

    QString s = "\\" + start + "{" + envname + "}" + parameter + "\n";

    s += whitespace;
    if(start != "end") {
        s += envIndent;
    }

    if(!remainder.isEmpty()) {
        s += remainder + ' ';
    }

    if(KileConfig::completeBullets() && !parameter.isEmpty()) {
        s += s_bullet;
    }

    if(KileConfig::completeCloseEnv() && start != "end") {
        s += '\n' + whitespace + "\\end{" + envname + "}\n";
    }

    if(parameter.isEmpty()) {
        ypos = 1;
        xpos = envIndent.length() + ((!remainder.isEmpty()) ? remainder.length() + 1 : 0);
    }
    else {
        ypos = 0;
        if(parameter.left(2) == "[<") {
            xpos = 10 + envname.length();
        }
        else {
            xpos = 9 + envname.length();
        }
    }

    return s;
}

QString LaTeXCompletionModel::buildWhiteSpaceString(const QString &s) const
{
    QString whitespace = s;
    for(int i = 0; i < whitespace.length(); ++i) {
        if(!whitespace[i].isSpace()) {
            whitespace[i] = ' ';
        }
    }
    return whitespace;
}

AbbreviationCompletionModel::AbbreviationCompletionModel(QObject *parent, KileAbbreviation::Manager *manager)
    : KTextEditor::CodeCompletionModel(parent), m_abbreviationManager(manager)
{
    setHasGroups(false);
}

AbbreviationCompletionModel::~AbbreviationCompletionModel()
{
}

QModelIndex AbbreviationCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= m_completionList.count() || column < 0 || column >= ColumnCount || parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QVariant AbbreviationCompletionModel::data(const QModelIndex& index, int role) const
{
    if(index.column() != KTextEditor::CodeCompletionModel::Name) {
        return QVariant();
    }
    switch(role) {
    case Qt::DisplayRole:
        return m_completionList.at(index.row());
    }

    return QVariant();
}

int AbbreviationCompletionModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return m_completionList.size();
}

bool AbbreviationCompletionModel::shouldStartCompletion(KTextEditor::View *view, const QString &insertedText,
        bool userInsertion, const KTextEditor::Cursor &position)
{
    Q_UNUSED(view);
    Q_UNUSED(userInsertion);
    Q_UNUSED(position);

    int len = insertedText.length();
    QRegExp whitespace(" |\t");
    whitespace.setMinimal(true);
    int pos = insertedText.lastIndexOf(whitespace, -1);
    // 'pos' is less than or equal to 'len - 1'
    QString searchText = (pos >= 0 && pos < len) ? insertedText.right(len - pos - 1) : insertedText;

    return (KileConfig::completeAutoAbbrev() && m_abbreviationManager->abbreviationStartsWith(searchText));
}

bool AbbreviationCompletionModel::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range,
        const QString &currentCompletion)
{
    Q_UNUSED(currentCompletion);
    if(view->cursorPosition() < range.start() || view->cursorPosition() > range.end()
            || m_completionList.size() == 0) {
        return true;
    }
    return false;
}

void AbbreviationCompletionModel::completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range,
        InvocationType invocationType)
{
    if(!range.isValid()
            || (invocationType == AutomaticInvocation && !KileConfig::completeAutoAbbrev())) {
        beginResetModel();
        m_completionList.clear();
        endResetModel();
        return;
    }
    KILE_DEBUG_CODECOMPLETION << "building model...";
    buildModel(view, range, (invocationType == UserInvocation || invocationType == ManualInvocation));
}

KTextEditor::Range AbbreviationCompletionModel::updateCompletionRange(KTextEditor::View *view,
        const KTextEditor::Range &range)
{
    if(!range.isValid()) {
        beginResetModel();
        m_completionList.clear();
        endResetModel();
        return range;
    }

    KILE_DEBUG_CODECOMPLETION << "updating model...";
    KTextEditor::Range newRange = completionRange(view, view->cursorPosition());
    if(newRange.isValid()) {
        buildModel(view, newRange);
    }
    return newRange;
}

KTextEditor::Range AbbreviationCompletionModel::completionRange(KTextEditor::View *view,
        const KTextEditor::Cursor &position)
{
    QString insertedText = view->document()->line(position.line()).left(position.column());
    int len = insertedText.length();

    QRegExp whitespace(" |\t");
    whitespace.setMinimal(true);
    int pos = insertedText.lastIndexOf(whitespace,-1);
    QString searchText = (pos>=0 && pos<len-2) ? insertedText.right(len-pos-1) : insertedText;
    pos++;

    return KTextEditor::Range( position.line(), pos, position.line(),position.column() );
}

QString AbbreviationCompletionModel::filterString(KTextEditor::View *view,
        const KTextEditor::Range &range,
        const KTextEditor::Cursor &position)
{
    Q_UNUSED(view);
    Q_UNUSED(range);
    Q_UNUSED(position);
    return "";
}

void AbbreviationCompletionModel::executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range& word,
        const QModelIndex &index) const
{
    // replace abbreviation and take care of newlines
    QString completionText = data(index.sibling(index.row(), Name), Qt::DisplayRole).toString();
    completionText.replace("%n","\n");
    KTextEditor::Document *document = view->document();
    document->replaceText(word, completionText);

    // look if there is a %C-wish to place the cursor
    if (completionText.indexOf("%C") >= 0) {
        KTextEditor::Range searchrange = KTextEditor::Range(word.start(),document->lines()+1,0);
        QVector<KTextEditor::Range> rangevec = document->searchText(searchrange,"%C");
        if (rangevec.size() >= 1) {
            KTextEditor::Range range = rangevec.at(0);
            document->removeText(range);
            if (view) {
                view->setCursorPosition(range.start());
            }
        }
    }
}

void AbbreviationCompletionModel::buildModel(KTextEditor::View *view, const KTextEditor::Range &range,
        bool singleMatchMode)
{
    beginResetModel();
    m_completionList.clear();
    endResetModel();
    QString text = view->document()->text(range);
    KILE_DEBUG_CODECOMPLETION << text;
    if(text.isEmpty()) {
        return;
    }
    if(singleMatchMode && m_abbreviationManager->isAbbreviationDefined(text)) {
        m_completionList << m_abbreviationManager->getAbbreviationTextMatch(text);
        executeCompletionItem(view, range, index(0, 0));
    }
    else {
        m_completionList = m_abbreviationManager->getAbbreviationTextMatches(text);
        m_completionList.sort();
        if(m_completionList.size() == 1
                && m_abbreviationManager->isAbbreviationDefined(text)) {
            executeCompletionItem(view, range, index(0, 0));
        }
    }
}

Manager::Manager(KileInfo *info, QObject *parent)
    : QObject(parent), m_ki(info)
{
    m_firstConfig = true;
}

Manager::~Manager()
{
}

QStringList Manager::getLaTeXCommands() const
{
    return m_texWordList;
}

QStringList Manager::getLocallyDefinedLaTeXCommands(KTextEditor::View *view) const
{
    //FIXME: the retrieval of these commands has to be revised!
    KileDocument::TextInfo *textInfo = m_ki->docManager()->textInfoFor(view->document());
    if(!textInfo) {
        return QStringList();
    }
    return m_ki->allNewCommands(textInfo);
}

void Manager::readConfig(KConfig *config)
{
    Q_UNUSED(config);
    KILE_DEBUG_CODECOMPLETION << "======================";

    // reading the wordlists is only necessary at the first start
    // and when the list of files changes
    if(m_firstConfig || KileConfig::completeChangedLists() || KileConfig::completeChangedCommands()) {
        KILE_DEBUG_CODECOMPLETION << "   setting regexp for references...";
        buildReferenceCitationRegularExpressions();

        KILE_DEBUG_CODECOMPLETION << "   read wordlists...";
        // wordlists for Tex/Latex mode
        QStringList files = KileConfig::completeTex();
        m_texWordList = readCWLFiles(files, "tex");
        addUserDefinedLaTeXCommands(m_texWordList);

        // wordlist for dictionary mode
        files = KileConfig::completeDict();
        m_dictWordList = readCWLFiles(files, "dictionary");
        m_dictWordList.sort();

        // remember changed lists
        // FIXME: remove these hacks
        m_firstConfig = false;
        KileConfig::setCompleteChangedLists(false);
        KileConfig::setCompleteChangedCommands(false);
    }
}

void Manager::startLaTeXCompletion(KTextEditor::View *view)
{
    if(!view) {
        view = m_ki->viewManager()->currentTextView();
        if(!view) {
            return;
        }
    }

    KileDocument::TextInfo *textInfo = m_ki->docManager()->textInfoFor(view->document());
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(textInfo);
    if(!latexInfo) {
        return;
    }
    latexInfo->startLaTeXCompletion(view);
}

void Manager::textInserted(KTextEditor::View* view, const KTextEditor::Cursor& /* position */, const QString& text)
{
    // auto insert '$' if the user just typed a '$' character
    if (KileConfig::autoInsertDollar() && text == "$") {
        // code completion seems to be never active, so there is no need to
        // check KTextEditor::CodeCompletionInterface::isCompletionActive()
        KTextEditor::Cursor currentCursorPos = view->cursorPosition();
        view->document()->insertText(currentCursorPos, "$");
        view->setCursorPosition(currentCursorPos);
    }
}


void Manager::startLaTeXEnvironment(KTextEditor::View *view)
{
    if(!view) {
        view = m_ki->viewManager()->currentTextView();
        if(!view) {
            return;
        }
    }

    KTextEditor::Cursor cursor = view->cursorPosition();
    QString line = view->document()->line(cursor.line()).left(cursor.column());

    QRegExp regexp("\\\\b|\\\\be|\\\\beg|\\\\begi|\\\\begin|\\\\begin\\{|\\\\begin\\{([a-zA-z]*)");
    int pos = regexp.lastIndexIn(line);
    if(pos >= 0) {
        view->document()->replaceText(KTextEditor::Range(cursor.line(), pos, cursor.line(), cursor.column()), "\\begin{"+regexp.cap(1));
    }
    else {
        // environment completion will start with "\begin{en" when the cursor is placed
        // after the following strings:
        // en
        // x=en
        // it en
        // =en
        // it=en
        //  en
        // but it will start with "\begin{" in the following situations:
        // \en
        // it\en
        // \aen
        QRegExp re("(^|[^\\\\A-Za-z])([a-zA-Z]+)$");
        pos = re.indexIn(line);
        if(pos >= 0) {
            view->document()->replaceText(KTextEditor::Range(cursor.line(), re.pos(2), cursor.line(), cursor.column()), "\\begin{" + re.cap(2));
        }
        else {
            view->document()->insertText(cursor, "\\begin{");
        }
    }

    startLaTeXCompletion(view);
}

void Manager::startAbbreviationCompletion(KTextEditor::View *view)
{
    if(!view) {
        view = m_ki->viewManager()->currentTextView();
        if(!view) {
            return;
        }
    }

    KileDocument::TextInfo *textInfo = m_ki->docManager()->textInfoFor(view->document());
    if(!textInfo) {
        return;
    }
    textInfo->startAbbreviationCompletion(view);
}

void Manager::buildReferenceCitationRegularExpressions()
{
    // build list of references
    QString references = getCommandsString(KileDocument::CmdAttrReference);
    references.replace('*', "\\*");
    m_referencesRegExp.setPattern("^\\\\(" + references + ")\\{");
    m_referencesExtRegExp.setPattern("^\\\\(" + references + ")\\{[^\\{\\}\\\\]+,$");

    // build list of citations
    QString citations = getCommandsString(KileDocument::CmdAttrCitations);
    citations.replace('*',"\\*");
    m_citeRegExp.setPattern("^\\\\(((c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext))" + citations +  ")\\{");
    m_citeExtRegExp.setPattern("^\\\\(((c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext))" + citations + ")\\{[^\\{\\}\\\\]+,$");
}

QString Manager::getCommandsString(KileDocument::CmdAttribute attrtype)
{
    QStringList cmdlist;
    QStringList::ConstIterator it;

    // get info about user-defined references
    KileDocument::LatexCommands *cmd = m_ki->latexCommands();
    cmd->commandList(cmdlist, attrtype, false);

    // build list of references
    QString commands;
    for(it = cmdlist.constBegin(); it != cmdlist.constEnd(); ++it) {
        if(cmd->isStarredEnv(*it) ) {
            commands += '|' + (*it).mid(1) + '*';
        }
        commands += '|' + (*it).mid(1);
    }
    return commands;
}

void Manager::addUserDefinedLaTeXCommands(QStringList &wordlist)
{
    QStringList cmdlist;
    QStringList::ConstIterator it;
    KileDocument::LatexCmdAttributes attr;

    // get info about user-defined commands and environments
    KileDocument::LatexCommands *cmd = m_ki->latexCommands();
    cmd->commandList(cmdlist, KileDocument::CmdAttrNone, true);

    // add entries to wordlist
    for(it = cmdlist.constBegin(); it != cmdlist.constEnd(); ++it) {
        if(cmd->commandAttributes(*it, attr)) {
            QString command,eos;
            QStringList entrylist;
            if(attr.type < KileDocument::CmdAttrLabel) {         // environment
                command = "\\begin{" + (*it);
                eos = '}';
            }
            else {                                                   // command
                command = (*it);
                // eos.clear();
            }

            // get all possibilities into a stringlist
            entrylist.append(command + eos);
            if(!attr.option.isEmpty()) {
                entrylist.append(command + eos + "[option]");
            }
            if(attr.starred) {
                entrylist.append(command + '*' + eos);
                if (!attr.option.isEmpty()) {
                    entrylist.append(command + '*' + eos + "[option]");
                }
            }

            // finally append entries to wordlist
            QStringList::ConstIterator itentry;
            for(itentry = entrylist.constBegin(); itentry != entrylist.constEnd(); ++itentry) {
                QString entry = (*itentry);
                if(!attr.parameter.isEmpty()) {
                    entry += "{param}";
                }
                if(attr.type == KileDocument::CmdAttrList) {
                    entry += "\\item";
                }
                wordlist.append(entry);
            }
        }
    }
}

QStringList Manager::readCWLFile(const QString &filename, bool fullPathGiven)
{
    QStringList toReturn;
    QString file = fullPathGiven ? filename : QStandardPaths::locate(QStandardPaths::DataLocation, "complete/" + filename);
    if(file.isEmpty()) {
        return toReturn;
    }

    QFile f(file);
    if(f.open(QIODevice::ReadOnly)) {     // file opened successfully
        QTextStream t(&f);         // use a text stream
        while(!t.atEnd()) {        // until end of file...
            QString s = t.readLine().trimmed();       // line of text excluding '\n'
            if(!(s.isEmpty() || s.at(0) == '#')) {
                toReturn.append(s);
            }
        }
        f.close();
    }
    return toReturn;
}

QStringList Manager::readCWLFiles(const QStringList &files, const QString &dir)
{

    // read wordlists from files
    QStringList wordlist;
    for(int i = 0; i < files.count(); ++i) {
        QString cwlfile = validCwlFile(files[i]);
        if( !cwlfile.isEmpty() ) {
            wordlist += readCWLFile(dir + '/' + cwlfile + ".cwl");
        }
    }
    return wordlist;
}

QString Manager::validCwlFile(const QString &filename)
{
    return (filename.at(0) == '1') ? filename.right( filename.length()-2 ) : QString();
}


// find local and global cwl files: global files are not added,
// if there is already a local file with this name. We fill a map
// with filename as key and filepath as value.

static void getCwlFiles(QMap<QString, QString> &map, const QString &dir)
{
    QStringList files = QDir(dir, "*.cwl").entryList();
    for (QStringList::ConstIterator it = files.constBegin(); it != files.constEnd(); ++it) {
        QString filename = QFileInfo(*it).fileName();
        if(!map.contains(filename)) {
            map[filename] = dir + '/' + (*it);
        }
    }
}

QMap<QString, QString> Manager::getAllCwlFiles(const QString &localCwlPath, const QString &globalCwlPath)
{
    // get a sorted list of all cwl files from both directories
    // Local files are preferred over global ones.
    QMap<QString, QString> fileMap;
    getCwlFiles(fileMap, localCwlPath);
    getCwlFiles(fileMap, globalCwlPath);
    return fileMap;
}

QPair<QString, QString> Manager::getCwlBaseDirs()
{
    QString localDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "complete";
    QString globalDir;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "complete", QStandardPaths::LocateDirectory);
    for(QStringList::ConstIterator it = dirs.constBegin(); it != dirs.constEnd(); ++it) {
        if((*it) != localDir) {
            globalDir = (*it);
            break;
        }
    }
    // we ensure that the directory strings end in '/'
    if(!localDir.endsWith('/')) {
        localDir += '/';
    }
    if(!globalDir.endsWith('/')) {
        globalDir += '/';
    }
    return QPair<QString, QString>(localDir, globalDir);
}

}

