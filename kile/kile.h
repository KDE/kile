/***************************************************************************
                          kile.h  -  description
                             -------------------
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// last change: 24.01.2004 (dani)

#ifndef KILE_H
#define KILE_H

#include <kate/view.h>
#include <kate/document.h>

#include <kmainwindow.h>
#include <dcopobject.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <khelpmenu.h>
#include <kpopupmenu.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <kspell.h>
#include <kprocess.h>
#include <kaction.h>
#include <kfileitem.h>
#include <klistview.h>
#include <kio/job.h>
#include <kurl.h>

#include <qmap.h>
#include <qsplitter.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qguardedptr.h>
#include <qlayout.h>
#include <qframe.h>
#include <qstring.h>
#include <qcolor.h>

#include "kileappIface.h"
#include "docpart.h"
#include "symbolview.h"
#include "kmultiverttabbar.h"
#include "kilefileselect.h"
#include "metapostview.h"
#include "kileinfo.h"

#include "latexoutputinfo.h"
#include "latexoutputfilter.h"

#include "codecompletion.h"        // code completion (dani)
#include "kileedit.h"              // advanced editor (dani)
#include "kilehelp.h"              // kile help (dani)

#define ID_HINTTEXT 301
#define ID_LINE_COLUMN 302

#define KILERC_VERSION 4

class QFileInfo;
class QTimer;
class QSignalMapper;

class KToolBar;
class KActionMenu;
class KRecentFilesAction;
class KToggleToolBarAction;

class KileLyxServer;
class KileEventFilter;
class KileProject;
class KileProjectItem;
class KileProjectView;
class TemplateItem;

namespace KileAction { class TagData; }
namespace KileTool { class Manager; class Factory; }
namespace KileWidget { class LogMsg; class Output; class Konsole; class Structure; }

#ifndef KILE_USERITEM
struct userItem
{
	QString name, tag;
};
#define KILE_USERITEM
#endif

/**
 * @author Jeroen Wijnhout
 **/

/**
 * The Kile main class. It acts as the mainwindow, information manager and DCOP interface.
 **/
class Kile : public KParts::MainWindow, public KileAppDCOPIface, public KileInfo
{
	Q_OBJECT

public:
	Kile( bool restore = true, QWidget *parent = 0, const char *name = 0 );
	~Kile();

public slots:
	/**
	 * @param line : Jump to give line in current editor (can be called via DCOP interface).
	 **/
	void setLine( const QString &line);
	void setCursor(int, int);
	void setActive();

/* actions */
private:
	void setupActions();
	void setupTools();
	void setupUserTagActions();
	void cleanUpActionList(QPtrList<KAction> &, const QStringList & tools);

	KToolBar					*m_toolsToolBar;
	KActionMenu 				*m_menuUserTags;
	QSignalMapper 			*m_mapUserTagSignals;
	QValueList<userItem> 		m_listUserTags, m_listUserTools;
	QPtrList<KAction> 			m_listUserTagsActions, m_listQuickActions, m_listCompilerActions, m_listConverterActions, m_listViewerActions, m_listOtherActions;
	KAction					*m_actionEditTag, *m_actionEditTool;
	KToggleToolBarAction		*m_paShowMainTB, *m_paShowToolsTB, *m_paShowBuildTB, *m_paShowErrorTB, *m_paShowEditTB, *m_paShowMathTB;
	KAction 					*m_paStop, *m_paPrint;
	KToggleAction 			*ModeAction, *StructureAction, *MessageAction, *WatchFileAction;
	KRecentFilesAction			*fileOpenRecentAction;

/* GUI */
private:
	//widgets
	KileFileSelect 				*KileFS;
	KMultiVertTabBar 		*ButtonBar;
	SymbolView 				*symbol_view;
	metapostview 				*mpview;
	KileWidget::Output		*OutputWidget;
	KileWidget::LogMsg		*LogWidget;
	KileWidget::Konsole		*texkonsole;
	QTabWidget 				*tabWidget, *Outputview;
	QFrame 						*Structview;
	KileProjectView			*m_projectview;
	QHBoxLayout 				*Structview_layout;
	QWidgetStack 				*topWidgetStack;
	QSplitter 						*splitter1, *splitter2 ;
	KileWidget::Structure		*m_kwStructure;
	QWidgetStack				*m_tabbarStack;

	//parts
	KParts::PartManager 	*partManager;
	QString 		m_wantState, m_currentState;

private slots:
	void ToggleMode();
	void ToggleStructView();
	void ToggleOutputView();
	void ToggleWatchFile();
	void ShowOutputView(bool change);
	void ShowEditorWidget();
	void showVertPage(int page);

	void LatexHelp();

private:
	bool 			showoutputview, m_bShowMainTB, m_bShowToolsTB, m_bShowBuildTB, m_bShowErrorTB, m_bShowEditTB, m_bShowMathTB;

private slots:
	void ResetPart();
	void ActivePartGUI(KParts::Part * the_part);
	void enableKileGUI(bool enable);

public slots:
	void prepareForPart(const QString &);
	
/* structure view */
private:
	bool 								showstructview;

private slots:
	void ShowStructView(bool change);
	void ShowStructure();
	void RefreshStructure();
	void UpdateStructure(bool parse = false, KileDocumentInfo * docinfo = 0);

/* config */
private:
	KConfig				*config;
	int 						split1_right, split1_left, split2_top, split2_bottom, quickmode, lastvtab;
	QString 		struct_level1, struct_level2, struct_level3, struct_level4, struct_level5;
	QString 		document_class, typeface_size, paper_size, document_encoding, author;
	QString 		lastDocument, input_encoding;
   	QString 		templAuthor, templDocClassOpt, templEncoding;
   	QStringList 	recentFilesList, m_listDocsOpenOnStart, m_listProjectsOpenOnStart;
	bool 				symbol_present;
	QStringList 	userClassList, userPaperList, userEncodingList, userOptionsList;

	bool				m_bCompleteEnvironment, m_bRestore, m_runlyxserver, m_bQuick;

signals:
	/**
	 * Emit this signal when the configuration is changed. Classes that read and write to the global KConfig object
	 * should connect to this signal so they can update their settings.
	 **/
	void configChanged();

private slots:
	void restore();
	void ReadSettings();
	void ReadRecentFileSettings();
	void SaveSettings();

	void readConfig();

	void GeneralOptions();
	void ConfigureKeys();
	void ConfigureToolbars();

/* spell check */
private slots:
	void spellcheck();
	void spell_started ( KSpell *);
	void spell_progress (unsigned int percent);
	void spell_done(const QString&);
	void spell_finished();
	void corrected (const QString & originalword, const QString & newword, unsigned int pos);
	void misspelling (const QString & originalword, const QStringList & suggestions,unsigned int pos);

private:
	KSpell 			*kspell;
    int 				ks_corrected;
	int 				par_start, par_end, index_start, index_end;
    QString 		spell_text;


/* views */
protected:
	Kate::View* createView(Kate::Document *doc);
	/**
	 * This event filter captures WindowActivate events. On window activating it checks if
	 * any files were modified on disc. This function will be obsolete once we decide to use
	 * KDE3.2.
	 **/
	bool eventFilter (QObject* o, QEvent* e);
	bool m_bBlockWindowActivateEvents;

private slots:
	/**
	 * Activates (sets up the GUI for the editor part) the view.
	 * @param checkModified If true, check if the document that corresponds to this view is modified on disc.
	 * @param updateStruct  If true, force an update of the structure view.
	 **/
	void activateView(QWidget* view ,bool checkModified = true, bool updateStruct = true);
	void removeView(Kate::View *view);

	void focusLog();
	void focusOutput();
	void focusKonsole();
	void focusEditor();

public:
	Kate::View* currentView() const;
	QPtrList<Kate::View>& views() {return m_viewList;}

private:
	QPtrList<Kate::View> 		m_viewList;
	Kate::View				*m_activeView;

/* document handling */
public slots:
	/**
	 * Creates a document/view pair and loads the URL with the specified encoding
	 * (default encoding is the encoding corresponding to the current locale).
	 *
	 * @returns pointer to the new view
	 **/
	Kate::View* load( const KURL &url , const QString & encoding = QString::null, bool create = true, const QString & highlight  = QString::null, bool load = true, const QString &text = QString::null);
	void load(const QString &path) { load(KURL::fromPathOrURL(path));}
	Kate::View* loadTemplate(TemplateItem*);

public slots:
	void fileSelected(const QString & url) { fileSelected(KURL::fromPathOrURL(url)); }
	void fileSelected(const KURL &);
	void fileNew(const KURL &);

private slots:
	void fileNew();
	void fileOpen();
	void fileOpen(const KURL& url, const QString & = QString::null);
	void fileSaveAll(bool amAutoSaving = false);
	bool fileClose(const KURL & url, bool delDocinfo = false);
	bool fileClose(Kate::Document *doc = 0, bool delDocinfo = false );
	bool fileCloseAll();

	void saveURL(const KURL &);
	void fileSelected(const KFileItem *file);

	bool queryExit();
	bool queryClose();

	bool isOpen(const KURL & url);

	void setHighlightMode(Kate::Document * doc, const QString & highlight = QString::null);
	void changeInputEncoding();

	void newStatus(const QString& = QString::null);
	void updateModeStatus();
	void newCaption();

	void slotNameChanged(Kate::Document *);
	void newDocumentStatus(Kate::Document *);

	void gotoNextDocument();
	void gotoPrevDocument();

	void projectNew();
	void projectOpen();
	void projectOpen(const KURL&, int = 0, int = 1);
	void projectOpenItem(KileProjectItem *item);
	/**
	 * Saves the state of the project, if @param project is zero, the active project is saved.
	 **/
	void projectSave(KileProject * project = 0);
	void projectAddFiles(const KURL &);
	void projectAddFiles(KileProject * project = 0);
	void toggleArchive(const KURL &);
	bool projectArchive(const KURL &);
	bool projectArchive(KileProject *project  = 0);
	void buildProjectTree(KileProject *project = 0);
	void buildProjectTree(const KURL &);
	void projectOptions(const KURL &);
	void projectOptions(KileProject *project = 0);
	bool projectClose(const KURL & url = KURL());
	bool projectCloseAll();

	KileProject* selectProject(const QString &);
	void storeProjectItem(KileProjectItem *item, Kate::Document *doc);

	void addProject(const KileProject *project);
	void addToProject(const KURL &);
	void addToProject(KileProject *, const KURL &);
	void removeFromProject(const KURL &, const KURL &);

public slots:
	void projectOpen(const QString& proj) { projectOpen(KURL::fromPathOrURL(proj)); }

private:
	void sanityCheck();

signals:
	void projectTreeChanged(const KileProject *);
	void closingDocument(KileDocumentInfo *);

private:
	KRecentFilesAction *m_actRecentProjects;

	//
	// documentinfo
	//
private slots:
	void showDocInfo(Kate::Document *doc = 0);
	void convertToASCII(Kate::Document *doc = 0);
	void convertToEnc(Kate::Document *doc = 0);

	//
	// implementation of:
	// KileInfo
	//
public:
	Kate::Document * activeDocument() const { Kate::View *view = currentView(); if (view) return view->getDoc(); else return 0;}

	const QStringList* labels(KileDocumentInfo * info = 0);
	const QStringList* bibItems(KileDocumentInfo * info = 0);
	const QStringList* bibliographies(KileDocumentInfo * info = 0);

	int lineNumber();

private:
	const QStringList* retrieveList(const QStringList* (KileDocumentInfo::*getit)() const, KileDocumentInfo * docinfo = 0);
	QStringList m_listTemp;

/* autosave */
private slots:
	void autoSaveAll();
	void enableAutosave(bool);
	void setAutosaveInterval(long interval) { autosaveinterval=interval;}

private:
	QTimer *m_AutosaveTimer;

private:
	long autosaveinterval;
	bool autosave;

/* templates */
private slots:
	void createTemplate();
	void removeTemplate();
	void replaceTemplateVariables(QString &line);

/* tools */
private:
	QString 		latex_command, viewdvi_command, dvips_command, dvipdf_command,
					viewps_command, ps2pdf_command, makeindex_command, bibtex_command,
					pdflatex_command, viewpdf_command, l2h_options, bibtexeditor_command,
					viewlatexhelp_command;

private slots:
	void runTool();

	void CleanAll(KileDocumentInfo *docinfo = 0, bool silent = false);
	void CleanBib();

	void FindInFiles();
	void GrepItemSelected(const QString &abs_filename, int line);

/* log view, error handling */
private slots:
	void ViewLog();
	void NextError();
	void PreviousError();
	void NextWarning();
	void PreviousWarning();
	void NextBadBox();
	void PreviousBadBox();

private:
	void jumpToProblem(int type, bool);

private:
	int			m_nCurrentError;
	bool 			logpresent;

	bool 				m_bCheckForLaTeXErrors;
	bool 				m_bNewInfolist;
	KileTool::Manager		*m_manager;
	KileTool::Factory		*m_toolFactory;

/* insert tags */
private slots:
	/**
	 * @param td Inserts the TagData td into the current editor.
	 *
	 * It can wrap a tag around selected text.
	 **/
	void insertTag(const KileAction::TagData& td);
	/**
	 * An overloaded member function, behaves essentially as above.
	 **/
	void insertTag(const QString& tagB, const QString& tagE, int dx, int dy);

	void QuickTabular();
	void QuickArray();
	void QuickTabbing();
	void QuickLetter();
	void QuickDocument();

	void insertSymbol(QIconViewItem*);
	void InsertMetaPost(QListBoxItem *);

	void insertUserTag(int i);
	void EditUserMenu();

private:
	KileLyxServer		*m_lyxserver;
	bool				m_bShowUserMovedMessage;

/* editor extensions */
private:
	KileEventFilter*	m_eventFilter;

private:
	KileEdit *m_edit;                // advanced editor (dani)
	KileHelp::Help *m_help;          // kile help (dani)
	
	// CodeCompletion  (dani)
	CodeCompletion *m_complete;
	QTimer *m_completetimer;
	
	void editComplete(CodeCompletion::Mode mode);
	void editCompleteList(Kate::View *view, CodeCompletion::Type type);
	bool getCompleteWord(bool latexmode, QString &text, CodeCompletion::Type &type);
	bool oddBackslashes(const QString& text, int index);
	void gotoBullet(bool backwards);
   
private slots:
	// CodeCompletion action slots (dani)
	void editCompleteWord();
	void editCompleteEnvironment();
	void editCompleteAbbreviation();
	void editNextBullet();
	void editPrevBullet();
	
	// includegraphics (dani)
	void includeGraphics();
	
	// advanced editor (dani)
	void selectEnvInside();
	void selectEnvOutside();
	void deleteEnvInside();
	void deleteEnvOutside();
	void gotoBeginEnv();
	void gotoEndEnv();
	void matchEnv();
	void closeEnv();
	
	void selectTexgroupInside();
	void selectTexgroupOutside();
	void deleteTexgroupInside();
	void deleteTexgroupOutside();
	void gotoBeginTexgroup();
	void gotoEndTexgroup();
	void matchTexgroup();
	void closeTexgroup();

	void selectParagraph();
	void selectLine();
	void selectWord();
	void deleteParagraph();
	void deleteWord();
	
	void helpTetexGuide();
	void helpTetexDoc();
	void helpLatexIndex();
	void helpLatexCommand();
	void helpLatexSubject();
	void helpLatexEnvironment();
	void helpKeyword();

	public slots:
	// CodeCompletion public slots (dani)
	void slotCharactersInserted(int,int,const QString&);
	void slotCompletionDone( );
	void slotCompleteValueList();
	void slotCompletionAborted();
	void slotFilterCompletion(KTextEditor::CompletionEntry* c,QString *s);
};

class KileAutoSaveJob : public QObject
{
	Q_OBJECT

public:
	KileAutoSaveJob(const KURL& from);
	~KileAutoSaveJob();

protected slots:
	void slotResult(KIO::Job *);

signals:
	void success();
};

/**
 * This class is capable of intercepting key-strokes from the editor. It can complete a \begin{env}
 * with a \end{env} when enter is pressed.
 **/
class KileEventFilter : public QObject
{
	Q_OBJECT

public:
	KileEventFilter();

public slots:
	void readConfig();

protected:
	bool eventFilter(QObject *o, QEvent *e);

private:
	bool				m_bHandleEnter, m_bCompleteEnvironment;
	QRegExp		m_regexpEnter;

};

#endif
