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
#include "messagewidget.h"
#include "structdialog.h"
#include "quickdocumentdialog.h"
#include "letterdialog.h"
#include "bibtexdialog.h"
#include "tabdialog.h"
#include "arraydialog.h"
#include "tabbingdialog.h"
#include "l2hdialog.h"
#include "gfe/qplotmaker.h"
#include "gfe/qplotdialog.h"
#include "docpart.h"
#include "symbolview.h"
#include "texkonsolewidget.h"
#include "kmultiverttabbar.h"
#include "kilefileselect.h"
#include "refdialog.h"
#include "metapostview.h"

#include "kileinfo.h"
#include "kiledocumentinfo.h"
#include "kileactions.h"

#include "commandprocess.h"

#define ID_HINTTEXT 301
#define ID_LINE_COLUMN 302

#define KILERC_VERSION 1

class QFileInfo;
class QTimer;
class QSignalMapper;
class KActionMenu;
class KRecentFilesAction;

class KileLyxServer;
class KileEventFilter;
class KileProject;
class KileProjectItem;
class KileProjectView;
class TemplateItem;

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
	Kile( QWidget *parent = 0, const char *name = 0 );
	~Kile();

public slots:
	/**
	 * @param line : Jump to give line in current editor (can be called via DCOP interface).
	 **/
	void setLine( const QString &line);
	
	void setActive();

/* actions */
private:
	void setupActions();
	void setupUserTagActions();
	void setupUserToolActions();

	/**
	 * Toggle between standard KDE shortcuts for the menus (such as Alt-F for the file menu) or no shortcuts.
	 **/
	void ToggleMenuShortcut(KMenuBar *bar, bool accelOn, const QString &accelText, const QString &noAccelText);
	/**
	 * Toggle between old (non-KDE compliant) shortcuts or KDE-compliant shortcuts
	 **/
	void ToggleKeyShortcut(KAction *action, bool addShiftModifier);
	bool									m_menuaccels; //TRUE : we're using KDE compliant shortcuts

	KActionMenu 					*m_menuUserTags, *m_menuUserTools;
	QSignalMapper 				*m_mapUserTagSignals, *m_mapUserToolsSignals;
	QValueList<userItem> 	m_listUserTags, m_listUserTools;
	QPtrList<KAction> 			m_listUserTagsActions, m_listUserToolsActions;
	KAction							*m_actionEditTag, *m_actionEditTool;

	KPopupMenu			*help;
	KHelpMenu				*help_menu;
	KAction 					*BackAction, *ForwardAction, *HomeAction, *StopAction;
	KToggleAction 		*ModeAction, *MenuAccelsAction, *StructureAction, *MessageAction, *WatchFileAction,
									*ShowMainToolbarAction, *ShowToolsToolbarAction, *ShowEditToolbarAction, *ShowMathToolbarAction;
	KAction 					*altH_action, *altI_action, *altA_action, *altB_action, *altT_action, *altC_action;
	KAction 					*altM_action, *altE_action, *altD_action, *altU_action, *altF_action, *altQ_action, *altS_action, *altL_action, *altR_action;
	KRecentFilesAction	*fileOpenRecentAction;



/* GUI */
private:
	//widgets
	KStatusBar 					*StatusBar;
	KileFileSelect 				*KileFS;
	KMultiVertTabBar 		*ButtonBar;
	SymbolView 				*symbol_view;
	metapostview 				*mpview;
    MessageWidget 			*OutputWidget, *LogWidget;
    TexKonsoleWidget		*texkonsole;
	QTabWidget 				*tabWidget, *Outputview;
	QFrame 						*Structview;
	KileProjectView			*m_projectview;
	QHBoxLayout 				*Structview_layout;
	QWidgetStack 				*topWidgetStack;
	QSplitter 						*splitter1, *splitter2 ;
	KListView						*outstruct;
	QWidgetStack				*m_tabbarStack;

	//dialogs
	structdialog 					*stDlg;
	quickdocumentdialog 	*startDlg;
	refdialog 						*refDlg;
	letterdialog 					*ltDlg;
	tabdialog 						*quickDlg;
	arraydialog 					*arrayDlg;
	tabbingdialog 				*tabDlg;
	l2hdialog 						*l2hDlg;
	bibtexdialog					*BibtexDlg;

	//parts
	docpart 						*htmlpart;
	KParts::PartManager 	*partManager;
	KParts::ReadOnlyPart 	*pspart, *dvipart;

private slots:
	void ToggleMode();
	void ToggleAccels();
	void ToggleStructView();
	void ToggleOutputView();
	void ToggleWatchFile();
	void ToggleShowMainToolbar();
	void ToggleShowToolsToolbar();
	void ToggleShowEditToolbar();
	void ToggleShowMathToolbar();
	void ShowOutputView(bool change);
	void ShowEditorWidget();
	void showVertPage(int page);

	void BrowserBack();
	void BrowserForward();
	void BrowserHome();
	void LatexHelp();
	void invokeHelp();

private:
	bool 			showoutputview, showmaintoolbar,showtoolstoolbar, showedittoolbar, showmathtoolbar;

private slots:
	void ResetPart();
	void ActivePartGUI(KParts::Part * the_part);

public slots:
	void prepareForPart();
	
/* structure view */
private:
	bool 								showstructview;

private slots:
	void ShowStructView(bool change);
	void ShowStructure();
	void RefreshStructure();
	void UpdateStructure(bool parse = false);

	void ClickedOnStructure(QListViewItem *);
	void DoubleClickedOnStructure(QListViewItem *);

/* config */
private:
	KConfig				*config;
	int 						split1_right, split1_left, split2_top, split2_bottom, quickmode, lastvtab, m_defaultLevel;

	QString 		document_class, typeface_size, paper_size, document_encoding, author;
	QString 		lastDocument, input_encoding;
   	QString 		templAuthor, templDocClassOpt, templEncoding;
   	QString 		struct_level1, struct_level2, struct_level3, struct_level4, struct_level5;
   	QStringList 	recentFilesList, m_listDocsOpenOnStart, m_listProjectsOpenOnStart;
	bool 				ams_packages, makeidx_package;
	bool 				htmlpresent,pspresent, dvipresent, symbol_present, watchfile, color_mode;
	QStringList 	userClassList, userPaperList, userEncodingList, userOptionsList;

	bool				m_bCompleteEnvironment, m_bRestore, m_bCheckForRoot, m_runlyxserver, m_bQuick;

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
   	void updateNavAction( bool, bool);

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
	Kate::View						*m_activeView;

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
	void projectOpen(const KURL&);
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

	void storeProjectItem(KileProjectItem *item, Kate::Document *doc);

	KileProject* selectProject(const QString &);
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

private:
	KRecentFilesAction *m_actRecentProjects;

	//
	// documentinfo
	//
private slots:
	void showDocInfo(Kate::Document *doc = 0);

	//
	// implementation of:
	// KileInfo
	//
public:
	Kate::Document * activeDocument() const { Kate::View *view = currentView(); if (view) return view->getDoc(); else return 0;}

	const QStringList* labels();
	const QStringList* bibItems();
	const QStringList* bibliographies();

private:
	const QStringList* retrieveList(const QStringList* (KileDocumentInfo::*getit)() const);
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
	KShellProcess 		*currentProcess;
	QString 		latex_command, viewdvi_command, dvips_command, dvipdf_command,
					viewps_command, ps2pdf_command, makeindex_command, bibtex_command,
					pdflatex_command, viewpdf_command, l2h_options, bibtexeditor_command,
					viewlatexhelp_command;

signals:
	void stopProcess();

private slots:
	void slotProcessOutput(KProcess* proc,char* buffer,int buflen);
	void slotProcessExited(KProcess* proc);
	void slotDisableStop();
	void slotl2hExited(KProcess* proc);

	void QuickBuild();
	void EndQuickCompile();
	void QuickDviToPS();
	void QuickDviPDF();
	void QuickPS2PDF();

	CommandProcess* execCommand(const QStringList & command, const QFileInfo &file, bool enablestop, bool runonfile = true);
	QString 		prepareForCompile(const QString & command);
	QStringList 	prepareForConversion(const QString &command, const QString &from, const QString &to);
	QString 		prepareForViewing(const QString & command, const QString &ext, const QString &target = QString::null);

	void Latex();
	void ViewDvi();
	void KdviForwardSearch();
	void DviToPS();
	void ViewPS();
	void PDFLatex();
	void ViewPDF();
	void CleanAll();
	void MakeBib();
	void MakeIndex();
	void PStoPDF();
	void DVItoPDF();
	void syncTerminal();
	void RunTerminal(QWidget *w);
	void LatexToHtml();
	void MetaPost();
	void HtmlPreview();
	void Bibtexeditor();

	void CleanBib();
	QString DetectEpsSize(const QString &epsfile);

	void execUserTool(int);
	void EditUserTool();

/* log view, error handling */
private slots:
	void ViewLog();
	void ClickedOnOutput(int parag, int index);
	void QuickLatexError() { LatexError(false);}
	void LatexError(bool warnings=true);
	void NextError();
	void PreviousError();
	void NextWarning();
	void PreviousWarning();

private:
	void jumpToProblem(QStrList *, bool &, bool);

private:
	QString 		tempLog;
	bool 				logpresent;

	QStrList 		*errorlist;
	QStrList		*warnlist;
	int 				m_nErrors,m_nWarnings;
	bool 				m_bCheckForLaTeXErrors;
	bool 				m_bNewErrorlist, m_bNewWarninglist;


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
	void insertGraphic(const KileAction::TagData&);

	void QuickTabular();
	void QuickArray();
	void QuickTabbing();
	void QuickLetter();
	void QuickDocument();

	void insertSymbol(QIconViewItem*);
	void InsertMetaPost(QListBoxItem *);

	void insertUserTag(int i);
	void EditUserMenu();

/*LyX server*/
public slots:
	void insertCite(const QString&);
	void insertBibTeX(const QString&);
	void insertBibTeXDatabaseAdd(const QString&);

private:
	KileLyxServer		*m_lyxserver;

/* editor extensions */
private:
	KileEventFilter*	m_eventFilter;

/* external programs */
private slots:
	void RunXfig();
	void RunGfe();

private:
	QGuardedPtr<Qplotmaker> gfe_widget;

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
