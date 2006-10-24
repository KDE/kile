/*************************************************************************************
    begin                : Sun Jul 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2006 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEDOCUMENTINFO_H
#define KILEDOCUMENTINFO_H

#include <kate/document.h>
#include <kdebug.h>
#include <klistview.h>
#include <kurl.h>
#include <kdialogbase.h>
#include <latexcmd.h>

#include "kileconstants.h"

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

namespace KileStruct
{
	//Different types of elements in the structure view
	enum  
	{
		None = 0x1, Label = 0x2, Sect = 0x4, Input = 0x8,
		BibItem = 0x10, Bibliography = 0x20, Package = 0x40, NewCommand = 0x80, 
		Graphics = 0x100, Reference = 0x200, BeginEnv = 0x400, EndEnv = 0x800,
		BeginFloat = 0x1000, EndFloat = 0x2000,  Caption = 0x4000
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
	KileStructData(int lvl = 0, int tp = KileStruct::None, QString px = QString::null, QString fldr = "root" )  : level(lvl), type(tp), pix(px), folder(fldr) {}
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
	BracketResult() : option(QString::null), value(QString::null), line(0), col(0) {}
	QString option, value;
	int line, col;
};

class Info : public QObject
{
	Q_OBJECT

public:
	static bool isTeXFile(const KURL &);
	static bool isBibFile(const KURL &);
	static bool isScriptFile(const KURL & url);
	static bool containsInvalidCharacters(const KURL&);
	static KURL repairInvalidCharacters(const KURL&, bool checkForFileExistence = true);
	static KURL repairExtension(const KURL&, bool checkForFileExistence = true);
	static KURL makeValidTeXURL(const KURL & url, bool checkForFileExistence = true);
	static KURL renameIfExist(const KURL& url);

public:
	Info();
	~Info();

	const QStringList* labels() const{ return &m_labels; }
	const QStringList* bibItems() const { return &m_bibItems; }
	const QStringList* dependencies() const {return &m_deps; }
	const QStringList* bibliographies() const { return &m_bibliography; }
	const QStringList* packages() const { return &m_packages; }
	const QStringList* newCommands() const { return &m_newCommands; }

	QString lastModifiedFile(const QStringList *list = 0L);

	bool openStructureLabels() { return m_openStructureLabels; }
	bool openStructureReferences() { return m_openStructureReferences; }
	bool openStructureBibitems() { return m_openStructureBibitems; }

	bool showStructureLabels() { return m_showStructureLabels; }


	const QString & preamble() const { return m_preamble; }

	virtual bool isLaTeXRoot() { return m_bIsRoot; }

	virtual KURL url();
	
	void cleanTempFiles(const QStringList &  );

	virtual void updateStructLevelInfo();

	void setBaseDirectory(const KURL& url);
	const KURL& getBaseDirectory() const;

	virtual bool isTextDocument();
	virtual Type getType();

	/**
	 * Returns a file filter suitable for loading and saving files of this class' type.
	 **/
	virtual QString getFileFilter() const;

	virtual bool isDocumentTypePromotionAllowed();
	void setDocumentTypePromotionAllowed(bool b);

public slots:
	/**
	 * Never call this function directly, use KileWidget::Structure::update(KileDocument::Info *, bool) instead
	 **/
	virtual void updateStruct();
	virtual void updateBibItems();

signals:
	void urlChanged(const KURL& url);
	void isrootChanged(bool);

	void foundItem(const QString &title, uint line, uint column, int type, int level, const QString & pix, const QString & folder);
	void doneUpdating();
	void depChanged();

protected:
	void count(const QString line, long *stat);

protected:
	enum State
	{
		stStandard=0, stComment=1, stControlSequence=3, stControlSymbol=4,
	 	stCommand=5,stEnvironment=6
	};

protected:
	bool						m_bIsRoot;
	QStringList					m_labels;
	QStringList					m_bibItems;
	QStringList					m_deps;
	QStringList					m_bibliography;
	QStringList					m_packages;
	QStringList					m_newCommands;
	QString						m_preamble;
	QString						m_prevbib;
	QMap<QString,KileStructData>			m_dictStructLevel;
	KURL						m_url;
	KConfig						*m_config;
	bool m_showStructureLabels;
	bool m_showStructureBibitems;
	bool m_showStructureGraphics;
	bool m_showStructureFloats;
	bool m_showStructureReferences;
	bool m_showStructureInputFiles;
	bool m_openStructureLabels;
	bool m_openStructureReferences;
	bool m_openStructureBibitems;
	KURL						m_baseDirectory;
	bool						documentTypePromotionAllowed;
};


/**
 * The URL of a text document is managed directly by the corresponding Kate::Document.
 **/
class TextInfo : public Info
{
	Q_OBJECT
public:
	/**
	 * @param defaultHighlightMode the highlight mode that will be set automatically
	 *                             once a new document is installed
	 **/
	TextInfo(Kate::Document *doc, const QString& defaultHighlightMode = QString::null);
	virtual ~TextInfo();

	/**
	 * @returns the document for which this class is a decorator
	 **/
	const Kate::Document* getDoc() const;
	Kate::Document* getDoc();
	void setDoc(Kate::Document *doc);
	void detach();

	/**
	 * Used by @ref KileDocInfoDlg to display statistics of the Document.
	 * @returns an array with some statistical data of the document.
	 * The array is filled as follows: [0] = #c in words, [1] = #c in latex commands and environments,
	   [2] = #c whitespace, [3] = #words, [4] = # latex_commands, [5] = latex_environments **/

	virtual const long* getStatistics();

	/**
	 * @returns the URL of the Kate::Document.
	 **/
	virtual KURL url();

	virtual Type getType();

	bool isTextDocument();

	void setHighlightMode(const QString & highlight = QString::null);

	void setDefaultHightlightMode(const QString& string);

	/**
	 * "Overridden" method that installs custom event filters by using the "installEventFilters"
	 * method.
	 * @warning Only this method should be used to create new views for text documents !
	 * @return NULL if no document is set (m_doc == NULL)
	 **/
	KTextEditor::View* createView(QWidget *parent, const char *name=0);

protected slots:
	void slotFileNameChanged();

protected:
	Kate::Document			*m_doc;
	long				*m_arStatistics;
	QString				m_defaultHighlightMode;

	QString matchBracket(QChar c, uint &, uint &);
	QString getTextline(uint line);

	/**
	 * Installs an event filter on a view. Subclasses can override this method to
	 * provide custom event filters. The default implementation does nothing. Whenever this
	 * method is overridden, "removeInstalledEventFilters" should be overridden as well.
	 * @param view the view that is considered
	 **/
	virtual void installEventFilters(KTextEditor::View *view);

	/**
	 * Removes the event filters that were previously installed by the "installEventFilters"
	 * function. Subclasses can override this method to remove custom event filters. The
	 * default implementation does nothing.
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
	 * managed document object. The function "removeInstalledEventFilters(KTextEditor::View *view)
	 * function is used for a specific view.
	 **/
	void removeInstalledEventFilters();
};



class LaTeXInfo : public TextInfo
{
	Q_OBJECT

public:
	/**
	 * @param eventFilter the event filter that will be installed on managed documents
	 **/
	LaTeXInfo(Kate::Document *doc, LatexCommands *commands, const QObject* eventFilter);
	virtual ~LaTeXInfo();

	const long* getStatistics();

	virtual Type getType();

	virtual QString getFileFilter() const;

	static QString LaTeXFileFilter();

public slots:
	virtual void updateStruct();

protected:
	LatexCommands *m_commands;
	const QObject *m_eventFilter;

	virtual void updateStructLevelInfo();

	/**
	 * Installs a custom event filter.
	 **/
	virtual void installEventFilters(KTextEditor::View *view);

	/**
	 * Revmoves the custom event filter.
	 **/
	virtual void removeInstalledEventFilters(KTextEditor::View *view);

private:
	BracketResult matchBracket(uint &, uint &);
};



class BibInfo : public TextInfo
{
	Q_OBJECT

public:
	BibInfo (Kate::Document *doc, LatexCommands* commands);
	virtual ~BibInfo();

	virtual bool isLaTeXRoot();

	virtual Type getType();

	virtual QString getFileFilter() const;

	static QString BibTeXFileFilter();

public slots:
	virtual void updateStruct();
};

}
#endif

