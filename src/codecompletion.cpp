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
#include <QRegularExpression>
#include <QRegExp>
#include <QTimer>

#include <KConfig>
#include <KLocalizedString>
#include <KTextEditor/Cursor>
#include <qregularexpression.h>

#include "kiledebug.h"
#include "abbreviationmanager.h"
#include "documentinfo.h"
#include "editorextension.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kileconfig.h"
#include "utilities.h"

namespace KileCodeCompletion {

LaTeXCompletionModel::LaTeXCompletionModel(QObject *parent, KileCodeCompletion::Manager *manager,
        KileDocument::EditorExtension *editorExtension)
    : KTextEditor::CodeCompletionModel(parent), m_codeCompletionManager(manager), m_editorExtension(editorExtension), m_currentView(nullptr)
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
    return (c == QLatin1Char('{')
         || c == QLatin1Char('[')
         || c == QLatin1Char('*')
         || c == QLatin1Char(']')
         || c == QLatin1Char('}'));
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
            return (c1 < c2);
        }
        else if(isSpecialLaTeXCommandCharacter(c1)) {
            if(isSpecialLaTeXCommandCharacter(c2)) {
                return (specialLaTeXCommandCharacterOrdering(c1)
                        < specialLaTeXCommandCharacterOrdering(c2));
            }
            else if(c2.isLetterOrNumber()) {
                return true;
            }
            return (c1 < c2);
        }
    }
    return true;
}

void LaTeXCompletionModel::buildModel(KTextEditor::View *view, const KTextEditor::Range &range)
{
    QString completionString = view->document()->text(range);
    KILE_DEBUG_CODECOMPLETION << "Text in completion range: " << completionString;
    m_completionList.clear();

    if(completionString.startsWith(QLatin1Char('\\'))) {
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
    QRegularExpression completionStartRegExp(QLatin1String("(\\\\([\\s\\{\\}\\[\\]\\w,.=\"'~:]|(\\&)|(\\$)|(\\%)(\\#)(\\_)|(\\{)|(\\})|(\\backslash)|(\\^)|(\\[)|(\\]))*)$"));
    completionStartRegExp.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    QString leftSubstring = line.left(position.column());
    KILE_DEBUG_CODECOMPLETION << "leftSubstring: " << leftSubstring;
    int startPos = leftSubstring.lastIndexOf(completionStartRegExp);
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
    int numOpenSquareBrackets = commandText.count(QRegularExpression(QLatin1String("[^\\\\]\\[")));
    int numClosedSquareBrackets = commandText.count(QRegularExpression(QLatin1String("[^\\\\]\\]")));
    int numOpenCurlyBrackets = commandText.count(QRegularExpression(QLatin1String("[^\\\\]\\{")));
    int numClosedCurlyBrackets = commandText.count(QRegularExpression(QLatin1String("[^\\\\]\\}")));
    if(numOpenSquareBrackets != numClosedSquareBrackets || numOpenCurlyBrackets != numClosedCurlyBrackets) {
        return true;
    }
    if(numOpenSquareBrackets == 0 && numOpenCurlyBrackets == 0 && commandText.count(QLatin1Char(' ')) == 0) {
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

    QRegularExpression completionEndRegExp(QStringLiteral("\\W|\\b|\\\\"));

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
        int openBracketIndex = completionString.indexOf(QLatin1Char('{'));
        if(openBracketIndex != -1) {
            // TeX allows '.' characters inside citation labels (bug 266670)
            QRegExp labelListRegExp(QLatin1String("\\s*(([:.\\w]+)|([:.\\w]+(\\s*,\\s*[:.\\w]*)+))"));
            labelListRegExp.setMinimal(false);
            int column = openBracketIndex + 1;
            KILE_DEBUG_CODECOMPLETION << "open bracket column + 1: " << column;
            KILE_DEBUG_CODECOMPLETION << labelListRegExp.indexIn(completionString, openBracketIndex + 1);
            if(labelListRegExp.indexIn(completionString, openBracketIndex + 1) == openBracketIndex + 1
                    && labelListRegExp.matchedLength() + openBracketIndex + 1 == completionString.length()) {
                QRegExp lastCommaRegExp(QLatin1String(",\\s*"));
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

    if(insertedText.endsWith(QLatin1Char('{'))) {
        return true;
    }

    return CodeCompletionModelControllerInterface::shouldStartCompletion(view, insertedText, userInsertion, position);
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
                              : QStringLiteral("(invalid range)"));

    return QString();
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
    const static QRegExp reEnv = QRegExp(QStringLiteral("^\\\\(begin|end)[^a-zA-Z]+"));

    int cursorXPos = -1, cursorYPos = -1;
    QString completionText = data(index.sibling(index.row(), Name), Qt::DisplayRole).toString();

    QString textToInsert;
    int envIndex = reEnv.indexIn(completionText);
    if(completionText != QStringLiteral("\\begin{}") && envIndex != -1) { // we are completing an environment
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
    const int numberOfOpenSimpleBrackets = replaceText.count(QLatin1Char('('));
    const int numberOfOpenSquareBrackets = replaceText.count(QLatin1Char('['));
    const int numberOfOpenCurlyBrackets = replaceText.count(QLatin1Char('{'));
    const int numberOfClosedSimpleBrackets = replaceText.count(QLatin1Char(')'));
    const int numberOfClosedSquareBrackets = replaceText.count(QLatin1Char(']'));
    const int numberOfClosedCurlyBrackets = replaceText.count(QLatin1Char('}'));
    const int numberOfClosedBracketsLeft = (numberOfOpenSimpleBrackets - numberOfClosedSimpleBrackets)
                                           + (numberOfOpenSquareBrackets - numberOfClosedSquareBrackets)
                                           + (numberOfOpenCurlyBrackets - numberOfClosedCurlyBrackets);
    if(numberOfOpenSimpleBrackets >= numberOfClosedSimpleBrackets
            && numberOfOpenSquareBrackets >= numberOfClosedSquareBrackets
            && numberOfOpenCurlyBrackets >= numberOfClosedCurlyBrackets
            && document->lineLength(word.end().line()) >= word.end().column() + numberOfClosedBracketsLeft) {
        KTextEditor::Range bracketRange = KTextEditor::Range(word.end(), numberOfClosedBracketsLeft);

        QString bracketText = document->text(bracketRange);
        if(bracketText.count(QLatin1Char(')')) == (numberOfOpenSimpleBrackets - numberOfClosedSimpleBrackets)
                && bracketText.count(QLatin1Char(']')) == (numberOfOpenSquareBrackets - numberOfClosedSquareBrackets)
                && bracketText.count(QLatin1Char('}')) == (numberOfOpenCurlyBrackets - numberOfClosedCurlyBrackets)) {
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
    const static QRegExp reEnv = QRegExp(QStringLiteral("^\\\\(begin|end)[^a-zA-Z]+"));

    cursorXPos = -1, cursorYPos = -1;
    QString textToInsert;
    int envIndex = reEnv.indexIn(text);
    if(text != QStringLiteral("\\begin{}") && envIndex != -1) {
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
                if(c == QLatin1Char('[') && (i + 1) < text.length() &&  text[i + 1] == QLatin1Char('<')) {
                    cursorXPos = i + 2;
                    s += text[i + 1];
                    i++;
                }// special handling for '[<'
                else {
                    cursorXPos = i + 1;
                }
                // insert bullet, if this is no cursorposition
                if((!setCursor) && setBullets && !(c == QLatin1Char('[') && (i + 1) < text.length() &&  text[i + 1] == QLatin1Char('<'))) {
                    s += s_bullet;
                }
            }
            // insert bullets after following braces
            else if(setBullets && !(c == QLatin1Char('[') && (i + 1) < text.length() &&  text[i + 1] == QLatin1Char('<'))) {
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
            if(checkGroup && (s.right(1) == QStringLiteral("."))) {
                foundgroup = true;
                s.truncate(s.length() - 1);
                if(setBullets) {
                    s += QLatin1Char(' ') + s_bullet + QLatin1Char(' ');
                }
                else {
                    s += QLatin1Char(' ');
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
            if(s.left(6) == QStringLiteral("\\left ")) {
                pos = 5;
            }
            break;
        case 'b':
            if(s.left(6) == QStringLiteral("\\bigl ")) {
                pos = 5;
            }
            else if(s.left(7) == QStringLiteral("\\biggl ")) {
                pos = 6;
            }
            break;
        case 'B' :
            if(s.left(6) == QStringLiteral("\\Bigl ")) {
                pos = 5;
            }
            else if(s.left(7) == QStringLiteral("\\Biggl ")) {
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
    static QRegularExpression reEnv(QStringLiteral("^\\\\(begin|end)\\{([^\\}]*)\\}([^\\\\]*)(.*)"));
    auto match = reEnv.match(text);

    if(!match.hasMatch()) {
        return text;
    }

    const QString start = match.captured(1);
    const QString envname = match.captured(2);
    const QString parameter = stripParameters(match.captured(3));
    const QString remainder = match.captured(4);
    const QString whitespace = buildWhiteSpaceString(prefix);
    const QString envIndent = m_editorExtension->autoIndentEnvironment();

    QString s = QLatin1String("\\") + start + QLatin1Char('{') + envname + QLatin1Char('}') + parameter + QLatin1Char('\n');

    s += whitespace;
    if(start != QStringLiteral("end")) {
        s += envIndent;
    }

    if(!remainder.isEmpty()) {
        s += remainder + QLatin1Char(' ');
    }

    if(KileConfig::completeBullets() && !parameter.isEmpty()) {
        s += s_bullet;
    }

    if(KileConfig::completeCloseEnv() && start != QStringLiteral("end")) {
        s += QLatin1Char('\n') + whitespace + QStringLiteral("\\end{") + envname + QStringLiteral("}\n");
    }

    if(parameter.isEmpty()) {
        ypos = 1;
        xpos = envIndent.length() + ((!remainder.isEmpty()) ? remainder.length() + 1 : 0);
    }
    else {
        ypos = 0;
        if(parameter.left(2) == QStringLiteral("[<")) {
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
            whitespace[i] = QLatin1Char(' ');
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

    const int len = insertedText.length();
    QRegularExpression whitespace(QStringLiteral(" |\t"));
    whitespace.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    const int pos = insertedText.lastIndexOf(whitespace, -1);
    // 'pos' is less than or equal to 'len - 1'
    const QString searchText = (pos >= 0 && pos < len) ? insertedText.right(len - pos - 1) : insertedText;

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

    QRegularExpression whitespace(QStringLiteral(" |\t"));
    whitespace.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    int pos = insertedText.lastIndexOf(whitespace, -1);

    return KTextEditor::Range(position.line(), pos + 1, position.line(), position.column());
}

QString AbbreviationCompletionModel::filterString(KTextEditor::View *view,
        const KTextEditor::Range &range,
        const KTextEditor::Cursor &position)
{
    Q_UNUSED(view);
    Q_UNUSED(range);
    Q_UNUSED(position);
    return QString();
}

void AbbreviationCompletionModel::executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range& word,
        const QModelIndex &index) const
{
    // replace abbreviation and take care of newlines
    QString completionText = data(index.sibling(index.row(), Name), Qt::DisplayRole).toString();
    completionText.replace(QLatin1String("%n"), QLatin1String("\n"));
    KTextEditor::Document *document = view->document();
    document->replaceText(word, completionText);

    // look if there is a %C-wish to place the cursor
    if (completionText.indexOf(QStringLiteral("%C")) >= 0) {
        KTextEditor::Range searchrange = KTextEditor::Range(word.start(),document->lines()+1,0);
        QVector<KTextEditor::Range> rangevec = document->searchText(searchrange, QStringLiteral("%C"));
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
        m_texWordList = readCWLFiles(files, QLatin1String("tex"));
        addUserDefinedLaTeXCommands(m_texWordList);

        // wordlist for dictionary mode
        files = KileConfig::completeDict();
        m_dictWordList = readCWLFiles(files, QLatin1String("dictionary"));
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
    if (KileConfig::autoInsertDollar() && text == QLatin1String("$")) {
        // code completion seems to be never active, so there is no need to
        // check KTextEditor::CodeCompletionInterface::isCompletionActive()
        KTextEditor::Cursor currentCursorPos = view->cursorPosition();
        view->document()->insertText(currentCursorPos, QLatin1String("$"));
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

    QRegExp regexp(QLatin1String("\\\\b|\\\\be|\\\\beg|\\\\begi|\\\\begin|\\\\begin\\{|\\\\begin\\{([a-zA-z]*)"));
    int pos = regexp.lastIndexIn(line);
    if(pos >= 0) {
        view->document()->replaceText(KTextEditor::Range(cursor.line(), pos, cursor.line(), cursor.column()), QLatin1String("\\begin{") + regexp.cap(1));
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
        QRegExp re(QLatin1String("(^|[^\\\\A-Za-z])([a-zA-Z]+)$"));
        pos = re.indexIn(line);
        if(pos >= 0) {
            view->document()->replaceText(KTextEditor::Range(cursor.line(), re.pos(2), cursor.line(), cursor.column()), QLatin1String("\\begin{") + re.cap(2));
        }
        else {
            view->document()->insertText(cursor, QLatin1String("\\begin{"));
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
    references.replace(QLatin1Char('*'), QLatin1String("\\*"));
    m_referencesRegExp.setPattern(QLatin1String("^\\\\(") + references + QLatin1String(")\\{"));
    m_referencesExtRegExp.setPattern(QLatin1String("^\\\\(") + references + QLatin1String(")\\{[^\\{\\}\\\\]+,$"));

    // build list of citations
    QString citations = getCommandsString(KileDocument::CmdAttrCitations);
    citations.replace(QLatin1Char('*'), QLatin1String("\\*"));
    m_citeRegExp.setPattern(QLatin1String("^\\\\(((c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext))") + citations +  QLatin1String(")\\{"));
    m_citeExtRegExp.setPattern(QLatin1String("^\\\\(((c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext))") + citations + QLatin1String(")\\{[^\\{\\}\\\\]+,$"));
}

QString Manager::getCommandsString(KileDocument::CmdAttribute attrtype)
{
    QStringList cmdlist;

    // get info about user-defined references
    KileDocument::LatexCommands *cmd = m_ki->latexCommands();
    cmd->commandList(cmdlist, attrtype, false);

    // build list of references
    QString commands;
    for(const QString& cmdEntry : std::as_const(cmdlist)) {
        if(cmd->isStarredEnv(cmdEntry) ) {
            commands += QLatin1Char('|') + cmdEntry.mid(1) + QLatin1Char('*');
        }
        commands += QLatin1Char('|') + cmdEntry.mid(1);
    }
    return commands;
}

void Manager::addUserDefinedLaTeXCommands(QStringList &wordlist)
{
    QStringList cmdlist;
    KileDocument::LatexCmdAttributes attr;

    // get info about user-defined commands and environments
    KileDocument::LatexCommands *cmd = m_ki->latexCommands();
    cmd->commandList(cmdlist, KileDocument::CmdAttrNone, true);

    // add entries to wordlist
    for(const QString& cmdEntry : std::as_const(cmdlist)) {
        if(cmd->commandAttributes(cmdEntry, attr)) {
            QString command,eos;
            QStringList entrylist;
            if(attr.type < KileDocument::CmdAttrLabel) {         // environment
                command = QLatin1String("\\begin{") + cmdEntry;
                eos = QLatin1Char('}');
            }
            else {                                                   // command
                command = cmdEntry;
                // eos.clear();
            }

            // get all possibilities into a stringlist
            entrylist.append(command + eos);
            if(!attr.option.isEmpty()) {
                entrylist.append(command + eos + QLatin1String("[option]"));
            }
            if(attr.starred) {
                entrylist.append(command + QLatin1Char('*') + eos);
                if (!attr.option.isEmpty()) {
                    entrylist.append(command + QLatin1Char('*') + eos + QLatin1String("[option]"));
                }
            }

            // finally append entries to wordlist
            for(QString entry : std::as_const(entrylist)) {
                if(!attr.parameter.isEmpty()) {
                    entry += QLatin1String("{param}");
                }
                if(attr.type == KileDocument::CmdAttrList) {
                    entry += QLatin1String("\\item");
                }
                wordlist.append(entry);
            }
        }
    }
}

QStringList Manager::readCWLFile(const QString &filename, bool fullPathGiven)
{
    QStringList toReturn;
    QString file = fullPathGiven ? filename : KileUtilities::locate(QStandardPaths::AppDataLocation, QLatin1String("complete/") + filename);
    if(file.isEmpty()) {
        return toReturn;
    }

    QFile f(file);
    if(f.open(QIODevice::ReadOnly)) {     // file opened successfully
        QTextStream t(&f);         // use a text stream
        while(!t.atEnd()) {        // until end of file...
            QString s = t.readLine().trimmed();       // line of text excluding '\n'
            if(!(s.isEmpty() || s.at(0) == QLatin1Char('#'))) {
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
            wordlist += readCWLFile(dir + QLatin1Char('/') + cwlfile + QLatin1String(".cwl"));
        }
    }
    return wordlist;
}

QString Manager::validCwlFile(const QString &filename)
{
    return (filename.at(0) == QLatin1Char('1')) ? filename.right(filename.length() - 2) : QString();
}


// find local and global cwl files: global files are not added,
// if there is already a local file with this name. We fill a map
// with filename as key and filepath as value.

static void getCwlFiles(QMap<QString, QString> &map, const QString &dir)
{
    const QStringList files = QDir(dir, QLatin1String("*.cwl")).entryList();
    for(const QString& file : files) {
        QString filename = QFileInfo(file).fileName();
        if(!map.contains(filename)) {
            map[filename] = dir + QLatin1Char('/') + file;
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

std::pair<QString, QString> Manager::getCwlBaseDirs()
{
    QString localDir = KileUtilities::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + QLatin1String("complete");
    QString globalDir;

    const QStringList dirs = KileUtilities::locateAll(QStandardPaths::AppDataLocation, QLatin1String("complete"), QStandardPaths::LocateDirectory);
    for(const QString& dir : dirs) {
        if(dir != localDir) {
            globalDir = dir;
            break;
        }
    }
    // we ensure that the directory strings end in '/'
    if(!localDir.endsWith(QLatin1Char('/'))) {
        localDir += QLatin1Char('/');
    }
    if(!globalDir.endsWith(QLatin1Char('/'))) {
        globalDir += QLatin1Char('/');
    }
    return std::pair<QString, QString>(localDir, globalDir);
}

}

