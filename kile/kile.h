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
#include <kdeversion.h>

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

#define KILERC_VERSION 5

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
class KileAutoSaveJob;
class KileSpell;
class KileErrorHandler;

namespace KileAction { class TagData; }
namespace KileTool { class Manager; class Factory; }
namespace KileWidget { class LogMsg; class Output; class Konsole; class Structure; }

//TODO remove once we stop supporting pre 1.7 user tools
struct userItem
{
	QString name, tag;
};

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
	void setCursor(const KURL &, int, int);
	void setActive();

/* actions */
private:
	void setupActions();
	void setupTools();
	void setupUserTagActions();
	void cleanUpActionList(QPtrList<KAction> &, const QStringList & tools);

	KToolBar					*m_toolsToolBar;
	KActionMenu 				*m_menuUserTags;
	QValueList<KileAction::TagData>	m_listUserTags;
	QValueList<userItem>			m_listUserTools;
	QPtrList<KAction> 			m_listUserTagsActions, m_listQuickActions, m_listCompilerActions, m_listConverterActions, m_listViewerActions, m_listOtherActions;
	KAction					*m_actionEditTag;
	KToggleToolBarAction		*m_paShowMainTB, *m_paShowToolsTB, *m_paShowBuildTB, *m_paShowErrorTB, *m_paShowEditTB, *m_paShowMathTB;
	KAction 					*m_paStop, *m_paPrint;
	KToggleAction 			*ModeAction, *StructureAction, *MessageAction, *WatchFileAction;
	KRecentFilesAction			*fileOpenRecentAction;
	KAction* m_pFullScreen;

/* GUI */
private:
	//widgets
	KMultiVertTabBar 		*ButtonBar;
	SymbolView 			*symbol_view;
	metapostview 			*mpview;
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
	bool 	showoutputview, m_bShowMainTB, m_bShowToolsTB, m_bShowBuildTB, m_bShowErrorTB, m_bShowEditTB, m_bShowMathTB, m_bFullScreen;

private slots:
	void ResetPart();
	void ActivePartGUI(KParts::Part * the_part);
	void showToolBars(const QString &);
	void enableKileGUI(bool enable);
	void slotToggleFullScreen();

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

private slots:
	/**
	 * Activates (sets up the GUI for the editor part) the view.
	 * @param updateStruct  If true, force an update of the structure view.
	 **/
	void activateView(QWidget* view , bool updateStruct = true);

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
	int lineNumber();

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

private slots:
	void runTool();

	void CleanAll(KileDocument::Info *docinfo = 0, bool silent = false);
	void CleanBib();

	void FindInFiles();
	void GrepItemSelected(const QString &abs_filename, int line);

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

// 	void insertUserTag(int i);
// 	void insertUserTag(const KileAction::TagData& td);
	void EditUserMenu();

	void includeGraphics();

private:
	KileLyxServer		*m_lyxserver;
	bool				m_bShowUserMovedMessage;

private:
	KileHelp::Help *m_help;
	KileErrorHandler *m_errorHandler;
	KileSpell		*m_spell;
};

#endif
