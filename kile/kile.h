/***************************************************************************
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

#include <kdeversion.h>
#include <kate/view.h>
#include <kate/document.h>
#include <dcopobject.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>

#include <qsplitter.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qwidgetstack.h>
#include <qstring.h>
#include <qtoolbox.h>

#include "kileappIface.h"
#include "docpart.h"
#include "kilefileselect.h"
#include "metapostview.h"
#include "kileinfo.h"
#include "symbolview.h"

#include "latexoutputinfo.h"
#include "latexoutputfilter.h"

#include "codecompletion.h"        // code completion (dani)
#include "kileedit.h"              // advanced editor (dani)
#include "kilehelp.h"              // kile help (dani)
#include "quickpreview.h"

#define ID_HINTTEXT 301
#define ID_LINE_COLUMN 302

#define KILERC_VERSION 5

class QFileInfo;
class QTimer;
class QSignalMapper;
class QIconViewItem;

class KToolBar;
class KAction;
class KActionMenu;
class KRecentFilesAction;
class KToggleFullScreenAction;
class KToggleToolBarAction;
class KMultiTabBar;

class KileLyxServer;
class KileEventFilter;
class KileProject;
class KileProjectItem;
class KileProjectView;
class TemplateItem;
class KileAutoSaveJob;
class KileSpell;
class KileErrorHandler;
class KileSideBar;

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
	Kile( bool allowRestore = true, QWidget *parent = 0, const char *name = 0 );
	~Kile();

public slots:
	/**
	 * @param line : Jump to give line in current editor (can be called via DCOP interface).
	 **/
	void setLine( const QString &line);
	void setCursor(const KURL &, int, int);
	void setActive();
	int run(const QString &);
	int runWith(const QString &, const QString &);
	void openDocument(const QString & url);
	void fileSelected(const QString & url) { openDocument(url); } //backwards compatibility
	void closeDocument();

	void showTip();
	
/* actions */
private:
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    QWidget *m_focusWidget;

	void setupStatusBar();
	void setupSideBar();
	void setupProjectView();
	void setupStructureView();
	void setupSymbolViews();
	void setupBottomBar();
	void setupGraphicTools();
	void setupPreviewTools();
	void setupActions();
	void setupTools();
	void setupUserTagActions();
	void cleanUpActionList(QPtrList<KAction> &, const QStringList & tools);

	bool kateCompletionPlugin();
	void checkKateSettings();
	
	void initMenu();
	void setMenuItems(QStringList &list, QMap<QString,bool> &dict);
	void updateMenu();
	void updateActionList(QPtrList<KAction> *list, bool state);
	QMap<QString,bool> m_dictMenuAction, m_dictMenuFile, m_dictMenuProject;
	
	KToolBar						*m_toolsToolBar;
	KActionMenu 					*m_menuUserTags;
	QValueList<KileAction::TagData>	m_listUserTags;
	QValueList<userItem>			m_listUserTools;
	QPtrList<KAction> 				m_listUserTagsActions, m_listQuickActions, m_listCompilerActions, m_listConverterActions, m_listViewerActions, m_listOtherActions;
	KAction							*m_actionEditTag;
	KActionSeparator			*m_actionEditSeparator;
	KAction 						*m_paStop, *m_paPrint;
	KToggleAction 					*ModeAction, *WatchFileAction;
	KToggleAction 					*m_actionMessageView;
	KRecentFilesAction				*m_actRecentFiles;
	KToggleFullScreenAction			*m_pFullScreen;

/* GUI */
private:
	//widgets
	KileSideBar			*m_sideBar;
	metapostview		*m_mpview;
	QWidgetStack 		*m_topWidgetStack;
	QSplitter 			*m_horizontalSplitter, *m_verticalSplitter;
	QToolBox				*m_toolBox;
	SymbolView				*m_symbolViewRelation, *m_symbolViewArrows, *m_symbolViewMiscMath, *m_symbolViewMiscText, *m_symbolViewOperators, *m_symbolViewUser, *m_symbolViewDelimiters, *m_symbolViewGreek, *m_symbolViewSpecial, *m_symbolViewCyrillic;

	//parts
	KParts::PartManager 	*m_partManager;
	QString 				m_wantState, m_currentState;
	
private slots:
	void toggleMode();
	void toggleWatchFile();
	void showEditorWidget();
	void refreshStructure();

	void helpLaTex();

private slots:
	bool resetPart();
	void activePartGUI(KParts::Part *);
	void showToolBars(const QString &);
	void enableKileGUI(bool);
	void slotToggleFullScreen();

public slots:
	void prepareForPart(const QString &);

/* config */
private:
	KConfig			*m_config;
	int 			m_horSplitRight, m_horSplitLeft, m_verSplitTop, m_verSplitBottom;
	QStringList 		m_recentFilesList, m_listDocsOpenOnStart, m_listProjectsOpenOnStart;

	void setViewerToolBars();
signals:
	/**
	 * Emit this signal when the configuration is changed. Classes that read and write to the global KConfig object
	 * should connect to this signal so they can update their settings.
	 **/
	void configChanged();

private slots:
	void restoreFilesAndProjects(bool allowRestore);
	void readGUISettings();
	void readUserSettings();
	void readRecentFileSettings();
	void saveSettings();

	void readConfig();

	void generalOptions();
	void configureKeys();
	void configureToolbars();
	void slotPerformCheck();

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
	void focusPreview();

    void sideOrBottomBarChanged(bool visible);

/* document handling */
public slots:
	void load(const QString &path);

public slots:
	bool queryExit();
	bool queryClose();

	void changeInputEncoding();

	void newStatus(const QString& = QString::null);
	void updateModeStatus();
	void newCaption();

public slots:
	void openProject(const QString& proj);

private:
	KRecentFilesAction *m_actRecentProjects;

	//
	// documentinfo
	//
private slots:
	void showDocInfo(Kate::Document *doc = 0);
	void convertToASCII(Kate::Document *doc = 0);
	void convertToEnc(Kate::Document *doc = 0);

public:
	int lineNumber();

/* autosave */
private slots:
	void autoSaveAll();
	void enableAutosave(bool);

private:
	QTimer *m_AutosaveTimer;

private slots:
	void runTool();

	void cleanAll(KileDocument::Info *docinfo = 0);
	void cleanBib();

	void findInFiles();
	void findInProjects();
	void grepItemSelected(const QString &abs_filename, int line);

/* insert tags */
private slots:
	/**
	 * @param td Inserts the TagData td into the current editor.
	 *
	 * It can wrap a tag around selected text.
	 **/
	void insertTag(const KileAction::TagData& td);
	void insertAmsTag(const KileAction::TagData& td);
	/**
	 * An overloaded member function, behaves essentially as above.
	 **/
	void insertTag(const QString& tagB, const QString& tagE, int dx, int dy);
	void insertText(const QString &text);

	void quickTabular();
	void quickArray();
	void quickTabbing();
	void quickDocument();
	void quickFloat();
	void quickMathenv();
	void quickPostscript();
	void quickTabulardialog(bool tabularenv);

	void insertMetaPost(QListBoxItem *);

	void editUserMenu();

	void includeGraphics();

private:
	KileLyxServer		*m_lyxserver;
	bool				m_bShowUserMovedMessage;

private:
	KileErrorHandler 	*m_errorHandler;
	KileSpell			*m_spell;
	
// QuickPreview
private slots:
	void slotQuickPreview(int type);

	void quickPreviewEnvironment() { slotQuickPreview(KileTool::qpEnvironment); }
	void quickPreviewSelection()   { slotQuickPreview(KileTool::qpSelection);   }
	void quickPreviewSubdocument() { slotQuickPreview(KileTool::qpSubdocument); }
	void quickPreviewMathgroup()   { slotQuickPreview(KileTool::qpMathgroup);   }
};

#endif
