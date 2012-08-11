/***************************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
                           (C) 2009 Thomas Braun (thomas.braun@virtuell-zuhause.de)

 ***************************************************************************************/

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

#include <QList>
#include <QSplitter>
#include <QStackedWidget>
#include <QString>
#include <QStringList>
#include <QToolBox>
#include <QWidget>
#include <QSignalMapper>

#include <kdeversion.h>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <ktogglefullscreenaction.h>
#include <KXmlGuiWindow>

#include "docpart.h"
#include "widgets/filebrowserwidget.h"
#include "kileinfo.h"
#include "kileactions.h"
#include "widgets/symbolview.h"
#include "widgets/commandview.h"

#include "outputinfo.h"
#include "latexoutputfilter.h"

#include "codecompletion.h"        // code completion (dani)
#include "editorextension.h"              // advanced editor (dani)
#include "kilehelp.h"              // kile help (dani)
#include "quickpreview.h"
#include "widgets/abbreviationview.h"

#define ID_HINTTEXT         301
#define ID_LINE_COLUMN      302
#define ID_VIEW_MODE        303
#define ID_SELECTION_MODE   304

#define KILERC_VERSION 7

class QFileInfo;
class QTimer;
class QSignalMapper;

class KToolBar;
class KAction;
class KActionMenu;
class KRecentFilesAction;
class KToggleFullScreenAction;
class KToggleToolBarAction;
class KMultiTabBar;

class KileLyxServer;
class KileProject;
class KileProjectItem;
class TemplateItem;
class KileAutoSaveJob;
class KileErrorHandler;

namespace KileAction { class TagData; }
namespace KileDocument { class Info; class TextInfo; class Extensions; }
namespace KileTool { class Manager; class Factory; }
namespace KileWidget { class LogWidget; class Output; class Konsole; class StructureWidget; class SideBar; class BottomBar; }

//TODO remove once we stop supporting pre 1.7 user tools
struct userItem
{
	QString name, tag;
};

/**
 * The Kile main class. It acts as information manager and DBUS interface. It also manages the main window.
 **/
class Kile : public KParts::MainWindow, public KileInfo
{
	Q_OBJECT

public:
	explicit Kile(bool allowRestore = true, QWidget *parent = 0, const char *name = 0);
	~Kile();

	int lineNumber();

public Q_SLOTS:
	void setCursor(const KUrl &, int, int);

	int run(const QString &);
	int runWith(const QString &, const QString &);
	void runArchiveTool();
	void runArchiveTool(const KUrl&);
	void showTip();

	void prepareForPart(const QString &);

	void updateModeStatus();
	void newCaption();
// 	void citeViewBib();

	void rebuildBibliographyMenu();

	// D-Bus Interface
	void openDocument(const QString & url);
	void closeDocument();
	void setActive();
	/**
	 * @param line : Jump to give line in current editor (can be called via DBUS interface).
	 **/
	void setLine(const QString &line);
	void openProject(const QString& proj);
	int runTool(const QString& tool);
	int runToolWithConfig(const QString &tool, const QString &config);
	void insertText(const QString &text);

protected:
	virtual bool queryExit();
	virtual bool queryClose();

private:
	QMap<QString,bool> m_dictMenuAction, m_dictMenuFile, m_dictMenuProject;

	KToolBar				*m_toolsToolBar;
	KActionMenu				*m_userHelpActionMenu;
	KSelectAction				*m_bibTagSettings;
	ToolbarSelectAction			*m_compilerActions, *m_viewActions, *m_convertActions, *m_quickActions;
	QList<KileAction::TagData>		m_listUserTags;
	QList<userItem>				m_listUserTools;
	QList<QAction*> 			m_listUserTagsActions, m_listQuickActions, m_listCompilerActions, m_listConverterActions, m_listViewerActions, m_listOtherActions;
	KActionMenu 				*m_bibTagActionMenu;
	KAction 				*m_paStop, *m_paPrint;
	KToggleAction 				*ModeAction, *WatchFileAction;
	KToggleAction 				*m_actionMessageView;
	KRecentFilesAction			*m_actRecentFiles;
	KToggleFullScreenAction			*m_pFullScreen;

	/* GUI */
	//widgets
	KileWidget::SideBar			*m_sideBar;
	KileWidget::AbbreviationView		*m_kileAbbrevView;
	QStackedWidget			*m_topWidgetStack;
	QSplitter 			*m_horizontalSplitter, *m_verticalSplitter;
	QToolBox			*m_toolBox;
	KileWidget::CommandViewToolBox *m_commandViewToolBox;
	KileWidget::SymbolView		*m_symbolViewMFUS, *m_symbolViewRelation, *m_symbolViewArrows, *m_symbolViewMiscMath, *m_symbolViewMiscText, *m_symbolViewOperators, *m_symbolViewUser, *m_symbolViewDelimiters, *m_symbolViewGreek, *m_symbolViewSpecial, *m_symbolViewCyrillic;
	KileWidget::CommandView *m_commandView;
	KToolBar			*m_latexOutputErrorToolBar;
    QMenu               *m_buildMenuTopLevel, *m_buildMenuCompile, *m_buildMenuConvert, *m_buildMenuViewer, *m_buildMenuOther, *m_buildMenuQuickPreview, *m_userTagMenu;

	//parts
	KParts::PartManager 		*m_partManager;
	QString 			m_wantState, m_currentState;
	QSignalMapper			*m_signalMapper;

	/* config */
	KSharedConfigPtr	m_config;
	int 			m_horSplitRight, m_horSplitLeft, m_verSplitTop, m_verSplitBottom;
	QStringList 		m_recentFilesList, m_listDocsOpenOnStart, m_listEncodingsOfDocsOpenOnStart, m_listProjectsOpenOnStart;

	KRecentFilesAction *m_actRecentProjects;

	QTimer *m_AutosaveTimer;

	KileLyxServer		*m_lyxserver;
	KileErrorHandler 	*m_errorHandler;

	QProgressBar		*m_parserProgressBar;
	QTimer			*m_parserProgressBarShowTimer;

	/* actions */
	void initSelectActions();
	void setupStatusBar();
	void setupSideBar();
	void setupProjectView();
	void setupStructureView();
	void setupScriptsManagementView();
	void setupCommandViewToolbox();
	void setupSymbolViews();
	void enableSymbolViewMFUS();
	void disableSymbolViewMFUS();
	void setupAbbreviationView();
	void setupBottomBar();
	void setupGraphicTools();
	void setupPreviewTools();
	void setupActions();
	QAction* createToolAction(QString toolName);
	void createToolActions();
	void setupTools();
	void updateUserDefinedMenus();
	void cleanUpActionList(QList<QAction*> &list, const QStringList &tools);
	void restoreLastSelectedAction();
	void saveLastSelectedAction();

	void createUserTagActions();
	void updateUserTagMenu();
	void readUserTagActions();
	void writeUserTagActions();

	void initMenu();
	void setMenuItems(QStringList &list, QMap<QString,bool> &dict);
	void updateMenu();
	bool updateMenuActivationStatus(QMenu *menu);

	void setViewerToolBars();

	KAction* createAction(const QString &text, const QString &name, const QObject *receiver = NULL, const char *member = NULL);
	KAction* createAction(const QString &text, const QString &name, const QString& iconName, const QObject *receiver = NULL, const char *member = NULL);
	KAction* createAction(const QString &text, const QString &name, const KShortcut& shortcut, const QObject *receiver = NULL, const char *member = NULL);
	KAction* createAction(const QString &text, const QString &name, const QString& iconName, const KShortcut& shortcut = KShortcut(), const QObject *receiver = NULL, const char *member = NULL);
	KAction* createAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver = NULL, const char *member = NULL);

	QString getMasterDocument() const;
	void setMasterDocument(const QString& fileName);
	void clearMasterDocument();

private Q_SLOTS:
	void toggleMasterDocumentMode();
	void toggleWatchFile();
	void showEditorWidget();
	void refreshStructure();

	void helpLaTex();

	bool resetPart();
	void activePartGUI(KParts::Part *);
	void updateGUI(const QString &wantingState);
	void enableKileGUI(bool);
	void slotToggleFullScreen();

	void restoreFilesAndProjects(bool allowRestore);
	void readGUISettings();
	void transformOldUserSettings();
	void readRecentFileSettings();
	void saveSettings();

	void readConfig();

	void generalOptions();
	void configureKeys();
	void configureToolbars();
	void slotPerformCheck();

	void aboutEditorComponent();

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

	void showDocInfo(KTextEditor::View *view = NULL);
	void convertToASCII(KTextEditor::Document *doc = 0);
	void convertToEnc(KTextEditor::Document *doc = 0);

	void cleanAll(KileDocument::TextInfo *docinfo = 0);
	void cleanBib();

	void findInFiles();
	void findInProjects();
	void grepItemSelected(const QString &abs_filename, int line);

	/* insert tags */
	/**
	 * @param td Inserts the TagData td into the current editor.
	 *
	 * It can wrap a tag around selected text.
	 **/
	void insertTag(const KileAction::TagData& td);
	/**
	* @param td Inserts the TagData td into the current editor
	* @param pkgs list of packages needed for this command
	*
	* warns if latex packages in pkgs are not included in the document
	**/
	void insertTag(const KileAction::TagData& td, const QStringList& pkgs);

	/**
	 * An overloaded member function, behaves essentially as above.
	 **/

	void insertTag(const KileAction::TagData& td, const QList<Package>& pkgs);
	void insertTag(const QString& tagB, const QString& tagE, int dx, int dy);
	void insertAmsTag(const KileAction::TagData& td);
	void insertText(const QString &text, const QStringList &pkgs);
	void insertText(const QString &text, const QList<Package> &pkgs);

	void quickTabular();
	void quickArray();
	void quickTabbing();
	void quickDocument();
	void quickFloat();
	void quickMathenv();
	void quickPostscript();
	void quickTabulardialog(bool tabularenv);

	void editUserMenu();

	void includeGraphics();

	/* autosave */
	void autoSaveAll();
	void enableAutosave(bool);

	// QuickPreview
	void slotQuickPreview(int type);

	void quickPreviewEnvironment() { slotQuickPreview(KileTool::qpEnvironment); }
	void quickPreviewSelection()   { slotQuickPreview(KileTool::qpSelection);   }
	void quickPreviewSubdocument() { slotQuickPreview(KileTool::qpSubdocument); }
	void quickPreviewMathgroup()   { slotQuickPreview(KileTool::qpMathgroup);   }

	void addRecentFile(const KUrl& url);
	void removeRecentFile(const KUrl& url);
	void addRecentProject(const KUrl& url);
	void removeRecentProject(const KUrl& url);

	void updateStatusBarCursorPosition(KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
	void updateStatusBarViewMode(KTextEditor::View *view);
	void updateStatusBarInformationMessage(KTextEditor::View *view, const QString &message);
	void updateStatusBarSelection(KTextEditor::View *view);

	void connectDocumentInfoWithParserProgressBar(KileDocument::Info *info);
	void parsingStarted(int maxValue);
	void parsingCompleted();
};

#endif
