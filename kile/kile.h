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
class KileEventFilter;
class KileAutoSaveJob;

namespace KileAction { class TagData; }
namespace KileTool { class Manager; class Factory; }
namespace KileWidget { class LogMsg; class Output; class Konsole; class Structure; }

//FIXME ugly, yuk!
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
	KMultiVertTabBar 		*ButtonBar;
	SymbolView 			*symbol_view;
	metapostview 			*mpview;
	KileWidget::Output		*OutputWidget;
	KileWidget::LogMsg		*LogWidget;
	QTabWidget 			 *Outputview;
	QFrame 				*Structview;
	QHBoxLayout 			*Structview_layout;
	QWidgetStack 			*topWidgetStack;
	QSplitter 				*splitter1, *splitter2 ;
	QWidgetStack			*m_tabbarStack;

	//parts
	KParts::PartManager 	*partManager;
	QString 				m_wantState, m_currentState;

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
	void showToolBars(const QString &);
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

/* config */
private:
	KConfig		*config;
	int 			split1_right, split1_left, split2_top, split2_bottom, quickmode, lastvtab;
	QString 		struct_level1, struct_level2, struct_level3, struct_level4, struct_level5;
	QString 		document_class, typeface_size, paper_size, document_encoding, author;
	QString 		lastDocument, input_encoding;
	QStringList 	recentFilesList, m_listDocsOpenOnStart, m_listProjectsOpenOnStart;
	bool 			symbol_present;
	QStringList 	userClassList, userPaperList, userEncodingList, userOptionsList;

	bool			m_bCompleteEnvironment, m_bRestore, m_runlyxserver, m_bQuick;

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
	void activateView(QWidget* view , bool checkModified = true, bool updateStruct = true);

	void focusLog();
	void focusOutput();
	void focusKonsole();
	void focusEditor();

/* document handling */
public slots:
	/**
	 * Creates a document/view pair and loads the URL with the specified encoding
	 * (default encoding is the encoding corresponding to the current locale).
	 *
	 * @returns pointer to the new view
	 **/
	void load(const QString &path);

public slots:
	void fileSelected(const QString & url);

	bool queryExit();
	bool queryClose();

	void changeInputEncoding();

	void newStatus(const QString& = QString::null);
	void updateModeStatus();
	void newCaption();

public slots:
	void projectOpen(const QString& proj);

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
	void QuickDocument();

	void insertSymbol(QIconViewItem*);
	void InsertMetaPost(QListBoxItem *);

	void insertUserTag(int i);
	void EditUserMenu();

	void includeGraphics();

private:
	KileLyxServer		*m_lyxserver;
	bool				m_bShowUserMovedMessage;

private:
	KileHelp::Help *m_help;          // kile help (dani)
};

#endif
