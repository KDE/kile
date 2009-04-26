/**********************************************************************************************
    date                 : Mar 21 2007
    version              : 0.40
    copyright            : (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                               2008-2009 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QFile>
#include <QList>
#include <QRegExp>
#include <QTimer>

#include "kiledebug.h"

#include <KConfig>
#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Cursor>

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
: KTextEditor::CodeCompletionModel(parent), m_codeCompletionManager(manager), m_editorExtension(editorExtension), m_currentView(NULL)
{
	setHasGroups(false);
}

LaTeXCompletionModel::~LaTeXCompletionModel()
{
}

void LaTeXCompletionModel::completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range,
                                                                      InvocationType invocationType)
{
	Q_UNUSED(invocationType);
	m_currentView = view;
	KILE_DEBUG() << "building model...";
	buildModel(view, range);
}

void LaTeXCompletionModel::updateCompletionRange(KTextEditor::View *view, KTextEditor::SmartRange &range)
{
	KILE_DEBUG() << "updating model...";
	range = completionRange(view, view->cursorPosition());
	buildModel(view, range);
}

static inline bool isSpecialLaTeXCommandCharacter(const QChar& c) {
	return (c == '{' || c == '[' || c == '*' || c.isLetterOrNumber());
}

static inline int specialLaTeXCommandCharacterOrdering(const QChar& c)
{
	switch(c.unicode()) {
		case '{':
			return 1;
		case '[':
			return 2;
		case '*':
			return 3;
		default: // does nothing
		break;
	}
	return 4; // must be 'isLetterOrNumber()' now
}

// required ordering on chars: '{' < '[' < '*' < isLetterOrNumber()
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
		if(isSpecialLaTeXCommandCharacter(c1)
		   && isSpecialLaTeXCommandCharacter(c2)) {
			if(specialLaTeXCommandCharacterOrdering(c1)
			     < specialLaTeXCommandCharacterOrdering(c2)) {
				return true;
			}
			else {
				return false;
			}
		}
		if(c1 < c2) {
			return true;
		}
	}
	return false;
}

void LaTeXCompletionModel::buildModel(KTextEditor::View *view, const KTextEditor::Range &range)
{
	if(!KileConfig::completeAuto() || !range.isValid()) {
		m_completionList.clear();
		reset();
		return;
	}
	KTextEditor::Cursor startCursor = range.start();
	QString completionString = view->document()->text(range);
	KILE_DEBUG() << "Text in completion range: " << completionString;
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
			m_completionList = *m_codeCompletionManager->m_ki->allLabels();
		}
		else if(citationIndex != -1) {
			m_completionList = *m_codeCompletionManager->m_ki->allBibItems();
		}
	}
	filterModel(completionString);
	qSort(m_completionList.begin(), m_completionList.end(), laTeXCommandLessThan);
	reset();
}

KTextEditor::Cursor LaTeXCompletionModel::determineLaTeXCommandStart(KTextEditor::Document *doc,
                                                                     const KTextEditor::Cursor& position) const
{
	QString line = doc->line(position.line());
// 	QRegExp completionStartRegExp("((\\s|^)?)((\\\\\\w*)|(\\w+))$");
// 	QRegExp completionStartRegExp("((\\\\\\w*)|([^\\\\]\\b\\w+))$");
// 	QRegExp completionStartRegExp("(\\\\\\w*)[^\\\\]*$");
 
	QRegExp completionStartRegExp("(\\\\([\\s\\{\\}\\[\\]\\w,=\"'~]|(\\&)|(\\$)|(\\%)(\\#)(\\_)|(\\{)|(\\})|(\\backslash)|(\\^)|(\\[)|(\\]))*)$");
	completionStartRegExp.setMinimal(true);
	QString leftSubstring = line.left(position.column());
	KILE_DEBUG() << "leftSubstring: " << leftSubstring;
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
	int numOpenSquareBrackets = commandText.count(QRegExp("[^\\]\\["));
	int numClosedSquareBrackets = commandText.count(QRegExp("[^\\]\\]"));
	int numOpenCurlyBrackets = commandText.count(QRegExp("[^\\]\\{"));
	int numClosedCurlyBrackets = commandText.count(QRegExp("[^\\]\\}"));

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
	if(!KileConfig::completeAuto()) {
		return KTextEditor::Range::invalid();
	}
	QString line = view->document()->line(position.line());
	KTextEditor::Cursor startCursor = position;
	KTextEditor::Cursor endCursor = position;

	QRegExp completionEndRegExp("\\b|\\\\");

	int cursorPos = position.column();

	KTextEditor::Cursor latexCommandStart = determineLaTeXCommandStart(view->document(), position);
	KILE_DEBUG() << "LaTeX command start " << latexCommandStart;
	if(!latexCommandStart.isValid() || !isWithinLaTeXCommand(view->document(), latexCommandStart, position)) {
		return KTextEditor::Range::invalid();
	}
	QString completionString = view->document()->text(KTextEditor::Range(latexCommandStart,
	                                                                     position));
	KILE_DEBUG() << "completionString " << completionString;
	//check whether we are completing a citation of reference
	if(completionString.indexOf(m_codeCompletionManager->m_citeRegExp) != -1
		|| completionString.indexOf(m_codeCompletionManager->m_referencesRegExp) != -1) {
		KILE_DEBUG() << "found citation or reference!";
		int openBracketIndex = completionString.indexOf('{');
		if(openBracketIndex != -1) {
			QRegExp labelListRegExp("\\s*(([\\w]+)|([\\w]+(\\s*,\\s*[\\w]*)+))");
			labelListRegExp.setMinimal(false);
			int column = openBracketIndex + 1;
			KILE_DEBUG() << "open bracket column + 1: " << column;
			KILE_DEBUG() << labelListRegExp.indexIn(completionString, openBracketIndex + 1);
			if(labelListRegExp.indexIn(completionString, openBracketIndex + 1) == openBracketIndex + 1
			      && labelListRegExp.matchedLength() + openBracketIndex + 1 == completionString.length()) {
				int lastCommaIndex = completionString.lastIndexOf(',');
				if(lastCommaIndex >= 0) {
					KILE_DEBUG() << "last comma found at: " << lastCommaIndex;
					column =  lastCommaIndex + 1;
				}
			}
			KILE_DEBUG() << labelListRegExp.errorString();
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
	if(endPos >= 0) {
		endCursor.setColumn(endPos);
	}
	KTextEditor::Range completionRange(startCursor, endCursor);
	int rangeLength = endCursor.column() - startCursor.column();

	if(latexCompletion && rangeLength < KileConfig::completeAutoThreshold() + 1) { // + 1 for the command backslash
		KILE_DEBUG() << "not reached the completion threshold yet";
		return KTextEditor::Range::invalid();
	}
	KILE_DEBUG() << "returning completion range: " << completionRange;
	return completionRange;
}

bool LaTeXCompletionModel::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::SmartRange &range,
                                                                          const QString &currentCompletion)
{
	Q_UNUSED(currentCompletion);
	if(view->cursorPosition() < range.start() || view->cursorPosition() > range.end()
	                                          || m_completionList.size() == 0) {
		return true;
	}
	return false;
}

QString LaTeXCompletionModel::filterString(KTextEditor::View *view, const KTextEditor::SmartRange &range,
                                                                    const KTextEditor::Cursor &position)
{
	Q_UNUSED(position);
	KILE_DEBUG() << "range: " << range;
	KILE_DEBUG() << "text: " << (range.isValid() ? view->document()->text(range)
	                                             : "(invalid range)");

	return "";
}

QVariant LaTeXCompletionModel::data(const QModelIndex& index, int role) const
{
	if(index.column() != KTextEditor::CodeCompletionModel::Name) {
		return QVariant();
	}
	switch(role) {
		case Qt::DisplayRole:
			return m_completionList.at(index.row());
		case CompletionRole:
			return static_cast<int>(FirstProperty | LastProperty | Public);
		case MatchQuality:
			return 10;
		case ScopeIndex:
			return 0;
		case InheritanceDepth:
			return 0;
		case HighlightingMethod:
			return QVariant::Invalid;
	}

	return QVariant();
}

QModelIndex LaTeXCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
	if (row < 0 || row >= m_completionList.count() || column < 0 || column >= ColumnCount || parent.isValid()) {
		return QModelIndex();
	}

	return createIndex(row, column, 0);
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

void LaTeXCompletionModel::executeCompletionItem(KTextEditor::Document *document,
                                                 const KTextEditor::Range& word, int row) const
{
	KTextEditor::Cursor startCursor = word.start();
	const static QRegExp reEnv = QRegExp("^\\\\(begin|end)[^a-zA-Z]+");

	int cursorXPos = -1, cursorYPos = -1;
	QString completionText = data(index(row, KTextEditor::CodeCompletionModel::Name, QModelIndex()), Qt::DisplayRole).toString();
	QString textToInsert;
	int envIndex = reEnv.indexIn(completionText);
	if(completionText != "\\begin{}" && envIndex != -1) { // we are completing an environment
		QString prefix;
		prefix = document->text(KTextEditor::Range(startCursor.line(), 0,
		                                           startCursor.line(), word.start().column()));
		textToInsert = buildEnvironmentCompletedText(completionText, prefix, cursorYPos, cursorXPos);
		KILE_DEBUG() << cursorYPos << ", " << cursorXPos;
	}
	else {
		textToInsert = buildRegularCompletedText(stripParameters(completionText), cursorYPos, cursorXPos, true);
	}
	document->replaceText(word, textToInsert);
	//HACK, but it's impossible to do this otherwise
	if(KileConfig::completeCursor() && (cursorXPos > 0 || cursorYPos > 0)
	    && m_currentView && document->views().contains(m_currentView)) {
		m_currentView->setCursorPosition(KTextEditor::Cursor(startCursor.line() + (cursorYPos >= 0 ? cursorYPos : 0),
		                                                     startCursor.column() + (cursorXPos >= 0 ? cursorXPos : 0)));
	}
}

// strip all names enclosed in braces
// consider also beamer like stuff [<...>] and <...>
QString LaTeXCompletionModel::stripParameters(const QString &text) const
{
	QString s;
	bool ignore = false;

	for(int i = 0; i < text.length(); ++i) {
		QChar c = text[i];
		switch(c.toAscii()) {
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
		switch(c.toAscii()) {
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
		switch(s[1].toAscii()) {
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
	static QRegExp reEnv = QRegExp("^\\\\(begin|end)\\{([^\\}]*)\\}(.*)");

	if(reEnv.indexIn(text) == -1) {
		return text;
	}

	QString parameter = stripParameters(reEnv.cap(3));
	QString start = reEnv.cap(1);
	QString envname = reEnv.cap(2);
	QString whitespace = buildWhiteSpaceString(prefix);
	QString envIndent = m_editorExtension->autoIndentEnvironment();

	QString s = "\\" + start + "{" + envname + "}" + parameter + "\n";

	s += whitespace;
	if(start != "end") {
		s += envIndent;
	}

	QString type;
	bool item = (type == "list");
	if(item) {
		s += "\\item ";
	}

	if(KileConfig::completeBullets() && !parameter.isEmpty()) {
		s += s_bullet;
	}

	if(KileConfig::completeCloseEnv() && start != "end") {
		s += '\n' + whitespace + "\\end{" + envname + "}\n";
	}

	if(parameter.isEmpty()) {
		ypos = 1;
		xpos = envIndent.length() + ((item) ? 6 : 0);
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

	return createIndex(row, column, 0);
}

QVariant AbbreviationCompletionModel::data(const QModelIndex& index, int role) const
{
	if(index.column() != KTextEditor::CodeCompletionModel::Name) {
		return QVariant();
	}
	switch(role) {
		case Qt::DisplayRole:
			return m_completionList.at(index.row());
		case CompletionRole:
			return static_cast<int>(FirstProperty | LastProperty | Public);
		case MatchQuality:
			return 10;
		case ScopeIndex:
			return 0;
		case InheritanceDepth:
			return 0;
		case HighlightingMethod:
			return QVariant::Invalid;
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

bool AbbreviationCompletionModel::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::SmartRange &range,
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
	Q_UNUSED(invocationType);
	KILE_DEBUG() << "building model...";
	buildModel(view, range);
}

void AbbreviationCompletionModel::updateCompletionRange(KTextEditor::View *view, KTextEditor::SmartRange &range)
{
	KILE_DEBUG() << "updating model...";
	range = completionRange(view, view->cursorPosition());
	buildModel(view, range);
}

KTextEditor::Range AbbreviationCompletionModel::completionRange(KTextEditor::View *view,
                                                                const KTextEditor::Cursor &position)
{
	KILE_DEBUG();
	if(!KileConfig::completeAutoAbbrev()) {
		return KTextEditor::Range::invalid();
	}
	return KTextEditor::CodeCompletionModelControllerInterface::completionRange(view, position);
}

QString AbbreviationCompletionModel::filterString(KTextEditor::View *view,
                                                  const KTextEditor::SmartRange &range,
                                                  const KTextEditor::Cursor &position)
{
	Q_UNUSED(view);
	Q_UNUSED(range);
	Q_UNUSED(position);
	return "";
}

void AbbreviationCompletionModel::executeCompletionItem(KTextEditor::Document *document, const KTextEditor::Range& word,
                                                        int row) const
{
	KTextEditor::CodeCompletionModel::executeCompletionItem(document, word, row);
}

void AbbreviationCompletionModel::buildModel(KTextEditor::View *view, const KTextEditor::Range &range)
{
	if(!KileConfig::completeAutoAbbrev() || !range.isValid()) {
		m_completionList.clear();
		reset();
		return;
	}
	QString text = view->document()->text(range);
	KILE_DEBUG() << text;
	m_completionList = m_abbreviationManager->getAbbreviationTextMatches(text);
	m_completionList.sort();
	if(m_completionList.size() == 1
	   && m_abbreviationManager->isAbbreviationDefined(text)) {
		executeCompletionItem(view->document(), range, 0);
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
	return *(m_ki->allNewCommands(textInfo));
}

void Manager::readConfig(KConfig *config)
{
	Q_UNUSED(config);
	KILE_DEBUG() << "======================";

	// save normal parameter
	//KILE_DEBUG() << "   read bool entries";
/*
	m_setcursor = KileConfig::completeCursor();
	m_setbullets = KileConfig::completeBullets();
	m_closeenv = KileConfig::completeCloseEnv();
	m_autocomplete = KileConfig::completeAuto();
	m_autocompletetext = KileConfig::completeAutoText();
	m_autocompleteabbrev = KileConfig::completeAutoAbbrev();
	m_latexthreshold = KileConfig::completeAutoThreshold();
	m_textthreshold = KileConfig::completeAutoTextThreshold();
	m_citationMove = KileConfig::completeCitationMove();
	m_autoDollar = KileConfig::autoInsertDollar();
*/
	// we need to read some of Kate's config flags
// 	readKateConfigFlags(config);

	// reading the wordlists is only necessary at the first start
	// and when the list of files changes
	if(m_firstConfig || KileConfig::completeChangedLists() || KileConfig::completeChangedCommands()) {
		KILE_DEBUG() << "   setting regexp for references...";
		buildReferenceCitationRegularExpressions();

		KILE_DEBUG() << "   read wordlists...";
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

void Manager::startLaTeXEnvironment(KTextEditor::View *view)
{
	if(!view) {
		view = m_ki->viewManager()->currentTextView();
		if(!view) {
			return;
		}
	}
	// FIXME: optimise this once we have a better LaTeX parser
	view->document()->insertText(view->cursorPosition(), "\\begin{");
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

	// get info about user defined references
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

	// get info about user defined commands and environments
	KileDocument::LatexCommands *cmd = m_ki->latexCommands();
	cmd->commandList(cmdlist, KileDocument::CmdAttrNone, true);

	// add entries to wordlist
	for(it = cmdlist.constBegin(); it != cmdlist.constEnd(); ++it) {
		if(cmd->commandAttributes(*it, attr)) {
			QString command,eos;
			QStringList entrylist;
			if(attr.type < KileDocument::CmdAttrLabel) {         // environment
				command = "\\begin{" + (*it);
				eos = "}";
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
	QString file = fullPathGiven ? filename : KGlobal::dirs()->findResource("appdata", "complete/" + filename);
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
		// if checked, the wordlist has to be read
		if(files[i].at(0) == '1') {
			wordlist += readCWLFile(dir + '/' + files[i].right( files[i].length()-2 ) + ".cwl");
		}
	}
	return wordlist;
}

}

#include "codecompletion.moc"
