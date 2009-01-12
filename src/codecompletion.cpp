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

#include "editorextension.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kileconfig.h"

#ifdef __GNUC__
#warning !!This whole file should be reworked completely!!
#endif

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

void LaTeXCompletionModel::buildModel(KTextEditor::View *view, const KTextEditor::Range &range)
{
	if(!KileConfig::completeAuto()) {
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
		filterModel(completionString);
		reset();
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
		filterModel(completionString);
		reset();
	}
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
	if(view->cursorPosition() < range.start() || view->cursorPosition() > range.end()
	                                          || m_completionList.size() == 0) {
		return true;
	}
	return false;
}

QString LaTeXCompletionModel::filterString(KTextEditor::View *view, const KTextEditor::SmartRange &range,
                                                                    const KTextEditor::Cursor &position)
{
	KILE_DEBUG() << "range: " << range;
	KILE_DEBUG() << "text: " << view->document()->text(range);

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

void Manager::readConfig(KConfig *config)
{
	KILE_DEBUG() << "======================";

	// save normal parameter
	//KILE_DEBUG() << "   read bool entries";
/*
	m_isenabled = KileConfig::completeEnabled();
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

		// wordlist for dictionary mode
		files = KileConfig::completeDict();
		m_dictWordList = readCWLFiles(files, "dictionary");

		// wordlist for abbreviation mode
		files = KileConfig::completeAbbrev();
		m_abbrevWordList = readCWLFiles(files, "abbreviation");

		// remember changed lists
		// FIXME: remove these hacks
		m_firstConfig = false;
		KileConfig::setCompleteChangedLists(false);
		KileConfig::setCompleteChangedCommands(false);
	}
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
	for(it = cmdlist.begin(); it != cmdlist.end(); ++it) {
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
	for(it = cmdlist.begin(); it != cmdlist.end(); ++it) {
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
			for(itentry = entrylist.begin(); itentry != entrylist.end(); ++itentry) {
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

void Manager::readCWLFile(QStringList &wordlist, const QString &filename)
{
	QString file = KGlobal::dirs()->findResource("appdata", "complete/" + filename);
	if(file.isEmpty()) {
		return;
	}

	QFile f(file);
	if(f.open(QIODevice::ReadOnly)) {     // file opened successfully
		QTextStream t(&f);         // use a text stream
		while(!t.atEnd()) {        // until end of file...
			QString s = t.readLine().trimmed();       // line of text excluding '\n'
			if(!(s.isEmpty() || s.at(0) == '#')) {
				wordlist.append(s);
			}
		}
		f.close();
	}
}

QStringList Manager::readCWLFiles(const QStringList &files, const QString &dir)
{

	// read wordlists from files
	QStringList wordlist;
	for(int i = 0; i < files.count(); ++i) {
		// if checked, the wordlist has to be read
		if(files[i].at(0) == '1') {
			readCWLFile(wordlist, dir + '/' + files[i].right( files[i].length()-2 ) + ".cwl");
		}
	}

//FIXME: better move the following out of this method
	// add user defined commands and environments
	if(dir == "tex") {
		addUserDefinedLaTeXCommands(wordlist);
	}
	else if(dir == "dictionary") {
		wordlist.sort();
	}
	else if(dir == "abbreviation") {
// FIXME
/*
		// read local wordlist
		QStringList localWordlist;
		readWordlist(localWordlist, QString(), false);

		// add local/global wordlists to the abbreviation view
		m_abbrevListview->init(&wordlist,&localWordlist);

		// finally add local wordlists to the abbreviation list
		QStringList::ConstIterator it;
		for(it = localWordlist.begin(); it != localWordlist.end(); ++it) {
			wordlist.append(*it);
		}

		wordlist.sort();
*/
	}

	return wordlist;
}

}

namespace KileDocument
{
	//static QRegExp reRef("^\\\\(pageref|ref)\\{");
	//static QRegExp reCite("^\\\\(c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext)\\{");
	//static QRegExp reRefExt("^\\\\(pageref|ref)\\{[^\\{\\}\\\\]+,$");
	//static QRegExp reCiteExt("^\\\\(c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext)\\{[^\\{\\}\\\\]+,$");

	static QRegExp reRef;
	static QRegExp reRefExt;
	static QRegExp reCite;
	static QRegExp reCiteExt;
	static QRegExp reNotRefChars("[^a-zA-Z0-9_@\\.\\+\\-\\*\\:]");
	static QRegExp reNotCiteChars("[^a-zA-Z0-9_@]");

	CodeCompletion::CodeCompletion(KileInfo *info) : m_ki(info)
	{
		m_firstconfig = true;
		m_undo = false;
		m_ref = false;
		m_kilecompletion = false;
		m_keylistType = CodeCompletion::ctNone;

		// local abbreviation list
		m_abbrevListview = NULL;
		m_localAbbrevFile = KStandardDirs::locateLocal("appdata", "complete/abbreviation/", true) + "kile-abbrevs.cwl";

		//reRef.setPattern("^\\\\(pageref|ref|xyz)\\{");
		m_completeTimer = new QTimer( this );
		connect(m_completeTimer, SIGNAL( timeout() ), this, SLOT( slotCompleteValueList() ) );
	}

	CodeCompletion::~CodeCompletion()
	{
	}

	bool CodeCompletion::isActive()
	{
		return m_isenabled;
	}

	bool CodeCompletion::inProgress(KTextEditor::View *view)
	{
		KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
		if(!completionInterface) {
			return false;
		}

		return completionInterface->isCompletionActive();
	}

	bool CodeCompletion::autoComplete()
	{
		return m_autocomplete;
	}

	CodeCompletion::Type CodeCompletion::getType()
	{
		return m_type;
	}

	CodeCompletion::Type CodeCompletion::getType(const QString &text)
	{
		if(text.indexOf(reRef) != -1) {
			return CodeCompletion::ctReference;
		}
		else if(text.indexOf(reCite) != -1) {
			return CodeCompletion::ctCitation;
		}
		else {
			return CodeCompletion::ctNone;
		}
	}

	CodeCompletion::Mode CodeCompletion::getMode()
	{
		return m_mode;
	}

	CodeCompletion::Type CodeCompletion::insideReference(KTextEditor::View *view, QString &startpattern)
	{
		if(view->document()) {
			KTextEditor::Cursor cursor = view->cursorPosition();
			uint column = cursor.column();
			QString currentline = view->document()->line(cursor.line()).left(column);
			int pos = currentline.lastIndexOf('\\');
			if(pos >= 0) {
				QString command = currentline.mid(pos,column-pos);
 				if(command.indexOf(reRef) != -1) {
					startpattern = command.right(command.length()-reRef.cap(0).length());
					if(startpattern.indexOf(reNotRefChars) == -1) {
						return CodeCompletion::ctReference;
					}
				}
				else if(command.indexOf(reCite) != -1) {
					startpattern = command.right(command.length()-reCite.cap(0).length());
					if(startpattern.indexOf(reNotCiteChars) == -1) {
						return CodeCompletion::ctCitation ;
					}
				}
			}
		}

		return CodeCompletion::ctNone;
	}

	//////////////////// configuration ////////////////////

	void CodeCompletion::readConfig(KConfig *config)
	{
		KILE_DEBUG() << "=== CodeCompletion::readConfig ===================";

		// save normal parameter
		//KILE_DEBUG() << "   read bool entries";
		m_isenabled = KileConfig::completeEnabled();
		m_setcursor = KileConfig::completeCursor();
		m_setbullets = KileConfig::completeBullets();
		m_closeenv = KileConfig::completeCloseEnv();
		m_autocomplete = KileConfig::completeAuto();
		m_autocompleteabbrev = KileConfig::completeAutoAbbrev();
		m_latexthreshold = KileConfig::completeAutoThreshold();
		m_citationMove = KileConfig::completeCitationMove();
		m_autoDollar = KileConfig::autoInsertDollar();

		// we need to read some of Kate's config flags
		readKateConfigFlags(config);

	}

	void CodeCompletion::readKateConfigFlags(KConfig *config)
	{
		KConfigGroup configGroup = config->group("Kate Document Defaults");
		m_autobrackets = ( configGroup.readEntry("Basic Config Flags",0) & cfAutoBrackets );
		m_autoindent   = ( configGroup.readEntry("Indentation Mode",0) > 0 );
	}

	// save local abbreviation changes
	// (for example before a new configuration should be read)

	void CodeCompletion::saveLocalChanges()
	{
		KILE_DEBUG() << "=== CodeCompletion::saveLocalChanges ===================";
		m_abbrevListview->saveLocalAbbreviation(m_localAbbrevFile);
	}

	//////////////////// references and citations ////////////////////

	void CodeCompletion::setReferences()
	{
		// has been moved already
	}

	QString CodeCompletion::getCommandList(KileDocument::CmdAttribute attrtype)
	{
		// has been moved already
		return "";
	}

	//////////////////// wordlists ////////////////////





	//////////////////// completion box ////////////////////

	void CodeCompletion::completeWord(KTextEditor::View* view, const KTextEditor::Range& range, CodeCompletion::Mode mode)
	{
		KILE_DEBUG() << "==CodeCompletion::completeWord(" << range << ")=========";

		KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
		if(!completionInterface) {
			return;
		}

		QString text = view->document()->text(range);

		// remember all parameters (view, pattern, length of pattern, mode)
		m_text = text;
		m_textlen = text.length();
		m_mode = mode;

		// and the current cursor position
		KTextEditor::Cursor cursorPosition = view->cursorPosition();
		m_xcursor = cursorPosition.line();
		m_ycursor = cursorPosition.column();
		m_xstart = m_xcursor - m_textlen;

		// and the current document
		KTextEditor::Document *doc = view->document();

		// switch to cmLatex mode, if cmLabel is chosen without any entries
		if(mode == cmLabel && m_labellist.count() == 0) {
			QString s = doc->line(m_ycursor);
			int pos = s.lastIndexOf("\\",m_xcursor);
			if (pos < 0) {
				//KILE_DEBUG() << "\tfound no backslash! s=" << s;
				return;
			}
			m_xstart = pos;
			m_text = doc->text(KTextEditor::Range(m_ycursor, m_xstart, m_ycursor, m_xcursor));
			m_textlen = m_text.length();
			m_mode = cmLatex;
		}

		// determine the current list
		QStringList list;
		switch(m_mode) {
			case cmLatex: // fall through here
			case cmEnvironment:
				list = m_texlist;
				appendNewCommands(list);
			break;
			case cmDictionary:
				list = m_dictlist;
			break;
			case cmAbbreviation:
				list = m_abbrevlist;
			break;
			case cmLabel:
				list = m_labellist;
			break;
			case cmDocumentWord:
				list = getDocumentWords(view, text);
			break;
		}

		// is it necessary to show the complete dialog?
		QString entry;
		QString pattern = (m_mode != cmEnvironment) ? text : "\\begin{" + text;
		uint n = countEntries(pattern, list, &entry);

		//KILE_DEBUG() << "entries = " << n;

		// nothing to do
		if (n == 0) {
 			return;
		}

		// Add a prefix ('\\begin{', length=7) in cmEnvironment mode,
		// because KateCompletion reads from the current line, This also
		// means that the original text has to be restored, if the user
		// aborts the completion dialog
		if(m_mode == cmEnvironment) {
			doc->removeText(KTextEditor::Range(m_ycursor, m_xstart, m_ycursor, m_xcursor));
			doc->insertText(KTextEditor::Cursor(m_ycursor, m_xstart), "\\begin{" + m_text);

			// set the cursor to the new position
			m_textlen += 7;
			m_xcursor += 7;
			view->setCursorPosition(KTextEditor::Cursor(m_ycursor, m_xcursor));

			// set restore mode
			m_undo = true;
		}

		//  set restore mode
		if(m_mode == cmAbbreviation) {
			m_undo = true;
		}
	}

	void CodeCompletion::appendNewCommands(QStringList& list)
	{
		const QStringList *ncommands = m_ki->allNewCommands();
		list = *ncommands + list;
	}

	void CodeCompletion::completeFromList(KTextEditor::View* view, const QStringList *list, const KTextEditor::Cursor &position, const QString &pattern)
	{
		KILE_DEBUG() << "completeFromList: " << list->count() << " items" <<   " << pattern=" << pattern<< endl;
		QStringList sortedlist(*list);
		sortedlist.sort();

		m_labellist.clear();
		QStringList::ConstIterator it;
		QStringList::ConstIterator itend(sortedlist.end());
		for(it = sortedlist.begin(); it != itend; ++it) {
			m_labellist.append(*it);
		}

		completeWord(view, KTextEditor::Range(position, position + KTextEditor::Cursor(pattern.length(), 0)), cmLabel);
	}

	//////////////////// completion was done ////////////////////

// 	void CodeCompletion::CompletionDone(KTextEditor::CompletionEntry)
// 	{
#ifdef __GNUC__
#warning Things left to be ported at line 507!
#endif
//FIXME: port for KDE4
/*

		// kile completion: is there a new cursor position?
		if ( m_kilecompletion && m_setcursor && ( m_xoffset != 0 || m_yoffset != 0 ) && m_view )
		{
			int newx = ( m_xoffset != 0 ) ? m_xcursor + m_xoffset - m_textlen : m_xcursor;
			int newy = ( m_yoffset != 0 ) ? m_ycursor + m_yoffset : m_ycursor;

			m_view->setCursorPositionReal( newy, newx );
		}

		m_undo = false;
		m_ref = false;
*/
// 	}

	void CodeCompletion::CompletionAborted()
	{
#ifdef __GNUC__
#warning Things left to be ported at line 530!
#endif
//FIXME: port for KDE4
/*

		// aborted: undo if kile completion is active
		if ( m_inprogress && m_undo && m_view )
		{
			uint row, col;
			m_view->cursorPositionReal( &row, &col );

			KTextEditor::Document *doc = m_view->document();
			doc->removeText( m_ycursor, m_xstart, m_ycursor, col );
			doc->insertText( m_ycursor, m_xstart, m_text );

			m_view->setCursorPositionReal( m_ycursor, m_xstart + m_text.length() );
		}

		m_undo = false;
		m_ref = false;
*/
	}

	void CodeCompletion::textInsertedInView(KTextEditor::View *view, const KTextEditor::Cursor &position, const QString &text)
	{
#if 0
		bool completionInProgress = inProgress(view);
		KILE_DEBUG() << "(" << m_kilecompletion << "," << completionInProgress << ", " << m_ref << ", " << text << ")=============";

		if (!completionInProgress && m_autoDollar && text == "$") {
			autoInsertDollar();
			return;
		}

		// only work, if autocomplete mode of Kile is active
		if(!isActive() || !autoComplete()) {
			return ;
		}

		// try to autocomplete abbreviations after punctuation symbol
		if(!completionInProgress && m_autocompleteabbrev && completeAutoAbbreviation(view, text)) {
			return;
		}

		// rather unsusual, but it may happen: the cursor is inside
		// of a reference command without a labellist.
		if(!m_ref) {
			QString startpattern;
			CodeCompletion::Type reftype = insideReference(view, startpattern);
			if(reftype != CodeCompletion::ctNone) {
				m_ref = true;
				editCompleteList(view, reftype, position, startpattern);
				return;
			}
		}

		Type type;
		KTextEditor::Range range = (m_ref) ? getReferenceWord(view) : getCompleteWord(view, true, type);
		if(range.isValid()) {
			QString word = view->document()->text(range);
			int wordlen = word.length();
			KILE_DEBUG() << "   auto completion: range=" << range << " mode=" << m_mode << " inprogress=" << completionInProgress;
			KILE_DEBUG() << "                    wordlen=" << wordlen;
			if(completionInProgress) {               // continue a running mode?
				KILE_DEBUG() << "   auto completion: continue current mode";
				completeWord(view, range, m_mode);
			}
			else if(wordlen >= m_latexthreshold && word.at(0) == '\\' && m_autocomplete) {
				KILE_DEBUG() << "   auto completion: latex mode";
				if(text.at(0).isLetter()) {
					completeWord(view, range, cmLatex);
				}
				else if(text.at(0) == '{') {
					editCompleteList(view, type, position);
				}
			}
			else if(wordlen >= m_textthreshold && word.at(0).isLetter() && m_autocompletetext) {
				KILE_DEBUG() << "   auto completion: document mode";
				completeWord(view, range, cmDocumentWord);
			}
		}
#endif
	}

	//////////////////// build the text for completion ////////////////////

	// parse an entry for kile completion modes:
	// - delete arguments/parameters
	// - set cursor position
	// - insert bullets

	QString CodeCompletion::filterCompletionText(KTextEditor::View *view, const QString &text, const QString &type)
	{
		static QRegExp reEnv = QRegExp("^\\\\(begin|end)[^a-zA-Z]+");
		//KILE_DEBUG() << "   complete filter: " << text << " type " << type;
		m_type = getType(text);    // remember current type

		if(text != "\\begin{}" && reEnv.indexIn(text) != -1) {
			m_mode = cmEnvironment;
		}

		// check the cursor position, because the user may have
		// typed some characters or the backspace key. This also
		// changes the length of the current pattern.
		int row, col;
		KTextEditor::Cursor cursorPosition = view->cursorPosition();
		row = cursorPosition.line();
		col = cursorPosition.column();
		if(m_xcursor != col) {
			m_textlen += (col - m_xcursor);
			m_xcursor = col;
		}

		// initialize offset for the new cursorposition
		m_xoffset = m_yoffset = 0;

		// build the text
		QString s, prefix;
		KTextEditor::Document *doc = view->document();
		QString textline = doc->line(row);
		switch(m_mode) {
			case cmLatex:
				s = buildLatexText(text, m_yoffset, m_xoffset);
				if(m_autobrackets && textline.at(col)=='}' && m_text.indexOf('{') >= 0) {
					doc->removeText(KTextEditor::Range(row, col, row, col + 1));
				}
				break;
			case cmEnvironment:
				prefix.clear();
				if(m_autoindent) {
					if(col - m_textlen > 0) {
						prefix = textline.left(col-m_textlen);
						if(prefix.right(7) == "\\begin{") {
							prefix.truncate(prefix.length()-7);
						}
						else if(prefix.right(5) == "\\end{") {
							prefix.truncate(prefix.length() - 5);
						}
					}
				}
				s = buildEnvironmentText(text, type, prefix, m_yoffset, m_xoffset);
				if(m_autobrackets && textline.at(col) == '}' && (textline[m_xstart] != '\\' || m_text.indexOf('{') >= 0)) {
					doc->removeText(KTextEditor::Range(row, col, row, col + 1));
				}
				if(m_xstart >= 7 && textline.mid(m_xstart - 7, 7) == "\\begin{") {
					m_textlen += 7;
				}
				else if(m_xstart >= 5 && textline.mid(m_xstart - 5, 5) == "\\end{") {
					m_textlen += 5;
				}
				break;
			case cmDictionary:
				s = text;
				break;
			case cmAbbreviation:
				s = buildAbbreviationText(view, text);
				break;
			case cmLabel:
				s = buildLabelText(view, text);
				if (m_keylistType == CodeCompletion::ctReference
				|| (m_keylistType == CodeCompletion::ctCitation && m_citationMove)) {
					m_xoffset = s.length() + 1;
				}
				break;
			case cmDocumentWord:
				s = text;
				break;
			default:
				s = text;
				break;
		}

		if(s.length() > m_textlen) {
			return s.right(s.length() - m_textlen);
		}
		else {
			return "";
		}
	}

	//////////////////// text in cmLatex mode ////////////////////

	QString CodeCompletion::buildLatexText(const QString &text, int &ypos, int &xpos)
	{
		return parseText(stripParameter(text), ypos, xpos, true);
	}

	////////////////////  text in cmEnvironment mode ////////////////////

	QString CodeCompletion::buildEnvironmentText(const QString &text, const QString &type,
	                                             const QString &prefix, int &ypos, int &xpos)
	{
		// Has been moved already
		return text;
	}

	//////////////////// text in  cmAbbreviation mode ////////////////////

	QString CodeCompletion::buildAbbreviationText(KTextEditor::View *view, const QString &text)
	{
		QString s;

		int index = text.indexOf('=');
		if(index >= 0) {
			// determine text to insert
			s = text.right(text.length() - index - 1);

			// delete abbreviation
			KTextEditor::Document *doc = view->document();
			doc->removeText(KTextEditor::Range(m_ycursor, m_xstart, m_ycursor, m_xcursor));
			view->setCursorPosition(KTextEditor::Cursor(m_ycursor, m_xstart));
			m_xcursor = m_xstart;

			m_textlen = 0;
		}
		else {
			s = "";
		}

		return s;
	}

	//////////////////// text in cmLabel mode ////////////////////

	QString CodeCompletion::buildLabelText(KTextEditor::View *view, const QString &text)
	{
		if(text.at(0) == ' ') {
			// delete space
			KTextEditor::Document * doc = view->document();
			doc->removeText(KTextEditor::Range(m_ycursor, m_xstart, m_ycursor, m_xstart + 1));
			view->setCursorPosition(KTextEditor::Cursor(m_ycursor, m_xstart));
			m_xcursor = m_xstart;

			m_textlen = 0;
			return text.right(text.length() - 1);
		}
		else {
			return text;
		}
	}


	//////////////////// some functions ////////////////////

	QString CodeCompletion::parseText(const QString &text, int &ypos, int &xpos, bool checkgroup)
	{
		// has been moved already
		return text;
	}

	QString CodeCompletion::stripParameter(const QString &text)
	{
		// has been moved already
		return text;
	}

	//////////////////// determine number of entries ////////////////////

	// Count the number of entries. Stop, wenn there are 2 entries,
	// because special functions are only called, when there are 0
	// or 1 entries.

	int CodeCompletion::countEntries(const QString &pattern, const QStringList& list,
	                                 QString *entry)
	{
		int n = 0;

		for(QStringList::const_iterator it = list.begin(); it != list.end() && n < 2; ++it) {
			if((*it).startsWith(pattern)) {
				*entry = *it;
				++n;
			}
		}

		return n;
	}

	QString CodeCompletion::findExpansion(const QString &abbrev)
	{
// 		QList<KTextEditor::CompletionEntry>::Iterator it;

// 		for(QStringList::iterator it = m_abbrevlist.begin(); it != m_abbrevlist.end(); ++it) {
// 			QString s = *it;
// 			int index = s.indexOf("=");
// 			if(index>=0 && s.left(index)==abbrev) {
// 				 return s.right(s.length() - index - 1);
// 			}
// 		}

		return QString();
	}

	void CodeCompletion::editComplete(KTextEditor::View *view, Mode mode)
	{
		if(!view || !isActive() || inProgress(view)) {
			return;
		}

		// check for a special case: call from inside of a reference command
		if(mode == cmLatex) {
			QString startpattern;
			CodeCompletion::Type reftype = insideReference(view, startpattern);
			if(reftype != CodeCompletion::ctNone) {
				m_ref = true;
				editCompleteList(view, reftype, view->cursorPosition(), startpattern);
				return;
			}
		}

		Type type;
		KTextEditor::Range completeWordRange = getCompleteWord(view, (mode == cmLatex) ? true : false, type);
		if(completeWordRange.isValid()) {
			QString word = view->document()->text(completeWordRange);
			if(mode == cmLatex && word.at(0) != '\\') {
				mode = cmDictionary;
			}

			if(type == CodeCompletion::ctNone) {
				completeWord(view, completeWordRange, mode);
			}
			else {
				editCompleteList(view, type, view->cursorPosition());
			}
		}
		//little hack to make multiple insertions like \cite{test1,test2} possible (only when
		//completion is invoke explicitly using ctrl+space.
		else if(view->document()) {
			KTextEditor::Cursor cursorPosition = view->cursorPosition();
			QString currentline = view->document()->line(cursorPosition.line()).left(cursorPosition.column() + 1);
			if(currentline.indexOf(reCiteExt) != -1) {
				editCompleteList(view, ctCitation, cursorPosition);
			}
			else if(currentline.indexOf(reRefExt) != -1) {
				editCompleteList(view, ctReference, cursorPosition);
			}
		}
	}

	void CodeCompletion::editCompleteList(KTextEditor::View* view, Type type, const KTextEditor::Cursor &position, const QString &pattern)
	{
		//KILE_DEBUG() << "==editCompleteList=============";
		m_keylistType = type;
		if(type == ctReference) {
			completeFromList(view, info()->allLabels(), position, pattern);
		}
		else if(type == ctCitation) {
			completeFromList(view, info()->allBibItems(), position, pattern);
		}
		else {
			m_keylistType = CodeCompletion::ctNone;
			kWarning() << "unsupported type in CodeCompletion::editCompleteList";
		}
	}

	//////////////////// slots for code completion ////////////////////

// 	void CodeCompletion::slotCompletionDone(KTextEditor::CompletionEntry entry)
// 	{
#ifdef __GNUC__
#warning Things left to be ported at line 1203!
#endif
//FIXME: port for KDE4
/*

		//KILE_DEBUG() << "==slotCompletionDone (" << m_kilecompletion << "," << m_inprogress << ")=============";
		CompletionDone(entry);

		// if kile completion was active, look if we need to show an additional list
		if ( m_kilecompletion )
		{
			m_kilecompletion = false;
			if ( getMode() == cmLatex )
			{
				m_type = getType(entry.text);
				if ( (m_type==CodeCompletion::ctReference && info()->allLabels()->count()>0)  ||
					  (m_type==CodeCompletion::ctCitation  && info()->allBibItems()->count()>0) )
				{
					m_keylistType = m_type;
					m_ref = true;
		 			m_completeTimer->start(20,true);
				}
			}
		}
*/
// 	}

	void CodeCompletion::slotCompleteValueList()
	{
		//KILE_DEBUG() << "==slotCompleteValueList (" << m_kilecompletion << "," << m_inprogress << ")=============";
		m_completeTimer->stop();
#ifdef __GNUC__
#warning Things left to be ported!
#endif
//FIXME: port for KDE4
//		editCompleteList(getType());
	}

	void CodeCompletion::slotCompletionAborted()
	{
		//KILE_DEBUG() << "==slotCompletionAborted (" << m_kilecompletion << "," << m_inprogress << ")=============";
		CompletionAborted();
	}

// 	void CodeCompletion::slotFilterCompletion(KTextEditor::CompletionEntry* c, QString *s)
// 	{
#ifdef __GNUC__
#warning Things left to be ported at line 1246!
#endif
//FIXME: port for KDE4
/*

		//KILE_DEBUG() << "==slotFilterCompletion (" << m_kilecompletion << "," << m_inprogress << ")=============";
		if ( inProgress() )                 // dani 28.09.2004
		{
			//KILE_DEBUG() << "\tin progress: s=" << *s;
			*s = filterCompletionText( c->text, c->type );
			//KILE_DEBUG() << "\tfilter --->" << *s;
			m_kilecompletion = true;
		}
*/
// 	}

	//////////////////// testing characters (dani) ////////////////////

	static bool isBackslash(QChar ch)
	{
		return (ch == '\\');
	}

	KTextEditor::Range CodeCompletion::getCompleteWord(KTextEditor::View *view, bool latexmode, KileDocument::CodeCompletion::Type &type)
	{
		int row, col;
		QChar ch;

		// get current position
		KTextEditor::Cursor cursor = view->cursorPosition();
		row = cursor.line();
		col = cursor.column();

		// there must be et least one sign
		if(col < 1) {
			return KTextEditor::Range::invalid();
		}

		// get current text line
		QString textline = view->document()->line(row);

		//
		int n = 0;                           // number of characters
		int index = col;                     // go back from here
		while(--index >= 0) {
			// get current character
			ch = textline.at(index);

			if(ch.isLetter() || ch=='.' || ch == '_' || ch.isDigit() || (latexmode && (index + 1 == col) && ch == '{')) {
				++n;                           // accept letters and '{' as first character in latexmode
			}
			else {
				if(latexmode && isBackslash(ch) && oddBackslashes(textline, index)) {        // backslash?
					++n;
				}
				break;                         // stop when a backslash was found
			}
		}

		// select pattern and set type of match
		KTextEditor::Range range(row, col - n, row, col);
		type = getType(view->document()->text(range));

		return range;
	}

	KTextEditor::Range CodeCompletion::getReferenceWord(KTextEditor::View *view)
	{
		int row, col;
		QChar ch;

		// get current position
		KTextEditor::Cursor cursor = view->cursorPosition();
		row = cursor.line();
		col = cursor.column();
		// there must be at least one sign
		if(col < 1) {
			return KTextEditor::Range::invalid();
		}

		// get current text line
		QString textline = view->document()->line(row);

		// search the current reference string
		int pos = textline.lastIndexOf(reNotRefChars, col - 1);
		if(pos < 0) {
			pos = 0;
		}

		// select pattern
		if(!(pos < col - 1)) {
			return KTextEditor::Range::invalid();
		}
		return KTextEditor::Range(row, pos + 1, row, col - 1);
	}

	QStringList CodeCompletion::getDocumentWords(KTextEditor::View *view, const QString &text)
	{
		//KILE_DEBUG() << "getDocumentWords: ";
		QStringList list;

		QRegExp reg("(\\\\?\\b" + QString(text[0]) + "[^\\W\\d_]+)\\b");
		KTextEditor::Document *doc = view->document();
	
		QString s;
		QHash<QString, bool> seen;

		for (int i = 0; i < doc->lines(); ++i) {
			s = doc->line(i);
			int pos = 0;
			while (pos >= 0) {
				pos = reg.indexIn(s, pos);
				if (pos >= 0) {
					if (reg.cap(1).at(0) != '\\' && text != reg.cap(1) && !seen.contains(reg.cap(1))) {
						QString word = reg.cap(1);
						list.append(word);
						seen.insert(word, true);
					}
					pos += reg.matchedLength();
				}
			}
		}
		return list;
	}

	//////////////////// counting backslashes (dani) ////////////////////

	bool CodeCompletion::oddBackslashes(const QString& text, int index)
	{
		uint n = 0;
		while ( index >= 0 && isBackslash(text.at(index))) {
			++n;
			--index;
		}
		return ( n % 2 ) ? true : false;
	}

	//////////////////// complete auto abbreviation ////////////////////

	bool CodeCompletion::completeAutoAbbreviation(KTextEditor::View *view, const QString &text)
	{
		if(text.length() != 1) {
			return false;
		}

		QChar ch = text[0];
		if(!(ch.isSpace() || ch.isPunct())) {
			return false;
		}

		int row, col;
		KTextEditor::Cursor cursor = view->cursorPosition();
		row = cursor.line();
		col = cursor.column();
		QString abbrev = getAbbreviationWord(view, row, col - 1);
		if(abbrev.isEmpty()) {
			return false;
		}

		QString expansion = findExpansion(abbrev);
		if(expansion.isEmpty()) {
			return false;
		}

		KILE_DEBUG() << "=== CodeCompletion::completeAutoAbbreviation: abbrev=" << abbrev << "  exp=" << expansion;

		uint len = abbrev.length();
		uint startcol = col - len - 1;
		KTextEditor::Document *doc = view->document();
		doc->removeText(KTextEditor::Range(row, startcol, row, startcol + abbrev.length() + 1));
		doc->insertText(KTextEditor::Cursor(row, startcol), expansion + ch);
		view->setCursorPosition(KTextEditor::Cursor(row, startcol + expansion.length() + 1));

		return true;
	}

	QString CodeCompletion::getAbbreviationWord(KTextEditor::View *view, int row, int col)
	{
		QString textline = view->document()->line(row);

		int index = (int)col;
		while(--index >= 0) {
			QChar ch = textline.at(index);
			if(!ch.isLetterOrNumber()) {
				break;
			}
		}

		return textline.mid(index + 1, col - index - 1);
	}

	//////////////////// connection with the abbreviation listview  ////////////////////

	void CodeCompletion::setAbbreviationListview(KileWidget::AbbreviationView *listview)
	{
		m_abbrevListview = listview;

		connect( m_abbrevListview, SIGNAL(updateAbbrevList(const QString &, const QString &)),
		         this, SLOT(slotUpdateAbbrevList(const QString &, const QString &)) );

	}

	void CodeCompletion::slotUpdateAbbrevList(const QString &ds, const QString &as)
	{
		if(!ds.isEmpty()) {
			deleteAbbreviationEntry(ds);
		}
		if(!as.isEmpty()) {
			addAbbreviationEntry(as);
		}
	}

	void CodeCompletion::deleteAbbreviationEntry(const QString &entry)
	{
		KILE_DEBUG() << "=== CodeCompletion::deleteAbbreviationEntry (" << entry << ")";

		m_abbrevlist.removeAll(entry);
	}

	void CodeCompletion::addAbbreviationEntry(const QString &entry)
	{
		KILE_DEBUG() << "=== CodeCompletion::addAbbreviationEntry (" << entry << ")";

		QStringList::iterator it;
		for(it = m_abbrevlist.begin(); it != m_abbrevlist.end(); ++it) {
			if(*it > entry) {
				break;
			}
		}

		m_abbrevlist.insert(it, entry);
	}

	//////////////////// autoinsert $ ////////////////////

	void CodeCompletion::autoInsertDollar()
	{
		KTextEditor::View *view = info()->viewManager()->currentTextView();
		if (view) {
			view = info()->viewManager()->currentTextView();
			KTextEditor::Cursor cursor = view->cursorPosition();
			view->document()->insertText(cursor, "$");
			view->setCursorPosition(cursor);
		}
	}

}

#include "codecompletion.moc"
