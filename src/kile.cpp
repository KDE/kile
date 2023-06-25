/****************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
            (C) 2007-2022 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include <KIO/DesktopExecParser>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KShortcutsDialog>
#include <KToggleAction>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>
#include <KSelectAction>
#include <KWindowSystem>

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
#include "scripting/script.h"

#include "dialogs/usermenu/usermenudialog.h"
#include "usermenu/usermenudata.h"
#include "usermenu/usermenu.h"
#include "utilities.h"

#define LOG_TAB     0
#define OUTPUT_TAB  1
#define KONSOLE_TAB 2
#define PREVIEW_TAB 3

/*
 * Class Kile.
 */

Kile::Kile(bool allowRestore, QWidget *parent)
    : KParts::MainWindow(),
      KileInfo(this),
      m_toolsToolBar(Q_NULLPTR),       // we have to set all of these to null as the constructor
      m_userHelpActionMenu(Q_NULLPTR), // might return early
      m_bibTagSettings(Q_NULLPTR),
      m_compilerActions(Q_NULLPTR),
      m_viewActions(Q_NULLPTR),
      m_convertActions(Q_NULLPTR),
      m_quickActions(Q_NULLPTR),
      m_bibTagActionMenu(Q_NULLPTR),
      ModeAction(Q_NULLPTR),
      WatchFileAction(Q_NULLPTR),
      m_actionMessageView(Q_NULLPTR),
      m_actionShowMenuBar(Q_NULLPTR),
      m_actRecentFiles(Q_NULLPTR),
      m_pFullScreen(Q_NULLPTR),
      m_sideBar(Q_NULLPTR),
      m_kileAbbrevView(Q_NULLPTR),
      m_topWidgetStack(Q_NULLPTR),
      m_horizontalSplitter(Q_NULLPTR),
      m_verticalSplitter(Q_NULLPTR),
      m_toolBox(Q_NULLPTR),
      m_commandViewToolBox(Q_NULLPTR),
      m_symbolViewMFUS(Q_NULLPTR),
      m_symbolViewRelation(Q_NULLPTR),
      m_symbolViewArrows(Q_NULLPTR),
      m_symbolViewMiscMath(Q_NULLPTR),
      m_symbolViewMiscText(Q_NULLPTR),
      m_symbolViewOperators(Q_NULLPTR),
      m_symbolViewUser(Q_NULLPTR),
      m_symbolViewDelimiters(Q_NULLPTR),
      m_symbolViewGreek(Q_NULLPTR),
      m_symbolViewSpecial(Q_NULLPTR),
      m_symbolViewCyrillic(Q_NULLPTR),
      m_commandView(Q_NULLPTR),
      m_latexOutputErrorToolBar(Q_NULLPTR),
      m_buildMenuTopLevel(Q_NULLPTR),
      m_buildMenuCompile(Q_NULLPTR),
      m_buildMenuConvert(Q_NULLPTR),
      m_buildMenuViewer(Q_NULLPTR),
      m_buildMenuOther(Q_NULLPTR),
      m_buildMenuQuickPreview(Q_NULLPTR),
      m_actRecentProjects(Q_NULLPTR),
      m_lyxserver(Q_NULLPTR)
{
    setObjectName("Kile");

    m_config = KSharedConfig::openConfig();

    setStandardToolBarMenuEnabled(true);

    m_singlemode = true;

    m_viewManager= new KileView::Manager(this, actionCollection(), parent, "KileView::Manager");
    viewManager()->setClient(this);

    // fail gracefully if we cannot instantiate Okular part correctly
    if(!m_viewManager->viewerPart()) {
        return;
    }

    QSplashScreen splashScreen(QPixmap(KileUtilities::locate(QStandardPaths::AppDataLocation, "pics/kile_splash.png")), Qt::WindowStaysOnTopHint);
    if(KileConfig::showSplashScreen()) {
        splashScreen.show();
        qApp->processEvents();
    }

    m_codeCompletionManager = new KileCodeCompletion::Manager(this, parent);

    // process events for correctly displaying the splash screen
    qApp->processEvents();

    m_latexCommands = new KileDocument::LatexCommands(m_config.data(), this);  // at first (dani)
    m_edit = new KileDocument::EditorExtension(this);
    m_help = new KileHelp::Help(m_edit, this);
    m_errorHandler = new KileErrorHandler(this, this, actionCollection());
    m_quickPreview = new KileTool::QuickPreview(this);
    m_extensions = new KileDocument::Extensions();
    m_jScriptManager = new KileScript::Manager(this, m_config.data(), actionCollection(), parent, "KileScript::Manager");

    // do initializations first
    m_bWatchFile = false;

    setStatusBar(new KileWidget::StatusBar(m_errorHandler, parent));

    // process events for correctly displaying the splash screen
    qApp->processEvents();

    connect(viewManager(), &KileView::Manager::currentViewChanged, this, &Kile::newCaption);
    connect(viewManager(), &KileView::Manager::currentViewChanged, this, [this](QWidget* view) { activateView(view); });
    connect(viewManager(), &KileView::Manager::currentViewChanged, this, &Kile::updateModeStatus);
    connect(viewManager(), &KileView::Manager::updateCaption, this, &Kile::newCaption);
    connect(viewManager(), &KileView::Manager::updateModeStatus, this, &Kile::updateModeStatus);
    connect(viewManager(), &KileView::Manager::cursorPositionChanged, this, &Kile::updateStatusBarCursorPosition);
    connect(viewManager(), &KileView::Manager::viewModeChanged,
            this, &Kile::updateStatusBarViewMode);
    connect(viewManager(), &KileView::Manager::informationMessage,
            this, &Kile::updateStatusBarInformationMessage);
    connect(viewManager(), &KileView::Manager::selectionChanged,
            this, &Kile::updateStatusBarSelection);

    connect(docManager(), &KileDocument::Manager::documentNameChanged, this, &Kile::newCaption);
    connect(docManager(), &KileDocument::Manager::documentUrlChanged, this, &Kile::newCaption);
    connect(docManager(), &KileDocument::Manager::documentReadWriteStateChanged, this, &Kile::newCaption);

    m_topWidgetStack = new QStackedWidget();
    m_topWidgetStack->setFocusPolicy(Qt::NoFocus);

    m_horizontalSplitter = new QSplitter(Qt::Horizontal);

    setupSideBar();
    m_horizontalSplitter->addWidget(m_sideBar);

    m_verticalSplitter = new QSplitter(Qt::Vertical);
    m_horizontalSplitter->addWidget(m_verticalSplitter);
    viewManager()->createTabs(m_verticalSplitter);

    connect(viewManager(), &KileView::Manager::activateView, this, &Kile::activateView);
    connect(viewManager(), &KileView::Manager::startQuickPreview, this, &Kile::slotQuickPreview);

    connect(parserManager(), &KileParser::Manager::documentParsingStarted, this, &Kile::handleDocumentParsingStarted);
    connect(parserManager(), &KileParser::Manager::documentParsingComplete, this, &Kile::handleDocumentParsingComplete);

    // process events for correctly displaying the splash screen
    qApp->processEvents();

    setupBottomBar();
    m_verticalSplitter->addWidget(m_bottomBar);
    m_topWidgetStack->addWidget(m_horizontalSplitter);
    setCentralWidget(m_topWidgetStack);

    // Parser manager and view manager must be created before the tool manager!
    m_manager = new KileTool::Manager(this, m_config.data(), m_outputWidget, m_topWidgetStack, 10000, actionCollection()); //FIXME make timeout configurable
    connect(m_manager, &KileTool::Manager::jumpToFirstError, m_errorHandler, &KileErrorHandler::jumpToFirstError);
    connect(m_manager, &KileTool::Manager::previewDone, this, &Kile::focusPreview);

    m_latexOutputErrorToolBar->addAction(actionCollection()->action(QLatin1String("Stop")));
    errorHandler()->setErrorHandlerToolBar(m_latexOutputErrorToolBar); // add the remaining actions to m_latexOutputErrorToolBar

    m_bottomBar->addExtraWidget(viewManager()->getViewerControlToolBar());

    m_livePreviewManager = new KileTool::LivePreviewManager(this, actionCollection());
    connect(this, &Kile::masterDocumentChanged, m_livePreviewManager, &KileTool::LivePreviewManager::handleMasterDocumentChanged);

    m_toolFactory = new KileTool::Factory(m_manager, m_config.data(), actionCollection());
    m_manager->setFactory(m_toolFactory);

    setupGraphicTools();
    setupPreviewTools();
    setupActions();

    initSelectActions();

    newCaption();

    m_help->setUserhelp(m_manager, m_userHelpActionMenu);     // kile user help (dani)

    // process events for correctly displaying the splash screen
    qApp->processEvents();
    connect(docManager(), &KileDocument::Manager::updateModeStatus, this, &Kile::updateModeStatus);
    connect(docManager(), &KileDocument::Manager::updateStructure, viewManager(), &KileView::Manager::updateStructure);
    connect(docManager(), &KileDocument::Manager::closingDocument,
            m_kwStructure, &KileWidget::StructureWidget::closeDocumentInfo);
    connect(docManager(), &KileDocument::Manager::documentInfoCreated,
            m_kwStructure, &KileWidget::StructureWidget::addDocumentInfo);
    connect(docManager(), &KileDocument::Manager::updateReferences,
            m_kwStructure, &KileWidget::StructureWidget::updateReferences);
    connect(docManager(), &KileDocument::Manager::documentModificationStatusChanged,
            viewManager(), &KileView::Manager::reflectDocumentModificationStatus);

    if(KileConfig::rCVersion() < 8) {
        transformOldUserSettings();
        transformOldUserTags();

        // before Kile 2.1 shortcuts were stored in a "Shortcuts" group inside
        // Kile's configuration file, but this led to problems with the way of how shortcuts
        // are generally stored in kdelibs; we now delete the "Shortcuts" group if it
        // still present in Kile's configuration file.
        if(m_config->hasGroup("Shortcuts")) {
            KConfigGroup shortcutGroup = m_config->group("Shortcuts");
            actionCollection()->readSettings(&shortcutGroup);
            m_config->deleteGroup("Shortcuts");
        }

        if(m_config->hasGroup("Complete")) {
            KConfigGroup completionGroup = m_config->group("Complete");
            completionGroup.deleteEntry("maxCwlFiles"); // in Kile 3.0 the UI has been changed so that this setting is no longer
            // needed
        }
    }
    if(KileConfig::rCVersion() < 9) {
        // in Kile 3.0 beta 4, the user help was updated, some old config settings were no longer needed
        if(m_config->hasGroup("Help")) {
            KConfigGroup helpGroup = m_config->group("Help");
            helpGroup.deleteEntry("location");
            helpGroup.deleteEntry("texrefs");
            helpGroup.deleteEntry("external");
            helpGroup.deleteEntry("embedded");
        }
    }

    readGUISettings();
    readRecentFileSettings();
    readConfig();

    createToolActions(); // this creates the actions for the tools and user tags, which is required before 'activePartGUI' is called

    setupGUI(KXmlGuiWindow::StatusBar | KXmlGuiWindow::Save, "kileui.rc");
    createShellGUI(true); // do not call guiFactory()->refreshActionProperties() after this! (bug 314580)

    m_userMenu = new KileMenu::UserMenu(this, this);
    connect(m_userMenu, &KileMenu::UserMenu::sendText, this, static_cast<void (Kile::*)(const QString &)>(&Kile::insertText));
    connect(m_userMenu, &KileMenu::UserMenu::updateStatus, this, &Kile::slotUpdateUserMenuStatus);

    updateUserDefinedMenus();

    // we can only do this here after the main GUI has been set up
    {
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
        horSplitLeft = m_sideBar->width();
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
    connect(m_lyxserver, &KileLyxServer::insert, this, [this](const KileAction::TagData &data) { insertTag(data); });

    if(m_listUserTools.count() > 0) {
        KMessageBox::information(0, i18n("You have defined some tools in the User menu. From now on these tools will be available from the Build->Other menu and can be configured in the configuration dialog (go to the Settings menu and choose Configure Kile). This has some advantages; your own tools can now be used in a QuickBuild command if you wish."), i18n("User Tools Detected"));
        m_listUserTools.clear();
    }

    if(KileConfig::rCVersion() < 8) {
        // if KileConfig::rCVersion() <= 0, then 'kilerc' is (most likely) fresh or empty,
        // otherwise, we have to ask the user if she wants to reset the tools
        if ((KileConfig::rCVersion() <= 0) || (KMessageBox::questionYesNo(mainWindow(),
                                               i18n("<p>The tool settings need to be reset for this version of Kile to function properly.<br/>"
                                                       "This will overwrite any changes you have made.</p>"
                                                       "<p>Do you want to reset the tools now?</p>"),
                                               i18n("Tools need to be reset"))  == KMessageBox::Yes)) {
            m_toolFactory->resetToolConfigurations();
        }
    }

    restoreFilesAndProjects(allowRestore);
    initMenu();
    updateModeStatus();

    // finally init all actions for the ScriptManager
    m_jScriptManager->initScriptActions();

    setUpdatesEnabled(false);
    setAutoSaveSettings(QLatin1String("KileMainWindow"),true);

    m_userMenu->refreshActionProperties();
    setUpdatesEnabled(true);

    // finally, we check whether the system check assistant should be run, which is important for
    // version 3.0 regarding the newly introduced live preview feature
    const QString& lastVersionRunFor = KileConfig::systemCheckLastVersionRunForAtStartUp();
    if(lastVersionRunFor.isEmpty() || compareVersionStrings(lastVersionRunFor, "2.9.91") < 0) {
#ifdef Q_OS_WIN
        // work around the problem that Sonnet's language auto-detection feature doesn't work
        // together with KatePart (as of 08 November 2019)
        QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
        settings.setValue(QStringLiteral("autodetectLanguage"), false);
#endif
        slotPerformCheck();
        KileConfig::setSystemCheckLastVersionRunForAtStartUp(kileFullVersion);
    }

    if(m_livePreviewManager) {
        m_livePreviewManager->buildLivePreviewMenu(m_config.data());
        m_livePreviewManager->disableBootUpMode();
    }

    menuBar()->show();
}

Kile::~Kile()
{
    KILE_DEBUG_MAIN << "cleaning up..." << Qt::endl;

    guiFactory()->removeClient(viewManager()->viewerPart());

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
    connect(m_fileBrowserWidget, &KileWidget::FileBrowserWidget::fileSelected,
            docManager(), [this](const KFileItem& item) { docManager()->fileSelected(item); });

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
    KileWidget::ProjectView *projectView = new KileWidget::ProjectView(m_sideBar, this);
// 	viewManager()->setProjectView(projectView);
    m_sideBar->addPage(projectView, QIcon::fromTheme("relation"), i18n("Files and Projects"));
    connect(projectView, QOverload<const KileProjectItem*>::of(&KileWidget::ProjectView::fileSelected),
            docManager(), QOverload<const KileProjectItem*>::of(&KileDocument::Manager::fileSelected));

    connect(projectView, QOverload<const QUrl&>::of(&KileWidget::ProjectView::fileSelected),
            docManager(), QOverload<const QUrl&>::of(&KileDocument::Manager::fileSelected));

    connect(projectView, &KileWidget::ProjectView::closeURL,
            docManager(), [this](const QUrl& url) { docManager()->fileClose(url); });

    connect(projectView, &KileWidget::ProjectView::closeProject,
            docManager(), [this](const QUrl& url) { docManager()->projectClose(url); });

    connect(projectView, &KileWidget::ProjectView::projectOptions,
            docManager(), [this](const QUrl& url) { docManager()->projectOptions(url); });

    connect(projectView, &KileWidget::ProjectView::projectArchive,
            this, [this](const QUrl& url) { runArchiveTool(url); });

    connect(projectView, &KileWidget::ProjectView::removeFromProject,
            docManager(), &KileDocument::Manager::removeFromProject);

    connect(projectView, &KileWidget::ProjectView::addFiles,
            docManager(), [this](const QUrl &url) { docManager()->projectAddFiles(url); });

    connect(projectView, &KileWidget::ProjectView::openAllFiles,
            docManager(), [this](const QUrl &url) { docManager()->projectOpenAllFiles(url); });

    connect(projectView, &KileWidget::ProjectView::toggleArchive,
            docManager(), &KileDocument::Manager::toggleArchive);

    connect(projectView, &KileWidget::ProjectView::addToProject,
            docManager(), [this](const QUrl &url) { docManager()->addToProject(url); });

    connect(projectView, &KileWidget::ProjectView::saveURL,
            docManager(), &KileDocument::Manager::saveURL);

    connect(projectView, &KileWidget::ProjectView::buildProjectTree,
            docManager(), [this](const QUrl &url) { docManager()->buildProjectTree(url); });

    connect(docManager(), &KileDocument::Manager::projectTreeChanged,
            projectView, &KileWidget::ProjectView::refreshProjectTree);

    connect(docManager(), QOverload<const QUrl&>::of(&KileDocument::Manager::removeFromProjectView),
            projectView, QOverload<const QUrl&>::of(&KileWidget::ProjectView::remove));

    connect(docManager(), QOverload<const KileProject*>::of(&KileDocument::Manager::removeFromProjectView),
            projectView, QOverload<const KileProject*>::of(&KileWidget::ProjectView::remove));

    connect(docManager(), QOverload<const QUrl&>::of(&KileDocument::Manager::addToProjectView),
            projectView, QOverload<const QUrl&>::of(&KileWidget::ProjectView::add));

    connect(docManager(), QOverload<const KileProject*>::of(&KileDocument::Manager::addToProjectView),
            projectView, QOverload<const KileProject*>::of(&KileWidget::ProjectView::add));

    connect(docManager(), &KileDocument::Manager::removeItemFromProjectView,
            projectView, &KileWidget::ProjectView::removeItem);

    connect(docManager(), QOverload<KileProjectItem*>::of(&KileDocument::Manager::addToProjectView),
            projectView, [projectView](KileProjectItem *item) { projectView->add(item); });
}

void Kile::setupStructureView()
{
    m_kwStructure = new KileWidget::StructureWidget(this, m_sideBar);
    m_sideBar->addPage(m_kwStructure, QIcon::fromTheme("view-list-tree"), i18n("Structure"));
    m_kwStructure->setFocusPolicy(Qt::ClickFocus);
    connect(configurationManager(), &KileConfiguration::Manager::configChanged,
            m_kwStructure, &KileWidget::StructureWidget::configChanged);

    connect(m_kwStructure, &KileWidget::StructureWidget::setCursor, this, &Kile::setCursor);

    connect(m_kwStructure, &KileWidget::StructureWidget::fileOpen,
            docManager(), [this](const QUrl &url, const QString &encoding) { docManager()->fileOpen(url, encoding); });

    connect(m_kwStructure, &KileWidget::StructureWidget::fileNew,
            docManager(), [this](const QUrl &url) { docManager()->fileNew(url); });

    connect(m_kwStructure, &KileWidget::StructureWidget::sendText,
            this, [this](const QString &text) { insertText(text); });

    connect(m_kwStructure, &KileWidget::StructureWidget::sectioningPopup,
            m_edit, &KileDocument::EditorExtension::sectioningCommand);
}

void Kile::setupScriptsManagementView()
{
    m_scriptsManagementWidget = new KileWidget::ScriptsManagement(this, m_sideBar);
    m_sideBar->addPage(m_scriptsManagementWidget, QIcon::fromTheme("preferences-plugin-script"), i18n("Scripts"));
}

void Kile::enableSymbolViewMFUS()
{
    m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),true);

    connect(m_symbolViewRelation, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewOperators, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewArrows, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewMiscMath, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewMiscText, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewDelimiters, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewGreek, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewSpecial, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewCyrillic, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    connect(m_symbolViewUser, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
}

void Kile::disableSymbolViewMFUS()
{
    m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),false);
    m_toolBox->setItemToolTip(m_toolBox->indexOf(m_symbolViewMFUS),QString());

    disconnect(m_symbolViewRelation, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewOperators, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewArrows, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewMiscMath, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewMiscText, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewDelimiters, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewGreek, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewSpecial, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewCyrillic, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
    disconnect(m_symbolViewUser, &KileWidget::SymbolView::addToList, m_symbolViewMFUS, &KileWidget::SymbolView::slotAddToList);
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
    connect(m_symbolViewGreek, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

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
    m_commandViewToolBox = new KileWidget::CommandViewToolBox(this, m_sideBar);
    m_sideBar->addPage(m_commandViewToolBox, QIcon::fromTheme("texlion"), i18n("LaTeX"));

    connect(m_commandViewToolBox, &KileWidget::CommandViewToolBox::sendText,
            this, QOverload<const QString&>::of(&Kile::insertText));
}

void Kile::setupAbbreviationView()
{
    m_kileAbbrevView = new KileWidget::AbbreviationView(abbreviationManager(), m_sideBar);
    connect(abbreviationManager(), &KileAbbreviation::Manager::abbreviationsChanged,
            m_kileAbbrevView, &KileWidget::AbbreviationView::updateAbbreviations);
    m_sideBar->addPage(m_kileAbbrevView, QIcon::fromTheme("complete3"), i18n("Abbreviation"));

    connect(m_kileAbbrevView, &KileWidget::AbbreviationView::sendText,
            this, QOverload<const QString&>::of(&Kile::insertText));
}

void Kile::setupBottomBar()
{
    m_bottomBar = new KileWidget::BottomBar(this);
    m_bottomBar->setFocusPolicy(Qt::ClickFocus);

    connect(errorHandler(), &KileErrorHandler::showingErrorMessage, this, &Kile::focusLog);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
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

template<class ContextType, class Func>
QAction* Kile::createAction(const QString &text, const QString &actionName, const QString& iconName, const QKeySequence& shortcut,
                            const ContextType* context, Func function)
{
    QAction *action = new QAction(this);
    action->setText(text);

    connect(action, &QAction::triggered, context, function);
    actionCollection()->addAction(actionName, action);

    if(!shortcut.isEmpty()) {
        actionCollection()->setDefaultShortcut(action, shortcut);
    }
    if(!iconName.isEmpty()) {
        action->setIcon(QIcon::fromTheme(iconName));
    }

    return action;
}

template<class ContextType, class Func>
QAction* Kile::createAction(const QString &text, const QString &actionName, const QString& iconName, const QList<QKeySequence>& shortcut, const ContextType* context, Func function)
{
    QAction *action = new QAction(this);
    action->setText(text);

    connect(action, &QAction::triggered, context, function);
    actionCollection()->addAction(actionName, action);

    if(!shortcut.isEmpty()) {
        actionCollection()->setDefaultShortcuts(action, shortcut);
    }
    if(!iconName.isEmpty()) {
        action->setIcon(QIcon::fromTheme(iconName));
    }

    return action;
}

template<class ContextType, class Func>
QAction* Kile::createAction(KStandardAction::StandardAction actionType, const QString &actionName, const ContextType* context, Func function)
{
    QAction *action = KStandardAction::create(actionType, context, function, this);
    if(!actionName.isEmpty()) {
        action->setObjectName(actionName);
    }
    actionCollection()->addAction(actionName, action);

    return action;
}

void Kile::setupActions()
{
    QAction *act;

    createAction(KStandardAction::New, "file_new", docManager(), [this]() { docManager()->fileNew(); });
    createAction(KStandardAction::Open, "file_open", docManager(), [this]() { docManager()->fileOpen(); });

    m_actRecentFiles = KStandardAction::openRecent(docManager(), [this](const QUrl& url) { docManager()->fileOpen(url); }, this);
    m_actRecentFiles->setObjectName("file_open_recent");
    actionCollection()->addAction("file_open_recent", m_actRecentFiles);
    connect(docManager(), &KileDocument::Manager::addToRecentFiles, this, &Kile::addRecentFile);
    m_actRecentFiles->loadEntries(m_config->group("Recent Files"));

    createAction(i18n("Save All"), "file_save_all", "document-save-all", docManager(), &KileDocument::Manager::fileSaveAll);

    createAction(i18n("Create Template From Document..."), "template_create", docManager(), &KileDocument::Manager::createTemplate);
    createAction(i18n("&Remove Template..."), "template_remove", docManager(), &KileDocument::Manager::removeTemplate);
    createAction(KStandardAction::Close, "file_close", docManager(), [this]() { docManager()->fileClose();} );
    createAction(i18n("Close All"), "file_close_all", docManager(), &KileDocument::Manager::fileCloseAll);
    createAction(i18n("Close All Ot&hers"), "file_close_all_others", docManager(), [this]() { docManager()->fileCloseAllOthers(); });
    createAction(i18n("S&tatistics"), "Statistics", this, [this]() { showDocInfo(); });
    createAction(i18n("&ASCII"), "file_export_ascii", this, [this]() { convertToASCII(); });
    createAction(i18n("Latin-&1 (iso 8859-1)"), "file_export_latin1", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&2 (iso 8859-2)"), "file_export_latin2", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&3 (iso 8859-3)"), "file_export_latin3", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&4 (iso 8859-4)"), "file_export_latin4", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&5 (iso 8859-5)"), "file_export_latin5", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&9 (iso 8859-9)"), "file_export_latin9", this, [this]() { convertToEnc(); });
    createAction(i18n("&Central European (cp-1250)"), "file_export_cp1250", this, [this]() { convertToEnc(); });
    createAction(i18n("&Western European (cp-1252)"), "file_export_cp1252", this, [this]() { convertToEnc(); });
    createAction(KStandardAction::Quit, "file_quit", this, &Kile::close);

    createAction(i18n("Move Tab Left"), "move_view_tab_left", "arrow-left", viewManager(), [this]() { viewManager()->moveTabLeft(); });
    createAction(i18n("Move Tab Right"), "move_view_tab_right", "arrow-right", viewManager(), [this]() { viewManager()->moveTabRight(); });

    createAction(i18n("Next section"), "edit_next_section", "nextsection", QKeySequence(Qt::ALT + Qt::Key_Down),
                 m_edit, &KileDocument::EditorExtension::gotoNextSectioning);
    createAction(i18n("Prev section"), "edit_prev_section", "prevsection", QKeySequence(Qt::ALT + Qt::Key_Up),
                 m_edit, &KileDocument::EditorExtension::gotoPrevSectioning);
    createAction(i18n("Next paragraph"), "edit_next_paragraph", "nextparagraph", QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Down),
                 m_edit, [this]() { m_edit->gotoNextParagraph(); });
    createAction(i18n("Prev paragraph"), "edit_prev_paragraph", "prevparagraph", QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Up),
                 m_edit, [this]() { m_edit->gotoPrevParagraph(); });

    createAction(i18n("Find &in Files..."), "FindInFiles", "filegrep", this, &Kile::findInFiles);

    createAction(i18n("Refresh Str&ucture"), "RefreshStructure", "refreshstructure", QKeySequence(Qt::Key_F12), this, &Kile::refreshStructure);

    //project actions
    createAction(i18n("&New Project..."), "project_new", "window-new", docManager(), &KileDocument::Manager::projectNew);
    createAction(i18n("&Open Project..."), "project_open", "project-open", docManager(), [this]() { docManager()->projectOpen(); });

    m_actRecentProjects = new KRecentFilesAction(i18n("Open &Recent Project"), actionCollection());
    actionCollection()->addAction("project_openrecent", m_actRecentProjects);
    connect(m_actRecentProjects, &KRecentFilesAction::urlSelected, docManager(), [this](const QUrl& url) { docManager()->projectOpen(url); });
    connect(docManager(), &KileDocument::Manager::removeFromRecentProjects, this, &Kile::removeRecentProject);
    connect(docManager(), &KileDocument::Manager::addToRecentProjects, this, &Kile::addRecentProject);
    m_actRecentProjects->loadEntries(m_config->group("Projects"));

    createAction(i18n("A&dd Files to Project..."), "project_add", "project_add", docManager(), [this]() { m_docManager->projectAddFiles(); });
    createAction(i18n("Refresh Project &Tree"), "project_buildtree", "project_rebuild", docManager(), [this]() { m_docManager->buildProjectTree(); });
    createAction(i18n("&Archive"), "project_archive", "project_archive", this, [this]() { runArchiveTool(); });
    createAction(i18n("Project &Options"), "project_options", "configure_project", docManager(), [this]() { m_docManager->projectOptions(); });
    createAction(i18n("&Close Project"), "project_close", "project-development-close", docManager(), [this]() { m_docManager->projectClose(); });

    // new project actions (dani)
    createAction(i18n("&Show Projects..."), "project_show", docManager(), &KileDocument::Manager::projectShow);
    createAction(i18n("Re&move Files From Project..."), "project_remove", "project_remove", docManager(), &KileDocument::Manager::projectRemoveFiles);
    createAction(i18n("Show Project &Files..."), "project_showfiles", "project_show", docManager(), &KileDocument::Manager::projectShowFiles);
    // tbraun
    createAction(i18n("Open All &Project Files"), "project_openallfiles", docManager(), [this]() { docManager()->projectOpenAllFiles(); });
    createAction(i18n("Find in &Project..."), "project_findfiles", "projectgrep", this, &Kile::findInProjects);

    //build actions
    act = createAction(i18n("Clean"), "CleanAll", "user-trash", this, [this]() { cleanAll(); });

    QList<QKeySequence> nextTabShorcuts;
    nextTabShorcuts.append(QKeySequence(Qt::ALT + Qt::Key_Right));
    nextTabShorcuts.append(KStandardShortcut::tabNext());
    createAction(i18n("Next Document"), "gotoNextDocument", "go-next-view-page",
                 nextTabShorcuts, viewManager(), &KileView::Manager::gotoNextView);

    QList<QKeySequence> prevTabShorcuts;
    prevTabShorcuts.append(QKeySequence(Qt::ALT + Qt::Key_Left));
    prevTabShorcuts.append(KStandardShortcut::tabPrev());
    createAction(i18n("Previous Document"), "gotoPrevDocument", "go-previous-view-page",
                 prevTabShorcuts, viewManager(), &KileView::Manager::gotoPrevView);

    createAction(i18n("Focus Log/Messages View"), "focus_log", QKeySequence("CTRL+Alt+M"), this, &Kile::focusLog);
    createAction(i18n("Focus Output View"), "focus_output", QKeySequence("CTRL+Alt+O"), this, &Kile::focusOutput);
    createAction(i18n("Focus Konsole View"), "focus_konsole", QKeySequence("CTRL+Alt+K"), this, &Kile::focusKonsole);
    createAction(i18n("Focus Editor View"), "focus_editor", QKeySequence("CTRL+Alt+F"), this, &Kile::focusEditor);

    createAction(i18nc("@action: Starts the completion of the current LaTeX command", "Complete (La)TeX Command"), "edit_complete_word", "complete1",
                 QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_Space), codeCompletionManager(), [this]() { codeCompletionManager()->startLaTeXCompletion(); });
    createAction(i18nc("@action: Starts the input (and completion) of a LaTeX environment", "Complete LaTeX Environment"), "edit_complete_env", "complete2",
                 QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Space), codeCompletionManager(), [this]() { codeCompletionManager()->startLaTeXEnvironment(); });
    createAction(i18nc("@action: Starts the completion of the current abbreviation", "Complete Abbreviation"), "edit_complete_abbrev", "complete3",
                 QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Space), codeCompletionManager(), [this]() { codeCompletionManager()->startAbbreviationCompletion(); });

    createAction(i18n("Next Bullet"), "edit_next_bullet", "nextbullet", QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Right),
                 m_edit, [this]() { m_edit->nextBullet(); });
    createAction(i18n("Prev Bullet"), "edit_prev_bullet", "prevbullet", QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Left),
                 m_edit, [this]() { m_edit->prevBullet(); });

// advanced editor (dani)
    createAction(i18n("Environment (inside)"), "edit_select_inside_env", "selenv_i", QKeySequence("CTRL+Alt+S, E"),
                 m_edit, &KileDocument::EditorExtension::selectEnvInside);
    createAction(i18n("Environment (outside)"), "edit_select_outside_env", "selenv_o", QKeySequence("CTRL+Alt+S, F"),
                 m_edit, &KileDocument::EditorExtension::selectEnvOutside);
    createAction(i18n("TeX Group (inside)"), "edit_select_inside_group", "selgroup_i", QKeySequence("CTRL+Alt+S, T"),
                 m_edit, &KileDocument::EditorExtension::selectTexgroupInside);
    createAction(i18n("TeX Group (outside)"), "edit_select_outside_group", "selgroup_o", QKeySequence("CTRL+Alt+S, U"),
                 m_edit, &KileDocument::EditorExtension::selectTexgroupOutside);
    createAction(i18n("Math Group"), "edit_select_mathgroup", "selmath", QKeySequence("CTRL+Alt+S, M"),
                 m_edit, [this]() { m_edit->selectMathgroup(); });
    createAction(i18n("Paragraph"), "edit_select_paragraph", "selpar", QKeySequence("CTRL+Alt+S, P"),
                 m_edit, [this]() { m_edit->selectParagraph(); });
    createAction(i18n("Line"), "edit_select_line", "selline", QKeySequence("CTRL+Alt+S, L"),
                 m_edit, [this]() { m_edit->selectLine(); });
    createAction(i18n("TeX Word"), "edit_select_word", "selword", QKeySequence("CTRL+Alt+S, W"),
                 m_edit, [this]() { m_edit->selectWord(); });

    createAction(i18n("Environment (inside)"), "edit_delete_inside_env", "delenv_i", QKeySequence("CTRL+Alt+T, E"),
                 m_edit, &KileDocument::EditorExtension::deleteEnvInside);
    createAction(i18n("Environment (outside)"), "edit_delete_outside_env", "delenv_o", QKeySequence("CTRL+Alt+T, F"),
                 m_edit, &KileDocument::EditorExtension::deleteEnvOutside);
    createAction(i18n("TeX Group (inside)"), "edit_delete_inside_group", "delgroup_i", QKeySequence("CTRL+Alt+T, T"),
                 m_edit, &KileDocument::EditorExtension::deleteTexgroupInside);
    createAction(i18n("TeX Group (outside)"), "edit_delete_outside_group", "delgroup_o",QKeySequence("CTRL+Alt+T, U"),
                 m_edit, &KileDocument::EditorExtension::deleteTexgroupInside);
    createAction(i18n("Math Group"), "edit_delete_mathgroup", "delmath", QKeySequence("CTRL+Alt+T, M"),
                 m_edit, [this]() { m_edit->deleteMathgroup(); });
    createAction(i18n("Paragraph"), "edit_delete_paragraph", "delpar", QKeySequence("CTRL+Alt+T, P"),
                 m_edit, [this]() { m_edit->deleteParagraph(); });
    createAction(i18n("To End of Line"), "edit_delete_eol", "deleol", QKeySequence("CTRL+Alt+T, L"),
                 m_edit, [this]() { m_edit->deleteEndOfLine(); });
    createAction(i18n("TeX Word"), "edit_delete_word", "delword", QKeySequence("CTRL+Alt+T, W"),
                 m_edit, [this]() { m_edit->deleteWord(); });

    createAction(i18n("Go to Begin"), "edit_begin_env", "gotobeginenv", QKeySequence("CTRL+Alt+E, B"),
                 m_edit, &KileDocument::EditorExtension::gotoBeginEnv);
    createAction(i18n("Go to End"), "edit_end_env", "gotoendenv", QKeySequence("CTRL+Alt+E, E"),
                 m_edit, &KileDocument::EditorExtension::gotoEndEnv);
    createAction(i18n("Match"), "edit_match_env", "matchenv", QKeySequence("CTRL+Alt+E, M"),
                 m_edit, &KileDocument::EditorExtension::matchEnv);
    createAction(i18n("Close"), "edit_close_env", "closeenv", QKeySequence("CTRL+Alt+E, C"),
                 m_edit, &KileDocument::EditorExtension::closeEnv);
    createAction(i18n("Close All"), "edit_closeall_env", "closeallenv", QKeySequence("CTRL+Alt+E, A"),
                 m_edit, &KileDocument::EditorExtension::closeAllEnv);

    createAction(i18n("Go to Begin"), "edit_begin_group", "gotobegingroup", QKeySequence("CTRL+Alt+G, B"),
                 m_edit, &KileDocument::EditorExtension::gotoBeginTexgroup);
    createAction(i18n("Go to End"), "edit_end_group", "gotoendgroup", QKeySequence("CTRL+Alt+G, E"),
                 m_edit, &KileDocument::EditorExtension::gotoEndTexgroup);
    createAction(i18n("Match"), "edit_match_group", "matchgroup", QKeySequence("CTRL+Alt+G, M"),
                 m_edit, [this]() { m_edit->matchTexgroup(); });
    createAction(i18n("Close"), "edit_close_group", "closegroup", QKeySequence("CTRL+Alt+G, C"),
                 m_edit, [this]() { m_edit->closeTexgroup(); });

    createAction(i18n("Selection"), "quickpreview_selection", "preview_sel", QKeySequence("CTRL+Alt+P, S"), this, &Kile::quickPreviewSelection);
    createAction(i18n("Environment"), "quickpreview_environment", "preview_env",QKeySequence("CTRL+Alt+P, E"), this, &Kile::quickPreviewEnvironment);
    createAction(i18n("Subdocument"), "quickpreview_subdocument", "preview_subdoc",QKeySequence("CTRL+Alt+P, D"), this, &Kile::quickPreviewSubdocument);
    createAction(i18n("Mathgroup"), "quickpreview_math", "preview_math", QKeySequence("CTRL+Alt+P, M"), this, &Kile::quickPreviewMathgroup);

    KileStdActions::setupStdTags(this, this, actionCollection(), this);
    KileStdActions::setupMathTags(this, actionCollection());

    m_bibTagActionMenu = new KActionMenu(i18n("&Bibliography"), actionCollection());
    m_bibTagActionMenu->setPopupMode(QToolButton::InstantPopup);
    actionCollection()->addAction("menu_bibliography", m_bibTagActionMenu);

    createAction(i18n("Clean"), "CleanBib", this, &Kile::cleanBib);

    m_bibTagSettings = new KSelectAction(i18n("&Settings"),actionCollection());
    actionCollection()->addAction("settings_menu_bibliography", m_bibTagSettings);

    act = createAction(i18n("Settings for BibTeX"), "setting_bibtex", this, &Kile::rebuildBibliographyMenu);
    act->setCheckable(true);
    m_bibTagSettings->addAction(act);

    act = createAction(i18n("Settings for Biblatex"), "setting_biblatex", this, &Kile::rebuildBibliographyMenu);
    act->setCheckable(true);
    m_bibTagSettings->addAction(act);
    m_bibTagSettings->setCurrentAction(action((QString("setting_") + KileConfig::bibliographyType()).toLatin1()));

    rebuildBibliographyMenu();

    createAction(i18n("Quick Start"), "wizard_document", "quickwizard", this, &Kile::quickDocument);
    connect(docManager(), &KileDocument::Manager::startWizard, this, &Kile::quickDocument);
    createAction(i18n("Tabular"), "wizard_tabular", "wizard_tabular", this, &Kile::quickTabular);
    createAction(i18n("Array"), "wizard_array", "wizard_array", this, &Kile::quickArray);
    createAction(i18n("Tabbing"), "wizard_tabbing", "wizard_tabbing", this, &Kile::quickTabbing);
    createAction(i18n("Floats"), "wizard_float", "wizard_float", this, &Kile::quickFloat);
    createAction(i18n("Math"), "wizard_mathenv", "wizard_math", this, &Kile::quickMathenv);
    createAction(i18n("Postscript Tools"), "wizard_postscript", "wizard_pstools", this, &Kile::quickPostscript);
    createAction(i18n("PDF Tools"), "wizard_pdf", "wizard_pdftools", this, &Kile::quickPdf);

    ModeAction = new KToggleAction(i18n("Define Current Document as '&Master Document'"), actionCollection());
    actionCollection()->addAction("Mode", ModeAction);
    ModeAction->setIcon(QIcon::fromTheme("master"));
    connect(ModeAction, &KToggleAction::triggered, this, &Kile::toggleMasterDocumentMode);

    KToggleAction *showDocumentViewer = new KToggleAction(i18n("Show Document Viewer"), actionCollection());
    actionCollection()->addAction("ShowDocumentViewer", showDocumentViewer);
    showDocumentViewer->setChecked(KileConfig::showDocumentViewer());
    connect(showDocumentViewer, &KToggleAction::toggled, viewManager(), &KileView::Manager::setDocumentViewerVisible);
    connect(viewManager(), &KileView::Manager::documentViewerWindowVisibilityChanged,
            showDocumentViewer, &KToggleAction::setChecked);

    KToggleAction *tact = new KToggleAction(i18n("Show S&ide Bar"), actionCollection());
    actionCollection()->addAction("StructureView", tact);
    tact->setChecked(KileConfig::sideBar());
    connect(tact, &KToggleAction::toggled, m_sideBar, &Kile::setVisible);
    connect(m_sideBar, &KileWidget::SideBar::visibilityChanged, this, &Kile::sideOrBottomBarChanged);

    m_actionMessageView = new KToggleAction(i18n("Show Mess&ages Bar"), actionCollection());
    actionCollection()->addAction("MessageView", m_actionMessageView);
    m_actionMessageView->setChecked(true);
    connect(m_actionMessageView, &KToggleAction::toggled, m_bottomBar, &Kile::setVisible);
    connect(m_bottomBar, &KileWidget::SideBar::visibilityChanged, this, &Kile::sideOrBottomBarChanged);
    if(m_singlemode) {
        ModeAction->setChecked(false);
    }
    else {
        ModeAction->setChecked(true);
    }

    WatchFileAction = new KToggleAction(i18n("Watch File Mode"), actionCollection());
    actionCollection()->addAction("WatchFile", WatchFileAction);
    WatchFileAction->setIcon(QIcon::fromTheme("watchfile"));
    connect(WatchFileAction, &KToggleAction::toggled, this, &Kile::toggleWatchFile);
    if(m_bWatchFile) {
        WatchFileAction->setChecked(true);
    }
    else {
        WatchFileAction->setChecked(false);
    }

    createAction(i18n("LaTeX"), "help_latex_index", QKeySequence("CTRL+Alt+H, L"), m_help, &KileHelp::Help::helpLatexIndex);
    createAction(i18n("LaTeX Commands"), "help_latex_command", QKeySequence("CTRL+Alt+H, C"), m_help, &KileHelp::Help::helpLatexCommand);
    createAction(i18n("LaTeX Environments"), "help_latex_env", QKeySequence("CTRL+Alt+H, E"), m_help, &KileHelp::Help::helpLatexEnvironment);
    createAction(i18n("Context Help"), "help_context", QKeySequence("CTRL+Alt+H, K"), m_help, [this]() { m_help->helpKeyword(); });
    createAction(i18n("Documentation Browser"), "help_docbrowser", QKeySequence("CTRL+Alt+H, B"), m_help, &KileHelp::Help::helpDocBrowser);

    createAction(i18n("&About Editor Component"), "help_about_editor", this, &Kile::aboutEditorComponent);

    QAction *kileconfig = KStandardAction::preferences(this, &Kile::generalOptions, actionCollection());
    kileconfig->setIcon(QIcon::fromTheme("configure-kile"));

    createAction(KStandardAction::KeyBindings, this, &Kile::configureKeys);
    createAction(KStandardAction::ConfigureToolbars, this, &Kile::configureToolbars);

    createAction(i18n("&System Check..."), "settings_perform_check", this, &Kile::slotPerformCheck);

    m_userHelpActionMenu = new KActionMenu(i18n("User Help"), actionCollection());
    actionCollection()->addAction("help_userhelp", m_userHelpActionMenu);

    m_pFullScreen = KStandardAction::fullScreen(this, &Kile::slotToggleFullScreen, this, actionCollection());
    m_actionShowMenuBar = KStandardAction::showMenubar(this,
                                                       [this]() { toggleShowMenuBar(true); },
                                                       actionCollection());
}

void Kile::rebuildBibliographyMenu() {

    KILE_DEBUG_MAIN << " current is " << m_bibTagSettings->currentText();

    QString currentItem = m_bibTagSettings->currentText();
    QString name;

    if( currentItem == i18n("BibTeX") ) { // avoid writing i18n'ed strings to config file
        name = QString("bibtex");
    }
    else if ( currentItem == i18n("Biblatex") ) {
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

QAction* Kile::createToolAction(const QString& toolName)
{
    return createAction(toolName, "tool_" + toolName,
                        KileTool::iconFor(toolName, m_config.data()), this, [this, toolName]() { runTool(toolName); });
}

void Kile::createToolActions()
{
    QStringList tools = KileTool::toolList(m_config.data());
    for (QStringList::iterator i = tools.begin(); i != tools.end(); ++i) {
        QString toolName = *i;
        if(!actionCollection()->action("tool_" + toolName)) {
            KILE_DEBUG_MAIN << "Creating action for tool" << toolName;
            createToolAction(toolName);
        }
    }
}

void Kile::setupTools()
{
    KILE_DEBUG_MAIN << "==Kile::setupTools()===================" << Qt::endl;

    if(!m_buildMenuCompile || !m_buildMenuConvert ||  !m_buildMenuTopLevel || !m_buildMenuQuickPreview || !m_buildMenuViewer || !m_buildMenuOther) {
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
    QList<QAction*> *pl;
    QAction *act;
    ToolbarSelectAction *pSelectAction = Q_NULLPTR;

    m_compilerActions->saveCurrentAction();
    m_viewActions->saveCurrentAction();
    m_convertActions->saveCurrentAction();
    m_quickActions->saveCurrentAction();

    // do plugActionList by hand ...
    foreach(act, m_listQuickActions) {
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
        QString grp = KileTool::groupFor(tools[i], m_config.data());
        QString toolMenu = KileTool::menuFor(tools[i], m_config.data());

        KILE_DEBUG_MAIN << tools[i] << " is using group: " << grp << " and menu: "<< toolMenu;
        if(toolMenu == "none") {
            continue;
        }

        if ( toolMenu == "Compile" ) {
            pl = &m_listCompilerActions;
            pSelectAction = m_compilerActions;
        }
        else if ( toolMenu == "View" ) {
            pl = &m_listViewerActions;
            pSelectAction = m_viewActions;
        }
        else if ( toolMenu == "Convert" ) {
            pl = &m_listConverterActions;
            pSelectAction = m_convertActions;
        }
        else if ( toolMenu == "Quick" ) {
            pl = &m_listQuickActions;
            pSelectAction = m_quickActions;
        }
        else {
            pl = &m_listOtherActions;
            pSelectAction = Q_NULLPTR;
        }

        KILE_DEBUG_MAIN << "\tadding " << tools[i] << " " << toolMenu << " #" << pl->count() << Qt::endl;

        act = actionCollection()->action("tool_" + tools[i]);
        if(!act) {
            KILE_DEBUG_MAIN << "no tool for " << tools[i];
            createToolAction(tools[i]);
        }
        pl->append(act);

        if(pSelectAction) {
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

void Kile::initSelectActions() {

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

void Kile::saveLastSelectedAction() {

    KILE_DEBUG_MAIN << "Kile::saveLastSelectedAction()" << Qt::endl;
    QStringList list;
    list << "Compile" << "Convert" << "View" << "Quick";

    ToolbarSelectAction *pSelectAction = Q_NULLPTR ;

    KConfigGroup grp = m_config->group("ToolSelectAction");

    for(QStringList::Iterator it = list.begin(); it != list.end() ; ++it) {
        if ( *it == "Compile" ) {
            pSelectAction = m_compilerActions;
        }
        else if ( *it == "View" ) {
            pSelectAction = m_viewActions;
        }
        else if ( *it == "Convert" ) {
            pSelectAction = m_convertActions;
        }
        else if ( *it == "Quick" ) {
            pSelectAction = m_quickActions;
        }

        KILE_DEBUG_MAIN << "current item is " << pSelectAction->currentItem();

        grp.writeEntry(*it, pSelectAction->currentItem());
    }
}

void Kile::restoreLastSelectedAction() {

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
    for ( it= list.begin(); it != list.end(); ++it) {
        QAction *act = *it;
        if ( act != Q_NULLPTR && !act->objectName().isEmpty() && !tools.contains(act->objectName().mid(5)) ) {
            if (act->associatedWidgets().contains(toolBar("toolsToolBar"))) {
                toolBar("toolsToolBar")->removeAction(act);
            }
//             KILE_DEBUG_MAIN << "about to delete action: " << act->objectName();
            testIt = list.erase(it);
            if( testIt == list.end()) {
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

    KILE_DEBUG_MAIN << "lastDocument=" << KileConfig::lastDocument() << Qt::endl;
    KTextEditor::Document *doc = docManager()->docFor(QUrl::fromUserInput(KileConfig::lastDocument()));
    if (doc) {
        viewManager()->switchToTextView(doc->url(), true); // request the focus on the view
    }
    setMasterDocumentFileName(KileConfig::singleFileMasterDocument());
}

void Kile::setActive()
{
    KILE_DEBUG_MAIN << "Activating" << Qt::endl;
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
    //KILE_DEBUG_MAIN << "==Kile::activateView==========================" << Qt::endl;
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
    KILE_DEBUG_MAIN << "==bool Kile::queryClose==========" << Qt::endl;

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

    KILE_DEBUG_MAIN << "#projects = " << docManager()->projects().count() << Qt::endl;
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
                    << abs_filename << " (" << line << ")" << Qt::endl;
    docManager()->fileOpen(QUrl::fromUserInput(abs_filename));
    setLine(QString::number(line));
}

void Kile::findInFiles()
{
    static QPointer<KileDialog::FindFilesDialog> dlg = 0;

    if (!dlg) {
        KILE_DEBUG_MAIN << "grep guard: create findInFiles dlg" << Qt::endl;
        dlg = new KileDialog::FindFilesDialog(mainWindow(), this, KileGrep::Directory);
        dlg->show();
        connect(dlg, &KileDialog::FindFilesDialog::itemSelected, this, &Kile::grepItemSelected);
    }
    else {
        KILE_DEBUG_MAIN << "grep guard: show findInFiles dlg" << Qt::endl;
        dlg->activateWindow();
        dlg->raise();
    }
}

void Kile::findInProjects()
{
    static QPointer<KileDialog::FindFilesDialog> project_dlg = Q_NULLPTR;

    if(!project_dlg) {
        KILE_DEBUG_MAIN << "grep guard: create findInProjects dlg" << Qt::endl;
        project_dlg = new KileDialog::FindFilesDialog(mainWindow(), this, KileGrep::Project);
        project_dlg->show();
        connect(project_dlg, &KileDialog::FindFilesDialog::itemSelected, this, &Kile::grepItemSelected);
    }
    else {
        KILE_DEBUG_MAIN << "grep guard: show findInProjects dlg" << Qt::endl;
        project_dlg->activateWindow();
        project_dlg->raise();
    }
}

/////////////////// PART & EDITOR WIDGET //////////
bool Kile::resetPart()
{
    KILE_DEBUG_MAIN << "==Kile::resetPart()=============================" << Qt::endl;

    statusBar()->reset();
    updateModeStatus();
    newCaption();

    KTextEditor::View *view = viewManager()->currentTextView();
    if (view) {
        activateView(view);
    }

    return true;
}

void Kile::updateUserDefinedMenus()
{
    m_buildMenuTopLevel = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_build", m_mainWindow));
    m_buildMenuCompile  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_compile", m_mainWindow));
    m_buildMenuConvert  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_convert", m_mainWindow));
    m_buildMenuViewer  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_viewer", m_mainWindow));
    m_buildMenuOther   = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("menu_other", m_mainWindow));
    m_buildMenuQuickPreview   = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container("quickpreview", m_mainWindow));

    m_userMenu->updateGUI();

    setupTools();
}

void Kile::enableGUI(bool enable)
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
            << "tag_ref" << "tag_pageref" << "tag_index" << "tag_footnote" << "tag_cite"
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
    KILE_DEBUG_MAIN << "==Kile::updateMenu()====================" << Qt::endl;
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
    KILE_DEBUG_MAIN << "\tprojectopen=" << project_open << " fileopen=" << file_open << Qt::endl;

    enableGUI(file_open);
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

void Kile::insertTag(const KileAction::TagData& data,const QList<Package> &pkgs) {

    QStringList packages;

    QList<Package>::const_iterator it;
    for(it = pkgs.begin(); it != pkgs.end() ; it++) {
        QString pkgName = (*it).name;
        if(!pkgName.isEmpty()) {
            packages.append(pkgName);
        }
    }

    insertTag(data,packages);
}

void Kile::insertTag(const KileAction::TagData& data,const QStringList &pkgs)
{
    KILE_DEBUG_MAIN << "void Kile::insertTag(const KileAction::TagData& data,const QStringList " << pkgs.join(",") << ")" << Qt::endl;
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
    connect(dlg, &QDialog::finished, dlg, &QObject::deleteLater);

    dlg->open();
}

void Kile::quickUserMenuDialog()
{
    m_userMenu->removeShortcuts();
    QPointer<KileMenu::UserMenuDialog> dlg = new KileMenu::UserMenuDialog(m_config.data(), this, m_userMenu, m_userMenu->xmlFile(), m_mainWindow);

    dlg->exec();

    connect(dlg, &QDialog::finished, this, [this] (int result) {
        Q_UNUSED(result);

        // tell all the documents and views to update their action shortcuts (bug 247646)
        docManager()->reloadXMLOnAllDocumentsAndViews();

        // a new usermenu could have been installed, even if the return value is QDialog::Rejected
        m_userMenu->refreshActionProperties();
    });

    delete dlg;
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

void Kile::readGUISettings()
{
}

// transform old user tags to xml file
void Kile::transformOldUserTags()
{
    KILE_DEBUG_MAIN << "Convert old user tags";
    QString xmldir = KileUtilities::writableLocation(QStandardPaths::AppDataLocation) + "/usermenu/";
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
        KileConfig::setUserMenuFile(usertagfile);
    }
    userGroup.deleteEntry("nUserTags");
}

void Kile::transformOldUserSettings()
{
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
            QString bin = KIO::DesktopExecParser::executablePath(tempItem.tag);
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

    if(m_livePreviewManager) {
        m_livePreviewManager->readConfig(m_config.data());
    }

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
    m_fileBrowserWidget->writeConfig();

    if(m_livePreviewManager) {
        m_livePreviewManager->writeConfig();
    }

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
    KILE_DEBUG_MAIN << "SETTING master to " << m_masterDocumentFileName << " singlemode = " << m_singlemode << Qt::endl;
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
    KileUtilities::scheduleCenteringOfWidget(dlg);

    if (dlg->exec()) {
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
    const bool livePreviewEnabledForFreshlyOpenedDocuments = KileConfig::previewEnabledForFreshlyOpenedDocuments();
    const bool livePreviewEnabledForCurrentDocument = livePreviewManager() && livePreviewManager()->isLivePreviewEnabledForCurrentDocument();
    if (livePreviewManager()) {
        KileConfig::setPreviewEnabledForFreshlyOpenedDocuments(false);
        livePreviewManager()->setLivePreviewEnabledForCurrentDocument(false);
    }

    // we show the message output widget in the bottom bar and shrink the side bar
    int sideBarTab = m_sideBar->currentTab();
    int bottomBarTab = m_bottomBar->currentTab();

    m_sideBar->shrink();
    m_bottomBar->switchToTab(0); // show the log widget

    int outputTab = m_errorHandler->currentOutputTabIndex();
    m_errorHandler->showMessagesOutput();

    QString currentMasterDocument = m_masterDocumentFileName;
    if(!m_singlemode) {
        clearMasterDocument();
    }
    // we hide the editor pane and tabs
    m_viewManager->setTabsAndEditorVisible(false);

    // now, we can run the tests
    KileDialog::ConfigChecker *dlg = new KileDialog::ConfigChecker(this);
    dlg->exec();
    delete dlg;

    m_errorHandler->clearMessages();
    m_errorHandler->clearErrorOutput();

    // finally, we restore the rest to what it was before launching the tests
    m_viewManager->setTabsAndEditorVisible(true);
    if(!currentMasterDocument.isEmpty()) {
        setMasterDocumentFileName(currentMasterDocument);
    }

    m_errorHandler->setCurrentOutputTab(outputTab);

    if(sideBarTab >= 0) {
        m_sideBar->switchToTab(sideBarTab);
    }
    if(bottomBarTab < 0) {
        m_bottomBar->shrink();
    }
    else {
        m_bottomBar->switchToTab(bottomBarTab);
    }

    if (livePreviewManager()) {
        KileConfig::setPreviewEnabledForFreshlyOpenedDocuments(livePreviewEnabledForFreshlyOpenedDocuments);
        if(livePreviewEnabledForCurrentDocument) {
            livePreviewManager()->setLivePreviewEnabledForCurrentDocument(true);
        }
    }
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
    connect(&dlg, &KShortcutsDialog::saved, this, [this]() {
        // tell all the documents and views to update their action shortcuts (bug 247646)
        docManager()->reloadXMLOnAllDocumentsAndViews();

        // tell m_userMenu that key bindings may have been changed
        m_userMenu->updateKeyBindings();

        // transfer the shortcuts to the scripts
        const QList<KileScript::Script*> scripts = scriptManager()->getScripts();
        for(KileScript::Script *script : scripts) {
            QAction *action = script->getActionObject();

            if(action && !action->shortcut().isEmpty()) {
                scriptManager()->setShortcut(script, action->shortcut());
            }
        }

        scriptManager()->writeConfig();
        m_scriptsManagementWidget->update();
    });
    dlg.configure();
}

void Kile::configureToolbars()
{
    {
        KConfigGroup configGroup = KSharedConfig::openConfig()->group("KileMainWindow");
        saveMainWindowSettings(configGroup);
    }

    KEditToolBar dlg(factory());
    connect(&dlg, &KEditToolBar::newToolBarConfig, this, [this] () {
        setUpdatesEnabled(false);
        applyMainWindowSettings(m_config->group("KileMainWindow"));

        updateUserDefinedMenus();
        setUpdatesEnabled(true);
    });
    dlg.exec();
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

    for (i = 0; i < view->document()->lines(); ++i) {
        int j = i+1;
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
    KILE_DEBUG_MAIN << "==Kile::slotQuickPreview()=========================="  << Qt::endl;

    KTextEditor::View *view = viewManager()->currentTextView();
    if ( ! view) return;

    KTextEditor::Document *doc = view->document();
    if ( ! doc )
        return;

    switch ( type )
    {
    case KileTool::qpSelection:
        m_quickPreview->previewSelection(view);
        break;
    case KileTool::qpEnvironment:
        m_quickPreview->previewEnvironment(doc);
        break;
    case KileTool::qpSubdocument:
        m_quickPreview->previewSubdocument(doc);
        break;
    case KileTool::qpMathgroup:
        m_quickPreview->previewMathgroup(doc);
        break;
    }
}

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

void Kile::toggleShowMenuBar(bool showMessage)
{
    if (m_actionShowMenuBar->isChecked()) {
        menuBar()->show();
        return;
    }
    
    if (showMessage) {
        const QString accel = m_actionShowMenuBar->shortcut().toString(QKeySequence::NativeText);
        KMessageBox::information(this,
                                    i18n("This will hide the menu bar completely."
                                        " You can show it again by typing %1.",
                                        accel),
                                    i18n("Hide menu bar"),
                                    QStringLiteral("HideMenuBarWarning"));
    }
    menuBar()->hide();
}
