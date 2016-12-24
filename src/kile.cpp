/****************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
            (C) 2007-2015 by Michel Ludwig (michel.ludwig@kdemail.net)
            (C) 2007 Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2009 Thomas Braun (thomas.braun@virtuell-zuhause.de)
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-03-12 dani
//  - use KileDocument::Extensions

#include "kile.h"

#include <config.h>

#include <QAction>
#include <QHideEvent>
#include <QMenuBar>
#include <QPointer>
#include <QShowEvent>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QStatusBar>
#include <QXmlStreamWriter>

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KActionMenu>
#include <KConfigGroup>
#include <KEditToolBar>
#include <KHelpMenu>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KRun>
#include <KShortcutsDialog>
#include <KToggleAction>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>
#include <KSelectAction>
#include <KWindowSystem>
#include <KParts/BrowserExtension>

#if LIVEPREVIEW_AVAILABLE
#include <okular/interfaces/viewerinterface.h>
#endif

#include "abbreviationmanager.h"
#include "configurationmanager.h"
#include "documentinfo.h"
#include "errorhandler.h"
#include "kileactions.h"
#include "kiledebug.h"
#include "kilestdactions.h"
#include "widgets/statusbar.h"
#include "dialogs/configurationdialog.h"
#include "kileproject.h"
#include "widgets/projectview.h"
#include "dialogs/projectdialogs.h"
#include "kilelyxserver.h"
#include "dialogs/findfilesdialog.h"
#include "kiletool_enums.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "widgets/outputview.h"
#include "widgets/konsolewidget.h"
#include "dialogs/quickdocumentdialog.h"
#include "dialogs/tabbingdialog.h"
#include "widgets/structurewidget.h"
#include "convert.h"
#include "dialogs/includegraphicsdialog.h"
#include "kiledocmanager.h"
#include "kileversion.h"
#include "kileviewmanager.h"
#include "kileconfig.h"
#include "dialogs/configcheckerdialog.h"
#include "widgets/sidebar.h"
#include "dialogs/floatdialog.h"
#include "dialogs/mathenvironmentdialog.h"
#include "dialogs/tabular/newtabulardialog.h"
#include "dialogs/postscriptdialog.h"
#include "dialogs/pdf-wizard/pdfdialog.h"
#include "latexcmd.h"
#include "mainadaptor.h"
#include "dialogs/statisticsdialog.h"
#include "widgets/scriptsmanagementwidget.h"
#include "scriptmanager.h"
#include "widgets/previewwidget.h"
#include "symbolviewclasses.h"
#include "livepreview.h"
#include "parser/parsermanager.h"

#include "dialogs/usermenu/usermenudialog.h"
#include "usermenu/usermenudata.h"
#include "usermenu/usermenu.h"

#define LOG_TAB     0
#define OUTPUT_TAB  1
#define KONSOLE_TAB 2
#define PREVIEW_TAB 3

/*
 * Class Kile.
 */

Kile::Kile(bool allowRestore, QWidget *parent)
	: KParts::MainWindow()
	, KileInfo(this)
	, m_paPrint(Q_NULLPTR)
{
	setObjectName("Kile");

	// Under some circumstances (Qt or KDE issues like a KIO process still running (?)), Kile doesn't terminate
	// when the main window is closed (bugs 220343 and 299569). So, we force this here.
	// This still seems to happen with Qt 4.8.1 and KDE 4.8.2.
	connect(m_mainWindow, SIGNAL(destroyed(QObject*)), qApp, SLOT(quit())); //FIXME: KF5 is this still necessary?

	QSplashScreen splashScreen(QPixmap(QStandardPaths::locate(QStandardPaths::DataLocation, "pics/kile_splash.png")), Qt::WindowStaysOnTopHint);
	if(KileConfig::showSplashScreen()) {
		splashScreen.show();
		qApp->processEvents();
	}

	m_config = KSharedConfig::openConfig();

	m_codeCompletionManager = new KileCodeCompletion::Manager(this, parent);

	setStandardToolBarMenuEnabled(true);

	m_singlemode = true;

	m_viewManager= new KileView::Manager(this, actionCollection(), parent, "KileView::Manager");
	viewManager()->setClient(this);

	// process events for correctly displaying the splash screen
	qApp->processEvents();

	m_latexCommands = new KileDocument::LatexCommands(m_config.data(), this);  // at first (dani)
	m_edit = new KileDocument::EditorExtension(this);
	m_help = new KileHelp::Help(m_edit, this);
	m_partManager = new KParts::PartManager(this);
	m_errorHandler = new KileErrorHandler(this, this, actionCollection());
	m_quickPreview = new KileTool::QuickPreview(this);
	m_extensions = new KileDocument::Extensions();
	m_jScriptManager = new KileScript::Manager(this, m_config.data(), actionCollection(), parent, "KileScript::Manager");
	m_userMenu = Q_NULLPTR;

	connect(m_partManager, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartGUI(KParts::Part*)));

	// do initializations first
	m_currentState = "Editor";
	m_wantState = "Editor";
	m_bWatchFile = false;

	setStatusBar(new KileWidget::StatusBar(m_errorHandler, parent));

	// process events for correctly displaying the splash screen
	qApp->processEvents();

	connect(viewManager(), SIGNAL(currentViewChanged(QWidget*)), this, SLOT(newCaption()));
	connect(viewManager(), SIGNAL(currentViewChanged(QWidget*)), this, SLOT(activateView(QWidget*)));
	connect(viewManager(), SIGNAL(currentViewChanged(QWidget*)), this, SLOT(updateModeStatus()));
	connect(viewManager(), SIGNAL(updateCaption()), this, SLOT(newCaption()));
	connect(viewManager(), SIGNAL(updateModeStatus()), this, SLOT(updateModeStatus()));
	connect(viewManager(), &KileView::Manager::cursorPositionChanged, this, &Kile::updateStatusBarCursorPosition);
	connect(viewManager(), SIGNAL(viewModeChanged(KTextEditor::View*, KTextEditor::View::ViewMode)),
	        this, SLOT(updateStatusBarViewMode(KTextEditor::View*)));
	connect(viewManager(), SIGNAL(informationMessage(KTextEditor::View*,const QString&)),
	        this, SLOT(updateStatusBarInformationMessage(KTextEditor::View*,const QString&)));
	connect(viewManager(), SIGNAL(selectionChanged(KTextEditor::View*)),
	        this, SLOT(updateStatusBarSelection(KTextEditor::View*)));

	connect(docManager(), SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(newCaption()));
	connect(docManager(), SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SLOT(newCaption()));
	connect(docManager(), SIGNAL(documentReadWriteStateChanged(KTextEditor::Document*)), this, SLOT(newCaption()));

	m_topWidgetStack = new QStackedWidget();
	m_topWidgetStack->setFocusPolicy(Qt::NoFocus);

	m_horizontalSplitter = new QSplitter(Qt::Horizontal);

	setupSideBar();
	m_horizontalSplitter->addWidget(m_sideBar);

	m_verticalSplitter = new QSplitter(Qt::Vertical);
	m_horizontalSplitter->addWidget(m_verticalSplitter);
	viewManager()->createTabs(m_verticalSplitter);

	connect(viewManager(), SIGNAL(activateView(QWidget*, bool)), this, SLOT(activateView(QWidget*, bool)));
	connect(viewManager(), SIGNAL(prepareForPart(const QString& )), this, SLOT(prepareForPart(const QString& )));
	connect(viewManager(), SIGNAL(startQuickPreview(int)), this, SLOT(slotQuickPreview(int)) );

	connect(parserManager(), SIGNAL(documentParsingStarted()), this, SLOT(handleDocumentParsingStarted()));
	connect(parserManager(), SIGNAL(documentParsingComplete()), this, SLOT(handleDocumentParsingComplete()));

	m_signalMapper = new QSignalMapper(this);
	connect(m_signalMapper, SIGNAL(mapped(const QString &)),
             this, SLOT(runTool(const QString &)));

	// process events for correctly displaying the splash screen
	qApp->processEvents();

	setupBottomBar();
	m_verticalSplitter->addWidget(m_bottomBar);
	m_topWidgetStack->addWidget(m_horizontalSplitter);
	setCentralWidget(m_topWidgetStack);

	setupGraphicTools();
	setupPreviewTools();
	setupActions(); // sets up m_paStop

	// Parser manager and view manager must be created before the tool manager!
	m_manager = new KileTool::Manager(this, m_config.data(), m_outputWidget, m_partManager, m_topWidgetStack, m_paStop, 10000, actionCollection()); //FIXME make timeout configurable
	connect(m_manager, SIGNAL(requestGUIState(const QString &)), this, SLOT(prepareForPart(const QString &)));
	connect(m_manager, SIGNAL(jumpToFirstError()), m_errorHandler, SLOT(jumpToFirstError()));
	connect(m_manager, SIGNAL(previewDone()), this, SLOT(focusPreview()));

#if LIVEPREVIEW_AVAILABLE
	m_livePreviewManager = new KileTool::LivePreviewManager(this, actionCollection());
	connect(this, &Kile::masterDocumentChanged, m_livePreviewManager, &KileTool::LivePreviewManager::handleMasterDocumentChanged);

	if(viewManager()->viewerPart()) {
		m_bottomBar->addExtraWidget(m_livePreviewManager->getControlToolBar());
	}
	else { // live preview part couldn't be created
		delete m_livePreviewManager;
		m_livePreviewManager = Q_NULLPTR;
	}
#else
	m_livePreviewManager = Q_NULLPTR;
#endif

	m_toolFactory = new KileTool::Factory(m_manager, m_config.data(), actionCollection());
	m_manager->setFactory(m_toolFactory);

	initSelectActions();

	newCaption();

	m_help->setUserhelp(m_manager, m_userHelpActionMenu);     // kile user help (dani)

	// process events for correctly displaying the splash screen
	qApp->processEvents();

	connect(docManager(), SIGNAL(updateModeStatus()), this, SLOT(updateModeStatus()));
	connect(docManager(), SIGNAL(updateStructure(bool, KileDocument::Info*)), viewManager(), SLOT(updateStructure(bool, KileDocument::Info*)));
	connect(docManager(), SIGNAL(closingDocument(KileDocument::Info* )), m_kwStructure, SLOT(closeDocumentInfo(KileDocument::Info *)));
	connect(docManager(), SIGNAL(documentInfoCreated(KileDocument::Info* )), m_kwStructure, SLOT(addDocumentInfo(KileDocument::Info* )));
	connect(docManager(), SIGNAL(updateReferences(KileDocument::Info *)), m_kwStructure, SLOT(updateReferences(KileDocument::Info *)));
	connect(docManager(), SIGNAL(documentModificationStatusChanged(KTextEditor::Document*, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
	        viewManager(), SLOT(reflectDocumentModificationStatus(KTextEditor::Document*, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason)));

	transformOldUserSettings();
	transformOldUserTags();
	readGUISettings();
	readRecentFileSettings();
	readConfig();

	createToolActions(); // this creates the actions for the tools and user tags, which is required before 'activePartGUI' is called

	setupGUI(KXmlGuiWindow::StatusBar | KXmlGuiWindow::Save, "kileui.rc");
	m_partManager->setActivePart(Q_NULLPTR); // 'createGUI' is called in response to this

	// we can only do this here after the main GUI has been set up
	if(m_livePreviewManager && viewManager()->viewerPart()) {
		guiFactory()->addClient(viewManager()->viewerPart());

		QMenu *documentViewerMenu = static_cast<QMenu*>(guiFactory()->container("menu_document_viewer", this));
		QMenu *popup = static_cast<QMenu*>(guiFactory()->container("menu_okular_part_viewer", viewManager()->viewerPart()));
		if(documentViewerMenu && popup) {
			// we populate our menu with the actions from the part's menu
			documentViewerMenu->addActions(popup->actions());
			documentViewerMenu->setEnabled(false);
			connect(viewManager()->viewerPart(), SIGNAL(viewerMenuStateChange(bool)), documentViewerMenu, SLOT(setEnabled(bool)));
		}
		else {
			if(documentViewerMenu) {
				documentViewerMenu->setVisible(false);
			}
			delete popup;
		}
	}
	else { // we hide the special document viewer menu
		QMenu *documentViewerMenu = static_cast<QMenu*>(guiFactory()->container("menu_document_viewer", this));
		if(documentViewerMenu) {
			documentViewerMenu->setVisible(false);
		}
	}

	resize(KileConfig::mainwindowWidth(), KileConfig::mainwindowHeight());
	applyMainWindowSettings(m_config->group("KileMainWindow"));

	restoreLastSelectedAction(); // don't call this inside 'setupTools' as it is not compatible with KParts switching!
	QList<int> sizes;
	int verSplitTop = KileConfig::verticalSplitterTop();
	int verSplitBottom = KileConfig::verticalSplitterBottom();
	sizes << verSplitTop << verSplitBottom;
	m_verticalSplitter->setSizes(sizes);
	sizes.clear();
	int horSplitLeft = KileConfig::horizontalSplitterLeft();
	int horSplitRight = KileConfig::horizontalSplitterRight();
	if(horSplitLeft <= 0 && horSplitRight <= 0) { // compute default values
		horSplitLeft = width() / 4;
		horSplitRight = width() / 2; // leave some room for the viewer part
	}
	// the size of the third widget is computed from the sizes of the two other widgets
	sizes << horSplitLeft << horSplitRight << width() - (horSplitLeft + horSplitRight);
	m_horizontalSplitter->setSizes(sizes);

	show();
	if(KileConfig::showSplashScreen()) {
		splashScreen.finish(this);
	}

	// Due to 'processEvents' being called earlier we only create the DBUS adaptor and
	// the LyX server when all of Kile's structures have been set up.
	// publish the D-Bus interfaces
	new MainAdaptor(this);
	QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerObject("/main", this);
	dbus.registerService("net.sourceforge.kile"); // register under a constant name

	m_lyxserver = new KileLyxServer(KileConfig::runLyxServer());
	connect(m_lyxserver, SIGNAL(insert(const KileAction::TagData &)), this, SLOT(insertTag(const KileAction::TagData &)));

	if(m_listUserTools.count() > 0) {
		KMessageBox::information(0, i18n("You have defined some tools in the User menu. From now on these tools will be available from the Build->Other menu and can be configured in the configuration dialog (go to the Settings menu and choose Configure Kile). This has some advantages; your own tools can now be used in a QuickBuild command if you wish."), i18n("User Tools Detected"));
		m_listUserTools.clear();
	}

	if(KileConfig::rCVersion() < 7) {
		if (KMessageBox::questionYesNo(mainWindow(),
		    i18n("<p>The standard tool list needs to be reset as Kile has been upgraded.<br/>"
		         "This will overwrite any changes you have made.</p>"
		         "<p>Do you want to reset the list now (recommended)?</p>"),
			i18n("Tools need to be updated"))  == KMessageBox::Yes){
				m_toolFactory->readStandardToolConfig();
		}
	}
	else if(KileConfig::rCVersion() < 8) { // we need to add the livepreview tools
		m_toolFactory->installStandardLivePreviewTools();
	}

	// lazy creation: last possible place to insert this user-defined menu
	m_userMenu = new KileMenu::UserMenu(this, this);
	connect(m_userMenu, &KileMenu::UserMenu::sendText, this, static_cast<void (Kile::*)(const QString &)>(&Kile::insertText));
	connect(m_userMenu, &KileMenu::UserMenu::updateStatus, this, &Kile::slotUpdateUserMenuStatus);

	restoreFilesAndProjects(allowRestore);
	slotStateChanged("Editor");
	initMenu();
	updateModeStatus();

	// before Kile 2.1 shortcuts were stored in a "Shortcuts" group inside
	// Kile's configuration file, but this led to problems with the way of how shortcuts
	// are generally stored in kdelibs; we now delete the "Shortcuts" group if it
	// still present in Kile's configuration file.
	if(m_config->hasGroup("Shortcuts")) {
		KConfigGroup shortcutGroup = m_config->group("Shortcuts");
		actionCollection()->readSettings(&shortcutGroup);
		m_config->deleteGroup("Shortcuts");
	}

	// finally init all actions for the ScriptManager
	m_jScriptManager->initScriptActions();

	setUpdatesEnabled(false);
	setAutoSaveSettings(QLatin1String("KileMainWindow"),true);
	guiFactory()->refreshActionProperties();

	m_userMenu->refreshActionProperties();
	setUpdatesEnabled(true);

	// finally, we check whether the system check assistant should be run, which is important for
	// version 3.0 regarding the newly introduced live preview feature
	const QString& lastVersionRunFor = KileConfig::systemCheckLastVersionRunForAtStartUp();
	if(lastVersionRunFor.isEmpty() || compareVersionStrings(lastVersionRunFor, "2.9.60") < 0) {
		slotPerformCheck();
		KileConfig::setSystemCheckLastVersionRunForAtStartUp(kileFullVersion);
	}

	if(m_livePreviewManager) {
		m_livePreviewManager->buildLivePreviewMenu(m_config.data());
		m_livePreviewManager->disableBootUpMode();
	}
}

Kile::~Kile()
{
	KILE_DEBUG_MAIN << "cleaning up..." << endl;

	if(m_livePreviewManager && viewManager()->viewerPart()) {
		guiFactory()->removeClient(viewManager()->viewerPart());
	}
	delete m_userMenu;
	delete m_livePreviewManager;
	delete m_toolFactory;
	delete m_manager;
	delete m_quickPreview;
	delete m_edit;
	delete m_help;
	delete m_lyxserver; //QObject without parent, have to delete it ourselves
	delete m_latexCommands;
	delete m_extensions;
	delete m_viewManager;
}

// currently not usable due to https://bugs.kde.org/show_bug.cgi?id=194732
// void Kile::plugActionList(const QString& name, const QList<QAction*>& actionList)
// {
// 	plugActionList(name, actionList);
// }
//
// void Kile::unplugActionList(const QString& name)
// {
// 	unplugActionList(name);
// }

void Kile::setupSideBar()
{
	m_sideBar = new KileWidget::SideBar(m_horizontalSplitter);

	m_fileBrowserWidget = new KileWidget::FileBrowserWidget(m_extensions, m_sideBar);
	m_sideBar->addPage(m_fileBrowserWidget, QIcon::fromTheme("document-open"), i18n("Open File"));
	connect(m_fileBrowserWidget,SIGNAL(fileSelected(const KFileItem&)), docManager(), SLOT(fileSelected(const KFileItem&)));

	setupProjectView();
	setupStructureView();
	setupSymbolViews();
	setupScriptsManagementView();
	setupCommandViewToolbox();
	setupAbbreviationView();

	m_sideBar->switchToTab(KileConfig::selectedLeftView());
	m_sideBar->setVisible(KileConfig::sideBar());
	m_sideBar->setDirectionalSize(KileConfig::sideBarSize());
}

void Kile::setupProjectView()
{
	KileWidget::ProjectView *projectview = new KileWidget::ProjectView(m_sideBar, this);
// 	viewManager()->setProjectView(projectview);
	m_sideBar->addPage(projectview, QIcon::fromTheme("relation"), i18n("Files and Projects"));
	connect(projectview, SIGNAL(fileSelected(const KileProjectItem *)), docManager(), SLOT(fileSelected(const KileProjectItem *)));
	connect(projectview, SIGNAL(fileSelected(const QUrl &)), docManager(), SLOT(fileSelected(const QUrl &)));
	connect(projectview, SIGNAL(closeURL(const QUrl&)), docManager(), SLOT(fileClose(const QUrl&)));
	connect(projectview, SIGNAL(closeProject(const QUrl&)), docManager(), SLOT(projectClose(const QUrl&)));
	connect(projectview, SIGNAL(projectOptions(const QUrl&)), docManager(), SLOT(projectOptions(const QUrl&)));
	connect(projectview, SIGNAL(projectArchive(const QUrl&)), this, SLOT(runArchiveTool(const QUrl&)));
	connect(projectview, SIGNAL(removeFromProject(KileProjectItem *)), docManager(), SLOT(removeFromProject(KileProjectItem*)));
	connect(projectview, SIGNAL(addFiles(const QUrl &)), docManager(), SLOT(projectAddFiles(const QUrl &)));
	connect(projectview, SIGNAL(openAllFiles(const QUrl &)), docManager(), SLOT(projectOpenAllFiles(const QUrl &)));
	connect(projectview, SIGNAL(toggleArchive(KileProjectItem *)), docManager(), SLOT(toggleArchive(KileProjectItem *)));
	connect(projectview, SIGNAL(addToProject(const QUrl &)), docManager(), SLOT(addToProject(const QUrl &)));
	connect(projectview, SIGNAL(saveURL(const QUrl &)), docManager(), SLOT(saveURL(const QUrl &)));
	connect(projectview, SIGNAL(buildProjectTree(const QUrl &)), docManager(), SLOT(buildProjectTree(const QUrl &)));
	connect(docManager(), SIGNAL(projectTreeChanged(const KileProject *)), projectview, SLOT(refreshProjectTree(const KileProject *)));
	connect(docManager(), SIGNAL(removeFromProjectView(const QUrl &)),projectview,SLOT(remove(const QUrl &)));
	connect(docManager(), SIGNAL(removeFromProjectView(const KileProject *)),projectview,SLOT(remove(const KileProject *)));
	connect(docManager(), SIGNAL(addToProjectView(const QUrl &)),projectview,SLOT(add(const QUrl &)));
	connect(docManager(), SIGNAL(addToProjectView(const KileProject *)),projectview,SLOT(add(const KileProject *)));
	connect(docManager(),SIGNAL(removeItemFromProjectView(const KileProjectItem *, bool)),projectview,SLOT(removeItem(const KileProjectItem *, bool)));
	connect(docManager(),SIGNAL(addToProjectView(KileProjectItem *)),projectview,SLOT(add(KileProjectItem *)));
}

void Kile::setupStructureView()
{
	m_kwStructure = new KileWidget::StructureWidget(this, m_sideBar);
	m_sideBar->addPage(m_kwStructure, QIcon::fromTheme("view-list-tree"), i18n("Structure"));
	m_kwStructure->setFocusPolicy(Qt::ClickFocus);
	connect(configurationManager(), SIGNAL(configChanged()), m_kwStructure, SIGNAL(configChanged()));
	connect(m_kwStructure, SIGNAL(setCursor(const QUrl &,int,int)), this, SLOT(setCursor(const QUrl &,int,int)));
	connect(m_kwStructure, SIGNAL(fileOpen(const QUrl&, const QString & )), docManager(), SLOT(fileOpen(const QUrl&, const QString& )));
	connect(m_kwStructure, SIGNAL(fileNew(const QUrl&)), docManager(), SLOT(fileNew(const QUrl&)));
	connect(m_kwStructure, SIGNAL(sendText(const QString &)), this, SLOT(insertText(const QString &)));
	connect(m_kwStructure, SIGNAL(sectioningPopup(KileWidget::StructureViewItem*,int)), m_edit, SLOT(sectioningCommand(KileWidget::StructureViewItem*,int)));
}

void Kile::setupScriptsManagementView()
{
	m_scriptsManagementWidget = new KileWidget::ScriptsManagement(this, m_sideBar);
	m_sideBar->addPage(m_scriptsManagementWidget, QIcon::fromTheme("preferences-plugin-script"), i18n("Scripts"));
}

void Kile::enableSymbolViewMFUS()
{
	m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),true);

	connect(m_symbolViewRelation, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewOperators, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewArrows, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewMiscMath, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewMiscText, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewDelimiters, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewGreek, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewSpecial, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewCyrillic, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	connect(m_symbolViewUser, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
}

void Kile::disableSymbolViewMFUS()
{
	m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),false);
	m_toolBox->setItemToolTip(m_toolBox->indexOf(m_symbolViewMFUS),QString());

	disconnect(m_symbolViewRelation, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewOperators, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewArrows, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewMiscMath, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewMiscText, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewDelimiters, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewGreek, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewSpecial, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewCyrillic, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
	disconnect(m_symbolViewUser, &KileWidget::SymbolView::addToList ,m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
}

void Kile::setupSymbolViews()
{
	m_toolBox = new QToolBox(m_sideBar);
	m_sideBar->addPage(m_toolBox,QIcon::fromTheme("math0"),i18n("Symbols"));

	m_symbolViewMFUS = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::MFUS);
	m_toolBox->addItem(m_symbolViewMFUS,i18n("Most Frequently Used"));
	m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),false);
	connect(m_symbolViewMFUS, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewRelation = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Relation);
	m_toolBox->addItem(m_symbolViewRelation,QIcon::fromTheme("math1"),i18n("Relation"));
	connect(m_symbolViewRelation, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewOperators = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Operator);
	m_toolBox->addItem(m_symbolViewOperators,QIcon::fromTheme("math2"),i18n("Operators"));
	connect(m_symbolViewOperators, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewArrows = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Arrow);
	m_toolBox->addItem(m_symbolViewArrows,QIcon::fromTheme("math3"),i18n("Arrows"));
	connect(m_symbolViewArrows, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewMiscMath = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::MiscMath);
	m_toolBox->addItem(m_symbolViewMiscMath,QIcon::fromTheme("math4"),i18n("Miscellaneous Math"));
	connect(m_symbolViewMiscMath, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewMiscText = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::MiscText);
	m_toolBox->addItem(m_symbolViewMiscText,QIcon::fromTheme("math5"),i18n("Miscellaneous Text"));
	connect(m_symbolViewMiscText, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewDelimiters= new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Delimiters);
	m_toolBox->addItem(m_symbolViewDelimiters,QIcon::fromTheme("math6"),i18n("Delimiters"));
	connect(m_symbolViewDelimiters, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewGreek = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Greek);
	m_toolBox->addItem(m_symbolViewGreek,QIcon::fromTheme("math7"),i18n("Greek"));
	connect(m_symbolViewGreek, SIGNAL(insertText(const QString& ,const QList<Package>&)),
		 this, SLOT(insertText(const QString& ,const QList<Package>&)));

	m_symbolViewSpecial = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Special);
	m_toolBox->addItem(m_symbolViewSpecial,QIcon::fromTheme("math8"),i18n("Special Characters"));
	connect(m_symbolViewSpecial, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewCyrillic = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Cyrillic);
	m_toolBox->addItem(m_symbolViewCyrillic,QIcon::fromTheme("math10"),i18n("Cyrillic Characters"));
	connect(m_symbolViewCyrillic, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	m_symbolViewUser = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::User);
	m_toolBox->addItem(m_symbolViewUser,QIcon::fromTheme("math9"),i18n("User Defined"));
	connect(m_symbolViewUser, &KileWidget::SymbolView::insertText,
		this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

	for(int i = 0; i < m_toolBox->count(); ++i) {
		m_toolBox->setItemToolTip(i, i18n("<p>Move the mouse over the icons to see the corresponding LaTeX commands.<br/>"
		                                  "Click on an image to insert the corresponding command, additionally pressing \"Shift\" inserts "
		                                  "it in math mode, pressing \"Ctrl\" in curly brackets.</p>"));
	}
}

void Kile::setupCommandViewToolbox()
{
	m_commandViewToolBox = new KileWidget::CommandViewToolBox(this,m_sideBar);
	m_sideBar->addPage(m_commandViewToolBox,QIcon::fromTheme("texlion"),i18n("LaTeX"));

	connect(m_commandViewToolBox, SIGNAL(sendText(const QString &)),this, SLOT(insertText(const QString &)));
}

void Kile::setupAbbreviationView()
{
	m_kileAbbrevView = new KileWidget::AbbreviationView(abbreviationManager(), m_sideBar);
	connect(abbreviationManager(), SIGNAL(abbreviationsChanged()), m_kileAbbrevView, SLOT(updateAbbreviations()));;
	m_sideBar->addPage(m_kileAbbrevView, QIcon::fromTheme("complete3"), i18n("Abbreviation"));

	connect(m_kileAbbrevView, SIGNAL(sendText(const QString&)), this, SLOT(insertText(const QString&)));
}

void Kile::setupBottomBar()
{
	m_bottomBar = new KileWidget::BottomBar(this);
	m_bottomBar->setFocusPolicy(Qt::ClickFocus);

	connect(errorHandler(), SIGNAL(showingErrorMessage(QWidget* )), this, SLOT(focusLog()));

	QWidget *widget = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(widget);
	layout->setMargin(0);
	widget->setLayout(layout);

	m_latexOutputErrorToolBar = new KToolBar(widget);
	m_latexOutputErrorToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_latexOutputErrorToolBar->setIconDimensions(KIconLoader::SizeSmall);
	m_latexOutputErrorToolBar->setOrientation(Qt::Vertical);

	layout->addWidget(errorHandler()->outputWidget());
	layout->addWidget(m_latexOutputErrorToolBar);
	m_bottomBar->addPage(widget, QIcon::fromTheme("utilities-log-viewer"), i18n("Log and Messages"));

	m_outputWidget = new KileWidget::OutputView(this);
	m_outputWidget->setFocusPolicy(Qt::ClickFocus);
	m_outputWidget->setMinimumHeight(40);
	m_outputWidget->setReadOnly(true);
	m_bottomBar->addPage(m_outputWidget, QIcon::fromTheme("output_win"), i18n("Output"));

	m_texKonsole = new KileWidget::Konsole(this, this);
	m_bottomBar->addPage(m_texKonsole, QIcon::fromTheme("utilities-terminal"),i18n("Konsole"));
	connect(viewManager(), static_cast<void (KileView::Manager::*)(QWidget*)>(&KileView::Manager::currentViewChanged),
		m_texKonsole, static_cast<void (KileWidget::Konsole::*)(void)>(&KileWidget::Konsole::sync));

	m_previewWidget = new KileWidget::PreviewWidget(this, m_bottomBar);
	m_bottomBar->addPage(m_previewWidget, QIcon::fromTheme ("document-preview"), i18n ("Preview"));

	m_bottomBar->setVisible(true);
	m_bottomBar->switchToTab(KileConfig::bottomBarIndex());
	m_bottomBar->setDirectionalSize(KileConfig::bottomBarSize());
}

void Kile::setupGraphicTools()
{
	KileConfig::setImagemagick(!(QStandardPaths::findExecutable("identify").isNull()));
}

void Kile::setupPreviewTools()
{
	// search for tools
	bool dvipng = !(QStandardPaths::findExecutable("dvipng").isNull());
	bool convert = !(QStandardPaths::findExecutable("convert").isNull());

	KileConfig::setDvipng(dvipng);
	KileConfig::setConvert(convert);

	// disable some previews, if tools are missing
	if ( ! dvipng )
	{
		KileConfig::setMathgroupPreviewInWidget(false);  // no mathgroup preview in bottom bar
		if ( ! convert )
		{
			KileConfig::setEnvPreviewInWidget(false);     // no preview in bottom bar at all
			KileConfig::setSelPreviewInWidget(false);
		}
	}
}

QAction * Kile::createAction(const QString &text, const QString &name, const QObject *receiver, const char *member)
{
	return createAction(text, name, QString(), QKeySequence(), receiver, member);
}

QAction * Kile::createAction(const QString &text, const QString &name, const QKeySequence& shortcut, const QObject *receiver, const char *member)
{
	return createAction(text, name, QString(), shortcut, receiver, member);
}

QAction * Kile::createAction(const QString &text, const QString &name, const QString& iconName, const QObject *receiver, const char *member)
{
	return createAction(text, name, iconName, QKeySequence(), receiver, member);
}

QAction * Kile::createAction(const QString &text, const QString &name, const QString& iconName, const QKeySequence& shortcut, const QObject *receiver, const char *member)
{
	QAction *action = actionCollection()->addAction(name, receiver, member);
	action->setText(text);
	if(!shortcut.isEmpty()) {
		actionCollection()->setDefaultShortcut(action, shortcut);
	}
	if(!iconName.isEmpty()) {
		action->setIcon(QIcon::fromTheme(iconName));
	}
	return action;
}

QAction * Kile::createAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member)
{
	return actionCollection()->addAction(actionType, name, receiver, member);
}
void Kile::setupActions()
{
	QAction *act;

	m_paPrint = createAction(KStandardAction::Print, "file_print", Q_NULLPTR, Q_NULLPTR);
	createAction(KStandardAction::New, "file_new", docManager(), SLOT(fileNew()));
	createAction(KStandardAction::Open, "file_open", docManager(), SLOT(fileOpen()));

	m_actRecentFiles = static_cast<KRecentFilesAction*>(actionCollection()->addAction(KStandardAction::OpenRecent, "file_open_recent", docManager(), SLOT(fileOpen(const QUrl&))));
	connect(docManager(), SIGNAL(addToRecentFiles(const QUrl&)), this, SLOT(addRecentFile(const QUrl&)));
	m_actRecentFiles->loadEntries(m_config->group("Recent Files"));

	createAction(i18n("Save All"), "file_save_all", "document-save-all", docManager(), SLOT(fileSaveAll()));
	createAction(i18n("Save Copy As..."), "file_save_copy_as", docManager(), SLOT(fileSaveCopyAs()));
	createAction(i18n("Create Template From Document..."), "template_create", docManager(), SLOT(createTemplate()));
	createAction(i18n("&Remove Template..."), "template_remove", docManager(), SLOT(removeTemplate()));
	createAction(KStandardAction::Close, "file_close", docManager(), SLOT(fileClose()));
	createAction(i18n("Close All"), "file_close_all", docManager(), SLOT(fileCloseAll()));
	createAction(i18n("Close All Ot&hers"), "file_close_all_others", docManager(), SLOT(fileCloseAllOthers()));
	createAction(i18n("S&tatistics"), "Statistics", this, SLOT(showDocInfo()));
	createAction(i18n("&ASCII"), "file_export_ascii", this, SLOT(convertToASCII()));
	createAction(i18n("Latin-&1 (iso 8859-1)"), "file_export_latin1", this, SLOT(convertToEnc()));
	createAction(i18n("Latin-&2 (iso 8859-2)"), "file_export_latin2", this, SLOT(convertToEnc()));
	createAction(i18n("Latin-&3 (iso 8859-3)"), "file_export_latin3", this, SLOT(convertToEnc()));
	createAction(i18n("Latin-&4 (iso 8859-4)"), "file_export_latin4", this, SLOT(convertToEnc()));
	createAction(i18n("Latin-&5 (iso 8859-5)"), "file_export_latin5", this, SLOT(convertToEnc()));
	createAction(i18n("Latin-&9 (iso 8859-9)"), "file_export_latin9", this, SLOT(convertToEnc()));
	createAction(i18n("&Central European (cp-1250)"), "file_export_cp1250", this, SLOT(convertToEnc()));
	createAction(i18n("&Western European (cp-1252)"), "file_export_cp1252", this, SLOT(convertToEnc()));
	createAction(KStandardAction::Quit, "file_quit", this, SLOT(close()));

	createAction(i18n("Move Tab Left"), "move_view_tab_left", "arrow-left", viewManager(), SLOT(moveTabLeft()));
	createAction(i18n("Move Tab Right"), "move_view_tab_right", "arrow-right", viewManager(), SLOT(moveTabRight()));

	createAction(i18n("Next section"), "edit_next_section", "nextsection", QKeySequence(Qt::ALT + Qt::Key_Down), m_edit, SLOT(gotoNextSectioning()));
	createAction(i18n("Prev section"), "edit_prev_section", "prevsection", QKeySequence(Qt::ALT + Qt::Key_Up), m_edit, SLOT(gotoPrevSectioning()));
	createAction(i18n("Next paragraph"), "edit_next_paragraph", "nextparagraph", QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Down), m_edit, SLOT(gotoNextParagraph()));
	createAction(i18n("Prev paragraph"), "edit_prev_paragraph", "prevparagraph", QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Up), m_edit, SLOT(gotoPrevParagraph()));

	createAction(i18n("Find &in Files..."), "FindInFiles", "filegrep", QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_F), this, SLOT(findInFiles()));

	createAction(i18n("Refresh Str&ucture"), "RefreshStructure", "refreshstructure", QKeySequence(Qt::Key_F12), this, SLOT(refreshStructure()));

	//project actions
	createAction(i18n("&New Project..."), "project_new", "window-new", docManager(), SLOT(projectNew()));
	createAction(i18n("&Open Project..."), "project_open", "project-open", docManager(), SLOT(projectOpen()));

	m_actRecentProjects = new KRecentFilesAction(i18n("Open &Recent Project"), actionCollection());
	actionCollection()->addAction("project_openrecent", m_actRecentProjects);
	connect(m_actRecentProjects, SIGNAL(urlSelected(const QUrl&)), docManager(), SLOT(projectOpen(const QUrl&)));
	connect(docManager(), SIGNAL(removeFromRecentProjects(const QUrl&)), this, SLOT(removeRecentProject(const QUrl&)));
	connect(docManager(), SIGNAL(addToRecentProjects(const QUrl& )), this, SLOT(addRecentProject(const QUrl&)));
	m_actRecentProjects->loadEntries(m_config->group("Projects"));

	createAction(i18n("A&dd Files to Project..."), "project_add", "project_add", docManager(), SLOT(projectAddFiles()));
	createAction(i18n("Refresh Project &Tree"), "project_buildtree", "project_rebuild", docManager(), SLOT(buildProjectTree()));
 	createAction(i18n("&Archive"), "project_archive", "project_archive", this, SLOT(runArchiveTool()));
	createAction(i18n("Project &Options"), "project_options", "configure_project", docManager(), SLOT(projectOptions()));
	createAction(i18n("&Close Project"), "project_close", "project-development-close", docManager(), SLOT(projectClose()));

	// new project actions (dani)
	createAction(i18n("&Show Projects..."), "project_show", docManager(), SLOT(projectShow()));
	createAction(i18n("Re&move Files From Project..."), "project_remove", "project_remove", docManager(), SLOT(projectRemoveFiles()));
	createAction(i18n("Show Project &Files..."), "project_showfiles", "project_show", docManager(), SLOT(projectShowFiles()));
	// tbraun
	createAction(i18n("Open All &Project Files"), "project_openallfiles", docManager(), SLOT(projectOpenAllFiles()));
	createAction(i18n("Find in &Project..."), "project_findfiles", "projectgrep", this, SLOT(findInProjects()));

	//build actions
	act = createAction(i18n("Clean"),"CleanAll", "user-trash", this, SLOT(cleanAll()));
	m_paStop = createAction(i18n("&Stop"),"Stop", "process-stop", QKeySequence(Qt::Key_Escape));
	m_paStop->setEnabled(false);
	m_latexOutputErrorToolBar->addAction(m_paStop);

	errorHandler()->setErrorHandlerToolBar(m_latexOutputErrorToolBar);

	createAction(i18n("Return to Editor"), "return_to_editor", "document-edit", QKeySequence("CTRL+E"), this, SLOT(showEditorWidget()));
	createAction(i18n("Next Document"), "gotoNextDocument", "go-next-view-page", QKeySequence(Qt::ALT + Qt::Key_Right), viewManager(), SLOT(gotoNextView()));
	createAction(i18n("Previous Document"), "gotoPrevDocument", "go-previous-view-page", QKeySequence(Qt::ALT + Qt::Key_Left), viewManager(), SLOT(gotoPrevView()));
	createAction(i18n("Focus Log/Messages View"), "focus_log", QKeySequence("CTRL+Alt+M"), this, SLOT(focusLog()));
	createAction(i18n("Focus Output View"), "focus_output", QKeySequence("CTRL+Alt+O"), this, SLOT(focusOutput()));
	createAction(i18n("Focus Konsole View"), "focus_konsole", QKeySequence("CTRL+Alt+K"), this, SLOT(focusKonsole()));
	createAction(i18n("Focus Editor View"), "focus_editor", QKeySequence("CTRL+Alt+F"), this, SLOT(focusEditor()));

	createAction(i18nc("@action: Starts the completion of the current LaTeX command", "Complete (La)TeX Command"), "edit_complete_word", "complete1", QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_Space), codeCompletionManager(), SLOT(startLaTeXCompletion()));
	createAction(i18nc("@action: Starts the input (and completion) of a LaTeX environment", "Complete LaTeX Environment"), "edit_complete_env", "complete2", QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Space), codeCompletionManager(), SLOT(startLaTeXEnvironment()));
	createAction(i18nc("@action: Starts the completion of the current abbreviation", "Complete Abbreviation"), "edit_complete_abbrev", "complete3", QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Space), codeCompletionManager(), SLOT(startAbbreviationCompletion()));

	createAction(i18n("Next Bullet"), "edit_next_bullet", "nextbullet", QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Right), m_edit, SLOT(nextBullet()));
	createAction(i18n("Prev Bullet"), "edit_prev_bullet", "prevbullet", QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Left), m_edit, SLOT(prevBullet()));

 // advanced editor (dani)
	createAction(i18n("Environment (inside)"), "edit_select_inside_env", "selenv_i", QKeySequence("CTRL+Alt+S, E"), m_edit, SLOT(selectEnvInside()));
	createAction(i18n("Environment (outside)"), "edit_select_outside_env", "selenv_o", QKeySequence("CTRL+Alt+S, F"), m_edit, SLOT(selectEnvOutside()));
	createAction(i18n("TeX Group (inside)"), "edit_select_inside_group", "selgroup_i", QKeySequence("CTRL+Alt+S, T"), m_edit, SLOT(selectTexgroupInside()));
	createAction(i18n("TeX Group (outside)"), "edit_select_outside_group", "selgroup_o", QKeySequence("CTRL+Alt+S, U"),m_edit, SLOT(selectTexgroupOutside()));
	createAction(i18n("Math Group"), "edit_select_mathgroup", "selmath", QKeySequence("CTRL+Alt+S, M"), m_edit, SLOT(selectMathgroup()));
	createAction(i18n("Paragraph"), "edit_select_paragraph", "selpar", QKeySequence("CTRL+Alt+S, P"), m_edit, SLOT(selectParagraph()));
	createAction(i18n("Line"), "edit_select_line", "selline", QKeySequence("CTRL+Alt+S, L"), m_edit, SLOT(selectLine()));
	createAction(i18n("TeX Word"), "edit_select_word", "selword", QKeySequence("CTRL+Alt+S, W"), m_edit, SLOT(selectWord()));

	createAction(i18n("Environment (inside)"), "edit_delete_inside_env", "delenv_i", QKeySequence("CTRL+Alt+T, E"), m_edit, SLOT(deleteEnvInside()));
	createAction(i18n("Environment (outside)"), "edit_delete_outside_env", "delenv_o", QKeySequence("CTRL+Alt+T, F"), m_edit, SLOT(deleteEnvOutside()));
	createAction(i18n("TeX Group (inside)"), "edit_delete_inside_group", "delgroup_i", QKeySequence("CTRL+Alt+T, T"), m_edit, SLOT(deleteTexgroupInside()));
	createAction(i18n("TeX Group (outside)"), "edit_delete_outside_group", "delgroup_o",QKeySequence("CTRL+Alt+T, U"),m_edit, SLOT(deleteTexgroupInside()));
	createAction(i18n("Math Group"), "edit_delete_mathgroup", "delmath", QKeySequence("CTRL+Alt+T, M"), m_edit, SLOT(deleteMathgroup()));
	createAction(i18n("Paragraph"), "edit_delete_paragraph", "delpar", QKeySequence("CTRL+Alt+T, P"), m_edit, SLOT(deleteParagraph()));
	createAction(i18n("To End of Line"), "edit_delete_eol", "deleol", QKeySequence("CTRL+Alt+T, L"), m_edit, SLOT(deleteEndOfLine()));
	createAction(i18n("TeX Word"), "edit_delete_word", "delword", QKeySequence("CTRL+Alt+T, W"), m_edit, SLOT(deleteWord()));

	createAction(i18n("Go to Begin"), "edit_begin_env", "gotobeginenv", QKeySequence("CTRL+Alt+E, B"), m_edit, SLOT(gotoBeginEnv()));
	createAction(i18n("Go to End"), "edit_end_env", "gotoendenv", QKeySequence("CTRL+Alt+E, E"), m_edit, SLOT(gotoEndEnv()));
	createAction(i18n("Match"), "edit_match_env", "matchenv", QKeySequence("CTRL+Alt+E, M"), m_edit, SLOT(matchEnv()));
	createAction(i18n("Close"), "edit_close_env", "closeenv", QKeySequence("CTRL+Alt+E, C"), m_edit, SLOT(closeEnv()));
	createAction(i18n("Close All"), "edit_closeall_env", "closeallenv", QKeySequence("CTRL+Alt+E, A"), m_edit, SLOT(closeAllEnv()));

	createAction(i18n("Go to Begin"), "edit_begin_group", "gotobegingroup", QKeySequence("CTRL+Alt+G, B"), m_edit, SLOT(gotoBeginTexgroup()));
	createAction(i18n("Go to End"), "edit_end_group", "gotoendgroup", QKeySequence("CTRL+Alt+G, E"), m_edit, SLOT(gotoEndTexgroup()));
	createAction(i18n("Match"), "edit_match_group", "matchgroup", QKeySequence("CTRL+Alt+G, M"), m_edit, SLOT(matchTexgroup()));
	createAction(i18n("Close"), "edit_close_group", "closegroup", QKeySequence("CTRL+Alt+G, C"), m_edit, SLOT(closeTexgroup()));

	createAction(i18n("Selection"), "quickpreview_selection", "preview_sel", QKeySequence("CTRL+Alt+P, S"), this, SLOT(quickPreviewSelection()));
	createAction(i18n("Environment"), "quickpreview_environment", "preview_env",QKeySequence("CTRL+Alt+P, E"), this, SLOT(quickPreviewEnvironment()));
	createAction(i18n("Subdocument"), "quickpreview_subdocument", "preview_subdoc",QKeySequence("CTRL+Alt+P, D"), this, SLOT(quickPreviewSubdocument()));
	createAction(i18n("Mathgroup"), "quickpreview_math", "preview_math", QKeySequence("CTRL+Alt+P, M"), this, SLOT(quickPreviewMathgroup()));

	KileStdActions::setupStdTags(this, this, actionCollection(), this);
	KileStdActions::setupMathTags(this, actionCollection());

	m_bibTagActionMenu = new KActionMenu(i18n("&Bibliography"), actionCollection());
	m_bibTagActionMenu->setDelayed(false);
	actionCollection()->addAction("menu_bibliography", m_bibTagActionMenu);

 	createAction(i18n("Clean"), "CleanBib", this, SLOT(cleanBib()));

	m_bibTagSettings = new KSelectAction(i18n("&Settings"),actionCollection());
	actionCollection()->addAction("settings_menu_bibliography", m_bibTagSettings);

	act = createAction(i18n("Settings for BibTeX"), "setting_bibtex", this, SLOT(rebuildBibliographyMenu()));
	act->setCheckable(true);
	m_bibTagSettings->addAction(act);

	act = createAction(i18n("Settings for Biblatex"), "setting_biblatex", this, SLOT(rebuildBibliographyMenu()));
	act->setCheckable(true);
	m_bibTagSettings->addAction(act);
	m_bibTagSettings->setCurrentAction(action((QString("setting_") + KileConfig::bibliographyType()).toLatin1()));

	rebuildBibliographyMenu();

	createAction(i18n("Quick Start"), "wizard_document", "quickwizard", this, SLOT(quickDocument()));
	connect(docManager(), SIGNAL(startWizard()), this, SLOT(quickDocument()));
	createAction(i18n("Tabular"), "wizard_tabular", "wizard_tabular", this, SLOT(quickTabular()));
	createAction(i18n("Array"), "wizard_array", "wizard_array", this, SLOT(quickArray()));
	createAction(i18n("Tabbing"), "wizard_tabbing", "wizard_tabbing", this, SLOT(quickTabbing()));
	createAction(i18n("Floats"), "wizard_float", "wizard_float", this, SLOT(quickFloat()));
	createAction(i18n("Math"), "wizard_mathenv", "wizard_math", this, SLOT(quickMathenv()));
	createAction(i18n("Postscript Tools"), "wizard_postscript", "wizard_pstools", this, SLOT(quickPostscript()));
	createAction(i18n("PDF Tools"), "wizard_pdf", "wizard_pdftools", this, SLOT(quickPdf()));

	ModeAction = new KToggleAction(i18n("Define Current Document as '&Master Document'"), actionCollection());
	actionCollection()->addAction("Mode", ModeAction);
	ModeAction->setIcon(QIcon::fromTheme("master"));
	connect(ModeAction, SIGNAL(triggered()), this, SLOT(toggleMasterDocumentMode()));

	if(viewManager()->viewerPart()) {
		KToggleAction *showDocumentViewer = new KToggleAction(i18n("Show Document Viewer"), actionCollection());
		actionCollection()->addAction("ShowDocumentViewer", showDocumentViewer);
		showDocumentViewer->setChecked(KileConfig::showDocumentViewer());
		connect(showDocumentViewer, SIGNAL(toggled(bool)), viewManager(), SLOT(setDocumentViewerVisible(bool)));
		connect(viewManager(), SIGNAL(documentViewerWindowVisibilityChanged(bool)),
		        showDocumentViewer, SLOT(setChecked(bool)));
	}

	KToggleAction *tact = new KToggleAction(i18n("Show S&ide Bar"), actionCollection());
	actionCollection()->addAction("StructureView", tact);
	tact->setChecked(KileConfig::sideBar());
	connect(tact, SIGNAL(toggled(bool)), m_sideBar, SLOT(setVisible(bool)));
	connect(m_sideBar, SIGNAL(visibilityChanged(bool)), this, SLOT(sideOrBottomBarChanged(bool)));

	m_actionMessageView = new KToggleAction(i18n("Show Mess&ages Bar"), actionCollection());
	actionCollection()->addAction("MessageView", m_actionMessageView);
	m_actionMessageView->setChecked(true);
	connect(m_actionMessageView, SIGNAL(toggled(bool)), m_bottomBar, SLOT(setVisible(bool)));
	connect(m_bottomBar, SIGNAL(visibilityChanged(bool)), this, SLOT(sideOrBottomBarChanged(bool)));
	if(m_singlemode) {
		ModeAction->setChecked(false);
	}
	else {
		ModeAction->setChecked(true);
	}

	WatchFileAction = new KToggleAction(i18n("Watch File Mode"), actionCollection());
	actionCollection()->addAction("WatchFile", WatchFileAction);
	WatchFileAction->setIcon(QIcon::fromTheme("watchfile"));
	connect(WatchFileAction, SIGNAL(triggered()), this, SLOT(toggleWatchFile()));
	if(m_bWatchFile) {
		WatchFileAction->setChecked(true);
	}
	else {
		WatchFileAction->setChecked(false);
	}

	setHelpMenuEnabled(false);

	KHelpMenu *help_menu = new KHelpMenu(this, KAboutData::applicationData());

	createAction(i18n("TeX Guide"), "help_tex_guide", QKeySequence("CTRL+Alt+H, G"), m_help, SLOT(helpTexGuide()));
	createAction(i18n("LaTeX"), "help_latex_index", QKeySequence("CTRL+Alt+H, L"), m_help, SLOT(helpLatexIndex()));
	createAction(i18n("LaTeX Command"), "help_latex_command", QKeySequence("CTRL+Alt+H, C"), m_help, SLOT(helpLatexCommand()));
	createAction(i18n("LaTeX Subject"), "help_latex_subject", QKeySequence("CTRL+Alt+H, S"), m_help, SLOT(helpLatexSubject()));
	createAction(i18n("LaTeX Env"), "help_latex_env", QKeySequence("CTRL+Alt+H, E"), m_help, SLOT(helpLatexEnvironment()));
	createAction(i18n("Context Help"), "help_context", QKeySequence("CTRL+Alt+H, K"), m_help, SLOT(helpKeyword()));
	createAction(i18n("Documentation Browser"), "help_docbrowser", QKeySequence("CTRL+Alt+H, B"), m_help, SLOT(helpDocBrowser()));

	createAction(i18n("LaTeX Reference"), "help_latex_reference", "help-latex", this, SLOT(helpLaTex()));

	actionCollection()->addAction(KStandardAction::HelpContents, help_menu, SLOT(appHelpActivated()));
	actionCollection()->addAction(KStandardAction::ReportBug, help_menu, SLOT(reportBug()));
	act = actionCollection()->addAction(KStandardAction::AboutApp, help_menu, SLOT(aboutApplication()));
	act->setMenuRole(QAction::AboutRole); // for Mac OS X, to get the right about menu in the application menu

	act = actionCollection()->addAction(KStandardAction::AboutKDE, help_menu, SLOT(aboutKDE()));
	act->setMenuRole(QAction::NoRole);
	act = createAction(i18n("&About Editor Component"), "help_about_editor", this, SLOT(aboutEditorComponent()));
	act->setMenuRole(QAction::NoRole);

	QAction *kileconfig = KStandardAction::preferences(this, SLOT(generalOptions()), actionCollection());
	kileconfig->setIcon(QIcon::fromTheme("configure-kile"));

	actionCollection()->addAction(KStandardAction::KeyBindings, this, SLOT(configureKeys()));
	actionCollection()->addAction(KStandardAction::ConfigureToolbars, this, SLOT(configureToolbars()));

	createAction(i18n("&System Check..."), "settings_perform_check", this, SLOT(slotPerformCheck()));

	m_userHelpActionMenu = new KActionMenu(i18n("User Help"), actionCollection());
	actionCollection()->addAction("help_userhelp", m_userHelpActionMenu);

	m_pFullScreen = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()), this, actionCollection());

}

void Kile::rebuildBibliographyMenu(){

	KILE_DEBUG_MAIN << " current is " << m_bibTagSettings->currentText();

	QString currentItem = m_bibTagSettings->currentText();
	QString name;

	if( currentItem == i18n("BibTeX") ){ // avoid writing i18n'ed strings to config file
		name = QString("bibtex");
	}
	else if ( currentItem == i18n("Biblatex") ){
		name = QString("biblatex");
	}
	else {
		KILE_DEBUG_MAIN << "wrong currentItem in bibliography settings menu";
		name = QString("bibtex");
	}

	KileConfig::setBibliographyType(name);
	m_bibTagActionMenu->menu()->clear();

	KileStdActions::setupBibTags(this, actionCollection(),m_bibTagActionMenu);
	m_bibTagActionMenu->addSeparator();
 	m_bibTagActionMenu->addAction(action("CleanBib"));
	m_bibTagActionMenu->addSeparator();
	m_bibTagActionMenu->addAction(action("settings_menu_bibliography"));
}

QAction* Kile::createToolAction(QString toolName)
{
	return createAction(toolName, "tool_" + toolName,
	                    KileTool::iconFor(toolName, m_config.data()), m_signalMapper, SLOT(map()));
}

void Kile::createToolActions()
{
	QStringList tools = KileTool::toolList(m_config.data());
	for (QStringList::iterator i = tools.begin(); i != tools.end(); ++i) {
		QString toolName = *i;
		if(!actionCollection()->action("tool_" + toolName)) {
			KILE_DEBUG_MAIN << "Creating action for tool" << toolName;
			QAction *act = createToolAction(toolName);
			m_signalMapper->removeMappings(act);
			m_signalMapper->setMapping(act, toolName);
		}
	}
}

void Kile::setupTools()
{
	KILE_DEBUG_MAIN << "==Kile::setupTools()===================" << endl;

	if(!m_buildMenuCompile || !m_buildMenuConvert ||  !m_buildMenuTopLevel || !m_buildMenuQuickPreview || !m_buildMenuViewer || !m_buildMenuOther){
		KILE_DEBUG_MAIN << "BUG, menu pointers are Q_NULLPTR"
		             << (m_buildMenuCompile == Q_NULLPTR)
		             << (m_buildMenuConvert == Q_NULLPTR)
		             << (m_buildMenuTopLevel == Q_NULLPTR)
		             << (m_buildMenuQuickPreview == Q_NULLPTR)
		             << (m_buildMenuViewer == Q_NULLPTR)
		             << (m_buildMenuOther == Q_NULLPTR);
		return;
	}

	QStringList tools = KileTool::toolList(m_config.data());
	QString toolMenu, grp;
	QList<QAction*> *pl;
	QAction *act;
	ToolbarSelectAction *pSelectAction = Q_NULLPTR;

	m_compilerActions->saveCurrentAction();
	m_viewActions->saveCurrentAction();
	m_convertActions->saveCurrentAction();
	m_quickActions->saveCurrentAction();

	// do plugActionList by hand ...
	foreach(act, m_listQuickActions){
		m_buildMenuTopLevel->removeAction(act);
	}

	m_buildMenuCompile->clear();
	m_buildMenuConvert->clear();
	m_buildMenuViewer->clear();
	m_buildMenuOther->clear();

	m_compilerActions->removeAllActions();
	m_viewActions->removeAllActions();
	m_convertActions->removeAllActions();
	m_quickActions->removeAllActions();

	for (int i = 0; i < tools.count(); ++i) {
		grp = KileTool::groupFor(tools[i], m_config.data());
		toolMenu = KileTool::menuFor(tools[i], m_config.data());

		KILE_DEBUG_MAIN << tools[i] << " is using group: " << grp << " and menu: "<< toolMenu;
		if(toolMenu == "none") {
			continue;
		}

		if ( toolMenu == "Compile" ){
			pl = &m_listCompilerActions;
			pSelectAction = m_compilerActions;
		}
		else if ( toolMenu == "View" ){
			pl = &m_listViewerActions;
			pSelectAction = m_viewActions;
		}
		else if ( toolMenu == "Convert" ){
			pl = &m_listConverterActions;
			pSelectAction = m_convertActions;
		}
		else if ( toolMenu == "Quick" ){
			pl = &m_listQuickActions;
			pSelectAction = m_quickActions;
		}
		else{
			pl = &m_listOtherActions;
			pSelectAction = Q_NULLPTR;
		}

		KILE_DEBUG_MAIN << "\tadding " << tools[i] << " " << toolMenu << " #" << pl->count() << endl;

		act = actionCollection()->action("tool_" + tools[i]);
		if(!act) {
			KILE_DEBUG_MAIN << "no tool for " << tools[i];
			act = createToolAction(tools[i]);
			m_signalMapper->removeMappings(act);
			m_signalMapper->setMapping(act, tools[i]);
		}
		pl->append(act);

		if(pSelectAction){
			pSelectAction->addAction(actionCollection()->action("tool_" + tools[i]));
		}
	}

	m_quickActions->addSeparator();
	m_quickActions->addAction(action("quickpreview_selection"));
	m_quickActions->addAction(action("quickpreview_environment"));
	m_quickActions->addAction(action("quickpreview_subdocument"));
	m_quickActions->addSeparator();
	m_quickActions->addAction(action("quickpreview_math"));

	cleanUpActionList(m_listCompilerActions, tools);
	cleanUpActionList(m_listViewerActions, tools);
	cleanUpActionList(m_listConverterActions, tools);
	cleanUpActionList(m_listQuickActions, tools);
	cleanUpActionList(m_listOtherActions, tools);

	m_buildMenuTopLevel->insertActions(m_buildMenuQuickPreview->menuAction(),m_listQuickActions);
	m_buildMenuCompile->addActions(m_listCompilerActions);
	m_buildMenuConvert->addActions(m_listConverterActions);
	m_buildMenuViewer->addActions(m_listViewerActions);
	m_buildMenuOther->addActions(m_listOtherActions);

	m_compilerActions->restoreCurrentAction();
	m_viewActions->restoreCurrentAction();
	m_convertActions->restoreCurrentAction();
	m_quickActions->restoreCurrentAction();
}

void Kile::initSelectActions(){

	m_compilerActions = new ToolbarSelectAction(i18n("Compile"), this);
	m_viewActions = new ToolbarSelectAction(i18n("View"), this);
	m_convertActions = new ToolbarSelectAction(i18n("Convert"), this);
	m_quickActions = new ToolbarSelectAction(i18n("Quick"), this);

	actionCollection()->setShortcutsConfigurable(m_compilerActions, false);
	actionCollection()->setShortcutsConfigurable(m_viewActions, false);
	actionCollection()->setShortcutsConfigurable(m_convertActions, false);
	actionCollection()->setShortcutsConfigurable(m_quickActions, false);

	actionCollection()->addAction("list_compiler_select", m_compilerActions);
	actionCollection()->addAction("list_convert_select", m_convertActions);
	actionCollection()->addAction("list_view_select", m_viewActions);
	actionCollection()->addAction("list_quick_select", m_quickActions);
}

void Kile::saveLastSelectedAction(){

	KILE_DEBUG_MAIN << "Kile::saveLastSelectedAction()" << endl;
	QStringList list;
	list << "Compile" << "Convert" << "View" << "Quick";

	ToolbarSelectAction *pSelectAction = Q_NULLPTR ;

	KConfigGroup grp = m_config->group("ToolSelectAction");

	for(QStringList::Iterator it = list.begin(); it != list.end() ; ++it) {
		if ( *it == "Compile" ){
			pSelectAction = m_compilerActions;
		}
		else if ( *it == "View" ){
			pSelectAction = m_viewActions;
		}
		else if ( *it == "Convert" ){
			pSelectAction = m_convertActions;
		}
		else if ( *it == "Quick" ){
			pSelectAction = m_quickActions;
		}

		KILE_DEBUG_MAIN << "current item is " << pSelectAction->currentItem();

		grp.writeEntry(*it, pSelectAction->currentItem());
	}
}

void Kile::restoreLastSelectedAction(){

	QStringList list;
	list << "Compile" << "Convert" << "View" << "Quick";

	ToolbarSelectAction *pSelectAction = Q_NULLPTR;
	int defaultAction = 0;

	KConfigGroup grp = m_config->group("ToolSelectAction");

	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		if ( *it == "Compile" ) {
			pSelectAction = m_compilerActions;
			defaultAction = 9; // PDFLatex
		}
		else if ( *it == "View" ) {
			pSelectAction = m_viewActions;
			defaultAction = 4; // ViewPDF
		}
		else if ( *it == "Convert" ) {
			pSelectAction = m_convertActions;
			defaultAction = 0;
		}
		else if ( *it == "Quick" ) {
			pSelectAction = m_quickActions;
			defaultAction = 0;
		}

		int actIndex = grp.readEntry(*it, defaultAction);
		KILE_DEBUG_MAIN << "selecting" << actIndex << "for" << *it;
		pSelectAction->setCurrentItem(actIndex);
	}
}

void Kile::cleanUpActionList(QList<QAction*> &list, const QStringList &tools)
{
//  	KILE_DEBUG_MAIN << "cleanUpActionList tools are" << tools.join("; ");
	QList<QAction*>::iterator it, testIt;
	for ( it= list.begin(); it != list.end(); ++it){
		QAction *act = *it;
		if ( act != Q_NULLPTR && !act->objectName().isEmpty() && !tools.contains(act->objectName().mid(5)) ) {
			if (act->associatedWidgets().contains(toolBar("toolsToolBar"))) {
				toolBar("toolsToolBar")->removeAction(act);
			}
//             KILE_DEBUG_MAIN << "about to delete action: " << act->objectName();
			testIt = list.erase(it);
			if( testIt == list.end()){
				break;
			}
		}
	}
}

void Kile::restoreFilesAndProjects(bool allowRestore)
{
	if (!(allowRestore && KileConfig::restore())) {
	  return;
	}

	QUrl url;
	for (int i=0; i < m_listProjectsOpenOnStart.count(); ++i) {
		// don't open project files as they will be opened later in this method
		docManager()->projectOpen(QUrl::fromUserInput(m_listProjectsOpenOnStart[i]), i, m_listProjectsOpenOnStart.count(), false);
	}

	for (int i = 0; i < m_listDocsOpenOnStart.count(); ++i) {
		docManager()->fileOpen(QUrl::fromUserInput(m_listDocsOpenOnStart[i]), m_listEncodingsOfDocsOpenOnStart[i]);
	}

	if (ModeAction) {
		ModeAction->setChecked(!m_singlemode);
	}
	updateModeStatus();

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();
	m_listEncodingsOfDocsOpenOnStart.clear();

        KILE_DEBUG_MAIN << "lastDocument=" << KileConfig::lastDocument() << endl;
	KTextEditor::Document *doc = docManager()->docFor(QUrl::fromUserInput(KileConfig::lastDocument()));
	if (doc) {
		viewManager()->switchToTextView(doc->url(), true); // request the focus on the view
	}
	setMasterDocumentFileName(KileConfig::singleFileMasterDocument());
}

void Kile::setActive()
{
	KILE_DEBUG_MAIN << "Activating" << endl;
	raise();
	activateWindow();
	show();
}

void Kile::setLine(const QString &line)
{
	bool ok;
	uint l = line.toUInt(&ok, 10);
	KTextEditor::View *view = viewManager()->currentTextView();
	if (view && ok) {
		show();
		raise();
		activateWindow();
		// be very aggressive when it comes to raising the main window to the top
		KWindowSystem::forceActiveWindow(winId());
		focusTextView(view);
		editorExtension()->goToLine(l - 1, view);

		showEditorWidget();
	}
}

void Kile::setCursor(const QUrl &url, int parag, int index)
{
	KTextEditor::Document *doc = docManager()->docFor(url);
	if(doc) {
		KTextEditor::View *view = (KTextEditor::View*)doc->views().first();
		if(view) {
			view->setCursorPosition(KTextEditor::Cursor(parag, index));
			focusTextView(view);
		}
	}
}

void Kile::runArchiveTool()
{
	runArchiveTool(QUrl());
}

void Kile::runArchiveTool(const QUrl &url)
{
	KileTool::Archive *tool = dynamic_cast<KileTool::Archive*>(m_manager->createTool("Archive", QString(), false));
	if(!tool) {
		KMessageBox::error(mainWindow(), i18n("It was impossible to create the \"Archive\" tool.\n\n"
		                                      "Please check and repair your installation of Kile."),
		                                 i18n("Unable to Create Archive Tool"));
		return;
	}
	if(url.isValid()) {
		tool->setSource(url.toLocalFile());
	}
	tool->prepareToRun();
	m_manager->run(tool);
}

//TODO: move to KileView::Manager
void Kile::activateView(QWidget* w, bool updateStruct /* = true */ )  //Needs to be QWidget because of QTabWidget::currentChanged
{
	//KILE_DEBUG_MAIN << "==Kile::activateView==========================" << endl;
	if (!w || !w->inherits("KTextEditor::View")) {
		return;
	}

	//disable gui updates to avoid flickering of toolbars
	setUpdatesEnabled(false);

	QList<KToolBar*> toolBarsList = toolBars();
	QHash<KToolBar*, bool> toolBarVisibilityHash;

	for(QList<KToolBar*>::iterator i = toolBarsList.begin();
	                              i != toolBarsList.end(); ++i) {
		KToolBar *toolBar = *i;
		toolBarVisibilityHash[toolBar] = toolBar->isVisible();
	}

	KTextEditor::View* view = dynamic_cast<KTextEditor::View*>(w);
	Q_ASSERT(view);

	for(int i = 0; i < viewManager()->textViewCount(); ++i) {
		KTextEditor::View *view2 = viewManager()->textView(i);
		if(view == view2) {
			continue;
		}
		guiFactory()->removeClient(view2);
		view2->clearFocus();
	}

	guiFactory()->addClient(view);

	for(QList<KToolBar*>::iterator i = toolBarsList.begin();
	                              i != toolBarsList.end(); ++i) {
		KToolBar *toolBar = *i;
		toolBar->setVisible(toolBarVisibilityHash[*i]);
	}

	setUpdatesEnabled(true);

	if(updateStruct) {
		viewManager()->updateStructure();
	}

	focusTextView(view);
}

void Kile::updateModeStatus()
{
	KILE_DEBUG_MAIN << "==Kile::updateModeStatus()==========";
	KileProject *project = docManager()->activeProject();
	QString shortName = m_masterDocumentFileName;
	int pos = shortName.lastIndexOf('/');
	shortName.remove(0, pos + 1);

	if(project) {
		if (m_singlemode) {
			statusBar()->setHintText(i18n("Project: %1", project->name()));
		}
		else {
			statusBar()->setHintText(i18n("Project: %1 (Master document: %2)", project->name(), shortName));
		}
	}
	else {
		if (m_singlemode) {
			statusBar()->setHintText(i18n("Normal mode"));
		}
		else {
			statusBar()->setHintText(i18n("Master document: %1", shortName));
		}
	}

	if(m_singlemode) {
		ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
		ModeAction->setChecked(false);
	}
	else {
		ModeAction->setText(i18n("Normal mode (current master document: %1)", shortName));
		ModeAction->setChecked(true);
	}

	// enable or disable entries in Kile'S menu
	updateMenu();

	KTextEditor::View *view = viewManager()->currentTextView();
	// Passing Q_NULLPTR is ok
	updateStatusBarCursorPosition(view, (view ? view->cursorPosition() : KTextEditor::Cursor()));
	updateStatusBarViewMode(view);
	updateStatusBarSelection(view);
}

void Kile::openDocument(const QUrl &url)
{
	docManager()->fileSelected(url);
}

void Kile::openDocument(const QString& s)
{
	openDocument(QUrl::fromUserInput(s));
}

void Kile::closeDocument()
{
	docManager()->fileClose();
}

void Kile::openProject(const QUrl &url)
{
	docManager()->projectOpen(url);
}

void Kile::openProject(const QString& proj)
{
	openProject(QUrl::fromUserInput(proj));
}

void Kile::focusPreview()
{
	m_bottomBar->switchToTab(PREVIEW_TAB);
}

void Kile::focusLog()
{
	m_bottomBar->switchToTab(LOG_TAB);
}

void Kile::focusOutput()
{
	m_bottomBar->switchToTab(OUTPUT_TAB);
}

void Kile::focusKonsole()
{
	m_bottomBar->switchToTab(KONSOLE_TAB);
}

void Kile::focusEditor()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		focusTextView(view);
	}
}

void Kile::sideOrBottomBarChanged(bool visible)
{
    if ( ! visible )
    {
        focusEditor();
    }
}

//FIXME: documents probably shouldn't be closed in this method yet (also see API doc of 'queryClose')
bool Kile::queryClose()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		KileConfig::setLastDocument(view->document()->url().toLocalFile());
	}
	else {
		KileConfig::setLastDocument("");
	}

	//don't close Kile if embedded viewers are present
	KILE_DEBUG_MAIN << "==bool Kile::queryClose(" << m_currentState << ")==========" << endl;
	if(m_currentState != "Editor") {
		resetPart();
		return false;
	}

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();
	m_listEncodingsOfDocsOpenOnStart.clear();

	for(int i = 0; i < viewManager()->textViewCount(); ++i) {
		KTextEditor::Document *doc = viewManager()->textView(i)->document();
		const QUrl url = doc->url();
		if(url.isEmpty()) {
			continue;
		}
		m_listDocsOpenOnStart.append(url.toLocalFile());
		m_listEncodingsOfDocsOpenOnStart.append(doc->encoding());
	}

	KILE_DEBUG_MAIN << "#projects = " << docManager()->projects().count() << endl;
	QList<KileProject*> projectList = docManager()->projects();
	for(QList<KileProject*>::iterator i = projectList.begin(); i != projectList.end(); ++i) {
		const QUrl url = (*i)->url();
		if(url.isEmpty()) { // shouldn't happen, but just in case...
			continue;
		}
		m_listProjectsOpenOnStart.append(url.toLocalFile());
	}

	bool stage1 = docManager()->projectCloseAll();
	bool stage2 = true;

	if(stage1) {
		stage2 = docManager()->fileCloseAll();
	}

	bool close = stage1 && stage2;
	if(close) {
		saveSettings();
	}

	return close;
}

void Kile::showDocInfo(KTextEditor::View *view)
{
	if(!view) {
		view = viewManager()->currentTextView();
	}

	if(!view) {
		return;
	}

	KileDocument::TextInfo *docinfo = docManager()->textInfoFor(view->document());
	KileProject *project = KileInfo::docManager()->activeProject();
	if(docinfo) { // we have to ensure that we always get a _valid_ docinfo object
		KileDialog::StatisticsDialog *dlg = new KileDialog::StatisticsDialog(project,
		                                                                     docinfo,
		                                                                     this,
		                                                                     view);
		dlg->exec();
		delete dlg;
	}
	else {
		qWarning() << "There is no KileDocument::Info object belonging to this document!";
	}
}

void Kile::convertToASCII(KTextEditor::Document *doc)
{
	if(!doc) {
		KTextEditor::View *view = viewManager()->currentTextView();

		if(view) {
			doc = view->document();
		}
		else {
			return;
		}
	}

	ConvertIO io(doc);
	ConvertEncToASCII conv = ConvertEncToASCII(doc->encoding(), &io);
	doc->setEncoding("ISO 8859-1");
	conv.convert();
}

void Kile::convertToEnc(KTextEditor::Document *doc)
{
	if(!doc) {
		KTextEditor::View *view = viewManager()->currentTextView();

		if (view) doc = view->document();
		else return;
	}

	if(sender()) {
		ConvertIO io(doc);
		QString name = QString(sender()->objectName()).section('_', -1);
		ConvertASCIIToEnc conv = ConvertASCIIToEnc(name, &io);
		conv.convert();
		doc->setEncoding(ConvertMap::encodingNameFor(name));
	}
}

KileWidget::StatusBar * Kile::statusBar()
{
	return static_cast<KileWidget::StatusBar *>(KXmlGuiWindow::statusBar());
}

////////////////// GENERAL SLOTS //////////////
int Kile::lineNumber()
{
	KTextEditor::View *view = viewManager()->currentTextView();

	int para = 0;

	if (view) {
		para = view->cursorPosition().line();
	}

	return para;
}

void Kile::newCaption()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		const bool showFullPath = KileConfig::showFullPathInWindowTitle();

		KTextEditor::Document *doc = view->document();
		const QString caption = (doc->isReadWrite() ? getName(doc, !showFullPath)
		                                            : i18nc("Window caption in read-only mode: <file name> [Read-Only]",
		                                                    "%1 [Read-Only]", getName(doc, !showFullPath)));
		setWindowTitle(caption);
		if (m_bottomBar->currentPage() && m_bottomBar->currentPage()->inherits("KileWidget::Konsole")) {
			m_texKonsole->sync();
		}
	}
	else {
		setWindowTitle("");
	}
}

void Kile::grepItemSelected(const QString &abs_filename, int line)
{
	KILE_DEBUG_MAIN << "Open file: "
		<< abs_filename << " (" << line << ")" << endl;
	docManager()->fileOpen(QUrl::fromUserInput(abs_filename));
	setLine(QString::number(line));
}

void Kile::findInFiles()
{
	static QPointer<KileDialog::FindFilesDialog> dlg = 0;

	if (!dlg) {
		KILE_DEBUG_MAIN << "grep guard: create findInFiles dlg" << endl;
		dlg = new KileDialog::FindFilesDialog(mainWindow(), this, KileGrep::Directory);
		dlg->show();
		connect(dlg, SIGNAL(itemSelected(const QString &, int)),
		        this, SLOT(grepItemSelected(const QString &, int)));
	}
	else {
		KILE_DEBUG_MAIN << "grep guard: show findInFiles dlg" << endl;
		dlg->activateWindow();
		dlg->raise();
	}
}

void Kile::findInProjects()
{
	static QPointer<KileDialog::FindFilesDialog> project_dlg = Q_NULLPTR;

	if(!project_dlg) {
		KILE_DEBUG_MAIN << "grep guard: create findInProjects dlg" << endl;
		project_dlg = new KileDialog::FindFilesDialog(mainWindow(), this, KileGrep::Project);
		project_dlg->show();
		connect(project_dlg, SIGNAL(itemSelected(const QString &, int)),
		        this, SLOT(grepItemSelected(const QString &, int)));
	}
	else {
		KILE_DEBUG_MAIN << "grep guard: show findInProjects dlg" << endl;
		project_dlg->activateWindow();
		project_dlg->raise();
	}
}

/////////////////// PART & EDITOR WIDGET //////////
void Kile::showEditorWidget()
{
	if(!resetPart()){
		return;
	}
	setCentralWidget(m_topWidgetStack);
	m_topWidgetStack->show();
	m_horizontalSplitter->show();
	m_verticalSplitter->show();
}


bool Kile::resetPart()
{
	KILE_DEBUG_MAIN << "==Kile::resetPart()=============================" << endl;
	KILE_DEBUG_MAIN << "\tcurrent state " << m_currentState << endl;
	KILE_DEBUG_MAIN << "\twant state " << m_wantState << endl;

	KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>(m_partManager->activePart());

	if (part && m_currentState != "Editor") {
		if(part->closeUrl()) {
			m_partManager->removePart(part);
			m_topWidgetStack->removeWidget(part->widget());
			delete part;
		}
		else {
			return false;
		}
	}

	statusBar()->reset();
	updateModeStatus();
	newCaption();

	KTextEditor::View *view = viewManager()->currentTextView();
	if (view){
		activateView(view);
	}

	m_currentState = "Editor";
	m_wantState = "Editor";
	return true;
}

void Kile::activePartGUI(KParts::Part *part)
{
	KILE_DEBUG_MAIN << "==Kile::activePartGUI()=============================" << endl;
	KILE_DEBUG_MAIN << "\tcurrent state " << m_currentState << endl;
	KILE_DEBUG_MAIN << "\twant state " << m_wantState << endl;

	//manually plug the print action into the toolbar for
	//kghostview (which has the print action defined in
	//a KParts::BrowserExtension)
	KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject(part);
	if(ext && ext->metaObject()->indexOfSlot("print()") > -1) { //part is a BrowserExtension, connect printAction()
		connect(m_paPrint, SIGNAL(triggered()), ext, SLOT(print()));
		toolBar("mainToolBar")->addAction(m_paPrint); //plug this action into its default location
		m_paPrint->setEnabled(true);
	}
	else {
		if (m_paPrint->associatedWidgets().contains(toolBar("mainToolBar"))) {
			toolBar("mainToolBar")->removeAction(m_paPrint);
		}
		m_paPrint->setEnabled(false);
	}

	createGUI(part);
	updateUserDefinedMenus();

	// finally update the GUI regarding the current state
	updateGUI(m_wantState);
	//set the current state
	m_currentState = m_wantState;
	m_wantState = "Editor";
}

void Kile::updateUserDefinedMenus()
{
	m_buildMenuTopLevel = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_build", m_mainWindow));
	m_buildMenuCompile  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_compile", m_mainWindow));
	m_buildMenuConvert  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_convert", m_mainWindow));
	m_buildMenuViewer  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_viewer", m_mainWindow));
	m_buildMenuOther   = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_other", m_mainWindow));
	m_buildMenuQuickPreview   = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("quickpreview", m_mainWindow));

	if ( m_userMenu ) {
		m_userMenu->updateGui();
	}
	else {
		QMenu *usermenu = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_usermenu", m_mainWindow));
		if ( usermenu ) {
			usermenu->menuAction()->setVisible(false);
		}

	}

	setupTools();
}

void Kile::updateGUI(const QString &wantState)
{
	// save state of all toolbars
	static bool mainToolBar = true;
	static bool toolsToolBar = true;
	static bool editToolBar = true;
	static bool mathToolBar = true;

	if(m_currentState == "Editor") {
		mainToolBar  = !toolBar("mainToolBar")->isHidden();
		toolsToolBar = !toolBar("toolsToolBar")->isHidden();
		editToolBar  = !toolBar("editToolBar")->isHidden();
		mathToolBar  = !toolBar("mathToolBar")->isHidden();
	}

	if(wantState == "HTMLpreview") {
		slotStateChanged("HTMLpreview");
		setViewerToolBars();
		enableKileGUI(false);
		actionCollection()->action("return_to_editor")->setVisible(true);
	}
	else if(wantState == "Viewer") {
		slotStateChanged("Viewer");
		setViewerToolBars();
		enableKileGUI(false);
		actionCollection()->action("return_to_editor")->setVisible(true);
	}
	else {
		slotStateChanged( "Editor" );
		m_wantState="Editor";
		m_topWidgetStack->setCurrentIndex(0);
		if ( !mainToolBar  ) toolBar("mainToolBar")->hide();
		if ( toolsToolBar ) toolBar("toolsToolBar")->show();
		if ( editToolBar  ) toolBar("editToolBar")->show();
		if ( mathToolBar  ) toolBar("mathToolBar")->show();
		actionCollection()->action("return_to_editor")->setVisible(false);
		enableKileGUI(true);
	}
}

void Kile::setViewerToolBars()
{
	toolBar("mainToolBar")->show();
	toolBar("toolsToolBar")->hide();
	toolBar("editToolBar")->hide();
	toolBar("mathToolBar")->hide();
}

void Kile::enableKileGUI(bool enable)
{
	// update action lists
	QList<QAction *> actions = actionCollection()->actions();
	for(QList<QAction *>::iterator itact = actions.begin(); itact != actions.end(); ++itact) {
		if (m_dictMenuAction.contains((*itact)->objectName())
		    || m_dictMenuFile.contains((*itact)->objectName())) {
			(*itact)->setEnabled(enable);
		}
	}

	// update latex usermenu actions
	if ( m_userMenu ) {
		QList<QAction *> useractions = m_userMenu->menuActions();
		foreach ( QAction *action, useractions ) {
			action->setEnabled(enable);
		}
	}

	// enable or disable userhelp entries
	m_help->enableUserhelpEntries(enable);

	QList<QAction*> actionList;
	actionList << m_listQuickActions
	           << m_listCompilerActions
	           << m_listConverterActions
	           << m_listViewerActions
	           << m_listOtherActions;
	// enable or disable list actions
	for(QList<QAction*>::iterator i = actionList.begin(); i != actionList.end(); ++i) {
		(*i)->setEnabled(enable);
	}

	// enable or disable bibliography menu entries
	actionList = m_bibTagActionMenu->menu()->actions();
	for(QList<QAction*>::iterator it = actionList.begin(); it != actionList.end(); ++it) {
		(*it)->setEnabled(enable);
	}

	QStringList menuList;
	menuList << "file" << "edit" << "view" << "menu_build" << "menu_project" << "menu_latex" << "wizard" << "tools";
	for(QStringList::iterator it = menuList.begin(); it != menuList.end(); ++it) {
		QMenu *menu = dynamic_cast<QMenu*>(guiFactory()->container(*it, this));
		if(menu) {
			updateMenuActivationStatus(menu);
		}
	}

	updateUserMenuStatus(enable);
}

// adds action names to their lists

void Kile::initMenu()
{
	QStringList projectlist,filelist,actionlist;

	projectlist
	   << "project_add" << "project_remove"
	   << "project_showfiles"
	   << "project_buildtree" << "project_options" << "project_findfiles"
	   << "project_archive" << "project_close" << "project_openallfiles"
	   ;

	filelist
	   // file
	   << "convert"
	   // edit
	   << "goto_menu" << "complete" << "bullet" << "select"
	   << "delete" << "environment" << "texgroup"
	   // build
	   << "quickpreview" << "menu_compile" << "menu_convert"
	   << "menu_viewers" << "menu_other"
	   // latex
	   << "menu_preamble" << "menu_lists" << "menu_sectioning" << "references"
	   << "menu_environment" << "menu_listenv" << "menu_tabularenv" << "menu_floatenv"
	   << "menu_code" << "menu_math" << "menu_mathenv" << "menu_mathamsenv"
	   << "menu_bibliography" << "menu_fontstyles" << "menu_spacing"
	   ;

	actionlist
	   // file
	   << "file_save_copy_as" << "file_save_all" << "template_create" << "Statistics"
	   << "file_close" << "file_close_all" << "file_close_all_others"
	   // edit
	   << "RefreshStructure"
	    // view
	   << "gotoPrevDocument" << "gotoNextDocument"
	   // build
	   << "quickpreview_selection" << "quickpreview_environment"
	   << "quickpreview_subdocument" << "quickpreview_math"
	   << "WatchFile" << "CleanAll"
	   // latex
	   << "tag_documentclass" << "tag_usepackage" << "tag_amspackages" << "tag_env_document"
	   << "tag_author" << "tag_title" << "tag_maketitle" << "tag_titlepage" << "tag_env_abstract"
	   << "tag_tableofcontents" << "tag_listoffigures" << "tag_listoftables"
	   << "tag_makeindex" << "tag_printindex" << "tag_makeglossary" << "tag_env_thebibliography"
	   << "tag_part" << "tag_chapter" << "tag_section" << "tag_subsection" << "tag_subsubsection"
	   << "tag_paragraph" << "tag_subparagraph" << "tag_label"
	   << "tag_ref" << "tag_pageref" << "tag_index" << "tag_footnote" << "tag_cite" // << "citeViewBib"
	   << "tag_center" << "tag_flushleft" << "tag_flushright"
	   << "tag_env_minipage" << "tag_quote" << "tag_quotation" << "tag_verse"
	   << "tag_env_itemize" << "tag_env_enumerate" << "tag_env_description" << "tag_item"
	   << "tag_env_tabular" << "tag_env_tabular*" << "tag_env_tabbing"
	   << "tag_multicolumn" << "tag_hline" << "tag_vline" << "tag_cline"
	   << "tag_figure" << "tag_table"
	   << "tag_verbatim" << "tag_env_verbatim*" << "tag_verb" << "tag_verb*"
	   << "tag_mathmode" << "tag_equation" << "tag_subscript" << "tag_superscript"
	   << "tag_sqrt" << "tag_nroot" << "tag_left" << "tag_right" << "tag_leftright"
	   << "tag_bigl" << "tag_bigr" << "tag_Bigl" << "tag_Bigr"
	   << "tag_biggl" << "tag_biggr" << "tag_Biggl" << "tag_Biggr"
	   << "tag_text" << "tag_intertext" << "tag_boxed"
	   << "tag_frac" << "tag_dfrac" << "tag_tfrac"
	   << "tag_binom" << "tag_dbinom" << "tag_tbinom"
	   << "tag_xleftarrow" << "tag_xrightarrow"
	   << "tag_mathrm" << "tag_mathit" << "tag_mathbf" << "tag_mathsf"
	   << "tag_mathtt" << "tag_mathcal" << "tag_mathbb" << "tag_mathfrak"
	   << "tag_acute" << "tag_grave" << "tag_tilde" << "tag_bar" << "tag_vec"
	   << "tag_hat" << "tag_check" << "tag_breve" << "tag_dot" << "tag_ddot"
	   << "tag_space_small" << "tag_space_medium" << "tag_space_large"
	   << "tag_quad" << "tag_qquad" << "tag_enskip"
	   << "tag_env_displaymath" << "tag_env_equation" << "tag_env_equation*"
	   << "tag_env_array"
	   << "tag_env_multline" << "tag_env_multline*" << "tag_env_split"
	   << "tag_env_gather" << "tag_env_gather*" << "tag_env_align" << "tag_env_align*"
	   << "tag_env_flalign" << "tag_env_flalign*" << "tag_env_alignat" << "tag_env_alignat*"
	   << "tag_env_aligned" << "tag_env_gathered" << "tag_env_alignedat" << "tag_env_cases"
		<< "tag_env_matrix" << "tag_env_pmatrix" << "tag_env_vmatrix"
		<< "tag_env_VVmatrix" << "tag_env_bmatrix" << "tag_env_BBmatrix"
	   // bibliography stuff
	   << "menu_bibliography"
	   << "setting_bibtex" << "setting_biblatex"
	   << "tag_textit" << "tag_textsl" << "tag_textbf" << "tag_underline"
	   << "tag_texttt" << "tag_textsc" << "tag_emph" << "tag_strong"
	   << "tag_rmfamily" << "tag_sffamily" << "tag_ttfamily"
	   << "tag_mdseries" << "tag_bfseries" << "tag_upshape"
	   << "tag_itshape" << "tag_slshape" << "tag_scshape"
	   << "tag_newline" << "tag_newpage" << "tag_linebreak" << "tag_pagebreak"
	   << "tag_bigskip" << "tag_medskip" << "tag_smallskip"
	   << "tag_hspace" << "tag_hspace*" << "tag_vspace" << "tag_vspace*"
	   << "tag_hfill" << "tag_hrulefill" << "tag_dotfill" << "tag_vfill"
	   << "tag_includegraphics" << "tag_include" << "tag_input"
	   // wizard
	   << "wizard_tabular" << "wizard_array" << "wizard_tabbing"
	   << "wizard_float" << "wizard_mathenv"
	   << "wizard_usermenu" << "wizard_usermenu2"
	   // settings
	   << "Mode"
	   // help
	   << "help_context"
	   // action lists
	   << "structure_list" << "size_list" << "other_list"
	   << "left_list" << "right_list"
	   // tool lists
	   << "list_compiler_select" << "list_convert_select" << "list_view_select" << "list_quick_select"
	   // user help
	   << "help_userhelp"
	   << "edit_next_bullet" << "edit_prev_bullet"
	   << "edit_next_section" << "edit_prev_section" << "edit_next_paragraph" << "edit_prev_paragraph"

	   << "edit_select_inside_env" << "edit_select_outside_env" << "edit_select_inside_group"
	   << "edit_select_outside_group" << "edit_select_mathgroup" << "edit_select_paragraph"
	   << "edit_select_line" << "edit_select_word"

	   << "edit_delete_inside_env" << "edit_delete_outside_env" << "edit_delete_inside_group"
	   << "edit_delete_outside_group" << "edit_delete_mathgroup" << "edit_delete_paragraph"
	   << "edit_delete_eol" << "edit_delete_word"

	   << "edit_complete_word" << "edit_complete_env" << "edit_complete_abbrev"

	   << "edit_begin_env" << "edit_end_env" << "edit_match_env" << "edit_close_env" << "edit_closeall_env"

	   << "edit_begin_group" << "edit_end_group" << "edit_match_group" << "edit_close_group"

	   << "file_export_ascii" << "file_export_latin1" << "file_export_latin2" << "file_export_latin3"
	   << "file_export_latin4" << "file_export_latin5" << "file_export_latin9" << "file_export_cp1250"
	   << "file_export_cp1252"
	   ;

	setMenuItems(projectlist,m_dictMenuProject);
	setMenuItems(filelist,m_dictMenuFile);
	setMenuItems(actionlist,m_dictMenuAction);
}

void Kile::setMenuItems(QStringList &list, QMap<QString,bool> &dict)
{
	for ( QStringList::Iterator it=list.begin(); it!=list.end(); ++it ) {
		dict[(*it)] = true;
	}
}

void Kile::updateMenu()
{
	KILE_DEBUG_MAIN << "==Kile::updateMenu()====================" << endl;
	QAction *a;
	QMap<QString,bool>::Iterator it;

	// update project menus
	m_actRecentProjects->setEnabled( m_actRecentProjects->items().count() > 0 );
	bool project_open = ( docManager()->isProjectOpen() ) ;

	for ( it=m_dictMenuProject.begin(); it!=m_dictMenuProject.end(); ++it ) {
		a = actionCollection()->action(it.key());
		if(a) {
			a->setEnabled(project_open);
		}
	}

	// project_show is only enabled, when more than 1 project is opened
	a = actionCollection()->action("project_show");
	if(a) {
		a->setEnabled(project_open && docManager()->projects().count() > 1);
	}

	// update file menus
	m_actRecentFiles->setEnabled( m_actRecentFiles->items().count() > 0 );
	bool file_open = ( viewManager()->currentTextView() );
	KILE_DEBUG_MAIN << "\tprojectopen=" << project_open << " fileopen=" << file_open << endl;

	enableKileGUI(file_open);
}

bool Kile::updateMenuActivationStatus(QMenu *menu)
{
  return updateMenuActivationStatus(menu, QSet<QMenu*>());
}


bool Kile::updateMenuActivationStatus(QMenu *menu, const QSet<QMenu*>& visited)
{
	if(visited.contains(menu)) {
		qWarning() << "Recursive menu structure detected - aborting!";
		return true;
	}
	if(menu->objectName() == "usermenu-submenu") {
		menu->setEnabled(true);
		return true;
	}

	bool enabled = false;
	QList<QAction*> actionList = menu->actions();

	for(QList<QAction*>::iterator it = actionList.begin(); it != actionList.end(); ++it) {
		QAction *action = *it;
		QMenu *subMenu = action->menu();
		if(subMenu) {
			QSet<QMenu*> newVisited(visited);
			newVisited.insert(menu);
			if(updateMenuActivationStatus(subMenu, newVisited)) {
				enabled = true;
			}
		}
		else if(!action->isSeparator() && action->isEnabled()) {
			enabled = true;
		}
	}
	menu->setEnabled(enabled);
	return enabled;
}

void Kile::updateLatexenuActivationStatus(QMenu *menu, bool state)
{
	if ( menu->isEmpty() || !viewManager()->currentTextView() ) {
		state = false;
	}
	menu->menuAction()->setVisible(state);
}

//TODO: move to KileView::Manager
void Kile::prepareForPart(const QString & state)
{
	KILE_DEBUG_MAIN << "==Kile::prepareForPart====================";

	if(m_currentState == "Editor" && state == "Editor") {
		return;
	}

	resetPart();
	m_wantState = state;

	//deactivate kateparts
	for(int i = 0; i < viewManager()->textViewCount(); ++i) {
		KTextEditor::View *view = viewManager()->textView(i);
		guiFactory()->removeClient(view);
		view->clearFocus();
	}
}

void Kile::runTool(const QString& tool)
{
	runToolWithConfig(tool, QString());
}

void Kile::runToolWithConfig(const QString &toolName, const QString &config)
{
	KILE_DEBUG_MAIN << toolName << config;

	focusLog();
	KileTool::Base *tool = m_manager->createTool(toolName, config);

	if(!tool || (tool->requestSaveAll() && !m_docManager->fileSaveAll())) {
		delete tool;
		return;
	}

	return m_manager->run(tool);
}

void Kile::cleanAll(KileDocument::TextInfo *docinfo)
{
	const QString noactivedoc = i18n("There is no active document or it is not saved.");
	if(!docinfo) {
		KTextEditor::Document *doc = activeTextDocument();
		if (doc) {
			docinfo = docManager()->textInfoFor(doc);
		}
		else {
			errorHandler()->printMessage(KileTool::Error, noactivedoc, i18n("Clean"));
			return;
		}
	}

	if (docinfo) {
		docManager()->cleanUpTempFiles(docinfo->url(), false);
	}
}

void Kile::refreshStructure()
{
	viewManager()->updateStructure(true);
}

void Kile::insertTag(const KileAction::TagData& data)
{
	errorHandler()->clearMessages();

	if(data.description.length() > 0) {
		focusLog();
		errorHandler()->printMessage(data.description);
	}

	KTextEditor::View *view = viewManager()->currentTextView();

	if(!view) {
		return;
	}

	focusTextView(view);

	editorExtension()->insertTag(data, view);
}

void Kile::insertTag(const QString& tagB, const QString& tagE, int dx, int dy)
{
	insertTag(KileAction::TagData(QString(), tagB, tagE, dx, dy));
}

void Kile::insertAmsTag(const KileAction::TagData& data)
{
	insertTag(data, QStringList("amsmath"));
}

void Kile::insertTag(const KileAction::TagData& data,const QList<Package> &pkgs){

   QStringList packages;
   QString pkgName;

   QList<Package>::const_iterator it;
   for(it = pkgs.begin(); it != pkgs.end() ; it++){
	 pkgName = (*it).name;
	 if(!pkgName.isEmpty()){
	    packages.append(pkgName);
	 }
   }

   insertTag(data,packages);
}

void Kile::insertTag(const KileAction::TagData& data,const QStringList &pkgs)
{
	KILE_DEBUG_MAIN << "void Kile::insertTag(const KileAction::TagData& data,const QStringList " << pkgs.join(",") << ")" << endl;
	insertTag(data);

	KileDocument::TextInfo *docinfo = docManager()->textInfoFor(getCompileName());
	if(docinfo) {
		QStringList packagelist = allPackages(docinfo);
		QStringList::const_iterator it;
		QStringList warnPkgs;

		for ( it = pkgs.begin(); it != pkgs.end(); ++it) {
			if(!packagelist.contains(*it)) {
				warnPkgs.append(*it);
			}
		}

		if(warnPkgs.count() > 0) {
			if(warnPkgs.count() == 1) {
				 errorHandler()->printMessage(KileTool::Error, i18n("You have to include the package %1.", warnPkgs.join(",")), i18n("Insert text"));
			}
			else {
				errorHandler()->printMessage(KileTool::Error, i18n("You have to include the packages %1.", warnPkgs.join(",")), i18n("Insert text"));
			}
		}
	}
}

void Kile::insertText(const QString &text)
{
	if(text.indexOf("%C")>=0)
		insertTag(KileAction::TagData(QString(), text, QString(), 0, 0));
	else
		insertTag(KileAction::TagData(QString(), text, "%C", 0, 0));
}

void Kile::insertText(const QString &text, const QStringList &pkgs)
{
	insertTag(KileAction::TagData(QString(), text, "%C", 0, 0), pkgs);
}

void Kile::insertText(const QString &text, const QList<Package> &pkgs)
{
   insertTag(KileAction::TagData(QString(), text, "%C", 0, 0), pkgs);
}

void Kile::quickDocument()
{
	KileDialog::QuickDocument *dlg = new KileDialog::QuickDocument(m_config.data(), this, "Quick Start", i18n("Quick Start"));

	if(dlg->exec()) {
		if(!viewManager()->currentTextView()) {
			docManager()->createNewLaTeXDocument();
		}
		insertTag( dlg->tagData() );
		viewManager()->updateStructure(true);
	}
	delete dlg;
}

void Kile::quickArray()
{
	quickTabulardialog(false);
}

void Kile::quickTabular()
{
	quickTabulardialog(true);
}

void Kile::quickTabulardialog(bool tabularenv)
{
	if(!viewManager()->currentTextView()) {
		return;
	}

	QString env;
	if(tabularenv) {
		KConfigGroup group = m_config->group("Wizard");
		env = group.readEntry("TabularEnvironment", "tabular");
	} else {
		env = "array";
	}

	KileDialog::NewTabularDialog dlg(env, m_latexCommands, m_config.data(), this);
	if(dlg.exec()) {
		insertTag(dlg.tagData(), dlg.requiredPackages());
		if(tabularenv) {
			KConfigGroup group = m_config->group("Wizard");
			group.writeEntry("TabularEnvironment", dlg.environment());
			m_config->sync();
		}
	}
}

void Kile::quickTabbing()
{
	if(!viewManager()->currentTextView()) {
		return;
	}
	KileDialog::QuickTabbing *dlg = new KileDialog::QuickTabbing(m_config.data(), this, this, "Tabbing", i18n("Tabbing"));
	if(dlg->exec()) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickFloat()
{
	if(!viewManager()->currentTextView()) {
		return;
	}

	KileDialog::FloatEnvironmentDialog *dlg = new KileDialog::FloatEnvironmentDialog(m_config.data(), this, this);
	if(dlg->exec()) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickMathenv()
{
	if(!viewManager()->currentTextView()) {
		return;
	}

	KileDialog::MathEnvironmentDialog *dlg = new KileDialog::MathEnvironmentDialog(this, m_config.data(), this, m_latexCommands);
	if(dlg->exec()) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickPostscript()
{
	QString startdir = QDir::homePath();
	QString texfilename;

	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		startdir = QFileInfo(view->document()->url().toLocalFile()).path();
		texfilename = getCompileName();
	}

	KileDialog::PostscriptDialog *dlg = new KileDialog::PostscriptDialog(this, texfilename, startdir, m_extensions->latexDocuments(), errorHandler(), m_outputWidget);
	dlg->exec();
	delete dlg;
}

void Kile::quickPdf()
{
	QString startDir = QDir::homePath();
	QString texFileName;

	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		startDir = QFileInfo(view->document()->url().toLocalFile()).path();
		texFileName = getCompileName();
	}

	KileDialog::PdfDialog *dlg = new KileDialog::PdfDialog(m_mainWindow, texFileName, startDir, m_extensions->latexDocuments(), m_manager, errorHandler(), m_outputWidget);
	dlg->exec();
	delete dlg;
}

void Kile::quickUserMenuDialog()
{
	m_userMenu->removeShortcuts();
	KileMenu::UserMenuDialog *dlg = new KileMenu::UserMenuDialog(m_config.data(), this, m_userMenu, m_userMenu->xmlFile(), m_mainWindow);
	dlg->exec();
	delete dlg;

	// tell all the documents and views to update their action shortcuts (bug 247646)
	docManager()->reloadXMLOnAllDocumentsAndViews();

	// a new usermenu could have been installed, even if the return value is QDialog::Rejected
	m_userMenu->refreshActionProperties();
}

void Kile::slotUpdateUserMenuStatus()
{
	KILE_DEBUG_MAIN << "slot update usermenu status";
	updateUserMenuStatus(true);
}

void Kile::updateUserMenuStatus(bool state)
{
	KILE_DEBUG_MAIN << "update usermenu status";

	if(m_userMenu) {
		QMenu *menu = m_userMenu->getMenuItem();
		if(menu) {
				updateLatexenuActivationStatus(menu,state);
		}
	}
}

void Kile::helpLaTex()
{
	QString loc = QStandardPaths::locate(QStandardPaths::DataLocation, "help/latexhelp.html");
	KileTool::Base *tool = toolManager()->createTool("ViewHTML", QString(), false);
	if(!tool) {
		errorHandler()->printMessage(KileTool::Error, i18n("Could not create the \"ViewHTML\" tool. Please reset the tools."));
		return;
	}
	tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
	tool->setSource(loc);
	tool->setTargetPath(loc);
	tool->prepareToRun();
	m_manager->run(tool);
}

void Kile::readGUISettings()
{
}

// transform old user tags to xml file
void Kile::transformOldUserTags()
{
	KILE_DEBUG_MAIN << "Convert old user tags";
	QString xmldir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/usermenu/";
	// create dir if not existing
	QDir testDir(xmldir);
	if (!testDir.exists()) {
		testDir.mkpath(xmldir);
	}

	KConfigGroup userGroup = m_config->group("User");
	int len = userGroup.readEntry("nUserTags", 0);

	if ( len > 0) {
		QString usertagfile = "usertags.xml";
		QString  filename = xmldir + usertagfile;
		KILE_DEBUG_MAIN << "-convert user tags " << filename;

		QFile file(filename);
		if ( !file.open(QFile::WriteOnly | QFile::Text) ) {
			KILE_DEBUG_MAIN << "-Error - could not open file to write: " << filename;
			return;
		}

		KILE_DEBUG_MAIN << "Write xml: " << filename;
		QXmlStreamWriter xml(&file);
		xml.setAutoFormatting(true);
		xml.setAutoFormattingIndent(2) ;

		xml.writeStartDocument();
		xml.writeStartElement("UserMenu");

		for (int i = 0; i < len; ++i) {
			const QString tagNameConfigKey = "userTagName" + QString::number(i);
			const QString tagname = userGroup.readEntry(tagNameConfigKey, i18n("No Name"));
			const QString tagConfigKey = "userTag" + QString::number(i);
			QString tag = userGroup.readEntry(tagConfigKey, "");
			tag = tag.replace('\n',"\\n");

			xml.writeStartElement("menu");
			xml.writeAttribute("type", "text");
			xml.writeTextElement(KileMenu::UserMenuData::xmlMenuTagName(KileMenu::UserMenuData::XML_TITLE), tagname);
			xml.writeTextElement(KileMenu::UserMenuData::xmlMenuTagName(KileMenu::UserMenuData::XML_PLAINTEXT), tag);
			xml.writeTextElement(KileMenu::UserMenuData::xmlMenuTagName(KileMenu::UserMenuData::XML_SHORTCUT), QString("Ctrl+Shift+%1").arg(i+1));
			xml.writeEndElement();

			userGroup.deleteEntry(tagNameConfigKey);
			userGroup.deleteEntry(tagConfigKey);
		}
		xml.writeEndDocument();
		file.close();

		// save current xml file
		KileConfig::setMenuFile(usertagfile);
	}
	userGroup.deleteEntry("nUserTags");
}

void Kile::transformOldUserSettings()
{
	//test for old kilerc
	int version = KileConfig::rCVersion();

	//if the kilerc file is old some of the configuration
	//data must be set by kile, even if the keys are present
	//in the kilerc file
	if(version < 4) {
		KILE_DEBUG_MAIN << "READING STD TOOL CONFIG" << endl;
		m_toolFactory->readStandardToolConfig();
	}

	//delete old editor key
	if(m_config->hasGroup("Editor")) {
		m_config->deleteGroup("Editor");
	}

	//convert user tools to new KileTool classes
	KConfigGroup userGroup = m_config->group("User");
	userItem tempItem;
	int len = userGroup.readEntry("nUserTools", 0);
	for (int i=0; i< len; ++i) {
		tempItem.name = userGroup.readEntry("userToolName" + QString::number(i), i18n("no name"));
		tempItem.tag = userGroup.readEntry("userTool" + QString::number(i), "");
		m_listUserTools.append(tempItem);
	}
	if(len > 0) {
 		//move the tools
		userGroup.writeEntry("nUserTools", 0);
		for(int i = 0; i < len; ++i) {
			tempItem = m_listUserTools[i];
			KConfigGroup toolsGroup = m_config->group("Tools");
			toolsGroup.writeEntry(tempItem.name, "Default");

			KileTool::setGUIOptions(tempItem.name, "Other", "preferences-other", m_config.data());

			KConfigGroup group = m_config->group(KileTool::groupFor(tempItem.name, "Default"));
			QString bin = KRun::binaryName(tempItem.tag, false);
			group.writeEntry("command", bin);
			group.writeEntry("options", tempItem.tag.mid(bin.length()));
			group.writeEntry("class", "Base");
			group.writeEntry("type", "Process");
			group.writeEntry("from", "");
			group.writeEntry("to", "");

			if(i < 10) {
				QAction *toolAction = static_cast<QAction*>(actionCollection()->action("tool_" + tempItem.name));
				actionCollection()->setDefaultShortcut(toolAction, "Alt+Shift+" + QString::number(i + 1)); //should be alt+shift+
			}
		}
	}
}

void Kile::readRecentFileSettings()
{
	KConfigGroup group = m_config->group("FilesOpenOnStart");
	int n = group.readEntry("NoDOOS", 0);
	for (int i = 0; i < n; ++i) {
		const QString urlString = group.readPathEntry("DocsOpenOnStart" + QString::number(i), "");
		if(urlString.isEmpty()) {
			continue;
		}
		m_listDocsOpenOnStart.append(urlString);
		m_listEncodingsOfDocsOpenOnStart.append(group.readPathEntry("EncodingsOfDocsOpenOnStart" + QString::number(i), ""));
	}

	n = group.readEntry("NoPOOS", 0);
	for(int i = 0; i < n; ++i) {
		const QString urlString = group.readPathEntry("ProjectsOpenOnStart" + QString::number(i), "");
		if(urlString.isEmpty()) {
			continue;
		}
		m_listProjectsOpenOnStart.append(urlString);
	}
}

void Kile::readConfig()
{
	m_codeCompletionManager->readConfig(m_config.data());
#if LIVEPREVIEW_AVAILABLE
	if(m_livePreviewManager) {
		m_livePreviewManager->readConfig(m_config.data());
	}
#endif
	//m_edit->initDoubleQuotes();
	m_edit->readConfig();
	docManager()->updateInfos();
	m_jScriptManager->readConfig();
	docManager()->readConfig();
	viewManager()->readConfig(m_horizontalSplitter);

	// set visible views in sidebar
	m_sideBar->setPageVisible(m_scriptsManagementWidget, KileConfig::scriptingEnabled());
	m_sideBar->setPageVisible(m_commandViewToolBox, KileConfig::showCwlCommands());
	m_sideBar->setPageVisible(m_kileAbbrevView, KileConfig::completeShowAbbrev());

	m_scriptsManagementWidget->setScriptNameColumnWidth(KileConfig::scriptNameColumnWidth());

	if(KileConfig::displayMFUS()) {
		enableSymbolViewMFUS();
	}
	else {
		disableSymbolViewMFUS();
	}
	m_commandViewToolBox->readCommandViewFiles();
	abbreviationManager()->readAbbreviationFiles();
}

void Kile::saveSettings()
{
	showEditorWidget();

	m_fileBrowserWidget->writeConfig();
#if LIVEPREVIEW_AVAILABLE
	if(m_livePreviewManager) {
		m_livePreviewManager->writeConfig();
	}
#endif
	m_symbolViewMFUS->writeConfig();
	saveLastSelectedAction();
	// Store recent files
	m_actRecentFiles->saveEntries(m_config->group("Recent Files"));
	m_actRecentProjects->saveEntries(m_config->group("Projects"));

	m_config->deleteGroup("FilesOpenOnStart");
	if (KileConfig::restore())
	{
		KConfigGroup configGroup = m_config->group("FilesOpenOnStart");
		KileConfig::setSingleFileMasterDocument(getMasterDocumentFileName());
		configGroup.writeEntry("NoDOOS", m_listDocsOpenOnStart.count());
		for (int i = 0; i < m_listDocsOpenOnStart.count(); ++i) {
			configGroup.writePathEntry("DocsOpenOnStart" + QString::number(i), m_listDocsOpenOnStart[i]);
			configGroup.writePathEntry("EncodingsOfDocsOpenOnStart" + QString::number(i), m_listEncodingsOfDocsOpenOnStart[i]);
		}

		configGroup.writeEntry("NoPOOS", m_listProjectsOpenOnStart.count());
		for (int i = 0; i < m_listProjectsOpenOnStart.count(); ++i) {
			configGroup.writePathEntry("ProjectsOpenOnStart"+QString::number(i), m_listProjectsOpenOnStart[i]);
		}
	}

	KConfigGroup configGroup = KSharedConfig::openConfig()->group("KileMainWindow");
	saveMainWindowSettings(configGroup);

	docManager()->writeConfig();
	viewManager()->writeConfig();

	scriptManager()->writeConfig();
	KileConfig::setScriptNameColumnWidth(m_scriptsManagementWidget->scriptNameColumnWidth());

	KileConfig::setRCVersion(KILERC_VERSION);
	KileConfig::setMainwindowWidth(width());
	KileConfig::setMainwindowHeight(height());

	QList<int> sizes;
	QList<int>::Iterator it;
	sizes = m_horizontalSplitter->sizes();
	it = sizes.begin();
	KileConfig::setHorizontalSplitterLeft(*it);
	++it;
	KileConfig::setHorizontalSplitterRight(*it);
	sizes.clear();
	sizes = m_verticalSplitter->sizes();
	it = sizes.begin();
	KileConfig::setVerticalSplitterTop(*it);
	++it;
	KileConfig::setVerticalSplitterBottom(*it);
#ifdef __GNUC__
#warning Restoring the side bar sizes from minimized after start up does not work perfectly yet!
#endif
// 	// sync vertical splitter and size of bottom bar
// 	int sizeBottomBar = m_bottomBar->directionalSize();
// 	if(m_bottomBar->isVisible()) {
// 		sizeBottomBar = m_verSplitBottom;
// 	}
// 	else {
// 		m_verSplitBottom = sizeBottomBar;
// 	}
	KileConfig::setSideBar(!m_sideBar->isHidden()); // do not use 'isVisible()'!
	KileConfig::setSideBarSize(m_sideBar->directionalSize());
	KileConfig::setBottomBar(!m_bottomBar->isHidden()); // do not use 'isVisible()'!
	KileConfig::setBottomBarSize(m_bottomBar->directionalSize());
	KileConfig::setBottomBarIndex(m_bottomBar->currentTab());

	KileConfig::setSelectedLeftView(m_sideBar->currentTab());

	abbreviationManager()->saveLocalAbbreviations();

	KileConfig::self()->save();
	m_config->sync();
}

/////////////////  OPTIONS ////////////////////
void Kile::setMasterDocumentFileName(const QString& fileName)
{
	if(fileName.isEmpty() || !viewManager()->viewForLocalFilePresent(fileName)) {
		return;
	}

	m_masterDocumentFileName = fileName;

	QString shortName = QFileInfo(m_masterDocumentFileName).fileName();

	ModeAction->setText(i18n("Normal mode (current master document: %1)", shortName));
	ModeAction->setChecked(true);
	m_singlemode = false;
	updateModeStatus();
	emit masterDocumentChanged();
	KILE_DEBUG_MAIN << "SETTING master to " << m_masterDocumentFileName << " singlemode = " << m_singlemode << endl;
}

void Kile::clearMasterDocument()
{
	ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
	ModeAction->setChecked(false);
	m_singlemode = true;
	m_masterDocumentFileName.clear();
	updateModeStatus();
	emit masterDocumentChanged();
	KILE_DEBUG_MAIN << "CLEARING master document";
}

void Kile::toggleMasterDocumentMode()
{
	if (!m_singlemode) {
		clearMasterDocument();
	}
	else if (m_singlemode && viewManager()->currentTextView()) {
		QString name = getName();
		if(name.isEmpty()) {
			ModeAction->setChecked(false);
			KMessageBox::error(this, i18n("In order to define the current document as a master document, it has to be saved first."));
			return;
		}
		setMasterDocumentFileName(name);
	}
	else {
		ModeAction->setChecked(false);
		updateModeStatus();
	}
}

void Kile::toggleWatchFile()
{
	m_bWatchFile=!m_bWatchFile;

	if (m_bWatchFile) {
		WatchFileAction->setChecked(true);
	}
	else {
		WatchFileAction->setChecked(false);
	}
}

// execute configuration dialog

void Kile::generalOptions()
{
	KileDialog::Config *dlg = new KileDialog::Config(m_config.data(), this, this);

	if (dlg->exec())
	{
		// update new settings
		readConfig();
		saveLastSelectedAction(); // save the old current tools before calling setupTools() which calls restoreLastSelectedActions()
		setupTools();
		m_help->update();
		newCaption(); // for the 'showFullPathInWindowTitle' setting

		configurationManager()->emitConfigChanged();

		//stop/restart LyX server if necessary
		if(KileConfig::runLyxServer() && !m_lyxserver->isRunning()) {
			m_lyxserver->start();
		}

		if(!KileConfig::runLyxServer() && m_lyxserver->isRunning()) {
			m_lyxserver->stop();
		}
	}

	delete dlg;
}

void Kile::slotPerformCheck()
{
	// first we have to disable the live preview that may be running, and clear the master document
#if LIVEPREVIEW_AVAILABLE
	const bool livePreviewEnabledForFreshlyOpenedDocuments = KileConfig::previewEnabledForFreshlyOpenedDocuments();
	const bool livePreviewEnabledForCurrentDocument = livePreviewManager() && livePreviewManager()->isLivePreviewEnabledForCurrentDocument();
	if (livePreviewManager()) {
		KileConfig::setPreviewEnabledForFreshlyOpenedDocuments(false);
		livePreviewManager()->setLivePreviewEnabledForCurrentDocument(false);
	}
#endif
	QString currentMasterDocument = m_masterDocumentFileName;
	if(!m_singlemode) {
		clearMasterDocument();
	}
	// now, we can run the tests
	KileDialog::ConfigChecker *dlg = new KileDialog::ConfigChecker(this);
	dlg->exec();
	delete dlg;
	// finally, we restore the rest to what it was before launching the tests
	if(!currentMasterDocument.isEmpty()) {
		setMasterDocumentFileName(currentMasterDocument);
	}
#if LIVEPREVIEW_AVAILABLE
	if (livePreviewManager()) {
		KileConfig::setPreviewEnabledForFreshlyOpenedDocuments(livePreviewEnabledForFreshlyOpenedDocuments);
		if(livePreviewEnabledForCurrentDocument) {
			livePreviewManager()->setLivePreviewEnabledForCurrentDocument(true);
		}
	}
#endif
}

void Kile::aboutEditorComponent()
{
	KTextEditor::Editor *editor = m_docManager->getEditor();
	if(!editor) {
		return;
	}
	KAboutApplicationDialog dialog(editor->aboutData(), this);
	dialog.exec();
}

/////////////// KEYS - TOOLBARS CONFIGURATION ////////////////
void Kile::configureKeys()
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
// due to bug 280988, we can't add all the clients...
// 	QList<KXMLGUIClient*> clients = guiFactory()->clients();
// 	for(QList<KXMLGUIClient*>::iterator it = clients.begin(); it != clients.end(); ++it) {
// 		dlg.addCollection((*it)->actionCollection());
// 	}
	dlg.addCollection(mainWindow()->actionCollection());
	KTextEditor::View *view = m_viewManager->currentTextView();
	if(view) {
		dlg.addCollection(view->actionCollection());
	}
	KParts::ReadOnlyPart *part = viewManager()->viewerPart();
	if(part) {
		dlg.addCollection(part->actionCollection());
	}
	dlg.configure();

	// tell all the documents and views to update their action shortcuts (bug 247646)
	docManager()->reloadXMLOnAllDocumentsAndViews();

	// tell m_userMenu that key bindings may have been changed
	m_userMenu->updateKeyBindings();
}

void Kile::configureToolbars()
{
	KConfigGroup configGroup = KSharedConfig::openConfig()->group("KileMainWindow");
	saveMainWindowSettings(configGroup);
	KEditToolBar dlg(factory());
	dlg.exec();

	setUpdatesEnabled(false);
	applyMainWindowSettings(m_config->group("KileMainWindow"));

	updateUserDefinedMenus();
	updateGUI(m_currentState);
	setUpdatesEnabled(true);
}

//////////////////// CLEAN BIB /////////////////////
void Kile::cleanBib()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if ( ! view )
		return;

	QRegExp reOptional( "(ALT|OPT)(\\w+)\\s*=\\s*(\\S.*)" );
	QRegExp reNonEmptyEntry( ".*\\w.*" );

	QString s;
	int i = 0;
	while(i < view->document()->lines()) {
		s = view->document()->line(i);

		// do we have a line that starts with ALT or OPT?
		if(reOptional.indexIn(s) >= 0) {
				// yes! capture type and entry
				QString type = reOptional.cap( 2 );
				QString entry = reOptional.cap( 3 );
				view->document()->removeLine( i );
				view->document()->setModified(true);
				if(reNonEmptyEntry.indexIn(entry) >= 0) {
					type.append(" = ");
					type.append(entry);
					view->document()->insertLine(i, type);
					++i;
				}
		}
		else {
			++i;
		}
	}
	int j = 0;
	for (i = 0; i < view->document()->lines(); ++i) {
		j = i+1;
		if(j < view->document()->lines() && view->document()->line(j).contains(QRegExp("^\\s*\\}\\s*$"))) {
			s =  view->document()->line(i);
			view->document()->removeLine(i);
			s.remove(QRegExp(",\\s*$"));
			view->document()->setModified(true);
			view->document()->insertLine(i, s);
		}
	}
}

void Kile::includeGraphics()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if ( !view ) return;

	QFileInfo fi( view->document()->url().toLocalFile() );
	KileDialog::IncludeGraphics *dialog = new KileDialog::IncludeGraphics(this, fi.path(), this);

	if ( dialog->exec() == QDialog::Accepted )
	{
		insertTag(dialog->getTemplate(), "%C", 0,0);
		docManager()->projectAddFile( dialog->getFilename(),true );
	}

	delete dialog;
}

void Kile::slotToggleFullScreen()
{
	if (!m_pFullScreen->isChecked()) {
		setWindowState( windowState() & ~Qt::WindowFullScreen );
	}
	else {
		setWindowState( windowState() | Qt::WindowFullScreen );
	}
}

/////////////// QuickPreview (dani) ////////////////

// all calls of QuickPreview will get here, so we can decide what to do
// rewritten Sep 05 2006 to work together with preview in the bottom bar

void Kile::slotQuickPreview(int type)
{
	KILE_DEBUG_MAIN << "==Kile::slotQuickPreview()=========================="  << endl;

	KTextEditor::View *view = viewManager()->currentTextView();
	if ( ! view) return;

	KTextEditor::Document *doc = view->document();
	if ( ! doc )
		return;

	switch ( type )
	{
		case KileTool::qpSelection:   m_quickPreview->previewSelection(view);   break;
		case KileTool::qpEnvironment: m_quickPreview->previewEnvironment(doc); break;
		case KileTool::qpSubdocument: m_quickPreview->previewSubdocument(doc); break;
		case KileTool::qpMathgroup:   m_quickPreview->previewMathgroup(doc);   break;
	}
}

/* FIXME
 Port the citeViewBib function as soon as we got a kbib version for KDE4.
void Kile::citeViewBib()
{
	KILE_DEBUG_MAIN  << "===void Kile::citeViewBib()===" << endl;

	DCOPClient *client = kapp->dcopClient();
	QByteArray params, replyData;
	Q3CString replyType;

	QDataStream stream(params, QIODevice::WriteOnly);
	QCStringList functions,remoteApps,remoteObjs;

 	const Q3CString viewBibApp = "kbib"; // currently these things are hardcoded because only kbib supports it
 	const Q3CString viewBibObj = "kbibapp";
	const Q3CString viewBibFncDef = "QString cite()";
	const Q3CString viewBibFnc = "cite()";

	remoteApps = client->registeredApplications();
	if( !remoteApps.contains(viewBibApp) )
	{
		errorHandler()->printMessage(KileTool::Warning,
		i18n("No ViewBib tool running, trying to start it now"),
		i18n("ViewBib Citation"));
		uint ret = runWith("ViewBib","KBib");
		if( ret == 0 )
			errorHandler()->printMessage(KileTool::Info,
				i18n("Please select the desired bibliographies and re-execute this command"),
				i18n("ViewBib Citation"));
		return;
	}

	remoteObjs = client->remoteObjects(viewBibApp);
	if( !remoteObjs.contains(viewBibObj) )
	{
		errorHandler()->printMessage(KileTool::Warning,
				      i18n("The ViewBib tool does not have the correct interface"),
				      i18n("ViewBib Citation"));
		return;
	}

	functions = client->remoteFunctions(viewBibApp,viewBibObj);
	if( !functions.contains(viewBibFncDef) )
	{
		errorHandler()->printMessage(KileTool::Warning,
					i18n("The ViewBib tool does not have the correct definition of the cite function"),
					i18n("ViewBib Citation"));
		return;
	}

	if ( !client->call(viewBibApp, viewBibObj, viewBibFnc, params, replyType, replyData) )
	{
		// we should never get here
		qWarning() << "internal error in viewbib citation" << endl;
		return;
	}
	else{
		QDataStream reply(replyData, QIODevice::ReadOnly);
		if (replyType == "QString")
		{
			QString result;
			reply >> result;

			if (result.isEmpty())
			{
				errorHandler()->printMessage(KileTool::Warning,
						i18n("No reference selected.\nPlease select a reference first!"),
						i18n("ViewBib Citation"));
			}
			else
			{
				insertTag(KileAction::TagData(i18n("ViewBib Citation"), result, QString(), result.length()));

			}
		}
	}
}
*/

void Kile::addRecentFile(const QUrl &url)
{
	m_actRecentFiles->addUrl(url);
}

void Kile::removeRecentFile(const QUrl &url)
{
	m_actRecentFiles->removeUrl(url);
}

void Kile::addRecentProject(const QUrl &url)
{
	m_actRecentProjects->addUrl(url);
}

void Kile::removeRecentProject(const QUrl &url)
{
	m_actRecentProjects->removeUrl(url);
}

void Kile::updateStatusBarCursorPosition(KTextEditor::View *view,
                                         const KTextEditor::Cursor &newPosition)
{
	if(!view) {
		statusBar()->clearLineColumn();
	}
	else {
		statusBar()->setLineColumn(newPosition.line() + 1, newPosition.column() + 1);
	}
}

void Kile::updateStatusBarViewMode(KTextEditor::View *view)
{
	if(!view) {
		statusBar()->clearViewMode();
	}
	else {
		statusBar()->setViewMode(view->viewModeHuman());
	}
}

void Kile::updateStatusBarInformationMessage(KTextEditor::View * /* view */, const QString &message)
{
	statusBar()->showMessage(message, 5000);
}

void Kile::updateStatusBarSelection(KTextEditor::View *view)
{
	if(!view) {
		statusBar()->clearSelectionMode();
	}
	else {
		const QString text = view->blockSelection() ?
					i18nc("@info:status status bar label for block selection mode", "BLOCK") + ' ' :
					i18nc("@info:status status bar label for line selection mode", "LINE") + ' ';
		statusBar()->setSelectionMode(text);
	}
}

void Kile::handleDocumentParsingStarted()
{
	statusBar()->setParserStatus(i18n("Refreshing structure..."));
}

void Kile::handleDocumentParsingComplete()
{
	statusBar()->clearParserStatus();
}
