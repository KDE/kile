/*********************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
              (C) 2005-2007 by Holger Danielsson (holger.danielsson@versanet.de)
              (C) 2006-2014 by Michel Ludwig (michel.ludwig@kdemail.net)
 *********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2005-11-02: dani
//  - cleaning up source of central function updateStruct()
//      - always use 'else if', because all conditions are exclusive or
//      - most often used commands are at the top
//  - add some new types of elements (and levels) for the structure view
//  - new commands, which are passed to the structure listview:
//       \includegraphics, \caption
//  - all user-defined commands for labels are recognized
//  - changed folder name of KileStruct::BibItem to "bibs", so that "refs"
//    is still unused and can be used for references (if wanted)
//  - \begin, \end to gather all environments. But only figure and table
//    environments are passed to the structure view

// 2005-11-26: dani
//  - add support for \fref, \Fref and \eqref references commands

// 2005-12-07: dani
//  - add support to enable and disable some structure view items

// 2006-01-16 tbraun
// - fix #59945 Now we call (through a signal ) project->buildProjectTree so the bib files are correct,
//   and therefore the keys in \cite completion

// 2006-02-09 tbraun/dani
// - fix #106261#4 improved parsing of (optional) command parameters
// - all comments are removed

//2006-09-09 mludwig
// - generalising the different document types

//2007-02-15
// - signal foundItem() not only sends the cursor position of the parameter,
//   but also the real cursor position of the command

// 2007-03-12 dani
//  - use KileDocument::Extensions

// 2007-03-24 dani
// - preliminary minimal support for Beamer class

// 2007-03-25 dani
// - merge labels and sections in document structure view as user configurable option

// 2007-04-06 dani
// - add TODO/FIXME section to structure view

#include "documentinfo.h"

#include <config.h>

#include <QDateTime>
#include <QFileInfo>
#include <QInputDialog>
#include <QRegExp>

#include <KConfig>
#include <KIconLoader>
#include <KJobWidgets>
#include <KIO/StatJob>
#include <KLocalizedString>
#include <KMessageBox>

#include "abbreviationmanager.h"
#include "codecompletion.h"
#include "configurationmanager.h"
#include "editorextension.h"
#include "eventfilter.h"
#include "kileconfig.h"
#include "kiledebug.h"
#include "kileviewmanager.h"
#include "parser/bibtexparser.h"
#include "parser/latexparser.h"
#include "parser/parsermanager.h"
#include "livepreview.h"
#include "utilities.h"

namespace KileDocument
{

bool Info::containsInvalidCharacters(const QUrl &url)
{
	QString filename = url.fileName();
	return filename.contains(" ") || filename.contains("~") || filename.contains("$") || filename.contains("#");
}

QUrl Info::repairInvalidCharacters(const QUrl &url, QWidget* mainWidget, bool checkForFileExistence /* = true */)
{
	QUrl ret(url);
	do {
		bool isOK;
		QString newURL = QInputDialog::getText(
			mainWidget,
			i18n("Invalid Characters"),
			i18n("The filename contains invalid characters ($~ #).<br>Please provide "
			     "another one, or click \"Cancel\" to save anyway."),
			QLineEdit::Normal,
			ret.fileName(),
			&isOK);
		if(!isOK) {
			break;
		}
		ret = ret.adjusted(QUrl::RemoveFilename);
		ret.setPath(ret.path() + newURL);
	} while(containsInvalidCharacters(ret));

	return (checkForFileExistence ? renameIfExist(ret, mainWidget) : ret);
}

QUrl Info::renameIfExist(const QUrl &url, QWidget* mainWidget)
{
	QUrl ret(url);

	auto statJob = KIO::stat(url, KIO::StatJob::SourceSide, 0);
	KJobWidgets::setWindow(statJob, mainWidget);
	while (statJob->exec()) { // check for writing possibility
		bool isOK;
		QString newURL = QInputDialog::getText(
			mainWidget,
			i18n("File Already Exists"),
			i18n("A file with filename '%1' already exists.<br>Please provide "
			     "another one, or click \"Cancel\" to overwrite it.", ret.fileName()),
			QLineEdit::Normal,
			ret.fileName(),
			&isOK);
		if (!isOK) {
			break;
		}
		ret = ret.adjusted(QUrl::RemoveFilename);
		ret.setPath(ret.path() + newURL);
	}
	return ret;
}

QUrl Info::repairExtension(const QUrl &url, QWidget *mainWidget, bool checkForFileExistence /* = true */)
{
	QUrl ret(url);

	QString filename = url.fileName();
	if(filename.contains(".") && filename[0] != '.') // There already is an extension
		return ret;

	if(KMessageBox::Yes == KMessageBox::questionYesNo(Q_NULLPTR,
		i18n("The given filename has no extension; do you want one to be automatically added?"),
		i18n("Missing Extension"),
		KStandardGuiItem::yes(),
		KStandardGuiItem::no(),
		"AutomaticallyAddExtension"))
	{
		ret = ret.adjusted(QUrl::RemoveFilename);
		ret.setPath(ret.path() + filename + ".tex");
	}
	return (checkForFileExistence ? renameIfExist(ret, mainWidget) : ret);
}

QUrl Info::makeValidTeXURL(const QUrl &url, QWidget *mainWidget, bool istexfile, bool checkForFileExistence)
{
	QUrl newURL(url);

	//add a .tex extension
	if(!istexfile) {
		newURL = repairExtension(newURL, mainWidget, checkForFileExistence);
	}

	//remove characters TeX does not accept, make sure the newURL does not exists yet
	if(containsInvalidCharacters(newURL)) {
		newURL = repairInvalidCharacters(newURL, mainWidget, checkForFileExistence);
	}

	return newURL;
}

Info::Info() :
 m_bIsRoot(false),
 m_dirty(false),
 m_config(KSharedConfig::openConfig().data()),
 documentTypePromotionAllowed(true)
{
	updateStructLevelInfo();
}

Info::~Info(void)
{
	KILE_DEBUG_MAIN << "DELETING DOCINFO" << this;
}

void Info::updateStructLevelInfo()
{
	KILE_DEBUG_MAIN << "===void Info::updateStructLevelInfo()===";
	// read config for structureview items
	m_showStructureLabels = KileConfig::svShowLabels();
	m_showStructureReferences = KileConfig::svShowReferences();
	m_showStructureBibitems = KileConfig::svShowBibitems();
	m_showStructureGraphics = KileConfig::svShowGraphics();
	m_showStructureFloats = KileConfig::svShowFloats();
	m_showStructureInputFiles = KileConfig::svShowInputFiles();
	m_showStructureTodo = KileConfig::svShowTodo();
	m_showSectioningLabels = KileConfig::svShowSectioningLabels();
	m_openStructureLabels = KileConfig::svOpenLabels();
	m_openStructureReferences = KileConfig::svOpenReferences();
	m_openStructureBibitems = KileConfig::svOpenBibitems();
	m_openStructureTodo = KileConfig::svOpenTodo();
}

void Info::setBaseDirectory(const QUrl &url)
{
	KILE_DEBUG_MAIN << "===void Info::setBaseDirectory(const QUrl&" << url << ")===";
	m_baseDirectory = url;
}

const QUrl &Info::getBaseDirectory() const
{
	return m_baseDirectory;
}

bool Info::isTextDocument()
{
	return false;
}

Type Info::getType()
{
	return Undefined;
}

QLinkedList<Extensions::ExtensionType> Info::getFileFilter() const
{
	return {};
}

bool Info::isDocumentTypePromotionAllowed()
{
	return documentTypePromotionAllowed;
}

void Info::setDocumentTypePromotionAllowed(bool b)
{
	documentTypePromotionAllowed = b;
}

bool Info::isDirty() const
{
	return m_dirty;
}

void Info::setDirty(bool b)
{
	m_dirty = b;
}

void Info::installParserOutput(KileParser::ParserOutput *parserOutput)
{
	Q_UNUSED(parserOutput);
}

QUrl Info::url()
{
	return QUrl();
}

void Info::count(const QString& line, long *stat)
{
	QChar c;
	int state = stStandard;
	bool word = false; // we are in a word

	int lineLength = line.length();
	for(int p = 0; p < lineLength; ++p) {
		c = line[p];

		switch(state) {
			case stStandard:
				if(c == TEX_CAT0) {
					state = stControlSequence;
					++stat[1];

					//look ahead to avoid counting words like K\"ahler as two words
					if( (p+1) < lineLength && ( !line[p+1].isPunct() || line[p+1] == '~' || line[p+1] == '^' )) {
						word = false;
					}
				}
				else if(c == TEX_CAT14) {
					state = stComment;
				}
				else {
					if (c.isLetterOrNumber()) {
						//only start new word if first character is a letter (42test is still counted as a word, but 42.2 not)
						if (c.isLetter() && !word) {
							word = true;
							++stat[3];
						}
						++stat[0];
					}
					else {
						++stat[2];
						word = false;
					}
				}
			break;

			case stControlSequence :
				if(c.isLetter()) {
				// "\begin{[a-zA-z]+}" is an environment, and you can't define a command like \begin
					if(line.mid(p, 5) == "begin") {
						++stat[5];
						state = stEnvironment;
						stat[1] +=5;
						p+=4; // after break p++ is executed
					}
					else if(line.mid(p, 3) == "end") {
						stat[1] +=3;
						state = stEnvironment;
						p+=2;
					} // we don't count \end as new environment, this can give wrong results in selections
					else {
						++stat[4];
						++stat[1];
						state = stCommand;
					}
				}
				else {
					++stat[4];
					++stat[1];
					state = stStandard;
				}
			break;

			case stCommand :
				if(c.isLetter()) {
					++stat[1];
				}
				else if(c == TEX_CAT0) {
					++stat[1];
					state = stControlSequence;
				}
				else if(c == TEX_CAT14) {
					state = stComment;
				}
				else {
					++stat[2];
					state = stStandard;
				}
			break;

			case stEnvironment :
				if(c == TEX_CAT2) { // until we find a closing } we have an environment
					++stat[1];
					state = stStandard;
				}
				else if(c == TEX_CAT14) {
					state = stComment;
				}
				else {
					++stat[1];
				}
			break;

			case stComment : // if we get a selection the line possibly contains \n and so the comment is only valid till \n and not necessarily till line.length()
				if(c == '\n') {
					++stat[2]; // \n was counted as punctuation in the old implementation
					state = stStandard;
					word = false;
				}
			break;

			default :
				qWarning() << "Unhandled state in getStatistics " << state;
			break;
		}
	}
}

void Info::updateStruct()
{
}

void Info::updateBibItems()
{
}

void Info::slotCompleted()
{
	setDirty(true);
	emit completed(this);
}

TextInfo::TextInfo(Extensions* extensions,
                   KileAbbreviation::Manager* abbreviationManager,
                   KileParser::Manager* parserManager,
                   const QString& defaultMode)
: m_doc(Q_NULLPTR),
  m_defaultMode(defaultMode),
  m_abbreviationManager(abbreviationManager),
  m_parserManager(parserManager)
{
	m_arStatistics = new long[SIZE_STAT_ARRAY];

	m_extensions = extensions;
	m_abbreviationCodeCompletionModel = new KileCodeCompletion::AbbreviationCompletionModel(this, m_abbreviationManager);
}

TextInfo::~TextInfo()
{
	emit(aboutToBeDestroyed(this));
	detach();
	delete [] m_arStatistics;
}


const KTextEditor::Document* TextInfo::getDoc() const
{
	return m_doc;
}

KTextEditor::Document* TextInfo::getDoc()
{
	return m_doc;
}

const KTextEditor::Document* TextInfo::getDocument() const
{
	return m_doc;
}

KTextEditor::Document* TextInfo::getDocument()
{
	return m_doc;
}

void TextInfo::setDoc(KTextEditor::Document *doc)
{
	setDocument(doc);
}

void TextInfo::setDocument(KTextEditor::Document *doc)
{
	KILE_DEBUG_MAIN << "===void TextInfo::setDoc(KTextEditor::Document *doc)===";

	if(m_doc == doc) {
		return;
	}

	detach();
	if(doc) {
		m_doc = doc;
		m_documentContents.clear();
		connect(m_doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(slotFileNameChanged()));
		connect(m_doc, SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SLOT(slotFileNameChanged()));
		connect(m_doc, SIGNAL(completed()), this, SLOT(slotCompleted()));
		connect(m_doc, SIGNAL(modifiedChanged(KTextEditor::Document*)), this, SLOT(makeDirtyIfModified()));
		// this could be a KatePart bug, and as "work-around" we manually set the highlighting mode again
		connect(m_doc, SIGNAL(completed()), this, SLOT(activateDefaultMode()));
		setMode(m_defaultMode);
		installEventFilters();
		registerCodeCompletionModels();
	}
}

void TextInfo::detach()
{
	if(m_doc) {
		m_doc->disconnect(this);
		removeInstalledEventFilters();
		removeSignalConnections();
		unregisterCodeCompletionModels();
		emit(documentDetached(m_doc));
	}
	m_doc = Q_NULLPTR;
}

void TextInfo::makeDirtyIfModified()
{
	if(m_doc && m_doc->isModified()) {
		setDirty(true);
	}
}

const long* TextInfo::getStatistics(KTextEditor::View *view)
{
	/* [0] = #c in words, [1] = #c in latex commands and environments,
	   [2] = #c whitespace, [3] = #words, [4] = # latex_commands, [5] = latex_environments */
	m_arStatistics[0] = m_arStatistics[1] = m_arStatistics[2] = m_arStatistics[3] = m_arStatistics[4] = m_arStatistics[5] = 0;

	QString line;

	if(view && view->selection()) {
		line = view->selectionText();
		KILE_DEBUG_MAIN << "line: " << line;
		count(line, m_arStatistics);
	}
	else if(m_doc) {
		for(int l = 0; l < m_doc->lines(); ++l) {
			line = m_doc->line(l);
			KILE_DEBUG_MAIN << "line : " << line;
			count(line, m_arStatistics);
		}
	}

	return m_arStatistics;
}

QUrl TextInfo::url()
{
	if(m_doc) {
		return m_doc->url();
	}
	else {
		return QUrl();
	}
}

Type TextInfo::getType()
{
	return Text;
}

bool TextInfo::isTextDocument()
{
	return true;
}

void TextInfo::setMode(const QString &mode)
{
	KILE_DEBUG_MAIN << "==Kile::setMode(" << m_doc->url() << "," << mode << " )==================";

	if (m_doc && !mode.isEmpty()) {
		m_doc->setMode(mode);
	}
}

void TextInfo::setHighlightingMode(const QString& highlight)
{
	KILE_DEBUG_MAIN << "==Kile::setHighlightingMode(" << m_doc->url() << "," << highlight << " )==================";

	if (m_doc && !highlight.isEmpty()) {
		m_doc->setHighlightingMode(highlight);
	}
}

void TextInfo::setDefaultMode(const QString& string)
{
	m_defaultMode = string;
}

// match a { with the corresponding }
// pos is the position of the {
QString TextInfo::matchBracket(QChar obracket, int &l, int &pos)
{
	QChar cbracket;
	if(obracket == '{') {
		cbracket = '}';
	}
	if(obracket == '[') {
		cbracket = ']';
	}
	if(obracket == '(') {
		cbracket = ')';
	}

	QString line, grab = "";
	int count=0, len;
	++pos;

	TodoResult todo;
	while(l <= m_doc->lines()) {
		line = getTextline(l,todo);
		len = line.length();
		for (int i=pos; i < len; ++i) {
			if(line[i] == '\\' && (line[i+1] == obracket || line[i+1] == cbracket)) {
				++i;
			}
			else if(line[i] == obracket) {
				++count;
			}
			else if(line[i] == cbracket) {
				--count;
				if (count < 0) {
					pos = i;
					return grab;
				}
			}

			grab += line[i];
		}
		++l;
		pos = 0;
	}

	return QString();
}

QString TextInfo::getTextline(uint line, TodoResult &todo)
{
	static QRegExp reComments("[^\\\\](%.*$)");

	todo.type = -1;
	QString s = m_doc->line(line);
	if(!s.isEmpty()) {
		// remove comment lines
		if(s[0] == '%') {
			searchTodoComment(s,0,todo);
			s.clear();
		}
		else {
			//remove escaped \ characters
			s.replace("\\\\", "  ");

			//remove comments
			int pos = s.indexOf(reComments);
			if(pos != -1) {
				searchTodoComment(s, pos,todo);
				s = s.left(reComments.pos(1));
			}
		}
	}
	return s;
}

void TextInfo::searchTodoComment(const QString &s, uint startpos, TodoResult &todo)
{
	static QRegExp reTodoComment("\\b(TODO|FIXME)\\b(:|\\s)?\\s*(.*)");

	if(s.indexOf(reTodoComment, startpos) != -1) {
		todo.type = (reTodoComment.cap(1) == "TODO") ? KileStruct::ToDo : KileStruct::FixMe;
		todo.colTag = reTodoComment.pos(1);
		todo.colComment = reTodoComment.pos(3);
		todo.comment = reTodoComment.cap(3).trimmed();
	}
}

KTextEditor::View* TextInfo::createView(QWidget *parent, const char* /* name */)
{
	if(!m_doc) {
		return Q_NULLPTR;
	}
	KTextEditor::View *view = m_doc->createView(parent);
	installEventFilters(view);
	installSignalConnections(view);
	registerCodeCompletionModels(view);
	view->setStatusBarEnabled(false);
	connect(view, SIGNAL(destroyed(QObject*)), this, SLOT(slotViewDestroyed(QObject*)));
	return view;
}

void TextInfo::startAbbreviationCompletion(KTextEditor::View *view)
{
	KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(!completionInterface) {
		return;
	}
	KTextEditor::Range range = m_abbreviationCodeCompletionModel->completionRange(view, view->cursorPosition());
	if(!range.isValid()) {
		range = KTextEditor::Range(view->cursorPosition(), view->cursorPosition());
	}
	completionInterface->startCompletion(range, m_abbreviationCodeCompletionModel);
}

void TextInfo::slotFileNameChanged()
{
	emit urlChanged(this, url());
}

void TextInfo::installEventFilters(KTextEditor::View *view)
{
	if(m_eventFilterHash.find(view) != m_eventFilterHash.end()) {
		return;
	}

	QList<QObject*> eventFilterList = createEventFilters(view);
	if(!eventFilterList.isEmpty()) {
		for(QList<QObject*>::iterator i = eventFilterList.begin(); i != eventFilterList.end(); ++i) {
			QObject* eventFilter = *i;
			KileView::Manager::installEventFilter(view, eventFilter);
		}
		m_eventFilterHash[view] = eventFilterList;
	}
}

void TextInfo::removeInstalledEventFilters(KTextEditor::View *view)
{
	QHash<KTextEditor::View*, QList<QObject*> >::iterator i = m_eventFilterHash.find(view);
	if(i != m_eventFilterHash.end()) {
		QList<QObject*> eventFilterList = *i;
		for(QList<QObject*>::iterator i2 = eventFilterList.begin(); i2 != eventFilterList.end(); ++i2) {
			QObject *eventFilter = *i2;
			KileView::Manager::removeEventFilter(view, eventFilter);
			delete(*i2);
		}
		m_eventFilterHash.erase(i);
	}
}

QList<QObject*> TextInfo::createEventFilters(KTextEditor::View* /* view */)
{
	return QList<QObject*>();
}

void TextInfo::installEventFilters()
{
	if(!m_doc) {
		return;
	}
	QList<KTextEditor::View*> views = m_doc->views();
	for(QList<KTextEditor::View*>::iterator i = views.begin(); i != views.end(); ++i) {
		installEventFilters(*i);
	}
}

void TextInfo::removeInstalledEventFilters()
{
	if(!m_doc) {
		return;
	}
	QList<KTextEditor::View*> views = m_doc->views();
	for(QList<KTextEditor::View*>::iterator i = views.begin(); i != views.end(); ++i) {
		removeInstalledEventFilters(*i);
	}
}

void TextInfo::installSignalConnections(KTextEditor::View *)
{
	/* does nothing */
}

void TextInfo::removeSignalConnections(KTextEditor::View *)
{
	/* does nothing */
}

void TextInfo::installSignalConnections()
{
	if(!m_doc) {
		return;
	}
	QList<KTextEditor::View*> views = m_doc->views();
	for(QList<KTextEditor::View*>::iterator i = views.begin(); i != views.end(); ++i) {
		installSignalConnections(*i);
	}
}

void TextInfo::removeSignalConnections()
{
	if(!m_doc) {
		return;
	}
	QList<KTextEditor::View*> views = m_doc->views();
	for(QList<KTextEditor::View*>::iterator i = views.begin(); i != views.end(); ++i) {
		removeSignalConnections(*i);
	}
}

void TextInfo::registerCodeCompletionModels(KTextEditor::View *view)
{
	KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(!completionInterface) {
		return;
	}
	completionInterface->registerCompletionModel(m_abbreviationCodeCompletionModel);
	completionInterface->setAutomaticInvocationEnabled(true);
}

void TextInfo::unregisterCodeCompletionModels(KTextEditor::View *view)
{
	KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(!completionInterface) {
		return;
	}
	completionInterface->unregisterCompletionModel(m_abbreviationCodeCompletionModel);
}

void TextInfo::registerCodeCompletionModels()
{
	if(!m_doc) {
		return;
	}
	QList<KTextEditor::View*> views = m_doc->views();
	for(QList<KTextEditor::View*>::iterator i = views.begin(); i != views.end(); ++i) {
		registerCodeCompletionModels(*i);
	}
}

void TextInfo::unregisterCodeCompletionModels()
{
	if(!m_doc) {
		return;
	}
	QList<KTextEditor::View*> views = m_doc->views();
	for(QList<KTextEditor::View*>::iterator i = views.begin(); i != views.end(); ++i) {
		unregisterCodeCompletionModels(*i);
	}
}

void TextInfo::slotViewDestroyed(QObject *object)
{
	KTextEditor::View* view = dynamic_cast<KTextEditor::View*>(object);
	if(view) {
		removeInstalledEventFilters(view);
		removeSignalConnections(view);
		unregisterCodeCompletionModels(view);
		QHash<KTextEditor::View*, QList<QObject*> >::iterator i = m_eventFilterHash.find(view);
		if(i != m_eventFilterHash.end()) {
			m_eventFilterHash.erase(i);
		}
	}
}

void TextInfo::activateDefaultMode()
{
	KILE_DEBUG_MAIN << "m_defaultMode = " <<  m_defaultMode << endl;

	if(m_doc && !m_defaultMode.isEmpty()) {
		m_doc->setMode(m_defaultMode);
	}
}

const QStringList TextInfo::documentContents() const
{
	if (m_doc) {
		return m_doc->textLines(m_doc->documentRange());
	}
	else {
		return m_documentContents;
	}
}

void TextInfo::setDocumentContents(const QStringList& contents)
{
	m_documentContents = contents;
}

LaTeXInfo::LaTeXInfo(Extensions* extensions,
                     KileAbbreviation::Manager* abbreviationManager,
                     LatexCommands* commands,
                     EditorExtension* editorExtension,
                     KileConfiguration::Manager* manager,
                     KileCodeCompletion::Manager* codeCompletionManager,
                     KileTool::LivePreviewManager* livePreviewManager,
                     KileParser::Manager* parserManager)
: TextInfo(extensions, abbreviationManager, parserManager, "LaTeX"),
  m_commands(commands),
  m_editorExtension(editorExtension),
  m_configurationManager(manager),
  m_eventFilter(Q_NULLPTR),
  m_livePreviewManager(livePreviewManager)
{
	documentTypePromotionAllowed = false;
	updateStructLevelInfo();
	m_latexCompletionModel = new KileCodeCompletion::LaTeXCompletionModel(this,
	                                                                      codeCompletionManager,
	                                                                      editorExtension);
}

LaTeXInfo::~LaTeXInfo()
{
}

Type LaTeXInfo::getType()
{
	return LaTeX;
}

QLinkedList<Extensions::ExtensionType> LaTeXInfo::getFileFilter() const
{
	return {Extensions::TEX, Extensions::PACKAGES};
}

void LaTeXInfo::startLaTeXCompletion(KTextEditor::View *view)
{
	KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(!completionInterface) {
		return;
	}
	KTextEditor::Range range = m_latexCompletionModel->completionRange(view, view->cursorPosition());
	if(!range.isValid()) {
		range = KTextEditor::Range(view->cursorPosition(), view->cursorPosition());
	}
	completionInterface->startCompletion(range, m_latexCompletionModel);
}

void LaTeXInfo::updateStructLevelInfo() {

	KILE_DEBUG_MAIN << "===void LaTeXInfo::updateStructLevelInfo()===";

	// read config stuff
	Info::updateStructLevelInfo();

	// clear all entries
	m_dictStructLevel.clear();

	//TODO: make sectioning and bibliography configurable

	// sectioning
	m_dictStructLevel["\\part"] = KileStructData(1, KileStruct::Sect, "part");
	m_dictStructLevel["\\chapter"] = KileStructData(2, KileStruct::Sect, "chapter");
	m_dictStructLevel["\\section"] = KileStructData(3, KileStruct::Sect, "section");
	m_dictStructLevel["\\subsection"] = KileStructData(4, KileStruct::Sect, "subsection");
	m_dictStructLevel["\\subsubsection"] = KileStructData(5, KileStruct::Sect, "subsubsection");
	m_dictStructLevel["\\paragraph"] = KileStructData(6, KileStruct::Sect, "subsubsection");
	m_dictStructLevel["\\subparagraph"] = KileStructData(7, KileStruct::Sect, "subsubsection");

	// hidden commands
	m_dictStructLevel["\\usepackage"] = KileStructData(KileStruct::Hidden, KileStruct::Package);
	m_dictStructLevel["\\newcommand"] = KileStructData(KileStruct::Hidden, KileStruct::NewCommand);
	m_dictStructLevel["\\newlength"] = KileStructData(KileStruct::Hidden, KileStruct::NewCommand);
	m_dictStructLevel["\\newenvironment"] = KileStructData(KileStruct::Hidden, KileStruct::NewEnvironment);
	m_dictStructLevel["\\addunit"] = KileStructData(KileStruct::Hidden, KileStruct::NewCommand); // hack to get support for the fancyunits package until we can configure the commands in the gui (tbraun)
	m_dictStructLevel["\\DeclareMathOperator"] = KileStructData(KileStruct::Hidden, KileStruct::NewCommand); // amsmath package
	m_dictStructLevel["\\caption"] = KileStructData(KileStruct::Hidden,KileStruct::Caption);

	// bibitems
	if(m_showStructureBibitems) {
		m_dictStructLevel["\\bibitem"] = KileStructData(KileStruct::NotSpecified, KileStruct::BibItem, QString(), "bibs");
	}

	// graphics
	if(m_showStructureGraphics) {
		m_dictStructLevel["\\includegraphics"] = KileStructData(KileStruct::Object,KileStruct::Graphics, "graphics");
	}

	// float environments
	if(m_showStructureFloats) {
		m_dictStructLevel["\\begin"] = KileStructData(KileStruct::Object,KileStruct::BeginEnv);
		m_dictStructLevel["\\end"] = KileStructData(KileStruct::Hidden,KileStruct::EndEnv);

		// some entries, which could never be found (but they are set manually)
		m_dictStructLevel["\\begin{figure}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "figure-env");
		m_dictStructLevel["\\begin{figure*}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "figure-env");
		m_dictStructLevel["\\begin{table}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "table-env");
		m_dictStructLevel["\\begin{table*}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "table-env");
		m_dictStructLevel["\\begin{asy}"]=KileStructData(KileStruct::Object,KileStruct::BeginFloat, "image-x-generic");
		m_dictStructLevel["\\end{float}"]=KileStructData(KileStruct::Hidden,KileStruct::EndFloat);
	}

	// preliminary minimal beamer support
	m_dictStructLevel["\\frame"] = KileStructData(KileStruct::Object, KileStruct::BeamerFrame, "beamerframe");
	m_dictStructLevel["\\frametitle"] = KileStructData(KileStruct::Hidden, KileStruct::BeamerFrametitle);
	m_dictStructLevel["\\begin{frame}"] = KileStructData(KileStruct::Object, KileStruct::BeamerBeginFrame, "beamerframe");
	m_dictStructLevel["\\end{frame}"] = KileStructData(KileStruct::Hidden, KileStruct::BeamerEndFrame);
	m_dictStructLevel["\\begin{block}"] = KileStructData(KileStruct::Object, KileStruct::BeamerBeginBlock, "beamerblock");

	// add user-defined commands

	QStringList list;
	QStringList::ConstIterator it;

	// labels, we also gather them
	m_commands->commandList(list,KileDocument::CmdAttrLabel, false);
	for(it=list.constBegin(); it != list.constEnd(); ++it) {
		m_dictStructLevel[*it] = KileStructData(KileStruct::NotSpecified, KileStruct::Label, QString(), "labels");
	}

	// input files
	if(m_showStructureInputFiles) {
		m_commands->commandList(list, KileDocument::CmdAttrIncludes, false);
		for(it = list.constBegin(); it != list.constEnd(); ++it) {
			m_dictStructLevel[*it] = KileStructData(KileStruct::File, KileStruct::Input, "input-file");
		}
	}

	// references
	if(m_showStructureReferences) {
		m_commands->commandList(list, KileDocument::CmdAttrReference, false);
		for(it=list.constBegin(); it != list.constEnd(); ++it ) {
			m_dictStructLevel[*it] = KileStructData(KileStruct::Hidden, KileStruct::Reference);
		}
	}

	//bibliography commands
	m_commands->commandList(list,KileDocument::CmdAttrBibliographies, false);
	for(it=list.constBegin(); it != list.constEnd(); ++it) {
		m_dictStructLevel[*it] = KileStructData(0, KileStruct::Bibliography, "viewbib");
	}
}

QList<QObject*> LaTeXInfo::createEventFilters(KTextEditor::View *view)
{
	QList<QObject*> toReturn;
	QObject *eventFilter = new LaTeXEventFilter(view, m_editorExtension);
	connect(m_configurationManager, SIGNAL(configChanged()), eventFilter, SLOT(readConfig()));
	toReturn << eventFilter;
	return toReturn;
}

void LaTeXInfo::installSignalConnections(KTextEditor::View *view)
{
#if LIVEPREVIEW_AVAILABLE
	connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)),
	        m_livePreviewManager, SLOT(handleCursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)));
	connect(view->document(), SIGNAL(textChanged(KTextEditor::Document*)),
	        m_livePreviewManager, SLOT(handleTextChanged(KTextEditor::Document*)), Qt::UniqueConnection);
	connect(view->document(), SIGNAL(documentSavedOrUploaded(KTextEditor::Document*,bool)),
	        m_livePreviewManager, SLOT(handleDocumentSavedOrUploaded(KTextEditor::Document*,bool)), Qt::UniqueConnection);
#endif
}

void LaTeXInfo::removeSignalConnections(KTextEditor::View *view)
{
#if LIVEPREVIEW_AVAILABLE
	disconnect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)),
	           m_livePreviewManager, SLOT(handleCursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)));
	disconnect(view->document(), SIGNAL(textChanged(KTextEditor::Document*)),
	           m_livePreviewManager, SLOT(handleTextChanged(KTextEditor::Document*)));
	disconnect(view->document(), SIGNAL(documentSavedOrUploaded(KTextEditor::Document*,bool)),
	           m_livePreviewManager, SLOT(handleDocumentSavedOrUploaded(KTextEditor::Document*,bool)));
#endif
}

void LaTeXInfo::registerCodeCompletionModels(KTextEditor::View *view)
{
	KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(!completionInterface) {
		return;
	}
	completionInterface->registerCompletionModel(m_latexCompletionModel);
	completionInterface->setAutomaticInvocationEnabled(true);
	TextInfo::registerCodeCompletionModels(view);
}

void LaTeXInfo::unregisterCodeCompletionModels(KTextEditor::View *view)
{
	KTextEditor::CodeCompletionInterface* completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
	if(!completionInterface) {
		return;
	}
	completionInterface->unregisterCompletionModel(m_latexCompletionModel);
	TextInfo::unregisterCodeCompletionModels(view);
}

BracketResult LaTeXInfo::matchBracket(int &l, int &pos)
{
	BracketResult result;
	TodoResult todo;

	if(m_doc->line(l)[pos] == '[') {
		result.option = TextInfo::matchBracket('[', l, pos);
		int p = 0;
		while(l < m_doc->lines()) {
			if((p = getTextline(l, todo).indexOf('{', pos)) != -1) {
				pos = p;
				break;
			}
			else {
				pos = 0;
				++l;
			}
		}
	}

	if(m_doc->line(l)[pos] == '{') {
		result.line = l;
		result.col = pos;
		result.value  = TextInfo::matchBracket('{', l, pos);
	}

	return result;
}

void LaTeXInfo::updateStruct()
{
	KILE_DEBUG_MAIN << "==void TeXInfo::updateStruct: (" << url() << ")=========";

	m_parserManager->parseDocument(this);
}

void LaTeXInfo::checkChangedDeps()
{
	if(m_depsPrev != m_deps) {
		KILE_DEBUG_MAIN << "===void LaTeXInfo::checkChangedDeps()===, deps have changed"<< endl;
		emit(depChanged());
		m_depsPrev = m_deps;
	}
}

void LaTeXInfo::installParserOutput(KileParser::ParserOutput *parserOutput)
{
	KILE_DEBUG_MAIN;
	KileParser::LaTeXParserOutput *latexParserOutput = dynamic_cast<KileParser::LaTeXParserOutput*>(parserOutput);
	Q_ASSERT(latexParserOutput);
	if(!latexParserOutput) {
		KILE_DEBUG_MAIN << "wrong type given";
		return;
	}

	m_labels = latexParserOutput->labels;
	m_bibItems = latexParserOutput->bibItems;
	m_deps = latexParserOutput->deps;
	m_bibliography = latexParserOutput->bibliography;
	m_packages = latexParserOutput->packages;
	m_newCommands = latexParserOutput->newCommands;
	m_asyFigures = latexParserOutput->asyFigures;
	m_preamble = latexParserOutput->preamble;
	m_bIsRoot = latexParserOutput->bIsRoot;

	checkChangedDeps();
	emit(isrootChanged(isLaTeXRoot()));
	setDirty(false);
	emit(parsingComplete());
}

BibInfo::BibInfo(Extensions* extensions,
                 KileAbbreviation::Manager* abbreviationManager,
                 KileParser::Manager* parserManager,
                 LatexCommands* /* commands */)
: TextInfo(extensions, abbreviationManager, parserManager, "BibTeX")
{
	documentTypePromotionAllowed = false;
}

BibInfo::~BibInfo()
{
}

bool BibInfo::isLaTeXRoot()
{
	return false;
}

void BibInfo::updateStruct()
{
	m_parserManager->parseDocument(this);
}

void BibInfo::installParserOutput(KileParser::ParserOutput *parserOutput)
{
	KILE_DEBUG_MAIN;
	KileParser::BibTeXParserOutput *bibtexParserOutput = dynamic_cast<KileParser::BibTeXParserOutput*>(parserOutput);
	Q_ASSERT(bibtexParserOutput);
	if(!bibtexParserOutput) {
		KILE_DEBUG_MAIN << "wrong type given";
		return;
	}

	m_bibItems = bibtexParserOutput->bibItems;

	setDirty(false);
	emit(parsingComplete());
}

Type BibInfo::getType()
{
	return BibTeX;
}

QLinkedList<Extensions::ExtensionType> BibInfo::getFileFilter() const
{
	return {Extensions::BIB};
}

ScriptInfo::ScriptInfo(Extensions* extensions,
                       KileAbbreviation::Manager* abbreviationManager,
                       KileParser::Manager* parserManager)
: TextInfo(extensions, abbreviationManager, parserManager, "JavaScript")
{
	documentTypePromotionAllowed = false;
}

ScriptInfo::~ScriptInfo()
{
}

bool ScriptInfo::isLaTeXRoot()
{
	return false;
}

Type ScriptInfo::getType()
{
	return Script;
}

QLinkedList<Extensions::ExtensionType> ScriptInfo::getFileFilter() const
{
	return {Extensions::JS};
}

}

