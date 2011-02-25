/*************************************************************************************
    begin                : Sun Jul 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2006-2010 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCUMENTINFO_H
#define DOCUMENTINFO_H

#include <QHash>

#include <KTextEditor/Document>
#include <KUrl>

#include "kiledebug.h"

#include <latexcmd.h>

#include "kileconstants.h"
#include "kileextensions.h"

#define TEX_CAT0 '\\'
#define TEX_CAT1 '{'
#define TEX_CAT2 '}'
#define TEX_CAT3 '$'
#define TEX_CAT4 '&'
#define TEX_CAT6 '#'
#define TEX_CAT7 '^'
#define TEX_CAT8 '_'
#define TEX_CAT13 '~'
#define TEX_CAT14 '%'

#define SIZE_STAT_ARRAY 6

namespace KileDocument { class EditorExtension; }
namespace KileConfiguration { class Manager; }
namespace KileCodeCompletion { class LaTeXCompletionModel; class AbbreviationCompletionModel; class Manager; }
namespace KileAbbreviation { class Manager; }
namespace KileStruct
{
	//Different types of elements in the structure view
	enum
	{
		None = 0x1, Label = 0x2, Sect = 0x4, Input = 0x8,
		BibItem = 0x10, Bibliography = 0x20, Package = 0x40, NewCommand = 0x80,
		Graphics = 0x100, Reference = 0x200, BeginEnv = 0x400, EndEnv = 0x800,
		BeginFloat = 0x1000, EndFloat = 0x2000,  Caption = 0x4000, BeamerFrame = 0x8000,
		BeamerBeginFrame = 0x10000, BeamerEndFrame = 0x20000, BeamerFrametitle = 0x40000, BeamerBeginBlock = 0x80000,
		ToDo = 0x100000, FixMe = 0x200000, NewEnvironment = 0x400000
	};

	//Different levels (in the parent-child hierarchy) in the structure view
	enum
	{
		Hidden = -4, NotSpecified = -3, Object = -2, File = -1
	};
}

/**
 * A convenience class to store info about how LaTeX elements should appear in the
 * structure view. A QMap<QString, KileStructData> should be created, so that the
 * actual LaTeX elements can be mapped to this class.
 **/
class KileStructData
{
public:
	explicit KileStructData(int lvl = 0, int tp = KileStruct::None, QString px = QString(), QString fldr = "root" )  : level(lvl), type(tp), pix(px), folder(fldr) {}
	/** At which level the element should be visible **/
	int				level;
	/** The type of element (see @ref KileStruct) **/
	int 			type;
	/** The name of the icon that goes with this element. The icon is located using SmallIcon(pix). **/
	QString 	pix, folder;
};

/**
 * KileDocument::Info is a decorator class for the Document class. We can't derive a class from an interface
 * without implementing the interface, a decorator class is a way to add some functionality to the Document class.
 **/

namespace KileDocument
{

struct BracketResult
{
	BracketResult() : line(0), col(0) {}
	QString option, value;
	int line, col;
};

struct TodoResult
{
	int type;
	uint colTag;
	uint colComment;
	QString comment;
};

class Info : public QObject
{
	Q_OBJECT

public:
	static bool containsInvalidCharacters(const KUrl&);
	static KUrl repairInvalidCharacters(const KUrl&, QWidget *mainWidget, bool checkForFileExistence = true);
	static KUrl repairExtension(const KUrl& url, QWidget *mainWidget, bool checkForFileExistence = true);
	static KUrl makeValidTeXURL(const KUrl& url, QWidget *mainWidget, bool istexfile, bool checkForFileExistence = true);
	static KUrl renameIfExist(const KUrl& url, QWidget *mainWidget);

public:
	Info();
	~Info();

	QStringList labels() const{ return m_labels; }
	QStringList bibItems() const { return m_bibItems; }
	QStringList dependencies() const {return m_deps; }
	QStringList bibliographies() const { return m_bibliography; }
	QStringList packages() const { return m_packages; }
	QStringList newCommands() const { return m_newCommands; }
	QStringList asyFigures() const {return m_asyFigures; }

	QString lastModifiedFile(const QStringList& list);

	bool openStructureLabels() { return m_openStructureLabels; }
	bool openStructureReferences() { return m_openStructureReferences; }
	bool openStructureBibitems() { return m_openStructureBibitems; }
	bool openStructureTodo() { return m_openStructureTodo; }

	bool showStructureLabels() { return m_showStructureLabels; }


	const QString & preamble() const { return m_preamble; }

	virtual bool isLaTeXRoot() { return m_bIsRoot; }

	virtual KUrl url();

	virtual void updateStructLevelInfo();

	void setBaseDirectory(const KUrl& url);
	const KUrl& getBaseDirectory() const;

	virtual bool isTextDocument();
	virtual Type getType();

	/**
	 * Returns a file filter suitable for loading and saving files of this class' type.
	 **/
	virtual QString getFileFilter() const;

	virtual bool isDocumentTypePromotionAllowed();
	void setDocumentTypePromotionAllowed(bool b);

	/**
	 * Returns true iff new parsing is required.
	 **/
	bool isDirty() const;
	void setDirty(bool b);

public Q_SLOTS:
	/**
	 * Never call this function directly, use KileWidget::Structure::update(KileDocument::Info *, bool) instead
	 **/
	virtual void updateStruct();
	virtual void updateBibItems();

Q_SIGNALS:
	void urlChanged(KileDocument::Info* info, const KUrl& url);
	void isrootChanged(bool);

	void foundItem(const QString &title, uint line, uint column, int type, int level, uint startline, uint startcol,
	               const QString & pix, const QString & folder);
	void doneUpdating();
	void depChanged();
	void completed(KileDocument::Info* info);
	void parsingStarted(int maxValue);
	void parsingCompleted();
	void parsingUpdate(int value);

protected Q_SLOTS:
	void slotCompleted();

protected:
	void count(const QString& line, long *stat);

	enum State {
		stStandard = 0, stComment = 1, stControlSequence = 3, stControlSymbol = 4,
	 	stCommand = 5, stEnvironment = 6
	};

	bool						m_bIsRoot;
	bool						m_dirty;
	QStringList					m_labels;
	QStringList					m_bibItems;
	QStringList					m_deps, m_depsPrev;
	QStringList					m_bibliography;
	QStringList					m_packages;
	QStringList					m_newCommands;
	QStringList					m_asyFigures;
	QString						m_preamble;
	QMap<QString,KileStructData>			m_dictStructLevel;
	KConfig						*m_config;
	bool m_showStructureLabels;
	bool m_showStructureBibitems;
	bool m_showStructureGraphics;
	bool m_showStructureFloats;
	bool m_showStructureReferences;
	bool m_showStructureInputFiles;
	bool m_showStructureTodo;
	bool m_showSectioningLabels;
	bool m_openStructureLabels;
	bool m_openStructureReferences;
	bool m_openStructureBibitems;
	bool m_openStructureTodo;
	KUrl						m_baseDirectory;
	bool						documentTypePromotionAllowed;
	Extensions *m_extensions;
};


/**
 * The URL of a text document is managed directly by the corresponding KTextEditor::Document.
 **/
class TextInfo : public Info
{
	Q_OBJECT
public:
	/**
	 * @param defaultMode the mode that will be set automatically
	 *                    once a new document is installed
	 **/
	TextInfo(KTextEditor::Document *doc,
	         Extensions *extensions,
	         KileAbbreviation::Manager *abbreviationManager,
	         const QString& defaultMode = QString());
	virtual ~TextInfo();

	/**
	 * @returns the document for which this class is a decorator
	 **/
	const KTextEditor::Document* getDoc() const;
	KTextEditor::Document* getDoc();
	const KTextEditor::Document* getDocument() const;
	KTextEditor::Document* getDocument();
	void setDoc(KTextEditor::Document *doc);
	void setDocument(KTextEditor::Document *doc);
	void detach();

	/**
	 * Used by @ref KileDocInfoDlg to display statistics of the Document.
	 * @returns an array with some statistical data of the document.
	 * The array is filled as follows: [0] = #c in words, [1] = #c in latex commands and environments,
	   [2] = #c whitespace, [3] = #words, [4] = # latex_commands, [5] = latex_environments **/

	virtual const long* getStatistics(KTextEditor::View *view = NULL);

	/**
	 * @returns the URL of the KTextEditor::Document.
	 **/
	virtual KUrl url();

	virtual Type getType();

	bool isTextDocument();

	void setHighlightingMode(const QString& highlight = QString());

	void setMode(const QString& mode = QString());

	void setDefaultMode(const QString& string);

	/**
	 * "Overridden" method that installs custom event filters by using the "installEventFilters"
	 * method. It also installs signal connections by using the "installSignalConnections"
	 * method. 
	 * @warning Only this method should be used to create new views for text documents !
	 * @return NULL if no document is set (m_doc == NULL)
	 **/
	KTextEditor::View* createView(QWidget *parent, const char *name = NULL);

	void startAbbreviationCompletion(KTextEditor::View *view);

Q_SIGNALS:
	void documentDetached(KTextEditor::Document*);

protected Q_SLOTS:
	void slotFileNameChanged();
	void slotViewDestroyed(QObject *object);
	void activateDefaultMode();

	void makeDirtyIfModified();

protected:
	KTextEditor::Document				*m_doc;
	bool						m_dirty;
	long						*m_arStatistics;
	QString						m_defaultMode;
	QHash<KTextEditor::View*, QList<QObject*> >	m_eventFilterHash;
	KileAbbreviation::Manager			*m_abbreviationManager;
	KileCodeCompletion::AbbreviationCompletionModel *m_abbreviationCodeCompletionModel;

	QString matchBracket(QChar c, int &, int &);
	QString getTextline(uint line, TodoResult &todo);
	void searchTodoComment(const QString &s, uint startpos, TodoResult &todo);

	/**
	 * Creates the event filters that should be used on a view. Subclasses can override
	 * this method to provide custom event filters. The default implementation does nothing and
	 * returns an empty list. The event filters that are returned by this method are managed by
	 * the "installEventFilters", "removeInstalledEventFilters" methods.
	 * @warning The event filters that are created must be children of the view!
	 * @param view the view that is considered
	 **/
	virtual QList<QObject*> createEventFilters(KTextEditor::View *view);

	/**
	 * Installs event filters on a view. The function "createEventFilters(KTextEditor::View *view)
	 * function is used for a specific view.
	 * @param view the view that is considered
	 **/
	virtual void installEventFilters(KTextEditor::View *view);

	/**
	 * Removes the event filters that were previously installed by the "installEventFilters"
	 * function.
	 * @param view the view that is considered
	 **/
	virtual void removeInstalledEventFilters(KTextEditor::View *view);

	/**
	 * Installs the event filters on all the views that are currently open for the
	 * managed document object. The function "installEventFilters(KTextEditor::View *view)
	 * function is used for a specific view.
	 **/
	void installEventFilters();

	/**
	 * Removes the event filters from all the views that are currently open for the
	 * managed document object.
	 **/
	void removeInstalledEventFilters();

	/**
	 * Installs signal connections on a view.
	 */
	virtual void installSignalConnections(KTextEditor::View *view);

	/**
	 * Disconnects the signals that were previously connected with the 
	 * "installSignalConnections" function.
	 */
	virtual void removeSignalConnections(KTextEditor::View *view);

	/**
	 * Installs signal connections on all the views that are currently open for the
	 * managed document object. The function "installSignalConnections(KTextEditor::View *view)
	 * function is used for a specific view.
	 **/
	void installSignalConnections();

	/**
	 * Removes signal connections from all the views that are currently open for the
	 * managed document object.
	 **/
	void removeSignalConnections();

	/**
	 * Register code completion models on a view.
	 */
	virtual void registerCodeCompletionModels(KTextEditor::View *view);

	/**
	 * Unregisters the code completion models that were previously registered by the 
	 * "registerCodeCompletionModels" method.
	 */
	virtual void unregisterCodeCompletionModels(KTextEditor::View *view);

	/**
	 * Register code completion models on all the views that are currently open for the
	 * managed document object. The function "registerCodeCompletionModels(KTextEditor::View *view)
	 * function is used for a specific view.
	 **/
	void registerCodeCompletionModels();

	/**
	 * Unregister the code completion models from all the views that are currently open for the
	 * managed document object.
	 **/
	void unregisterCodeCompletionModels();
};



class LaTeXInfo : public TextInfo
{
	Q_OBJECT

public:
	/**
	 * @param eventFilter the event filter that will be installed on managed documents
	 **/
	LaTeXInfo(KTextEditor::Document *doc,
	          Extensions *extensions,
	          KileAbbreviation::Manager *abbreviationManager,
	          LatexCommands *commands,
	          KileDocument::EditorExtension *editorExtension,
	          KileConfiguration::Manager *manager,
	          KileCodeCompletion::Manager *codeCompletionManager);
	virtual ~LaTeXInfo();

	virtual Type getType();

	virtual QString getFileFilter() const;

	void startLaTeXCompletion(KTextEditor::View *view);

public Q_SLOTS:
	virtual void updateStruct();

protected:
	LatexCommands *m_commands;
	EditorExtension *m_editorExtension;
	KileConfiguration::Manager *m_configurationManager;
	QObject *m_eventFilter;
	KileCodeCompletion::LaTeXCompletionModel *m_latexCompletionModel;

	virtual void updateStructLevelInfo();
	virtual void checkChangedDeps();

	/**
	 * Creates a custom event filter.
	 */
	virtual QList<QObject*> createEventFilters(KTextEditor::View *view);

	virtual void installSignalConnections(KTextEditor::View *view);
	virtual void removeSignalConnections(KTextEditor::View *view);

	virtual void registerCodeCompletionModels(KTextEditor::View *view);
	virtual void unregisterCodeCompletionModels(KTextEditor::View *view);

private:
	BracketResult matchBracket(int &, int &);
};



class BibInfo : public TextInfo
{
	Q_OBJECT

public:
	BibInfo (KTextEditor::Document *doc,
                 Extensions *extensions,
                 KileAbbreviation::Manager *abbreviationManager,
                 LatexCommands* commands);
	virtual ~BibInfo();

	virtual bool isLaTeXRoot();

	virtual Type getType();

	virtual QString getFileFilter() const;

public Q_SLOTS:
	virtual void updateStruct();
};

class ScriptInfo : public TextInfo
{
	Q_OBJECT

public:
	ScriptInfo(KTextEditor::Document *doc,
	           Extensions *extensions,
                   KileAbbreviation::Manager *abbreviationManager);
	virtual ~ScriptInfo();

	virtual bool isLaTeXRoot();

	virtual Type getType();

	virtual QString getFileFilter() const;
};

}
#endif

