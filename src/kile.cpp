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
#include <KIO/OpenFileManagerWindowJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KShortcutsDialog>
#include <KToggleAction>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>
#include <KSelectAction>
#include <KWindowSystem>
#include <qregularexpression.h>
#if __has_include(<kx11extras.h>)
#include <kx11extras.h>
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
      m_toolsToolBar(nullptr),       // we have to set all of these to null as the constructor
      m_userHelpActionMenu(nullptr), // might return early
      m_bibTagSettings(nullptr),
      m_compilerActions(nullptr),
      m_viewActions(nullptr),
      m_convertActions(nullptr),
      m_quickActions(nullptr),
      m_bibTagActionMenu(nullptr),
      ModeAction(nullptr),
      WatchFileAction(nullptr),
      m_actionMessageView(nullptr),
      m_actionShowMenuBar(nullptr),
      m_actRecentFiles(nullptr),
      m_pFullScreen(nullptr),
      m_sideBar(nullptr),
      m_kileAbbrevView(nullptr),
      m_topWidgetStack(nullptr),
      m_horizontalSplitter(nullptr),
      m_verticalSplitter(nullptr),
      m_toolBox(nullptr),
      m_commandViewToolBox(nullptr),
      m_symbolViewMFUS(nullptr),
      m_symbolViewRelation(nullptr),
      m_symbolViewArrows(nullptr),
      m_symbolViewMiscMath(nullptr),
      m_symbolViewMiscText(nullptr),
      m_symbolViewOperators(nullptr),
      m_symbolViewUser(nullptr),
      m_symbolViewDelimiters(nullptr),
      m_symbolViewGreek(nullptr),
      m_symbolViewSpecial(nullptr),
      m_symbolViewCyrillic(nullptr),
      m_commandView(nullptr),
      m_latexOutputErrorToolBar(nullptr),
      m_buildMenuTopLevel(nullptr),
      m_buildMenuCompile(nullptr),
      m_buildMenuConvert(nullptr),
      m_buildMenuViewer(nullptr),
      m_buildMenuOther(nullptr),
      m_buildMenuQuickPreview(nullptr),
      m_actRecentProjects(nullptr),
      m_lyxserver(nullptr)
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

    QSplashScreen splashScreen(QPixmap(KileUtilities::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/kile_splash.png"))), Qt::WindowStaysOnTopHint);
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
        if(m_config->hasGroup(QStringLiteral("Shortcuts"))) {
            KConfigGroup shortcutGroup = m_config->group(QStringLiteral("Shortcuts"));
            actionCollection()->readSettings(&shortcutGroup);
            m_config->deleteGroup(QStringLiteral("Shortcuts"));
        }

        if(m_config->hasGroup(QStringLiteral("Complete"))) {
            KConfigGroup completionGroup = m_config->group(QStringLiteral("Complete"));
            completionGroup.deleteEntry(QStringLiteral("maxCwlFiles")); // in Kile 3.0 the UI has been changed so that this setting is no longer
            // needed
        }
    }
    if(KileConfig::rCVersion() < 9) {
        // in Kile 3.0 beta 4, the user help was updated, some old config settings were no longer needed
        if(m_config->hasGroup(QStringLiteral("Help"))) {
            KConfigGroup helpGroup = m_config->group(QStringLiteral("Help"));
            helpGroup.deleteEntry(QStringLiteral("location"));
            helpGroup.deleteEntry(QStringLiteral("texrefs"));
            helpGroup.deleteEntry(QStringLiteral("external"));
            helpGroup.deleteEntry(QStringLiteral("embedded"));
        }
    }

    readGUISettings();
    readRecentFileSettings();
    readConfig();

    createToolActions(); // this creates the actions for the tools and user tags, which is required before 'activePartGUI' is called

    setupGUI(KXmlGuiWindow::StatusBar | KXmlGuiWindow::Save, QStringLiteral("kileui.rc"));
    createShellGUI(true); // do not call guiFactory()->refreshActionProperties() after this! (bug 314580)

    m_userMenu = new KileMenu::UserMenu(this, this);
    connect(m_userMenu, &KileMenu::UserMenu::sendText, this, static_cast<void (Kile::*)(const QString &)>(&Kile::insertText));
    connect(m_userMenu, &KileMenu::UserMenu::updateStatus, this, &Kile::slotUpdateUserMenuStatus);

    updateUserDefinedMenus();

    // we can only do this here after the main GUI has been set up
    {
        guiFactory()->addClient(viewManager()->viewerPart());

        QMenu *documentViewerMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("menu_document_viewer"), this));
        QMenu *popup = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("menu_okular_part_viewer"), viewManager()->viewerPart()));
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
    applyMainWindowSettings(m_config->group(QStringLiteral("KileMainWindow")));

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
    dbus.registerObject(QStringLiteral("/main"), this);
    dbus.registerService(QStringLiteral("org.kde.kile")); // register under a constant name

    m_lyxserver = new KileLyxServer(KileConfig::runLyxServer());
    connect(m_lyxserver, &KileLyxServer::insert, this, [this](const KileAction::TagData &data) { insertTag(data); });

    if(m_listUserTools.count() > 0) {
        KMessageBox::information(nullptr, i18n("You have defined some tools in the User menu. From now on these tools will be available from the Build->Other menu and can be configured in the configuration dialog (go to the Settings menu and choose Configure Kile). This has some advantages; your own tools can now be used in a QuickBuild command if you wish."), i18n("User Tools Detected"));
        m_listUserTools.clear();
    }

    if(KileConfig::rCVersion() < 8) {
        // if KileConfig::rCVersion() <= 0, then 'kilerc' is (most likely) fresh or empty,
        // otherwise, we have to ask the user if she wants to reset the tools
        if ((KileConfig::rCVersion() <= 0) ||
            (KMessageBox::questionTwoActions(mainWindow(),
                                             i18n("<p>The tool settings need to be reset for this version of Kile to function properly.<br/>"
                                                  "This will overwrite any changes you have made.</p>"
                                                  "<p>Do you want to reset the tools now?</p>"),
                                             i18n("Tools need to be reset"),
                                             KStandardGuiItem::reset(), KStandardGuiItem::cancel())  == KMessageBox::PrimaryAction)) {
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
    if(lastVersionRunFor.isEmpty() || compareVersionStrings(lastVersionRunFor, QStringLiteral("2.9.91")) < 0) {
#ifdef Q_OS_WIN
        // work around the problem that Sonnet's language auto-detection feature doesn't work
        // together with KatePart (as of 08 November 2019)
        QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
        settings.setValue(QStringLiteral("autodetectLanguage"), false);
#endif
        slotPerformCheck();
        KileConfig::setSystemCheckLastVersionRunForAtStartUp(QLatin1StringView(KILE_VERSION_STRING));
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
    m_sideBar->addPage(m_fileBrowserWidget, QIcon::fromTheme(QStringLiteral("document-open")), i18n("Open File"));
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
    m_sideBar->addPage(projectView, QIcon::fromTheme(QStringLiteral("relation")), i18n("Files and Projects"));
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
    m_sideBar->addPage(m_kwStructure, QIcon::fromTheme(QStringLiteral("view-list-tree")), i18n("Structure"));
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
    m_sideBar->addPage(m_scriptsManagementWidget, QIcon::fromTheme(QStringLiteral("preferences-plugin-script")), i18n("Scripts"));
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
    m_sideBar->addPage(m_toolBox,QIcon::fromTheme(QStringLiteral("math0")), i18n("Symbols"));

    m_symbolViewMFUS = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::MFUS);
    m_toolBox->addItem(m_symbolViewMFUS,i18n("Most Frequently Used"));
    m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),false);
    connect(m_symbolViewMFUS, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewRelation = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Relation);
    m_toolBox->addItem(m_symbolViewRelation,QIcon::fromTheme(QStringLiteral("math1")), i18n("Relation"));
    connect(m_symbolViewRelation, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewOperators = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Operator);
    m_toolBox->addItem(m_symbolViewOperators,QIcon::fromTheme(QStringLiteral("math2")), i18n("Operators"));
    connect(m_symbolViewOperators, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewArrows = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Arrow);
    m_toolBox->addItem(m_symbolViewArrows,QIcon::fromTheme(QStringLiteral("math3")), i18n("Arrows"));
    connect(m_symbolViewArrows, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewMiscMath = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::MiscMath);
    m_toolBox->addItem(m_symbolViewMiscMath,QIcon::fromTheme(QStringLiteral("math4")), i18n("Miscellaneous Math"));
    connect(m_symbolViewMiscMath, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewMiscText = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::MiscText);
    m_toolBox->addItem(m_symbolViewMiscText,QIcon::fromTheme(QStringLiteral("math5")), i18n("Miscellaneous Text"));
    connect(m_symbolViewMiscText, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewDelimiters= new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Delimiters);
    m_toolBox->addItem(m_symbolViewDelimiters,QIcon::fromTheme(QStringLiteral("math6")), i18n("Delimiters"));
    connect(m_symbolViewDelimiters, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewGreek = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Greek);
    m_toolBox->addItem(m_symbolViewGreek,QIcon::fromTheme(QStringLiteral("math7")), i18n("Greek"));
    connect(m_symbolViewGreek, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewSpecial = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Special);
    m_toolBox->addItem(m_symbolViewSpecial,QIcon::fromTheme(QStringLiteral("math8")), i18n("Special Characters"));
    connect(m_symbolViewSpecial, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewCyrillic = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::Cyrillic);
    m_toolBox->addItem(m_symbolViewCyrillic,QIcon::fromTheme(QStringLiteral("math10")), i18n("Cyrillic Characters"));
    connect(m_symbolViewCyrillic, &KileWidget::SymbolView::insertText,
            this, static_cast<void (Kile::*)(const QString&, const QList<Package>&)>(&Kile::insertText));

    m_symbolViewUser = new KileWidget::SymbolView(this, m_toolBox, KileWidget::SymbolView::User);
    m_toolBox->addItem(m_symbolViewUser,QIcon::fromTheme(QStringLiteral("math9")), i18n("User Defined"));
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
    m_sideBar->addPage(m_commandViewToolBox, QIcon::fromTheme(QStringLiteral("texlion")), i18n("LaTeX"));

    connect(m_commandViewToolBox, &KileWidget::CommandViewToolBox::sendText,
            this, QOverload<const QString&>::of(&Kile::insertText));
}

void Kile::setupAbbreviationView()
{
    m_kileAbbrevView = new KileWidget::AbbreviationView(abbreviationManager(), m_sideBar);
    connect(abbreviationManager(), &KileAbbreviation::Manager::abbreviationsChanged,
            m_kileAbbrevView, &KileWidget::AbbreviationView::updateAbbreviations);
    m_sideBar->addPage(m_kileAbbrevView, QIcon::fromTheme(QStringLiteral("complete3")), i18n("Abbreviation"));

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
    layout->setSpacing(0);
    widget->setLayout(layout);

    m_latexOutputErrorToolBar = new KToolBar(widget);
    m_latexOutputErrorToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_latexOutputErrorToolBar->setIconDimensions(KIconLoader::SizeSmall);
    m_latexOutputErrorToolBar->setOrientation(Qt::Vertical);

    auto horizontalSeparator = new QFrame(widget);
    horizontalSeparator->setFrameShape(QFrame::VLine);
    horizontalSeparator->setMaximumWidth(1);

    layout->addWidget(errorHandler()->outputWidget());
    layout->addWidget(horizontalSeparator);
    layout->addWidget(m_latexOutputErrorToolBar);
    m_bottomBar->addPage(widget, QIcon::fromTheme(QStringLiteral("utilities-log-viewer")), i18n("Log and Messages"));

    m_outputWidget = new KileWidget::OutputView(this);
    m_outputWidget->setFocusPolicy(Qt::ClickFocus);
    m_outputWidget->setMinimumHeight(40);
    m_outputWidget->setReadOnly(true);
    m_bottomBar->addPage(m_outputWidget, QIcon::fromTheme(QStringLiteral("output_win")), i18n("Output"));

    m_texKonsole = new KileWidget::Konsole(this, this);
    m_bottomBar->addPage(m_texKonsole, QIcon::fromTheme(QStringLiteral("utilities-terminal")), i18n("Konsole"));
    connect(viewManager(), static_cast<void (KileView::Manager::*)(QWidget*)>(&KileView::Manager::currentViewChanged),
            m_texKonsole, static_cast<void (KileWidget::Konsole::*)(void)>(&KileWidget::Konsole::sync));

    m_previewWidget = new KileWidget::PreviewWidget(this, m_bottomBar);
    m_bottomBar->addPage(m_previewWidget, QIcon::fromTheme(QStringLiteral("document-preview")), i18n ("Preview"));

    m_bottomBar->setVisible(true);
    m_bottomBar->switchToTab(KileConfig::bottomBarIndex());
    m_bottomBar->setDirectionalSize(KileConfig::bottomBarSize());
}

void Kile::setupGraphicTools()
{
    KileConfig::setImagemagick(!(QStandardPaths::findExecutable(QStringLiteral("identify")).isNull()));
}

void Kile::setupPreviewTools()
{
    // search for tools
    bool dvipng = !(QStandardPaths::findExecutable(QStringLiteral("dvipng")).isNull());
    bool convert = !(QStandardPaths::findExecutable(QStringLiteral("convert")).isNull());

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

    createAction(KStandardAction::New, QStringLiteral("file_new"), docManager(), [this]() { docManager()->fileNew(); });
    createAction(KStandardAction::Open, QStringLiteral("file_open"), docManager(), [this]() { docManager()->fileOpen(); });

    m_actRecentFiles = KStandardAction::openRecent(docManager(), [this](const QUrl& url) { docManager()->fileOpen(url); }, this);
    m_actRecentFiles->setObjectName(QStringLiteral("file_open_recent"));
    actionCollection()->addAction(QStringLiteral("file_open_recent"), m_actRecentFiles);
    connect(docManager(), &KileDocument::Manager::addToRecentFiles, this, &Kile::addRecentFile);
    m_actRecentFiles->loadEntries(m_config->group(QStringLiteral("Recent Files")));

    createAction(i18n("Save All"), QStringLiteral("file_save_all"), QStringLiteral("document-save-all"), docManager(), &KileDocument::Manager::fileSaveAll);

    createAction(i18n("Create Template From Document..."), "template_create", docManager(), &KileDocument::Manager::createTemplate);
    createAction(i18n("&Remove Template..."), "template_remove", docManager(), &KileDocument::Manager::removeTemplate);
    createAction(KStandardAction::Close, QStringLiteral("file_close"), docManager(), [this]() { docManager()->fileClose();} );
    createAction(i18n("Close All"), "file_close_all", docManager(), &KileDocument::Manager::fileCloseAll);
    createAction(i18n("Close All Ot&hers"), "file_close_all_others", docManager(), [this]() { docManager()->fileCloseAllOthers(); });
    createAction(i18n("S&tatistics"), "Statistics", this, [this]() { showDocInfo(); });
    createAction(i18nc("@action:inmenu", "&Open Containing Folder"), QStringLiteral("open_containing_folder"), QStringLiteral("document-open-folder"), this, [this]() { openContainingFolder(); });
    createAction(i18n("&ASCII"), "file_export_ascii", this, [this]() { convertToASCII(); });
    createAction(i18n("Latin-&1 (iso 8859-1)"), "file_export_latin1", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&2 (iso 8859-2)"), "file_export_latin2", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&3 (iso 8859-3)"), "file_export_latin3", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&4 (iso 8859-4)"), "file_export_latin4", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&5 (iso 8859-5)"), "file_export_latin5", this, [this]() { convertToEnc(); });
    createAction(i18n("Latin-&9 (iso 8859-9)"), "file_export_latin9", this, [this]() { convertToEnc(); });
    createAction(i18n("&Central European (cp-1250)"), "file_export_cp1250", this, [this]() { convertToEnc(); });
    createAction(i18n("&Western European (cp-1252)"), "file_export_cp1252", this, [this]() { convertToEnc(); });
    createAction(KStandardAction::Quit, QStringLiteral("file_quit"), this, &Kile::close);

    createAction(i18n("Move Tab Left"), QStringLiteral("move_view_tab_left"), QStringLiteral("arrow-left"), viewManager(), [this]() { viewManager()->moveTabLeft(); });
    createAction(i18n("Move Tab Right"), QStringLiteral("move_view_tab_right"), QStringLiteral("arrow-right"), viewManager(), [this]() { viewManager()->moveTabRight(); });

    createAction(i18n("Next section"), QStringLiteral("edit_next_section"), QStringLiteral("nextsection"), QKeySequence(Qt::ALT | Qt::Key_Down),
                 m_edit, &KileDocument::EditorExtension::gotoNextSectioning);
    createAction(i18n("Prev section"), QStringLiteral("edit_prev_section"), QStringLiteral("prevsection"), QKeySequence(Qt::ALT | Qt::Key_Up),
                 m_edit, &KileDocument::EditorExtension::gotoPrevSectioning);
    createAction(i18n("Next paragraph"), QStringLiteral("edit_next_paragraph"), QStringLiteral("nextparagraph"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Down),
                 m_edit, [this]() { m_edit->gotoNextParagraph(); });
    createAction(i18n("Prev paragraph"), QStringLiteral("edit_prev_paragraph"), QStringLiteral("prevparagraph"), QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Up),
                 m_edit, [this]() { m_edit->gotoPrevParagraph(); });

    createAction(i18n("Find &in Files..."), QStringLiteral("FindInFiles"), QStringLiteral("filegrep"), this, &Kile::findInFiles);

    createAction(i18n("Refresh Str&ucture"), QStringLiteral("RefreshStructure"), QStringLiteral("refreshstructure"), QKeySequence(Qt::Key_F12), this, &Kile::refreshStructure);

    //project actions
    createAction(i18n("&New Project..."), QStringLiteral("project_new"), QStringLiteral("window-new"), docManager(), &KileDocument::Manager::projectNew);
    createAction(i18n("&Open Project..."), QStringLiteral("project_open"), QStringLiteral("project-open"), docManager(), [this]() { docManager()->projectOpen(); });

    m_actRecentProjects = new KRecentFilesAction(i18n("Open &Recent Project"), actionCollection());
    actionCollection()->addAction(QStringLiteral("project_openrecent"), m_actRecentProjects);
    connect(m_actRecentProjects, &KRecentFilesAction::urlSelected, docManager(), [this](const QUrl& url) { docManager()->projectOpen(url); });
    connect(docManager(), &KileDocument::Manager::removeFromRecentProjects, this, &Kile::removeRecentProject);
    connect(docManager(), &KileDocument::Manager::addToRecentProjects, this, &Kile::addRecentProject);
    m_actRecentProjects->loadEntries(m_config->group(QStringLiteral("Projects")));

    createAction(i18n("A&dd Files to Project..."), QStringLiteral("project_add"), QStringLiteral("project_add"), docManager(), [this]() { m_docManager->projectAddFiles(); });
    createAction(i18n("Refresh Project &Tree"), QStringLiteral("project_buildtree"), QStringLiteral("project_rebuild"), docManager(), [this]() { m_docManager->buildProjectTree(); });
    createAction(i18n("&Archive"), QStringLiteral("project_archive"), QStringLiteral("project_archive"), this, [this]() { runArchiveTool(); });
    createAction(i18n("Project &Options"), QStringLiteral("project_options"), QStringLiteral("configure_project"), docManager(), [this]() { m_docManager->projectOptions(); });
    createAction(i18n("&Close Project"), QStringLiteral("project_close"), QStringLiteral("project-development-close"), docManager(), [this]() { m_docManager->projectClose(); });

    // new project actions (dani)
    createAction(i18n("&Show Projects..."), "project_show", docManager(), &KileDocument::Manager::projectShow);
    createAction(i18n("Re&move Files From Project..."), QStringLiteral("project_remove"), QStringLiteral("project_remove"), docManager(), &KileDocument::Manager::projectRemoveFiles);
    createAction(i18n("Show Project &Files..."), QStringLiteral("project_showfiles"), QStringLiteral("project_show"), docManager(), &KileDocument::Manager::projectShowFiles);
    // tbraun
    createAction(i18n("Open All &Project Files"), "project_openallfiles", docManager(), [this]() { docManager()->projectOpenAllFiles(); });
    createAction(i18n("Find in &Project..."), QStringLiteral("project_findfiles"), QStringLiteral("projectgrep"), this, &Kile::findInProjects);

    //build actions
    act = createAction(i18n("Clean"), QStringLiteral("CleanAll"), QStringLiteral("user-trash"), this, [this]() { cleanAll(); });

    QList<QKeySequence> nextTabShorcuts;
    nextTabShorcuts.append(QKeySequence(Qt::ALT | Qt::Key_Right));
    nextTabShorcuts.append(KStandardShortcut::tabNext());
    createAction(i18n("Next Document"), QStringLiteral("gotoNextDocument"), QStringLiteral("go-next-view-page"),
                 nextTabShorcuts, viewManager(), &KileView::Manager::gotoNextView);

    QList<QKeySequence> prevTabShorcuts;
    prevTabShorcuts.append(QKeySequence(Qt::ALT | Qt::Key_Left));
    prevTabShorcuts.append(KStandardShortcut::tabPrev());
    createAction(i18n("Previous Document"), QStringLiteral("gotoPrevDocument"), QStringLiteral("go-previous-view-page"),
                 prevTabShorcuts, viewManager(), &KileView::Manager::gotoPrevView);

    createAction(i18n("Focus Log/Messages View"), "focus_log", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M), this, &Kile::focusLog);
    createAction(i18n("Focus Output View"), "focus_output", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_O), this, &Kile::focusOutput);
    createAction(i18n("Focus Konsole View"), "focus_konsole", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_K), this, &Kile::focusKonsole);
    createAction(i18n("Focus Editor View"), "focus_editor", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F), this, &Kile::focusEditor);

    createAction(i18nc("@action: Starts the completion of the current LaTeX command", "Complete (La)TeX Command"), QStringLiteral("edit_complete_word"), QStringLiteral("complete1"),
                 QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::Key_Space), codeCompletionManager(), [this]() { codeCompletionManager()->startLaTeXCompletion(); });
    createAction(i18nc("@action: Starts the input (and completion) of a LaTeX environment", "Complete LaTeX Environment"), QStringLiteral("edit_complete_env"), QStringLiteral("complete2"),
                 QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_Space), codeCompletionManager(), [this]() { codeCompletionManager()->startLaTeXEnvironment(); });
    createAction(i18nc("@action: Starts the completion of the current abbreviation", "Complete Abbreviation"), QStringLiteral("edit_complete_abbrev"), QStringLiteral("complete3"),
                 QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Space), codeCompletionManager(), [this]() { codeCompletionManager()->startAbbreviationCompletion(); });

    createAction(i18n("Next Bullet"), QStringLiteral("edit_next_bullet"), QStringLiteral("nextbullet"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Right),
                 m_edit, [this]() { m_edit->nextBullet(); });
    createAction(i18n("Prev Bullet"), QStringLiteral("edit_prev_bullet"), QStringLiteral("prevbullet"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Left),
                 m_edit, [this]() { m_edit->prevBullet(); });

// advanced editor (dani)
    createAction(i18n("Environment (inside)"), QStringLiteral("edit_select_inside_env"), QStringLiteral("selenv_i"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_E)),
                 m_edit, &KileDocument::EditorExtension::selectEnvInside);
    createAction(i18n("Environment (outside)"), QStringLiteral("edit_select_outside_env"), QStringLiteral("selenv_o"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_F)),
                 m_edit, &KileDocument::EditorExtension::selectEnvOutside);
    createAction(i18n("TeX Group (inside)"), QStringLiteral("edit_select_inside_group"), QStringLiteral("selgroup_i"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_T)),
                 m_edit, &KileDocument::EditorExtension::selectTexgroupInside);
    createAction(i18n("TeX Group (outside)"), QStringLiteral("edit_select_outside_group"), QStringLiteral("selgroup_o"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_U)),
                 m_edit, &KileDocument::EditorExtension::selectTexgroupOutside);
    createAction(i18n("Math Group"), QStringLiteral("edit_select_mathgroup"), QStringLiteral("selmath"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_M)),
                 m_edit, [this]() { m_edit->selectMathgroup(); });
    createAction(i18n("Paragraph"), QStringLiteral("edit_select_paragraph"), QStringLiteral("selpar"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_P)),
                 m_edit, [this]() { m_edit->selectParagraph(); });
    createAction(i18n("Line"), QStringLiteral("edit_select_line"), QStringLiteral("selline"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_L)),
                 m_edit, [this]() { m_edit->selectLine(); });
    createAction(i18n("TeX Word"), QStringLiteral("edit_select_word"), QStringLiteral("selword"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_S), QKeyCombination(Qt::Key_W)),
                 m_edit, [this]() { m_edit->selectWord(); });

    createAction(i18n("Environment (inside)"), QStringLiteral("edit_delete_inside_env"), QStringLiteral("delenv_i"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_E)),
                 m_edit, &KileDocument::EditorExtension::deleteEnvInside);
    createAction(i18n("Environment (outside)"), QStringLiteral("edit_delete_outside_env"), QStringLiteral("delenv_o"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_F)),
                 m_edit, &KileDocument::EditorExtension::deleteEnvOutside);
    createAction(i18n("TeX Group (inside)"), QStringLiteral("edit_delete_inside_group"), QStringLiteral("delgroup_i"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_T)),
                 m_edit, &KileDocument::EditorExtension::deleteTexgroupInside);
    createAction(i18n("TeX Group (outside)"), QStringLiteral("edit_delete_outside_group"), QStringLiteral("delgroup_o"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_U)),
                 m_edit, &KileDocument::EditorExtension::deleteTexgroupInside);
    createAction(i18n("Math Group"), QStringLiteral("edit_delete_mathgroup"), QStringLiteral("delmath"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_M)),
                 m_edit, [this]() { m_edit->deleteMathgroup(); });
    createAction(i18n("Paragraph"), QStringLiteral("edit_delete_paragraph"), QStringLiteral("delpar"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_P)),
                 m_edit, [this]() { m_edit->deleteParagraph(); });
    createAction(i18n("To End of Line"), QStringLiteral("edit_delete_eol"), QStringLiteral("deleol"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_L)),
                 m_edit, [this]() { m_edit->deleteEndOfLine(); });
    createAction(i18n("TeX Word"), QStringLiteral("edit_delete_word"), QStringLiteral("delword"), 
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_T), QKeyCombination(Qt::Key_W)),
                 m_edit, [this]() { m_edit->deleteWord(); });

    createAction(i18n("Go to Begin"), QStringLiteral("edit_begin_env"), QStringLiteral("gotobeginenv"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_E), QKeyCombination(Qt::Key_B)),
                 m_edit, &KileDocument::EditorExtension::gotoBeginEnv);
    createAction(i18n("Go to End"), QStringLiteral("edit_end_env"), QStringLiteral("gotoendenv"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_E), QKeyCombination(Qt::Key_E)),
                 m_edit, &KileDocument::EditorExtension::gotoEndEnv);
    createAction(i18n("Match"), QStringLiteral("edit_match_env"), QStringLiteral("matchenv"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_E), QKeyCombination(Qt::Key_M)),
                 m_edit, &KileDocument::EditorExtension::matchEnv);
    createAction(i18n("Close"), QStringLiteral("edit_close_env"), QStringLiteral("closeenv"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_E), QKeyCombination(Qt::Key_C)),
                 m_edit, &KileDocument::EditorExtension::closeEnv);
    createAction(i18n("Close All"), QStringLiteral("edit_closeall_env"), QStringLiteral("closeallenv"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_E), QKeyCombination(Qt::Key_A)),
                 m_edit, &KileDocument::EditorExtension::closeAllEnv);

    createAction(i18n("Go to Begin"), QStringLiteral("edit_begin_group"), QStringLiteral("gotobegingroup"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_G), QKeyCombination(Qt::Key_B)),
                 m_edit, &KileDocument::EditorExtension::gotoBeginTexgroup);
    createAction(i18n("Go to End"), QStringLiteral("edit_end_group"), QStringLiteral("gotoendgroup"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_G), QKeyCombination(Qt::Key_E)),
                 m_edit, &KileDocument::EditorExtension::gotoEndTexgroup);
    createAction(i18n("Match"), QStringLiteral("edit_match_group"), QStringLiteral("matchgroup"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_G), QKeyCombination(Qt::Key_M)),
                 m_edit, [this]() { m_edit->matchTexgroup(); });
    createAction(i18n("Close"), QStringLiteral("edit_close_group"), QStringLiteral("closegroup"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_G), QKeyCombination(Qt::Key_C)),
                 m_edit, [this]() { m_edit->closeTexgroup(); });

    createAction(i18n("Selection"), QStringLiteral("quickpreview_selection"), QStringLiteral("preview_sel"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_P), QKeyCombination(Qt::Key_S)),
                 this, &Kile::quickPreviewSelection);
    createAction(i18n("Environment"), QStringLiteral("quickpreview_environment"), QStringLiteral("preview_env"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_P), QKeyCombination(Qt::Key_E)),
                 this, &Kile::quickPreviewEnvironment);
    createAction(i18n("Subdocument"), QStringLiteral("quickpreview_subdocument"), QStringLiteral("preview_subdoc"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_P), QKeyCombination(Qt::Key_D)),
                 this, &Kile::quickPreviewSubdocument);
    createAction(i18n("Mathgroup"), QStringLiteral("quickpreview_math"), QStringLiteral("preview_math"),
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_P), QKeyCombination(Qt::Key_M)),
                 this, &Kile::quickPreviewMathgroup);

    KileStdActions::setupStdTags(this, this, actionCollection(), this);
    KileStdActions::setupMathTags(this, actionCollection());

    m_bibTagActionMenu = new KActionMenu(i18n("&Bibliography"), actionCollection());
    m_bibTagActionMenu->setPopupMode(QToolButton::InstantPopup);
    actionCollection()->addAction(QStringLiteral("menu_bibliography"), m_bibTagActionMenu);

    createAction(i18n("Clean"), "CleanBib", this, &Kile::cleanBib);

    m_bibTagSettings = new KSelectAction(i18n("&Settings"),actionCollection());
    actionCollection()->addAction(QStringLiteral("settings_menu_bibliography"), m_bibTagSettings);

    act = createAction(i18n("Settings for BibTeX"), "setting_bibtex", this, &Kile::rebuildBibliographyMenu);
    act->setCheckable(true);
    m_bibTagSettings->addAction(act);

    act = createAction(i18n("Settings for Biblatex"), "setting_biblatex", this, &Kile::rebuildBibliographyMenu);
    act->setCheckable(true);
    m_bibTagSettings->addAction(act);
    m_bibTagSettings->setCurrentAction(action((QStringLiteral("setting_") + KileConfig::bibliographyType())));

    rebuildBibliographyMenu();

    createAction(i18n("Quick Start"), QStringLiteral("wizard_document"), QStringLiteral("quickwizard"), this, &Kile::quickDocument);
    connect(docManager(), &KileDocument::Manager::startWizard, this, &Kile::quickDocument);
    createAction(i18n("Tabular"), QStringLiteral("wizard_tabular"), QStringLiteral("wizard_tabular"), this, &Kile::quickTabular);
    createAction(i18n("Array"), QStringLiteral("wizard_array"), QStringLiteral("wizard_array"), this, &Kile::quickArray);
    createAction(i18n("Tabbing"), QStringLiteral("wizard_tabbing"), QStringLiteral("wizard_tabbing"), this, &Kile::quickTabbing);
    createAction(i18n("Floats"), QStringLiteral("wizard_float"), QStringLiteral("wizard_float"), this, &Kile::quickFloat);
    createAction(i18n("Math"), QStringLiteral("wizard_mathenv"), QStringLiteral("wizard_math"), this, &Kile::quickMathenv);
    createAction(i18n("Postscript Tools"), QStringLiteral("wizard_postscript"), QStringLiteral("wizard_pstools"), this, &Kile::quickPostscript);
    createAction(i18n("PDF Tools"), QStringLiteral("wizard_pdf"), QStringLiteral("wizard_pdftools"), this, &Kile::quickPdf);

    ModeAction = new KToggleAction(i18n("Define Current Document as '&Master Document'"), actionCollection());
    actionCollection()->addAction(QStringLiteral("Mode"), ModeAction);
    ModeAction->setIcon(QIcon::fromTheme(QStringLiteral("master")));
    connect(ModeAction, &KToggleAction::triggered, this, &Kile::toggleMasterDocumentMode);

    KToggleAction *showDocumentViewer = new KToggleAction(i18n("Show Document Viewer"), actionCollection());
    actionCollection()->addAction(QStringLiteral("ShowDocumentViewer"), showDocumentViewer);
    showDocumentViewer->setChecked(KileConfig::showDocumentViewer());
    connect(showDocumentViewer, &KToggleAction::toggled, viewManager(), &KileView::Manager::setDocumentViewerVisible);
    connect(viewManager(), &KileView::Manager::documentViewerWindowVisibilityChanged,
            showDocumentViewer, &KToggleAction::setChecked);

    KToggleAction *tact = new KToggleAction(i18n("Show S&ide Bar"), actionCollection());
    actionCollection()->addAction(QStringLiteral("StructureView"), tact);
    tact->setChecked(KileConfig::sideBar());
    connect(tact, &KToggleAction::toggled, m_sideBar, &Kile::setVisible);
    connect(m_sideBar, &KileWidget::SideBar::visibilityChanged, this, &Kile::sideOrBottomBarChanged);

    m_actionMessageView = new KToggleAction(i18n("Show Mess&ages Bar"), actionCollection());
    actionCollection()->addAction(QStringLiteral("MessageView"), m_actionMessageView);
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
    actionCollection()->addAction(QStringLiteral("WatchFile"), WatchFileAction);
    WatchFileAction->setIcon(QIcon::fromTheme(QStringLiteral("watchfile")));
    connect(WatchFileAction, &KToggleAction::toggled, this, &Kile::toggleWatchFile);
    if(m_bWatchFile) {
        WatchFileAction->setChecked(true);
    }
    else {
        WatchFileAction->setChecked(false);
    }

    createAction(i18n("LaTeX"), "help_latex_index",
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_H), QKeyCombination(Qt::Key_L)),
                 m_help, &KileHelp::Help::helpLatexIndex);
    createAction(i18n("LaTeX Commands"), "help_latex_command",
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_H), QKeyCombination(Qt::Key_C)),
                 m_help, &KileHelp::Help::helpLatexCommand);
    createAction(i18n("LaTeX Environments"), "help_latex_env",
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_H), QKeyCombination(Qt::Key_E)),
                 m_help, &KileHelp::Help::helpLatexEnvironment);
    createAction(i18n("Context Help"), "help_context",
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_H), QKeyCombination(Qt::Key_K)),
                 m_help, [this]() { m_help->helpKeyword(); });
    createAction(i18n("Documentation Browser"), "help_docbrowser",
                 QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_H), QKeyCombination(Qt::Key_B)),
                 m_help, &KileHelp::Help::helpDocBrowser);

    createAction(i18n("&About Editor Component"), "help_about_editor", this, &Kile::aboutEditorComponent);

    QAction *kileconfig = KStandardAction::preferences(this, &Kile::generalOptions, actionCollection());
    kileconfig->setIcon(QIcon::fromTheme(QStringLiteral("configure-kile")));

    createAction(KStandardAction::KeyBindings, this, &Kile::configureKeys);
    createAction(KStandardAction::ConfigureToolbars, this, &Kile::configureToolbars);

    createAction(i18n("&System Check..."), "settings_perform_check", this, &Kile::slotPerformCheck);

    m_userHelpActionMenu = new KActionMenu(i18n("User Help"), actionCollection());
    actionCollection()->addAction(QStringLiteral("help_userhelp"), m_userHelpActionMenu);

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
        name = QString(QStringLiteral("bibtex"));
    }
    else if ( currentItem == i18n("Biblatex") ) {
        name = QString(QStringLiteral("biblatex"));
    }
    else {
        KILE_DEBUG_MAIN << "wrong currentItem in bibliography settings menu";
        name = QString(QStringLiteral("bibtex"));
    }

    KileConfig::setBibliographyType(name);
    m_bibTagActionMenu->menu()->clear();

    KileStdActions::setupBibTags(this, actionCollection(),m_bibTagActionMenu);
    m_bibTagActionMenu->addSeparator();
    m_bibTagActionMenu->addAction(action(QStringLiteral("CleanBib")));
    m_bibTagActionMenu->addSeparator();
    m_bibTagActionMenu->addAction(action(QStringLiteral("settings_menu_bibliography")));
}

QAction* Kile::createToolAction(const QString& toolName)
{
    return createAction(toolName, QStringLiteral("tool_") + toolName,
                        KileTool::iconFor(toolName, m_config.data()), this, [this, toolName]() { runTool(toolName); });
}

void Kile::createToolActions()
{
    const QStringList tools = KileTool::toolList(m_config.data());
    for(const QString& toolName : tools) {
        if(!actionCollection()->action(QStringLiteral("tool_") + toolName)) {
            KILE_DEBUG_MAIN << "Creating action for tool" << toolName;
            createToolAction(toolName);
        }
    }
}

void Kile::setupTools()
{
    KILE_DEBUG_MAIN << "==Kile::setupTools()===================" << Qt::endl;

    if(!m_buildMenuCompile || !m_buildMenuConvert ||  !m_buildMenuTopLevel || !m_buildMenuQuickPreview || !m_buildMenuViewer || !m_buildMenuOther) {
        KILE_DEBUG_MAIN << "BUG, menu pointers are nullptr"
                        << (m_buildMenuCompile == nullptr)
                        << (m_buildMenuConvert == nullptr)
                        << (m_buildMenuTopLevel == nullptr)
                        << (m_buildMenuQuickPreview == nullptr)
                        << (m_buildMenuViewer == nullptr)
                        << (m_buildMenuOther == nullptr);
        return;
    }

    QStringList tools = KileTool::toolList(m_config.data());
    QList<QAction*> *pl;
    ToolbarSelectAction *pSelectAction = nullptr;

    m_compilerActions->saveCurrentAction();
    m_viewActions->saveCurrentAction();
    m_convertActions->saveCurrentAction();
    m_quickActions->saveCurrentAction();

    // do plugActionList by hand ...
    for(QAction *act: std::as_const(m_listQuickActions)) {
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
        if (toolMenu == QStringLiteral("none")) {
            continue;
        }

        if (toolMenu == QStringLiteral("Compile")) {
            pl = &m_listCompilerActions;
            pSelectAction = m_compilerActions;
        }
        else if (toolMenu == QStringLiteral("View")) {
            pl = &m_listViewerActions;
            pSelectAction = m_viewActions;
        }
        else if (toolMenu == QStringLiteral("Convert")) {
            pl = &m_listConverterActions;
            pSelectAction = m_convertActions;
        }
        else if (toolMenu == QStringLiteral("Quick")) {
            pl = &m_listQuickActions;
            pSelectAction = m_quickActions;
        }
        else {
            pl = &m_listOtherActions;
            pSelectAction = nullptr;
        }

        KILE_DEBUG_MAIN << "\tadding " << tools[i] << " " << toolMenu << " #" << pl->count() << Qt::endl;

        QAction *act = actionCollection()->action(QStringLiteral("tool_") + tools[i]);
        if(!act) {
            KILE_DEBUG_MAIN << "no tool for " << tools[i];
            createToolAction(tools[i]);
        }
        pl->append(act);

        if(pSelectAction) {
            pSelectAction->addAction(actionCollection()->action(QStringLiteral("tool_") + tools[i]));
        }
    }

    m_quickActions->addSeparator();
    m_quickActions->addAction(action(QStringLiteral("quickpreview_selection")));
    m_quickActions->addAction(action(QStringLiteral("quickpreview_environment")));
    m_quickActions->addAction(action(QStringLiteral("quickpreview_subdocument")));
    m_quickActions->addSeparator();
    m_quickActions->addAction(action(QStringLiteral("quickpreview_math")));

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

    actionCollection()->addAction(QStringLiteral("list_compiler_select"), m_compilerActions);
    actionCollection()->addAction(QStringLiteral("list_convert_select"), m_convertActions);
    actionCollection()->addAction(QStringLiteral("list_view_select"), m_viewActions);
    actionCollection()->addAction(QStringLiteral("list_quick_select"), m_quickActions);
}

void Kile::saveLastSelectedAction() {

    KILE_DEBUG_MAIN << "Kile::saveLastSelectedAction()" << Qt::endl;
    const QStringList list =
        {QLatin1String("Compile"), QLatin1String("Convert"), QLatin1String("View"), QLatin1String("Quick")};

    const ToolbarSelectAction *pSelectAction = nullptr ;

    KConfigGroup grp = m_config->group(QStringLiteral("ToolSelectAction"));

    for(const QString& action : list) {
        if (action == QStringLiteral("Compile")) {
            pSelectAction = m_compilerActions;
        }
        else if (action == QStringLiteral("View")) {
            pSelectAction = m_viewActions;
        }
        else if (action == QStringLiteral("Convert")) {
            pSelectAction = m_convertActions;
        }
        else if (action == QStringLiteral("Quick")) {
            pSelectAction = m_quickActions;
        }

        KILE_DEBUG_MAIN << "current item is " << pSelectAction->currentItem();

        grp.writeEntry(action, pSelectAction->currentItem());
    }
}

void Kile::restoreLastSelectedAction() {

    const QStringList list =
        {QLatin1String("Compile"), QLatin1String("Convert"), QLatin1String("View"), QLatin1String("Quick")};

    ToolbarSelectAction *pSelectAction = nullptr;
    int defaultAction = 0;

    KConfigGroup grp = m_config->group(QStringLiteral("ToolSelectAction"));

    for(const QString& action : list) {
        if (action == QStringLiteral("Compile")) {
            pSelectAction = m_compilerActions;
            defaultAction = 9; // PDFLatex
        }
        else if (action == QStringLiteral("View")) {
            pSelectAction = m_viewActions;
            defaultAction = 4; // ViewPDF
        }
        else if (action == QStringLiteral("Convert")) {
            pSelectAction = m_convertActions;
            defaultAction = 0;
        }
        else if (action == QStringLiteral("Quick")) {
            pSelectAction = m_quickActions;
            defaultAction = 0;
        }

        int actIndex = grp.readEntry(action, defaultAction);
        KILE_DEBUG_MAIN << "selecting" << actIndex << "for" << action;
        pSelectAction->setCurrentItem(actIndex);
    }
}

void Kile::cleanUpActionList(QList<QAction*> &list, const QStringList &tools)
{
//  	KILE_DEBUG_MAIN << "cleanUpActionList tools are" << tools.join("; ");
    QList<QAction*>::iterator it, testIt;
    for ( it= list.begin(); it != list.end(); ++it) {
        QAction *act = *it;
        if ( act != nullptr && !act->objectName().isEmpty() && !tools.contains(act->objectName().mid(5)) ) {
            const QList<QObject*> widgetList = act->associatedObjects();
            for (QObject *widget : widgetList) {
                if (qobject_cast<QWidget*>(widget) == toolBar(QStringLiteral("toolsToolBar"))) {
                    toolBar(QStringLiteral("toolsToolBar"))->removeAction(act);
                    break;
                }
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

void Kile::setLine(const QString &line, const QString &startupId)
{
    bool ok;
    uint l = line.toUInt(&ok, 10);
    KTextEditor::View *view = viewManager()->currentTextView();
    if (view && ok) {
        show();
        raise();
        activateWindow();
        // be very aggressive when it comes to raising the main window to the top
        if (!startupId.isEmpty() && KWindowSystem::isPlatformWayland()) {
            KWindowSystem::setCurrentXdgActivationToken(startupId);
        }
        focusTextView(view);
        editorExtension()->goToLine(l - 1, view);
    }
}

void Kile::setCursor(const QUrl &url, int parag, int index)
{
    KTextEditor::Document *doc = docManager()->docFor(url);
    if(doc) {
        KTextEditor::View *view = static_cast<KTextEditor::View*>(doc->views().constFirst());
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
    KileTool::Archive *tool = dynamic_cast<KileTool::Archive*>(m_manager->createTool(QStringLiteral("Archive"), QString(), false));
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

    const QList<KToolBar*> toolBarsList = toolBars();
    QHash<KToolBar*, bool> toolBarVisibilityHash;

    for(KToolBar* toolBar : toolBarsList) {
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

    for(KToolBar* toolBar : toolBarsList) {
        toolBar->setVisible(toolBarVisibilityHash[toolBar]);
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
    int pos = shortName.lastIndexOf(QLatin1Char('/'));
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
    // Passing nullptr is ok
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
        KileConfig::setLastDocument(QString());
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
    const QList<KileProject*> projectList = docManager()->projects();
    for(const KileProject* project : projectList) {
        const QUrl url = project->url();
        if(!url.isEmpty()) { // shoul always be the case, but just in case...
            m_listProjectsOpenOnStart.append(url.toLocalFile());
        }
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

void Kile::openContainingFolder(KTextEditor::View *view)
{
    if(!view) {
        view = viewManager()->currentTextView();
    }

    if(!view) {
        return;
    }

    const KTextEditor::Document *currentDocument = view->document();
    if(!currentDocument) {
        return;
    }

    const QUrl currentDocumentUrl = currentDocument->url();
    if(!currentDocumentUrl.isValid()) {
        return;
    }

    KIO::highlightInFileManager({currentDocumentUrl});
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
    doc->setEncoding(QStringLiteral("ISO 8859-1"));
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
        QString name = QString(sender()->objectName()).section(QLatin1Char('_'), -1);
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
        setWindowTitle(QString());
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
    static QPointer<KileDialog::FindFilesDialog> dlg = nullptr;

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
    static QPointer<KileDialog::FindFilesDialog> project_dlg = nullptr;

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
    m_buildMenuTopLevel = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container(QStringLiteral("menu_build"), m_mainWindow));
    m_buildMenuCompile  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container(QStringLiteral("menu_compile"), m_mainWindow));
    m_buildMenuConvert  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container(QStringLiteral("menu_convert"), m_mainWindow));
    m_buildMenuViewer  = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container(QStringLiteral("menu_viewer"), m_mainWindow));
    m_buildMenuOther   = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container(QStringLiteral("menu_other"), m_mainWindow));
    m_buildMenuQuickPreview   = dynamic_cast<QMenu*>(m_mainWindow->guiFactory()->container(QStringLiteral("quickpreview"), m_mainWindow));

    m_userMenu->updateGUI();

    setupTools();
}

void Kile::enableGUI(bool enable)
{
    // update action lists
    const QList<QAction *> actions = actionCollection()->actions();
    for(QAction *action : actions) {
        if (m_dictMenuAction.contains(action->objectName())
                || m_dictMenuFile.contains(action->objectName())) {
            action->setEnabled(enable);
        }
    }

    // update latex usermenu actions
    if(m_userMenu) {
        const QList<QAction *> useractions = m_userMenu->menuActions();
        for(QAction *action : useractions) {
            if(action) {
                action->setEnabled(enable);
            }
            else
            {
                KILE_WARNING_MAIN << "null action found.";
            }
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
    for(QAction* action : std::as_const(actionList)) {
        action->setEnabled(enable);
    }

    // enable or disable bibliography menu entries
    const QList<QAction*> bibTagActionList = m_bibTagActionMenu->menu()->actions();
    for(QAction* action : bibTagActionList) {
        action->setEnabled(enable);
    }

    const QStringList menuList =
        {QStringLiteral("file"), QStringLiteral("edit"), QStringLiteral("view"), QStringLiteral("menu_build"),
         QStringLiteral("menu_project"), QStringLiteral("menu_latex"), QStringLiteral("wizard"), QStringLiteral("tools")};
    for(const QString& entry : menuList) {
        QMenu *menu = dynamic_cast<QMenu*>(guiFactory()->container(entry, this));
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
            << QStringLiteral("project_add") << QStringLiteral("project_remove")
            << QStringLiteral("project_showfiles")
            << QStringLiteral("project_buildtree") << QStringLiteral("project_options") << QStringLiteral("project_findfiles")
            << QStringLiteral("project_archive") << QStringLiteral("project_close") << QStringLiteral("project_openallfiles")
            ;

    filelist
    // file
            << QStringLiteral("convert")
            // edit
            << QStringLiteral("goto_menu") << QStringLiteral("complete") << QStringLiteral("bullet") << QStringLiteral("select")
            << QStringLiteral("delete") << QStringLiteral("environment") << QStringLiteral("texgroup")
            // build
            << QStringLiteral("quickpreview") << QStringLiteral("menu_compile") << QStringLiteral("menu_convert")
            << QStringLiteral("menu_viewers") << QStringLiteral("menu_other")
            // latex
            << QStringLiteral("menu_preamble") << QStringLiteral("menu_lists") << QStringLiteral("menu_sectioning") << QStringLiteral("references")
            << QStringLiteral("menu_environment") << QStringLiteral("menu_listenv") << QStringLiteral("menu_tabularenv") << QStringLiteral("menu_floatenv")
            << QStringLiteral("menu_code") << QStringLiteral("menu_math") << QStringLiteral("menu_mathenv") << QStringLiteral("menu_mathamsenv")
            << QStringLiteral("menu_bibliography") << QStringLiteral("menu_fontstyles") << QStringLiteral("menu_spacing")
            ;

    actionlist
    // file
            << QStringLiteral("file_save_copy_as") << QStringLiteral("file_save_all") << QStringLiteral("template_create") << QStringLiteral("Statistics") << QStringLiteral("open_containing_folder")
            << QStringLiteral("file_close") << QStringLiteral("file_close_all") << QStringLiteral("file_close_all_others")
            // edit
            << QStringLiteral("RefreshStructure")
            // view
            << QStringLiteral("gotoPrevDocument") << QStringLiteral("gotoNextDocument")
            // build
            << QStringLiteral("quickpreview_selection") << QStringLiteral("quickpreview_environment")
            << QStringLiteral("quickpreview_subdocument") << QStringLiteral("quickpreview_math")
            << QStringLiteral("WatchFile") << QStringLiteral("CleanAll")
            // latex
            << QStringLiteral("tag_documentclass") << QStringLiteral("tag_usepackage") << QStringLiteral("tag_amspackages") << QStringLiteral("tag_env_document")
            << QStringLiteral("tag_author") << QStringLiteral("tag_title") << QStringLiteral("tag_maketitle") << QStringLiteral("tag_titlepage") << QStringLiteral("tag_env_abstract")
            << QStringLiteral("tag_tableofcontents") << QStringLiteral("tag_listoffigures") << QStringLiteral("tag_listoftables")
            << QStringLiteral("tag_makeindex") << QStringLiteral("tag_printindex") << QStringLiteral("tag_makeglossary") << QStringLiteral("tag_env_thebibliography")
            << QStringLiteral("tag_part") << QStringLiteral("tag_chapter") << QStringLiteral("tag_section") << QStringLiteral("tag_subsection") << QStringLiteral("tag_subsubsection")
            << QStringLiteral("tag_paragraph") << QStringLiteral("tag_subparagraph") << QStringLiteral("tag_label")
            << QStringLiteral("tag_ref") << QStringLiteral("tag_pageref") << QStringLiteral("tag_index") << QStringLiteral("tag_footnote") << QStringLiteral("tag_cite")
            << QStringLiteral("tag_center") << QStringLiteral("tag_flushleft") << QStringLiteral("tag_flushright")
            << QStringLiteral("tag_env_minipage") << QStringLiteral("tag_quote") << QStringLiteral("tag_quotation") << QStringLiteral("tag_verse")
            << QStringLiteral("tag_env_itemize") << QStringLiteral("tag_env_enumerate") << QStringLiteral("tag_env_description") << QStringLiteral("tag_item")
            << QStringLiteral("tag_env_tabular") << QStringLiteral("tag_env_tabular*") << QStringLiteral("tag_env_tabbing")
            << QStringLiteral("tag_multicolumn") << QStringLiteral("tag_hline") << QStringLiteral("tag_vline") << QStringLiteral("tag_cline")
            << QStringLiteral("tag_figure") << QStringLiteral("tag_table")
            << QStringLiteral("tag_verbatim") << QStringLiteral("tag_env_verbatim*") << QStringLiteral("tag_verb") << QStringLiteral("tag_verb*")
            << QStringLiteral("tag_mathmode") << QStringLiteral("tag_mathmode_latex") << QStringLiteral("tag_equation") << QStringLiteral("tag_subscript") << QStringLiteral("tag_superscript")
            << QStringLiteral("tag_sqrt") << QStringLiteral("tag_nroot") << QStringLiteral("tag_left") << QStringLiteral("tag_right") << QStringLiteral("tag_leftright")
            << QStringLiteral("tag_bigl") << QStringLiteral("tag_bigr") << QStringLiteral("tag_Bigl") << QStringLiteral("tag_Bigr")
            << QStringLiteral("tag_biggl") << QStringLiteral("tag_biggr") << QStringLiteral("tag_Biggl") << QStringLiteral("tag_Biggr")
            << QStringLiteral("tag_text") << QStringLiteral("tag_intertext") << QStringLiteral("tag_boxed")
            << QStringLiteral("tag_frac") << QStringLiteral("tag_dfrac") << QStringLiteral("tag_tfrac")
            << QStringLiteral("tag_binom") << QStringLiteral("tag_dbinom") << QStringLiteral("tag_tbinom")
            << QStringLiteral("tag_xleftarrow") << QStringLiteral("tag_xrightarrow")
            << QStringLiteral("tag_mathrm") << QStringLiteral("tag_mathit") << QStringLiteral("tag_mathbf") << QStringLiteral("tag_mathsf")
            << QStringLiteral("tag_mathtt") << QStringLiteral("tag_mathcal") << QStringLiteral("tag_mathbb") << QStringLiteral("tag_mathfrak")
            << QStringLiteral("tag_acute") << QStringLiteral("tag_grave") << QStringLiteral("tag_tilde") << QStringLiteral("tag_bar") << QStringLiteral("tag_vec")
            << QStringLiteral("tag_hat") << QStringLiteral("tag_check") << QStringLiteral("tag_breve") << QStringLiteral("tag_dot") << QStringLiteral("tag_ddot")
            << QStringLiteral("tag_space_small") << QStringLiteral("tag_space_medium") << QStringLiteral("tag_space_large")
            << QStringLiteral("tag_quad") << QStringLiteral("tag_qquad") << QStringLiteral("tag_enskip")
            << QStringLiteral("tag_env_math") << QStringLiteral("tag_env_displaymath") << QStringLiteral("tag_env_equation") << QStringLiteral("tag_env_equation*")
            << QStringLiteral("tag_env_array")
            << QStringLiteral("tag_env_multline") << QStringLiteral("tag_env_multline*") << QStringLiteral("tag_env_split")
            << QStringLiteral("tag_env_gather") << QStringLiteral("tag_env_gather*") << QStringLiteral("tag_env_align") << QStringLiteral("tag_env_align*")
            << QStringLiteral("tag_env_flalign") << QStringLiteral("tag_env_flalign*") << QStringLiteral("tag_env_alignat") << QStringLiteral("tag_env_alignat*")
            << QStringLiteral("tag_env_aligned") << QStringLiteral("tag_env_gathered") << QStringLiteral("tag_env_alignedat") << QStringLiteral("tag_env_cases")
            << QStringLiteral("tag_env_dmath") << QStringLiteral("tag_env_dmath*") << QStringLiteral("tag_env_dseries") << QStringLiteral("tag_env_dseries*") << QStringLiteral("tag_env_dgroup") << QStringLiteral("tag_env_dgroup*")
            << QStringLiteral("tag_env_matrix") << QStringLiteral("tag_env_pmatrix") << QStringLiteral("tag_env_vmatrix")
            << QStringLiteral("tag_env_VVmatrix") << QStringLiteral("tag_env_bmatrix") << QStringLiteral("tag_env_BBmatrix")
            // bibliography stuff
            << QStringLiteral("menu_bibliography")
            << QStringLiteral("setting_bibtex") << QStringLiteral("setting_biblatex")
            << QStringLiteral("tag_textit") << QStringLiteral("tag_textsl") << QStringLiteral("tag_textbf") << QStringLiteral("tag_underline")
            << QStringLiteral("tag_texttt") << QStringLiteral("tag_textsc") << QStringLiteral("tag_emph") << QStringLiteral("tag_strong")
            << QStringLiteral("tag_rmfamily") << QStringLiteral("tag_sffamily") << QStringLiteral("tag_ttfamily")
            << QStringLiteral("tag_mdseries") << QStringLiteral("tag_bfseries") << QStringLiteral("tag_upshape")
            << QStringLiteral("tag_itshape") << QStringLiteral("tag_slshape") << QStringLiteral("tag_scshape")
            << QStringLiteral("tag_newline") << QStringLiteral("tag_newpage") << QStringLiteral("tag_linebreak") << QStringLiteral("tag_pagebreak")
            << QStringLiteral("tag_bigskip") << QStringLiteral("tag_medskip") << QStringLiteral("tag_smallskip")
            << QStringLiteral("tag_hspace") << QStringLiteral("tag_hspace*") << QStringLiteral("tag_vspace") << QStringLiteral("tag_vspace*")
            << QStringLiteral("tag_hfill") << QStringLiteral("tag_hrulefill") << QStringLiteral("tag_dotfill") << QStringLiteral("tag_vfill")
            << QStringLiteral("tag_includegraphics") << QStringLiteral("tag_include") << QStringLiteral("tag_input")
            // wizard
            << QStringLiteral("wizard_tabular") << QStringLiteral("wizard_array") << QStringLiteral("wizard_tabbing")
            << QStringLiteral("wizard_float") << QStringLiteral("wizard_mathenv")
            << QStringLiteral("wizard_usermenu") << QStringLiteral("wizard_usermenu2")
            // settings
            << QStringLiteral("Mode")
            // help
            << QStringLiteral("help_context")
            // action lists
            << QStringLiteral("structure_list") << QStringLiteral("size_list") << QStringLiteral("other_list")
            << QStringLiteral("left_list") << QStringLiteral("right_list")
            // tool lists
            << QStringLiteral("list_compiler_select") << QStringLiteral("list_convert_select") << QStringLiteral("list_view_select") << QStringLiteral("list_quick_select")
            // user help
            << QStringLiteral("help_userhelp")
            << QStringLiteral("edit_next_bullet") << QStringLiteral("edit_prev_bullet")
            << QStringLiteral("edit_next_section") << QStringLiteral("edit_prev_section") << QStringLiteral("edit_next_paragraph") << QStringLiteral("edit_prev_paragraph")

            << QStringLiteral("edit_select_inside_env") << QStringLiteral("edit_select_outside_env") << QStringLiteral("edit_select_inside_group")
            << QStringLiteral("edit_select_outside_group") << QStringLiteral("edit_select_mathgroup") << QStringLiteral("edit_select_paragraph")
            << QStringLiteral("edit_select_line") << QStringLiteral("edit_select_word")

            << QStringLiteral("edit_delete_inside_env") << QStringLiteral("edit_delete_outside_env") << QStringLiteral("edit_delete_inside_group")
            << QStringLiteral("edit_delete_outside_group") << QStringLiteral("edit_delete_mathgroup") << QStringLiteral("edit_delete_paragraph")
            << QStringLiteral("edit_delete_eol") << QStringLiteral("edit_delete_word")

            << QStringLiteral("edit_complete_word") << QStringLiteral("edit_complete_env") << QStringLiteral("edit_complete_abbrev")

            << QStringLiteral("edit_begin_env") << QStringLiteral("edit_end_env") << QStringLiteral("edit_match_env") << QStringLiteral("edit_close_env") << QStringLiteral("edit_closeall_env")

            << QStringLiteral("edit_begin_group") << QStringLiteral("edit_end_group") << QStringLiteral("edit_match_group") << QStringLiteral("edit_close_group")

            << QStringLiteral("file_export_ascii") << QStringLiteral("file_export_latin1") << QStringLiteral("file_export_latin2") << QStringLiteral("file_export_latin3")
            << QStringLiteral("file_export_latin4") << QStringLiteral("file_export_latin5") << QStringLiteral("file_export_latin9") << QStringLiteral("file_export_cp1250")
            << QStringLiteral("file_export_cp1252")
            ;

    setMenuItems(projectlist,m_dictMenuProject);
    setMenuItems(filelist,m_dictMenuFile);
    setMenuItems(actionlist,m_dictMenuAction);
}

void Kile::setMenuItems(const QStringList &list, QMap<QString,bool> &dict)
{
    for(const QString& entry : list) {
        dict[entry] = true;
    }
}

void Kile::updateMenu()
{
    KILE_DEBUG_MAIN << "==Kile::updateMenu()====================" << Qt::endl;

    // update project menus
    m_actRecentProjects->setEnabled( m_actRecentProjects->items().count() > 0 );
    bool project_open = ( docManager()->isProjectOpen() ) ;

    for(auto [key, value] : m_dictMenuProject.asKeyValueRange()) {
        QAction *a = actionCollection()->action(key);
        if(a) {
            a->setEnabled(project_open);
        }
    }

    // project_show is only enabled, when more than 1 project is opened
    QAction *a = actionCollection()->action(QStringLiteral("project_show"));
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
    if (menu->objectName() == QStringLiteral("usermenu-submenu")) {
        menu->setEnabled(true);
        return true;
    }

    bool enabled = false;
    const QList<QAction*> actionList = menu->actions();

    for(QAction *action : actionList) {
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
    insertTag(data, QStringList(QStringLiteral("amsmath")));
}

void Kile::insertTag(const KileAction::TagData& data,const QList<Package> &pkgs) {

    QStringList packages;

    for(const Package& pkg : pkgs) {
        if(!pkg.name.isEmpty()) {
            packages.append(pkg.name);
        }
    }

    insertTag(data,packages);
}

void Kile::insertTag(const KileAction::TagData& data,const QStringList &pkgs)
{
    KILE_DEBUG_MAIN << "void Kile::insertTag(const KileAction::TagData& data,const QStringList " << pkgs.join(QStringLiteral(",")) << ")" << Qt::endl;
    insertTag(data);

    KileDocument::TextInfo *docinfo = docManager()->textInfoFor(getCompileName());
    if(docinfo) {
        QStringList packagelist = allPackages(docinfo);
        QStringList warnPkgs;

        for(const QString& pkg : pkgs) {
            if(!packagelist.contains(pkg)) {
                warnPkgs.append(pkg);
            }
        }

        if(warnPkgs.count() > 0) {
            if(warnPkgs.count() == 1) {
                errorHandler()->printMessage(KileTool::Error, i18n("You have to include the package %1.", warnPkgs.join(QStringLiteral(","))), i18n("Insert text"));
            }
            else {
                errorHandler()->printMessage(KileTool::Error, i18n("You have to include the packages %1.", warnPkgs.join(QStringLiteral(","))), i18n("Insert text"));
            }
        }
    }
}

void Kile::insertText(const QString &text)
{
    if(text.indexOf(QStringLiteral("%C")) >= 0)
        insertTag(KileAction::TagData(QString(), text, QString(), 0, 0));
    else
        insertTag(KileAction::TagData(QString(), text, QStringLiteral("%C"), 0, 0));
}

void Kile::insertText(const QString &text, const QStringList &pkgs)
{
    insertTag(KileAction::TagData(QString(), text, QStringLiteral("%C"), 0, 0), pkgs);
}

void Kile::insertText(const QString &text, const QList<Package> &pkgs)
{
    insertTag(KileAction::TagData(QString(), text, QStringLiteral("%C"), 0, 0), pkgs);
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
        KConfigGroup group = m_config->group(QStringLiteral("Wizard"));
        env = group.readEntry("TabularEnvironment", "tabular");
    } else {
        env = QStringLiteral("array");
    }

    KileDialog::NewTabularDialog dlg(env, m_latexCommands, m_config.data(), this);
    if(dlg.exec()) {
        insertTag(dlg.tagData(), dlg.requiredPackages());
        if(tabularenv) {
            KConfigGroup group = m_config->group(QStringLiteral("Wizard"));
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
    QString xmldir = KileUtilities::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/usermenu/");
    // create dir if not existing
    QDir testDir(xmldir);
    if (!testDir.exists()) {
        testDir.mkpath(xmldir);
    }

    KConfigGroup userGroup = m_config->group(QStringLiteral("User"));
    int len = userGroup.readEntry("nUserTags", 0);

    if ( len > 0) {
        QString usertagfile = QStringLiteral("usertags.xml");
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
            const QString tagNameConfigKey = QStringLiteral("userTagName") + QString::number(i);
            const QString tagname = userGroup.readEntry(tagNameConfigKey, i18n("No Name"));
            const QString tagConfigKey = QStringLiteral("userTag") + QString::number(i);
            QString tag = userGroup.readEntry(tagConfigKey, "");
            tag = tag.replace(QLatin1Char('\n'), QStringLiteral("\\n"));

            xml.writeStartElement("menu");
            xml.writeAttribute("type", "text");
            xml.writeTextElement(KileMenu::UserMenuData::xmlMenuTagName(KileMenu::UserMenuData::XML_TITLE), tagname);
            xml.writeTextElement(KileMenu::UserMenuData::xmlMenuTagName(KileMenu::UserMenuData::XML_PLAINTEXT), tag);
            xml.writeTextElement(KileMenu::UserMenuData::xmlMenuTagName(KileMenu::UserMenuData::XML_SHORTCUT), QStringLiteral("Ctrl+Shift+%1").arg(i+1));
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
    if (m_config->hasGroup(QStringLiteral("Editor"))) {
        m_config->deleteGroup(QStringLiteral("Editor"));
    }

    //convert user tools to new KileTool classes
    KConfigGroup userGroup = m_config->group(QStringLiteral("User"));
    userItem tempItem;
    int len = userGroup.readEntry("nUserTools", 0);
    for (int i=0; i< len; ++i) {
        tempItem.name = userGroup.readEntry(QStringLiteral("userToolName") + QString::number(i), i18n("no name"));
        tempItem.tag = userGroup.readEntry(QStringLiteral("userTool") + QString::number(i), QString());
        m_listUserTools.append(tempItem);
    }
    if(len > 0) {
        //move the tools
        userGroup.writeEntry("nUserTools", 0);
        for(int i = 0; i < len; ++i) {
            tempItem = m_listUserTools[i];
            KConfigGroup toolsGroup = m_config->group(QStringLiteral("Tools"));
            toolsGroup.writeEntry(tempItem.name, "Default");

            KileTool::setGUIOptions(tempItem.name, QStringLiteral("Other"), QStringLiteral("preferences-other"), m_config.data());

            KConfigGroup group = m_config->group(KileTool::groupFor(tempItem.name, QStringLiteral("Default")));
            QString bin = KIO::DesktopExecParser::executablePath(tempItem.tag);
            group.writeEntry("command", bin);
            group.writeEntry("options", tempItem.tag.mid(bin.length()));
            group.writeEntry("class", "Base");
            group.writeEntry("type", "Process");
            group.writeEntry("from", "");
            group.writeEntry("to", "");

            if(i < 10) {
                QAction *toolAction = static_cast<QAction*>(actionCollection()->action(QStringLiteral("tool_") + tempItem.name));
                actionCollection()->setDefaultShortcut(toolAction, QString(QStringLiteral("Alt+Shift+") + QString::number(i + 1))); //should be alt+shift+
            }
        }
    }
}

void Kile::readRecentFileSettings()
{
    KConfigGroup group = m_config->group(QStringLiteral("FilesOpenOnStart"));
    int n = group.readEntry("NoDOOS", 0);
    for (int i = 0; i < n; ++i) {
        const QString urlString = group.readPathEntry(QStringLiteral("DocsOpenOnStart") + QString::number(i), QString());
        if(urlString.isEmpty()) {
            continue;
        }
        m_listDocsOpenOnStart.append(urlString);
        m_listEncodingsOfDocsOpenOnStart.append(group.readPathEntry(QStringLiteral("EncodingsOfDocsOpenOnStart") + QString::number(i), QString()));
    }

    n = group.readEntry("NoPOOS", 0);
    for(int i = 0; i < n; ++i) {
        const QString urlString = group.readPathEntry(QStringLiteral("ProjectsOpenOnStart") + QString::number(i), QString());
        if(!urlString.isEmpty()) {
            m_listProjectsOpenOnStart.append(urlString);
        }
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
    m_actRecentFiles->saveEntries(m_config->group(QStringLiteral("Recent Files")));
    m_actRecentProjects->saveEntries(m_config->group(QStringLiteral("Projects")));

    m_config->deleteGroup(QStringLiteral("FilesOpenOnStart"));
    if (KileConfig::restore())
    {
        KConfigGroup configGroup = m_config->group(QStringLiteral("FilesOpenOnStart"));
        KileConfig::setSingleFileMasterDocument(getMasterDocumentFileName());
        configGroup.writeEntry("NoDOOS", m_listDocsOpenOnStart.count());
        for (int i = 0; i < m_listDocsOpenOnStart.count(); ++i) {
            configGroup.writePathEntry(QStringLiteral("DocsOpenOnStart") + QString::number(i), m_listDocsOpenOnStart[i]);
            configGroup.writePathEntry(QStringLiteral("EncodingsOfDocsOpenOnStart") + QString::number(i), m_listEncodingsOfDocsOpenOnStart[i]);
        }

        configGroup.writeEntry("NoPOOS", m_listProjectsOpenOnStart.count());
        for (int i = 0; i < m_listProjectsOpenOnStart.count(); ++i) {
            configGroup.writePathEntry(QStringLiteral("ProjectsOpenOnStart") + QString::number(i), m_listProjectsOpenOnStart[i]);
        }
    }

    KConfigGroup configGroup = KSharedConfig::openConfig()->group(QStringLiteral("KileMainWindow"));
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
    Q_EMIT masterDocumentChanged();
    KILE_DEBUG_MAIN << "SETTING master to " << m_masterDocumentFileName << " singlemode = " << m_singlemode << Qt::endl;
}

void Kile::clearMasterDocument()
{
    ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
    ModeAction->setChecked(false);
    m_singlemode = true;
    m_masterDocumentFileName.clear();
    updateModeStatus();
    Q_EMIT masterDocumentChanged();
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
        KConfigGroup configGroup = KSharedConfig::openConfig()->group(QStringLiteral("KileMainWindow"));
        saveMainWindowSettings(configGroup);
    }

    KEditToolBar dlg(factory());
    connect(&dlg, &KEditToolBar::newToolBarConfig, this, [this] () {
        setUpdatesEnabled(false);
        applyMainWindowSettings(m_config->group(QStringLiteral("KileMainWindow")));

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

    QRegExp reOptional(QStringLiteral("(ALT|OPT)(\\w+)\\s*=\\s*(\\S.*)"));
    QRegExp reNonEmptyEntry(QStringLiteral(".*\\w.*"));

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
                type.append(QStringLiteral(" = "));
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
        if(j < view->document()->lines() && view->document()->line(j).contains(QRegularExpression(QStringLiteral("^\\s*\\}\\s*$")))) {
            s =  view->document()->line(i);
            view->document()->removeLine(i);
            s.remove(QRegularExpression(QStringLiteral(",\\s*$")));
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
        insertTag(dialog->getTemplate(), QStringLiteral("%C"), 0,0);
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
                             i18nc("@info:status status bar label for block selection mode", "BLOCK") + QLatin1Char(' '):
                             i18nc("@info:status status bar label for line selection mode", "LINE") + QLatin1Char(' ');
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
