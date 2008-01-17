/****************************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)
                           (C) 2007 Holger Danielsson (holger.danielsson@versanet.de)
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

#include <qtooltip.h>
#include <qpointer.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QHideEvent>
#include <Q3CString>
#include <Q3PopupMenu>

#include <QSplashScreen>

#include <kaction.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include "kiledebug.h"
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <krecentfilesaction.h>
#include <krun.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kstandarddirs.h>
#include <kmultitabbar.h>
#include <ktabwidget.h>
#include <ktip.h>
#include <ktexteditor/configinterface.h>
#include <kxmlguifactory.h>

#include "documentinfo.h"
#include "kileactions.h"
#include "kilestdactions.h"
#include "usermenudialog.h"
#include "dialogs/configurationdialog.h"
#include "kileproject.h"
#include "widgets/projectview.h"
#include "dialogs/projectdialogs.h"
#include "kilelistselector.h"
#include "kilelyxserver.h"
#include "dialogs/findfilesdialog.h"
#include "kiletool_enums.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "widgets/logwidget.h"
#include "widgets/outputview.h"
#include "widgets/konsolewidget.h"
#include "dialogs/quickdocumentdialog.h"
#include "tabbingdialog.h"
#include "widgets/structurewidget.h"
#include "convert.h"
#include "dialogs/includegraphicsdialog.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "kileeventfilter.h"
#include "kileconfig.h"
#include "kileerrorhandler.h"
#include "dialogs/configcheckerdialog.h"
#include "kilesidebar.h"
#include "dialogs/floatdialog.h"
#include "mathenvdialog.h"
#include "tabulardialog.h"
#include "postscriptdialog.h"
#include "latexcmd.h"
#include "kileuntitled.h"
#include "dialogs/statisticsdialog.h"
#include "widgets/scriptsmanagementwidget.h"
#include "scriptmanager.h"
#include "widgets/previewwidget.h"
#include "widgets/extendedscrollarea.h"

/*
 * Class KileMainWindow.
 */

class KileMainWindow : public KXmlGuiWindow
{
	public:
		KileMainWindow(Kile *kile, QWidget *parent = 0, Qt::WindowFlags f = KDE_DEFAULT_WINDOWFLAGS);
		virtual ~KileMainWindow();

	protected:
		virtual bool queryExit();
		virtual bool queryClose();

	private:
		Kile *m_kile;
};

KileMainWindow::KileMainWindow(Kile *kile, QWidget *parent, Qt::WindowFlags f) : KXmlGuiWindow(parent, f), m_kile(kile)
{
}

KileMainWindow::~KileMainWindow()
{
}

bool KileMainWindow::queryExit()
{
	return m_kile->queryExit();
}

bool KileMainWindow::queryClose()
{
	return m_kile->queryClose();
}

/*
 * Class Kile.
 */

Kile::Kile( bool allowRestore, QWidget *parent, const char *name ) :
	KApplication(),
	KileInfo(this),
	m_paPrint(NULL)
{
	m_mainWindow = new KileMainWindow(this);
	m_focusWidget = NULL;

	QSplashScreen splashScreen(QPixmap(KGlobal::dirs()->findResource("appdata", "pics/kile_splash.png")), Qt::WindowStaysOnTopHint);
	splashScreen.show();

	m_config = KGlobal::config();
	readUserSettings();
	readRecentFileSettings();

	m_jScriptManager = new KileScript::Manager(this, m_config.data(), actionCollection(), parent, "KileScript::Manager");

	m_mainWindow->setStandardToolBarMenuEnabled(true);

	m_masterName = KileConfig::master();
	m_singlemode = (m_masterName.isEmpty());

	m_AutosaveTimer = new QTimer();
	connect(m_AutosaveTimer,SIGNAL(timeout()),this,SLOT(autoSaveAll()));

	m_latexCommands = new KileDocument::LatexCommands(m_config.data(), this);  // at first (dani)
	m_edit = new KileDocument::EditorExtension(this);
	m_help = new KileHelp::Help(m_edit, m_mainWindow);
	m_partManager = new KParts::PartManager(m_mainWindow);
	m_eventFilter = new KileEventFilter(m_edit);
	m_errorHandler = new KileErrorHandler(this, this);
	m_quickPreview = new KileTool::QuickPreview(this);
	m_extensions = new KileDocument::Extensions();

	connect( m_partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(activePartGUI ( KParts::Part * ) ) );
	connect(this,SIGNAL(configChanged()), m_eventFilter, SLOT(readConfig()));

	readGUISettings();

	KGlobal::dirs()->addResourceType("app_symbols",KStandardDirs::kde_default("data") + "kile/mathsymbols/"); // needed for Symbolview

	// do initializations first
	m_currentState = m_wantState = "Editor";
	m_bWatchFile = m_logPresent = false;

	viewManager()->setClient(this, m_mainWindow);

	setupStatusBar();

	m_topWidgetStack = new QStackedWidget(m_mainWindow);
	m_topWidgetStack->setFocusPolicy(Qt::NoFocus);

	m_horizontalSplitter = new QSplitter(Qt::Horizontal, m_mainWindow);

	setupSideBar();
	m_horizontalSplitter->addWidget(m_sideBar);

	m_verticalSplitter = new QSplitter(Qt::Vertical);
	m_horizontalSplitter->addWidget(m_verticalSplitter);
	QWidget *tabWidget = viewManager()->createTabs(m_verticalSplitter);
	m_verticalSplitter->addWidget(tabWidget);

	connect(viewManager(), SIGNAL(activateView(QWidget*, bool)), this, SLOT(activateView(QWidget*, bool)));
	connect(viewManager(), SIGNAL(prepareForPart(const QString& )), this, SLOT(prepareForPart(const QString& )));
	connect(viewManager(), SIGNAL(startQuickPreview(int)), this, SLOT(slotQuickPreview(int)) );

	setupBottomBar();
	m_verticalSplitter->addWidget(m_bottomBar);
	setupGraphicTools();
	setupPreviewTools();
	setupActions();
	setupTools();

	QList<int> sizes;
	sizes << m_verSplitTop << m_verSplitBottom;
	m_verticalSplitter->setSizes(sizes);
	sizes.clear();
	sizes << m_horSplitLeft << m_horSplitRight;
	m_horizontalSplitter->setSizes(sizes);
	if (!KileConfig::bottomBar()) {
#ifdef __GNUC__
#warning Disabling this for now.
#endif
// 		showFullScreen();
		m_bottomBar->setDirectionalSize(KileConfig::bottomBarSize());
	}

	m_topWidgetStack->addWidget(m_horizontalSplitter);
	m_mainWindow->setCentralWidget(m_topWidgetStack);
	newCaption();

	m_partManager->setActivePart( 0L );

	m_lyxserver = new KileLyxServer(KileConfig::runLyxServer());
	connect(m_lyxserver, SIGNAL(insert(const KileAction::TagData &)), this, SLOT(insertTag(const KileAction::TagData &)));

	m_mainWindow->applyMainWindowSettings(m_config->group("KileMainWindow"));

	m_manager = new KileTool::Manager(this, m_config.data(), m_logWidget, m_outputWidget, m_partManager, m_topWidgetStack, m_paStop, 10000); //FIXME make timeout configurable
	connect(m_manager, SIGNAL(requestGUIState(const QString &)), this, SLOT(prepareForPart(const QString &)));
	connect(m_manager, SIGNAL(requestSaveAll(bool, bool)), docManager(), SLOT(fileSaveAll(bool, bool)));
	connect(m_manager, SIGNAL(jumpToFirstError()), m_errorHandler, SLOT(jumpToFirstError()));
	connect(m_manager, SIGNAL(toolStarted()), m_errorHandler, SLOT(reset()));
	connect(m_manager, SIGNAL(previewDone()), this, SLOT(focusPreview()));

	m_toolFactory = new KileTool::Factory(m_manager, m_config.data());
	m_manager->setFactory(m_toolFactory);
	m_help->setUserhelp(m_manager,menuBar());     // kile user help (dani)

	connect(docManager(), SIGNAL(updateModeStatus()), this, SLOT(updateModeStatus()));
	connect(docManager(), SIGNAL(updateStructure(bool, KileDocument::Info*)), viewManager(), SLOT(updateStructure(bool, KileDocument::Info*)));
	connect(docManager(), SIGNAL(closingDocument(KileDocument::Info* )), m_kwStructure, SLOT(closeDocumentInfo(KileDocument::Info *)));
	connect(docManager(), SIGNAL(documentInfoCreated(KileDocument::Info* )), m_kwStructure, SLOT(addDocumentInfo(KileDocument::Info* )));
	connect(docManager(), SIGNAL(updateReferences(KileDocument::Info *)), m_kwStructure, SLOT(updateReferences(KileDocument::Info *)));

	readConfig();

	m_mainWindow->resize(KileConfig::mainwindowWidth(), KileConfig::mainwindowHeight());
	m_mainWindow->show();
	splashScreen.finish(m_mainWindow);

	if(m_listUserTools.count() > 0) {
		KMessageBox::information(0, i18n("You have defined some tools in the User menu. From now on these tools will be available from the Build->Other menu and can be configured in the configuration dialog (go to the Settings menu and choose Configure Kile). This has some advantages; your own tools can now be used in a QuickBuild command if you wish."), i18n("User Tools Detected"));
		m_listUserTools.clear();
	}

	KTipDialog::showTip(m_mainWindow, "kile/tips");

	restoreFilesAndProjects(allowRestore);
	initMenu();
	updateModeStatus();

	KConfigGroup shortcutsGroup = m_config->group("Shortcuts");
	actionCollection()->readSettings(&shortcutsGroup);
}

Kile::~Kile()
{
	KILE_DEBUG() << "cleaning up..." << endl;
	// m_mainWindow is deleted automatically after it has been closed
	delete m_extensions;
	delete m_latexCommands;
	delete m_quickPreview;
	delete m_edit;
	delete m_help;
	delete m_AutosaveTimer;
	delete m_lyxserver; //QObject without parent, have to delete it ourselves
	delete m_outputInfo;
	delete m_outputFilter;
	delete m_eventFilter;
}

KActionCollection* Kile::actionCollection()
{
	return m_mainWindow->actionCollection();
}

KMenuBar* Kile::menuBar()
{
	return m_mainWindow->menuBar();
}

KToolBar* Kile::toolBar(const QString &name)
{
	return m_mainWindow->toolBar(name);
}

KStatusBar* Kile::statusBar()
{
	return m_mainWindow->statusBar();
}

QAction* Kile::action(const char* name) const
{
	return m_mainWindow->action(name);
}

void Kile::plugActionList(const QString& name, const QList<QAction*>& actionList)
{
	m_mainWindow->plugActionList(name, actionList);
}

void Kile::unplugActionList(const QString& name)
{
	m_mainWindow->unplugActionList(name);
}

void Kile::showEvent(QShowEvent *)
{
#ifdef __GNUC__
#warning Check whether this is still needed!
#endif
	if(m_focusWidget) {
		m_focusWidget->setFocus();
	}
}

void Kile::hideEvent(QHideEvent *)
{
	m_focusWidget = focusWidget();
}

void Kile::setupStatusBar()
{
    statusBar()->removeItem(ID_LINE_COLUMN);
    statusBar()->removeItem(ID_HINTTEXT);

	statusBar()->insertPermanentItem(i18n("Line: 1 Col: 1"), ID_LINE_COLUMN, 0);
	statusBar()->setItemAlignment(ID_LINE_COLUMN, Qt::AlignLeft | Qt::AlignVCenter);
	statusBar()->insertItem(i18n("Normal Mode"), ID_HINTTEXT,10);
	statusBar()->setItemAlignment(ID_HINTTEXT, Qt::AlignLeft | Qt::AlignVCenter);
}

void Kile::setupSideBar()
{
	m_sideBar = new KileSideBar(m_horizontalSplitter);

	m_fileBrowserWidget = new KileWidget::FileBrowserWidget(m_extensions, m_sideBar, "File Selector");
	m_sideBar->addPage(m_fileBrowserWidget, SmallIcon("document-open"), i18n("Open File"));
	connect(m_fileBrowserWidget,SIGNAL(fileSelected(const KFileItem&)), docManager(), SLOT(fileSelected(const KFileItem&)));
	connect(m_fileBrowserWidget->comboEncoding(), SIGNAL(activated(int)),this,SLOT(changeInputEncoding()));
	m_fileBrowserWidget->comboEncoding()->lineEdit()->setText(KileConfig::defaultEncoding());
	m_fileBrowserWidget->readConfig();

	setupProjectView();
	setupStructureView();
	setupSymbolViews();
	setupScriptsManagementView();
	setupAbbreviationView();

	m_sideBar->switchToTab(KileConfig::selectedLeftView());
	m_sideBar->setVisible(KileConfig::sideBar());
	m_sideBar->setDirectionalSize(KileConfig::sideBarSize());
}

void Kile::setupProjectView()
{
	KileWidget::ProjectView *projectview = new KileWidget::ProjectView(m_sideBar, this);
// 	viewManager()->setProjectView(projectview);
	m_sideBar->addPage(projectview, SmallIcon("relation"), i18n("Files and Projects"));
	connect(projectview, SIGNAL(fileSelected(const KileProjectItem *)), docManager(), SLOT(fileSelected(const KileProjectItem *)));
	connect(projectview, SIGNAL(fileSelected(const KUrl &)), docManager(), SLOT(fileSelected(const KUrl &)));
	connect(projectview, SIGNAL(closeURL(const KUrl&)), docManager(), SLOT(fileClose(const KUrl&)));
	connect(projectview, SIGNAL(closeProject(const KUrl&)), docManager(), SLOT(projectClose(const KUrl&)));
	connect(projectview, SIGNAL(projectOptions(const KUrl&)), docManager(), SLOT(projectOptions(const KUrl&)));
	connect(projectview, SIGNAL(projectArchive(const KUrl&)), this, SLOT(runArchiveTool(const KUrl&)));
	connect(projectview, SIGNAL(removeFromProject(const KileProjectItem *)), docManager(), SLOT(removeFromProject(const KileProjectItem *)));
	connect(projectview, SIGNAL(addFiles(const KUrl &)), docManager(), SLOT(projectAddFiles(const KUrl &)));
	connect(projectview, SIGNAL(openAllFiles(const KUrl &)), docManager(), SLOT(projectOpenAllFiles(const KUrl &)));
	connect(projectview, SIGNAL(toggleArchive(KileProjectItem *)), docManager(), SLOT(toggleArchive(KileProjectItem *)));
	connect(projectview, SIGNAL(addToProject(const KUrl &)), docManager(), SLOT(addToProject(const KUrl &)));
	connect(projectview, SIGNAL(saveURL(const KUrl &)), docManager(), SLOT(saveURL(const KUrl &)));
	connect(projectview, SIGNAL(buildProjectTree(const KUrl &)), docManager(), SLOT(buildProjectTree(const KUrl &)));
	connect(docManager(), SIGNAL(projectTreeChanged(const KileProject *)), projectview, SLOT(refreshProjectTree(const KileProject *)));
	connect(docManager(), SIGNAL(removeFromProjectView(const KUrl &)),projectview,SLOT(remove(const KUrl &)));
	connect(docManager(), SIGNAL(removeFromProjectView(const KileProject *)),projectview,SLOT(remove(const KileProject *)));
	connect(docManager(), SIGNAL(addToProjectView(const KUrl &)),projectview,SLOT(add(const KUrl &)));
	connect(docManager(), SIGNAL(addToProjectView(const KileProject *)),projectview,SLOT(add(const KileProject *)));
	connect(docManager(),SIGNAL(removeItemFromProjectView(const KileProjectItem *, bool)),projectview,SLOT(removeItem(const KileProjectItem *, bool)));
	connect(docManager(),SIGNAL(addToProjectView(KileProjectItem *)),projectview,SLOT(add(KileProjectItem *)));
}

void Kile::setupStructureView()
{
	m_kwStructure = new KileWidget::Structure(this, m_sideBar);
	m_sideBar->addPage(m_kwStructure, SmallIcon("view-list-tree"), i18n("Structure"));
	m_kwStructure->setFocusPolicy(Qt::ClickFocus);
	connect(this, SIGNAL(configChanged()), m_kwStructure, SIGNAL(configChanged()));
	connect(m_kwStructure, SIGNAL(setCursor(const KUrl &,int,int)), this, SLOT(setCursor(const KUrl &,int,int)));
	connect(m_kwStructure, SIGNAL(fileOpen(const KUrl&, const QString & )), docManager(), SLOT(fileOpen(const KUrl&, const QString& )));
	connect(m_kwStructure, SIGNAL(fileNew(const KUrl&)), docManager(), SLOT(fileNew(const KUrl&)));
	connect(m_kwStructure, SIGNAL(sendText(const QString &)), this, SLOT(insertText(const QString &)));
	connect(m_kwStructure, SIGNAL(sectioningPopup(KileListViewItem *,int)), m_edit, SLOT(sectioningCommand(KileListViewItem *,int)));
}

void Kile::setupScriptsManagementView()
{
	m_scriptsManagementWidget = new KileWidget::ScriptsManagement(this, m_sideBar);
	m_sideBar->addPage(m_scriptsManagementWidget, SmallIcon("preferences-plugin-script"), i18n("Scripts"));
}

void Kile::enableSymbolViewMFUS()
{
	m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),true);
	m_toolBox->setItemToolTip(m_toolBox->indexOf(m_symbolViewMFUS),
			i18n("Move the mouse over an icon to see the corresponding LaTeX command.\n \
			Click on an icon to insert the command, additionally pressing Qt::SHIFT inserts it in math mode,\
			pressing Qt::CTRL in curly brackets."));

	connect(m_symbolViewRelation,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewOperators,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewArrows,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewMiscMath,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewMiscText,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewDelimiters,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewGreek,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewSpecial,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewCyrillic,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
	connect(m_symbolViewUser,SIGNAL(addToList(const QListWidgetItem *)),m_symbolViewMFUS,SLOT(slotAddToList(const QListWidgetItem *)));
}

void Kile::disableSymbolViewMFUS()
{
	m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),false);
	m_toolBox->setItemToolTip(m_toolBox->indexOf(m_symbolViewMFUS),QString());
	disconnect(m_symbolViewMFUS,SIGNAL(addtoList(const Q3IconViewItem *)));
}



void Kile::setupSymbolViews()
{
	m_toolBox = new QToolBox(m_sideBar);
	m_sideBar->addPage(m_toolBox,SmallIcon("math0"),i18n("Symbols"));

	m_symbolViewMFUS = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::MFUS);
	m_toolBox->addItem(m_symbolViewMFUS,i18n("Most Frequently Used"));
	m_toolBox->setItemEnabled(m_toolBox->indexOf(m_symbolViewMFUS),false);
	connect(m_symbolViewMFUS, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString& ,const QStringList&)));
	
	m_symbolViewRelation = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Relation);
	m_toolBox->addItem(m_symbolViewRelation,SmallIcon("math1"),i18n("Relation"));
	connect(m_symbolViewRelation, SIGNAL(insertText(const QString& ,const QStringList&)),
		 this, SLOT(insertText(const QString& ,const QStringList&)));
		
	m_symbolViewOperators = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Operator);
	m_toolBox->addItem(m_symbolViewOperators,SmallIcon("math2"),i18n("Operators"));
	connect(m_symbolViewOperators, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString& ,const QStringList&)));

	m_symbolViewArrows = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Arrow);
	m_toolBox->addItem(m_symbolViewArrows,SmallIcon("math3"),i18n("Arrows"));
	connect(m_symbolViewArrows, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString& ,const QStringList&)));

	m_symbolViewMiscMath = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::MiscMath);
	m_toolBox->addItem(m_symbolViewMiscMath,SmallIcon("math4"),i18n("Miscellaneous Math"));
	connect(m_symbolViewMiscMath, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString& ,const QStringList&)));

	m_symbolViewMiscText = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::MiscText);
	m_toolBox->addItem(m_symbolViewMiscText,SmallIcon("math5"),i18n("Miscellaneous Text"));
	connect(m_symbolViewMiscText, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString& ,const QStringList&)));

	m_symbolViewDelimiters= new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Delimiters);
	m_toolBox->addItem(m_symbolViewDelimiters,SmallIcon("math6"),i18n("Delimiters"));
	connect(m_symbolViewDelimiters, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString& ,const QStringList&)));

	m_symbolViewGreek = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Greek);
	m_toolBox->addItem(m_symbolViewGreek,SmallIcon("math7"),i18n("Greek"));
	connect(m_symbolViewGreek, SIGNAL(insertText(const QString&, const QStringList&)),
		this, SLOT(insertText(const QString&, const QStringList&)));

	m_symbolViewSpecial = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Special);
	m_toolBox->addItem(m_symbolViewSpecial,SmallIcon("math8"),i18n("Special Characters"));
	connect(m_symbolViewSpecial, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString&, const QStringList&)));

	m_symbolViewCyrillic = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::Cyrillic);
	m_toolBox->addItem(m_symbolViewCyrillic,SmallIcon("math10"),i18n("Cyrillic Characters"));
	connect(m_symbolViewCyrillic, SIGNAL(insertText(const QString& ,const QStringList&)),
		this, SLOT(insertText(const QString&, const QStringList&)));

	m_symbolViewUser = new KileWidget::SymbolView(m_toolBox, KileWidget::SymbolView::User);
	m_toolBox->addItem(m_symbolViewUser,SmallIcon("math9"),i18n("User Defined"));
	connect(m_symbolViewUser, SIGNAL(insertText(const QString&, const QStringList&)),
		this, SLOT(insertText(const QString&, const QStringList& )));

	for(int i = 0; i < m_toolBox->count(); ++i) {
		m_toolBox->setItemToolTip(i, i18n("Move the mouse over the icons to see the corresponding LaTeX commands.\nClick on the images to insert the command, additionally pressing SHIFT inserts it in math mode, pressing CTRL in curly brackets."));
	}
}

void Kile::setupAbbreviationView()
{
	m_kileAbbrevView = new KileWidget::AbbreviationView(m_sideBar);
	m_edit->complete()->setAbbreviationListview(m_kileAbbrevView);
	m_sideBar->addPage(m_kileAbbrevView, SmallIcon("complete3"), i18n("Abbreviation"));

	connect(m_kileAbbrevView, SIGNAL(sendText(const QString& )), this, SLOT(insertText(const QString& )));
}

void Kile::setupBottomBar()
{
	m_bottomBar = new KileBottomBar(m_mainWindow);
	m_bottomBar->setFocusPolicy(Qt::ClickFocus);

	m_logWidget = new KileWidget::LogWidget(this, m_mainWindow);
	connect(m_logWidget, SIGNAL(showingErrorMessage(QWidget* )), m_bottomBar, SLOT(showPage(QWidget* )));
	connect(m_logWidget, SIGNAL(fileOpen(const KUrl&, const QString & )), docManager(), SLOT(fileOpen(const KUrl&, const QString& )));
	connect(m_logWidget, SIGNAL(setLine(const QString& )), this, SLOT(setLine(const QString& )));
	connect(m_docManager,SIGNAL(printMsg(int, const QString &, const QString &)),m_logWidget,SLOT(printMsg(int, const QString &, const QString &)));

	m_logWidget->setFocusPolicy(Qt::ClickFocus);
	m_logWidget->setMinimumHeight(40);
	m_logWidget->setReadOnly(true);
	m_bottomBar->addPage(m_logWidget, SmallIcon("utilities-log-viewer"), i18n("Log and Messages"));

	m_outputWidget = new KileWidget::OutputView(m_mainWindow);
	m_outputWidget->setFocusPolicy(Qt::ClickFocus);
	m_outputWidget->setMinimumHeight(40);
	m_outputWidget->setReadOnly(true);
	m_bottomBar->addPage(m_outputWidget, SmallIcon("output_win"), i18n("Output"));

	m_outputInfo=new LatexOutputInfoArray();
	m_outputFilter=new LatexOutputFilter(m_outputInfo,m_extensions);
	connect(m_outputFilter, SIGNAL(problem(int, const QString& )), m_logWidget, SLOT(printProblem(int, const QString& )));

	m_texKonsole = new KileWidget::Konsole(this, m_mainWindow);
	m_bottomBar->addPage(m_texKonsole, SmallIcon("utilities-terminal"),i18n("Konsole"));
	connect(viewManager()->tabs(), SIGNAL(currentChanged(QWidget*)), m_texKonsole, SLOT(sync()));

	m_previewScrollArea = new KileWidget::ExtendedScrollArea(m_mainWindow);
	m_previewWidget = new KileWidget::PreviewWidget(this, m_previewScrollArea);
	m_previewScrollArea->setBackgroundColor(QColor(Qt::white));
	m_previewScrollArea->setWidget(m_previewWidget);
	m_bottomBar->addPage(m_previewScrollArea, SmallIcon ("document-preview"), i18n ("Preview"));

	m_bottomBar->setVisible(true);
	m_bottomBar->setDirectionalSize(KileConfig::bottomBarSize());
}

void Kile::setupGraphicTools()
{
	KileConfig::setImagemagick(!(KStandardDirs::findExe("identify").isNull()));
}

void Kile::setupPreviewTools()
{
	// search for tools
	bool dvipng = !(KStandardDirs::findExe("dvipng").isNull());
	bool convert = !(KStandardDirs::findExe("convert").isNull());

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

KAction* Kile::createAction(const QString &text, const QString &name, const QObject *receiver, const char *member)
{
	return createAction(text, name, QString(), KShortcut(), receiver, member);
}

KAction* Kile::createAction(const QString &text, const QString &name, const KShortcut& shortcut, const QObject *receiver, const char *member)
{
	return createAction(text, name, QString(), shortcut, receiver, member);
}

KAction* Kile::createAction(const QString &text, const QString &name, const QString& iconName, const QObject *receiver, const char *member) 
{
	return createAction(text, name, iconName, KShortcut(), receiver, member);
}

KAction* Kile::createAction(const QString &text, const QString &name, const QString& iconName, const KShortcut& shortcut, const QObject *receiver, const char *member)
{
	KAction *action = actionCollection()->addAction(name, receiver, member);
	action->setText(text);
	if(!shortcut.isEmpty()) {
		action->setShortcut(shortcut);
	}
	if(!iconName.isEmpty()) {
		action->setIcon(KIcon(iconName));
	}
	return action;
}

KAction* Kile::createAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member)
{
	return actionCollection()->addAction(actionType, name, receiver, member);
}
void Kile::setupActions()
{
	m_paPrint = createAction(KStandardAction::Print, "file_print", NULL, NULL);
	createAction(KStandardAction::New, "file_new", docManager(), SLOT(fileNew()));
	createAction(KStandardAction::Open, "file_open", docManager(), SLOT(fileOpen()));

	m_actRecentFiles = static_cast<KRecentFilesAction*>(actionCollection()->addAction(KStandardAction::OpenRecent, "file_open_recent", docManager(), SLOT(fileOpen(const KUrl&))));
	connect(docManager(), SIGNAL(addToRecentFiles(const KUrl& )), m_actRecentFiles, SLOT(addUrl(const KUrl&)));
	m_actRecentFiles->loadEntries(m_config->group("Recent Files"));

	createAction(KStandardAction::Save, "kile_file_save", docManager(), SLOT(fileSave()));
	createAction(KStandardAction::SaveAs, "kile_file_save_as", docManager(), SLOT(fileSaveAs()));

	createAction(i18n("Save All"), "file_save_all", "document-save-all", docManager(), SLOT(fileSaveAll()));
	createAction(i18n("Save Copy As..."), "file_save_copy_as", "save_copy_as", docManager(), SLOT(fileSaveCopyAs()));
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
	createAction(KStandardAction::Quit, "file_quit", m_mainWindow, SLOT(close()));

	createAction(KStandardAction::GotoLine, "edit_goto_line", m_edit, SLOT(gotoLine()));
	createAction(i18n("Next section"), "edit_next_section", "nextsection", KShortcut(Qt::ALT + Qt::Key_Down), m_edit, SLOT(gotoNextSectioning()));
	createAction(i18n("Prev section"), "edit_prev_section", "prevsection", KShortcut(Qt::ALT + Qt::Key_Up), m_edit, SLOT(gotoPrevSectioning()));
	createAction(i18n("Next paragraph"), "edit_next_paragraph", "nextparagraph", KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_Down), m_edit, SLOT(gotoNextParagraph()));
	createAction(i18n("Prev paragraph"), "edit_prev_paragraph", "prevparagraph", KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_Up), m_edit, SLOT(gotoPrevParagraph()));

	createAction(i18n("Find &in Files..."), "FindInFiles", "filegrep", KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_F), this, SLOT(findInFiles()));

	createAction(i18n("Refresh Str&ucture"), "RefreshStructure", "refreshstructure", KShortcut(Qt::Key_F12), this, SLOT(refreshStructure()));

	//project actions
	createAction(i18n("&New Project..."), "project_new", "window_new", docManager(), SLOT(projectNew()));
	createAction(i18n("&Open Project..."), "project_open", "project-open", docManager(), SLOT(projectOpen()));

	m_actRecentProjects = new KRecentFilesAction(i18n("Open &Recent Project"), actionCollection());
	actionCollection()->addAction("project_openrecent", m_actRecentProjects);
	connect(m_actRecentProjects, SIGNAL(triggered()), docManager(), SLOT(projectOpen(const KUrl&)));
	connect(docManager(), SIGNAL(removeFromRecentProjects(const KUrl& )), m_actRecentProjects, SLOT(removeURL(const KUrl& )));
	connect(docManager(), SIGNAL(addToRecentProjects(const KUrl& )), m_actRecentProjects, SLOT(addUrl(const KUrl& )));
	m_actRecentProjects->loadEntries(m_config->group("Projects"));

	createAction(i18n("A&dd Files to Project..."), "project_add", "project_add", docManager(), SLOT(projectAddFiles()));
	createAction(i18n("Refresh Project &Tree"), "project_buildtree", "rebuild", docManager(), SLOT(buildProjectTree()));
 	createAction(i18n("&Archive"), "package", "project_archive", this, SLOT(runArchiveTool()));
	createAction(i18n("Project &Options"), "project_options", "configure_project", docManager(), SLOT(projectOptions()));
	createAction(i18n("&Close Project"), "project_close", "window-close", docManager(), SLOT(projectClose()));

	// new project actions (dani)
	createAction(i18n("&Show Projects..."), "project_show", docManager(), SLOT(projectShow()));
	createAction(i18n("Re&move Files From Project..."), "project_remove", "project_remove", docManager(), SLOT(projectRemoveFiles()));
	createAction(i18n("Show Project &Files..."), "project_showfiles", "project_show", docManager(), SLOT(projectShowFiles()));
	// tbraun
	createAction(i18n("Open All &Project Files"), "project_openallfiles", docManager(), SLOT(projectOpenAllFiles()));
	createAction(i18n("Find in &Project..."), "project_findfiles", "projectgrep", this, SLOT(findInProjects()));

	//build actions
	createAction(i18n("Clean"),"CleanAll", "user-trash", this, SLOT(cleanAll()));
	createAction(i18n("View Log File"), "ViewLog", "viewlog", KShortcut(Qt::ALT + Qt::Key_0), m_errorHandler, SLOT(ViewLog()));
	createAction(i18n("Previous LaTeX Error"), "PreviousError", "errorprev", m_errorHandler, SLOT(PreviousError()));
	createAction(i18n("Next LaTeX Error"), "NextError", "errornext", m_errorHandler, SLOT(NextError()));
	createAction(i18n("Previous LaTeX Warning"), "PreviousWarning", "warnprev", m_errorHandler, SLOT(PreviousWarning()));
	createAction(i18n("Next LaTeX Warning"), "NextWarning", "warnnext", m_errorHandler, SLOT(NextWarning()));
	createAction(i18n("Previous LaTeX BadBox"), "PreviousBadBox", "bboxprev", m_errorHandler, SLOT(PreviousBadBox()));
	createAction(i18n("Next LaTeX BadBox"), "NextBadBox", "bboxnext", m_errorHandler, SLOT(NextBadBox()));
	m_paStop = createAction(i18n("&Stop"),"Stop", "process-stop", KShortcut(Qt::Key_Escape), NULL, NULL);
	m_paStop->setEnabled(false);

	createAction(i18n("Editor View"), "EditorView", "edit", KShortcut(Qt::CTRL + Qt::Key_E), this, SLOT(showEditorWidget()));
	createAction(i18n("Next Document"), "gotoNextDocument", "arrow-right", KShortcut(Qt::ALT + Qt::Key_Right), viewManager(), SLOT(gotoNextView()));
	createAction(i18n("Previous Document"), "gotoPrevDocument", "arrow-left", KShortcut(Qt::ALT + Qt::Key_Left), viewManager(), SLOT(gotoPrevView()));
	createAction(i18n("Focus Log/Messages View"), "focus_log", KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_M), this, SLOT(focusLog()));
	createAction(i18n("Focus Output View"), "focus_output", KShortcut(Qt::CTRL + Qt::ALT+ Qt::Key_O), this, SLOT(focusOutput()));
	createAction(i18n("Focus Konsole View"), "focus_konsole", KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_K), this, SLOT(focusKonsole()));
	createAction(i18n("Focus Editor View"), "focus_editor", KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_E), this, SLOT(focusEditor()));

 // CodeCompletion (dani)
	createAction(i18n("(La)TeX Command"), "edit_complete_word", "complete1", KShortcut(Qt::CTRL + Qt::Key_Space), m_edit, SLOT(completeWord()));
	createAction(i18n("Environment"), "edit_complete_env", "complete2", KShortcut(Qt::ALT + Qt::Key_Space), m_edit, SLOT(completeEnvironment()));
	createAction(i18n("Abbreviation"), "edit_complete_abbrev", "complete3", KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_Space), m_edit, SLOT(completeAbbreviation()));

	createAction(i18n("Next Bullet"), "edit_next_bullet", "nextbullet", KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_Right), m_edit, SLOT(nextBullet()));
	createAction(i18n("Prev Bullet"), "edit_prev_bullet", "prevbullet", KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_Left), m_edit, SLOT(prevBullet()));

 // advanced editor (dani)
	createAction(i18n("Environment (inside)"), "edit_select_inside_env", "selenv_i", KShortcut("CTRL+Alt+S,E"), m_edit, SLOT(selectEnvInside()));
	createAction(i18n("Environment (outside)"), "edit_select_outside_env", "selenv_o", KShortcut("CTRL+Alt+S,F"), m_edit, SLOT(selectEnvOutside()));
	createAction(i18n("TeX Group (inside)"), "edit_select_inside_group", "selgroup_i", KShortcut("CTRL+Alt+S,T"), m_edit, SLOT(selectTexgroupInside()));
	createAction(i18n("TeX Group (outside)"), "edit_select_outside_group", "selgroup_o", KShortcut("CTRL+Alt+S,U"),m_edit, SLOT(selectTexgroupOutside()));
	createAction(i18n("Math Group"), "edit_select_mathgroup", "selmath", KShortcut("CTRL+Alt+S,M"), m_edit, SLOT(selectMathgroup()));
	createAction(i18n("Paragraph"), "edit_select_paragraph", "selpar", KShortcut("CTRL+Alt+S,P"), m_edit, SLOT(selectParagraph()));
	createAction(i18n("Line"), "edit_select_line", "selline", KShortcut("CTRL+Alt+S,L"), m_edit, SLOT(selectLine()));
	createAction(i18n("TeX Word"), "edit_select_word", "selword", KShortcut("CTRL+Alt+S,W"), m_edit, SLOT(selectWord()));

	createAction(i18n("Environment (inside)"), "edit_delete_inside_env", "delenv_i", KShortcut("CTRL+Alt+T,E"), m_edit, SLOT(deleteEnvInside()));
	createAction(i18n("Environment (outside)"), "edit_delete_outside_env", "delenv_o", KShortcut("CTRL+Alt+T,F"), m_edit, SLOT(deleteEnvOutside()));
	createAction(i18n("TeX Group (inside)"), "edit_delete_inside_group", "delgroup_i", KShortcut("CTRL+Alt+T,T"), m_edit, SLOT(deleteTexgroupInside()));
	createAction(i18n("TeX Group (outside)"), "edit_delete_outside_group", "delgroup_o",KShortcut("CTRL+Alt+T,U"),m_edit, SLOT(deleteTexgroupInside()));
	createAction(i18n("Math Group"), "edit_delete_mathgroup", "delmath", KShortcut("CTRL+Alt+T,M"), m_edit, SLOT(deleteMathgroup()));
	createAction(i18n("Paragraph"), "edit_delete_paragraph", "delpar", KShortcut("CTRL+Alt+T,P"), m_edit, SLOT(deleteParagraph()));
	createAction(i18n("To End of Line"), "edit_delete_eol", "deleol", KShortcut("CTRL+Alt+T,I"), m_edit, SLOT(deleteEndOfLine()));
	createAction(i18n("TeX Word"), "edit_delete_word", "delword", KShortcut("CTRL+Alt+T,W"), m_edit, SLOT(deleteWord()));

	createAction(i18n("Goto Begin"), "edit_begin_env", "gotobeginenv", KShortcut("CTRL+Alt+E,B"), m_edit, SLOT(gotoBeginEnv()));
	createAction(i18n("Goto End"), "edit_end_env", "gotoendenv", KShortcut("CTRL+Alt+E,E"), m_edit, SLOT(gotoEndEnv()));
	createAction(i18n("Match"), "edit_match_env", "matchenv", KShortcut("CTRL+Alt+E,M"), m_edit, SLOT(matchEnv()));
	createAction(i18n("Close"), "edit_close_env", "closeenv", KShortcut("CTRL+Alt+E,C"), m_edit, SLOT(closeEnv()));
	createAction(i18n("Close All"), "edit_closeall_env", "closeallenv", KShortcut("CTRL+Alt+E,A"), m_edit, SLOT(closeAllEnv()));

	createAction(i18n("Goto Begin"), "edit_begin_group", "gotobegingroup", KShortcut("CTRL+Alt+G,B"), m_edit, SLOT(gotoBeginTexgroup()));
	createAction(i18n("Goto End"), "edit_end_group", "gotoendgroup", KShortcut("CTRL+Alt+G,E"), m_edit, SLOT(gotoEndTexgroup()));
	createAction(i18n("Match"), "edit_match_group", "matchgroup", KShortcut("CTRL+Alt+G,M"), m_edit, SLOT(matchTexgroup()));
	createAction(i18n("Close"), "edit_close_group", "closegroup", KShortcut("CTRL+Alt+G,C"), m_edit, SLOT(closeTexgroup()));

	createAction(i18n("Selection"), "quickpreview_selection", "preview_sel", KShortcut("CTRL+Alt+P,S"), this, SLOT(quickPreviewSelection()));
	createAction(i18n("Environment"), "quickpreview_environment", "preview_env",KShortcut("CTRL+Alt+P,E"), this, SLOT(quickPreviewEnvironment()));
	createAction(i18n("Subdocument"), "quickpreview_subdocument", "preview_subdoc",KShortcut("CTRL+Alt+P,D"), this, SLOT(quickPreviewSubdocument()));
	createAction(i18n ("Mathgroup"), "quickpreview_math", "edu_mathematics", KShortcut("CTRL+Alt+P,M"), this, SLOT(quickPreviewMathgroup()));

	KileStdActions::setupStdTags(this, this, actionCollection(), m_mainWindow);
	KileStdActions::setupMathTags(this, actionCollection());
	KileStdActions::setupBibTags(this, actionCollection());

	createAction(i18n("Quick Start"), "wizard_document", "quickwizard", this, SLOT(quickDocument()));
	connect(docManager(), SIGNAL(startWizard()), this, SLOT(quickDocument()));
	createAction(i18n("Tabular"), "wizard_tabular", "wizard_tabular", this, SLOT(quickTabular()));
	createAction(i18n("Array"), "wizard_array", "wizard_array", this, SLOT(quickArray()));
	createAction(i18n("Tabbing"), "wizard_tabbing", "wizard_tabbing", this, SLOT(quickTabbing()));
	createAction(i18n("Floats"), "wizard_float", "wizard_float", this, SLOT(quickFloat()));
	createAction(i18n("Math"), "wizard_mathenv", "wizard_math", this, SLOT(quickMathenv()));
	createAction(i18n("Postscript Tools"), "wizard_postscript", "wizard_pstools", this, SLOT(quickPostscript()));

	createAction(i18n("Clean"), "CleanBib", this, SLOT(cleanBib()));

	ModeAction = new KToggleAction(i18n("Define Current Document as '&Master Document'"), actionCollection());
	actionCollection()->addAction("Mode", ModeAction);
	ModeAction->setIcon(KIcon("master"));
	connect(ModeAction, SIGNAL(triggered()), this, SLOT(toggleMode()));

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
	WatchFileAction->setIcon(KIcon("watchfile"));
	connect(WatchFileAction, SIGNAL(triggered()), this, SLOT(toggleWatchFile()));
	if(m_bWatchFile) {
		WatchFileAction->setChecked(true);
	}
	else {
		WatchFileAction->setChecked(false);
	}

	m_mainWindow->setHelpMenuEnabled(false);
	const KAboutData *aboutData = KGlobal::mainComponent().aboutData();
	KHelpMenu *help_menu = new KHelpMenu(m_mainWindow, aboutData);

	actionCollection()->addAction(KStandardAction::TipofDay, "help_tipofday", this, SLOT(showTip()));

	createAction(i18n("TeX Guide"), "help_tex_guide", KShortcut("CTRL+Alt+H,G"), m_help, SLOT(helpTexGuide()));
	createAction(i18n("LaTeX"), "help_latex_index", KShortcut("CTRL+Alt+H,L"), m_help, SLOT(helpLatexIndex()));
	createAction(i18n("LaTeX Command"), "help_latex_command", KShortcut("CTRL+Alt+H,C"), m_help, SLOT(helpLatexCommand()));
	createAction(i18n("LaTeX Subject"), "help_latex_subject", KShortcut("CTRL+Alt+H,S"), m_help, SLOT(helpLatexSubject()));
	createAction(i18n("LaTeX Env"), "help_latex_env", KShortcut("CTRL+Alt+H,E"), m_help, SLOT(helpLatexEnvironment()));
	createAction(i18n("Context Help"), "help_context", KShortcut("CTRL+Alt+H,K"), m_help, SLOT(helpKeyword()));
	createAction(i18n("Documentation Browser"), "help_docbrowser", KShortcut("CTRL+Alt+H,B"), m_help, SLOT(helpDocBrowser()));

	createAction(i18n("LaTeX Reference"), "help_latex_reference", "help-contents", this, SLOT(helpLaTex()));
	actionCollection()->addAction(KStandardAction::HelpContents, "help_handbook", help_menu, SLOT(appHelpActivated()));
	actionCollection()->addAction(KStandardAction::ReportBug, "report_bug", help_menu, SLOT(reportBug()));
	actionCollection()->addAction(KStandardAction::AboutApp, "help_aboutKile", help_menu, SLOT(aboutApplication()));
	actionCollection()->addAction(KStandardAction::AboutKDE, "help_aboutKDE", help_menu, SLOT(aboutKDE()));

	KAction *kileconfig = actionCollection()->addAction(KStandardAction::Preferences, "settings_configure", this, SLOT(generalOptions()));
	kileconfig->setIcon(KIcon("configure-kile"));

	actionCollection()->addAction(KStandardAction::KeyBindings, "settings_keys", this, SLOT(configureKeys()));
	actionCollection()->addAction(KStandardAction::ConfigureToolbars, "settings_toolbars", this, SLOT(configureToolbars()));

	createAction(i18n("&System Check..."), "settings_perform_check", this, SLOT(slotPerformCheck()));

#ifdef __GNUC__
#warning m_menuUserTags still needs to be ported!
#endif
//FIXME: port for KDE4
/*
	m_menuUserTags = new KActionMenu(i18n("User Tags"), SmallIcon("label"), actionCollection(),"menuUserTags");
	m_menuUserTags->setDelayed(false);
	setupUserTagActions();
*/
	actionCollection()->readSettings();

	m_pFullScreen = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()), m_mainWindow, actionCollection());
}

void Kile::setupTools()
{
	KILE_DEBUG() << "==Kile::setupTools()===================" << endl;
	QStringList tools = KileTool::toolList(m_config.data());
	QString toolMenu;
	QList<QAction*> *pl;

	unplugActionList("list_compilers");
	unplugActionList("list_converters");
	unplugActionList("list_quickies");
	unplugActionList("list_viewers");
	unplugActionList("list_other");

	for (int i = 0; i < tools.count(); ++i) {
		QString grp = KileTool::groupFor(tools[i], m_config.data());
		KILE_DEBUG() << tools[i] << " is using group: " << grp << endl;
		toolMenu = KileTool::menuFor(tools[i], m_config.data());

		if ( toolMenu == "none" ) continue;

		if ( toolMenu == "Compile" )
			pl = &m_listCompilerActions;
		else if ( toolMenu == "View" )
			pl = &m_listViewerActions;
		else if ( toolMenu == "Convert" )
			pl = &m_listConverterActions;
		else if ( toolMenu == "Quick" )
			pl = &m_listQuickActions;
		else
			pl = &m_listOtherActions;

		KILE_DEBUG() << "\tadding " << tools[i] << " " << toolMenu << " #" << pl->count() << endl;

		if (action(QString("tool_" + tools[i]).ascii()) == NULL) {
			KAction *act = createAction(tools[i], QString("tool_"+tools[i]).ascii(), KileTool::iconFor(tools[i], m_config.data()), this, SLOT(runTool()));
			pl->append(act);
		}
	}

	cleanUpActionList(m_listCompilerActions, tools);
	cleanUpActionList(m_listViewerActions, tools);
	cleanUpActionList(m_listConverterActions, tools);
	cleanUpActionList(m_listQuickActions, tools);
	cleanUpActionList(m_listOtherActions, tools);

	plugActionList("list_compilers", static_cast<QList<QAction*> >(m_listCompilerActions));
	plugActionList("list_viewers", m_listViewerActions);
	plugActionList("list_converters", m_listConverterActions);
	plugActionList("list_quickies", m_listQuickActions);
	plugActionList("list_other", m_listOtherActions);

	KConfigGroup shortcutsGroup = m_config->group("Shortcuts");
	actionCollection()->readSettings(&shortcutsGroup);
}

void Kile::cleanUpActionList(QList<QAction*> &list, const QStringList &tools)
{
	for (QList<QAction*>::iterator act = list.begin(); act != list.end(); ++act) {
		if (action((*act)->name()) != NULL && !tools.contains(QString((*act)->name()).mid(5))) {
			list.remove(act);
			if ((*act)->associatedWidgets().contains(toolBar("toolsToolBar"))) {
				toolBar("toolsToolBar")->removeAction(*act);
			}
		}
	}
}

void Kile::setupUserTagActions()
{
#ifdef __GNUC__
#warning Need to port the setupUserTagActions() method!
#endif
//FIXME: port for KDE4
/*
	KShortcut tagaccels[10] = {KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_1),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_2),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_3),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_4),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_5),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_6),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_7),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_8),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_9),
				   KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_0)};

	m_actionEditTag = new KAction(i18n("Edit User Tags..."),0 , this, SLOT(editUserMenu()), m_menuUserTags,"EditUserMenu" );

	m_menuUserTags->insert(m_actionEditTag);
	if ( m_listUserTags.size() > 0 )  {
		m_actionEditSeparator = new KActionSeparator();
		m_menuUserTags->insert(m_actionEditSeparator);
	}
	for (uint i=0; i<m_listUserTags.size(); ++i)
	{
		KShortcut sc; if (i<10)  { sc = tagaccels[i]; } else { sc = 0; }
		QString name = QString::number(i+1)+": "+m_listUserTags[i].text;
		KileAction::Tag *menuItem = new KileAction::Tag(name, sc, this, SLOT(insertTag(const KileAction::TagData &)), actionCollection(), QString("tag_user_" + m_listUserTags[i].text).ascii(), m_listUserTags[i]);
		m_listUserTagsActions.append(menuItem);
		m_menuUserTags->insert(menuItem);
	}

	actionCollection()->readSettings(m_config->group("Shortcuts"));
*/
}

void Kile::restoreFilesAndProjects(bool allowRestore)
{
	if (! (allowRestore && KileConfig::restore()) )
	  return;

	QFileInfo fi;

	KUrl url;
	for (int i=0; i < m_listProjectsOpenOnStart.count(); ++i) {
		fi.setFile(m_listProjectsOpenOnStart[i]);
		// don't open project files as they will be opened later in this method 
		if (fi.isReadable()) docManager()->projectOpen(KUrl::fromPathOrUrl(m_listProjectsOpenOnStart[i]), i, m_listProjectsOpenOnStart.count(), false);
	}

	for (int i = 0; i < m_listDocsOpenOnStart.count(); ++i) {
		fi.setFile(m_listDocsOpenOnStart[i]);
		if (fi.isReadable())
			docManager()->fileOpen(KUrl::fromPathOrUrl(m_listDocsOpenOnStart[i]));
	}

	if (ModeAction) ModeAction->setChecked(!m_singlemode);
	updateModeStatus();

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();

    KILE_DEBUG() << "lastDocument=" << KileConfig::lastDocument() << endl;
	KTextEditor::Document *doc = docManager()->docFor(KUrl::fromPathOrUrl(KileConfig::lastDocument()));
	if (doc) viewManager()->switchToTextView(doc->url(), true); // request the focus on the view
}

void Kile::setActive()
{
	KILE_DEBUG() << "ACTIVATING" << endl;
	kapp->mainWidget()->raise();
	kapp->mainWidget()->setActiveWindow();
}

void Kile::showTip()
{
    KTipDialog::showTip(m_mainWindow, "kile/tips", true);
}

void Kile::setLine(const QString &line)
{
	bool ok;
	uint l=line.toUInt(&ok,10);
	KTextEditor::View *view = viewManager()->currentTextView();
	if (view && ok) {
		m_mainWindow->show();
		m_mainWindow->raise();
		view->setFocus();
#ifdef __GNUC__
#warning Introduce a generic gotoLine function!
#endif
		KTextEditor::Cursor cursor = view->cursorPosition();
		cursor.setPosition(l-1, 0);
		view->setCursorPosition(cursor);

		showEditorWidget();
		newStatus();
  	}
}

void Kile::setCursor(const KUrl &url, int parag, int index)
{
	KTextEditor::Document *doc = docManager()->docFor(url);
	if (doc)
	{
		KTextEditor::View *view = (KTextEditor::View*)doc->views().first();
		if (view)
		{
			view->setCursorPosition(KTextEditor::Cursor(parag, index));
			view->setFocus();
		}
	}
}

void Kile::runArchiveTool()
{
	this->run("Archive");
}

void Kile::runArchiveTool(const KUrl &url)
{
	KileTool::Archive *tool = new KileTool::Archive("Archive", m_manager, false);
	tool->setSource(url.path());
	tool->prepareToRun();
	m_manager->run(tool);
}


int Kile::run(const QString & tool)
{
	return m_manager->runBlocking(tool);
}

int Kile::runWith(const QString &tool, const QString &config)
{
	return m_manager->runBlocking(tool, config);
}

//TODO: move to KileView::Manager
void Kile::activateView(QWidget* w, bool updateStruct /* = true */ )  //Needs to be QWidget because of QTabWidget::currentChanged
{
	//KILE_DEBUG() << "==Kile::activateView==========================" << endl;
	if (!w->inherits("KTextEditor::View")) {
		return;
	}

	//disable gui updates to avoid flickering of toolbars
	m_mainWindow->setUpdatesEnabled(false);
	KTextEditor::View* view = (KTextEditor::View*)w;

#ifdef __GNUC__
#warning Commenting Kate::View->isActive() out!
#endif
//FIXME:
// 	if (view->isActive()) return;

	for(int i = 0; i< viewManager()->textViews().count(); ++i) {
		m_mainWindow->guiFactory()->removeClient(viewManager()->textView(i));
#ifdef __GNUC__
#warning Commenting Kate::View->setActive() out!
#endif
//FIXME:
// 		viewManager()->textView(i)->setActive(false);
	}

	m_mainWindow->guiFactory()->addClient(view);

#ifdef __GNUC__
#warning Commenting Kate::View->isActive() out!
#endif
//FIXME:
// 	view->setActive( true );

	// remove menu entry to config Kate
	checkKateSettings();

	m_mainWindow->setUpdatesEnabled(true);

	if(updateStruct) {
		viewManager()->updateStructure();
	}
}

void Kile::updateModeStatus()
{
	KILE_DEBUG() << "==Kile::updateModeStatus()==========" << endl;
	KileProject *project = docManager()->activeProject();
	QString shortName = m_masterName;
	int pos = shortName.findRev('/');
	shortName.remove(0,pos+1);

	if (project)
	{
		if (m_singlemode)
			statusBar()->changeItem(i18n("Project: %1", project->name()), ID_HINTTEXT);
		else
			statusBar()->changeItem(i18n("Project: %1 (Master document: %2)", project->name(), shortName), ID_HINTTEXT);
	}
	else
	{
		if (m_singlemode)
			statusBar()->changeItem(i18n("Normal mode"), ID_HINTTEXT);
		else
			statusBar()->changeItem(i18n("Master document: %1", shortName), ID_HINTTEXT);
	}

	if (m_singlemode)
	{
		ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
		ModeAction->setChecked(false);
	}
	else
	{
		ModeAction->setText(i18n("Normal mode (current master document: %1)", shortName));
		ModeAction->setChecked(true);
	}

	// enable or disable entries in Kile'S menu
	updateMenu();
}

void Kile::openDocument(const QString & url)
{
	KILE_DEBUG() << "==Kile::openDocument(" << url << ")==========" << endl;
	docManager()->fileSelected(KUrl::fromPathOrUrl(url));
}

void Kile::closeDocument()
{
	docManager()->fileClose();
}

void Kile::autoSaveAll()
{
	docManager()->fileSaveAll(true);
}

void Kile::enableAutosave(bool as)
{
	if (as)
	{
		//paranoia pays, we're really screwed if somehow autosaveinterval equals zero
		int interval = KileConfig::autosaveInterval();
		if ( interval < 1 || interval > 99 ) interval = 10;
		m_AutosaveTimer->start(interval * 60000);
	}
	else m_AutosaveTimer->stop();
}

void Kile::openProject(const QString& proj)
{
	docManager()->projectOpen(KUrl::fromPathOrUrl(proj));
}

void Kile::focusPreview()
{
	m_bottomBar->showPage(m_previewScrollArea);
}

void Kile::focusLog()
{
	m_bottomBar->showPage(m_logWidget);
}

void Kile::focusOutput()
{
	m_bottomBar->showPage(m_outputWidget);
}

void Kile::focusKonsole()
{
	m_bottomBar->showPage(m_texKonsole);
}

void Kile::focusEditor()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if (view) view->setFocus();
}

void Kile::sideOrBottomBarChanged(bool visible)
{
    if ( ! visible )
    {
        focusEditor();
    }
}

bool Kile::queryExit()
{
	saveSettings();
	return true;
}

bool Kile::queryClose()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		KileConfig::setLastDocument(view->document()->url().path());
	}
	else {
		KileConfig::setLastDocument("");
	}

	//don't close Kile if embedded viewers are present
	KILE_DEBUG() << "==bool Kile::queryClose(" << m_currentState << ")==========" << endl;
	if(m_currentState != "Editor") {
		resetPart();
		return false;
	}

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();

	for(int i = 0; i < viewManager()->textViews().count(); ++i) {
		m_listDocsOpenOnStart.append(viewManager()->textView(i)->document()->url().path());
	}

	KILE_DEBUG() << "#projects = " << docManager()->projects().count() << endl;
	QList<KileProject*> projectList = docManager()->projects();
	for(QList<KileProject*>::iterator i = projectList.begin(); i != projectList.end(); ++i) {
		m_listProjectsOpenOnStart.append((*i)->url().path());
	}

	bool stage1 = docManager()->projectCloseAll();
	bool stage2 = true;

	if(stage1) {
		stage2 = docManager()->fileCloseAll();
	}

	return stage1 && stage2;
}

void Kile::showDocInfo(KTextEditor::Document *doc)
{
	if (!doc) {
		KTextEditor::View *view = viewManager()->currentTextView();

		if (view)
			doc = view->document();
		else
			return;
	}

	KileDocument::TextInfo *docinfo = docManager()->textInfoFor(doc);
	KileProject *project = KileInfo::docManager()->activeProject();
	if(docinfo) { // we have to ensure that we always get a _valid_ docinfo object
		KileDialog::StatisticsDialog *dlg = new KileDialog::StatisticsDialog(project,docinfo, m_mainWindow, 0, "");
		dlg->exec();
		delete dlg;
	}
	else
		kWarning() << "There is no KileDocument::Info object belonging to this document!" << endl;
}

void Kile::convertToASCII(KTextEditor::Document *doc)
{
	if (doc == 0)
	{
		KTextEditor::View *view = viewManager()->currentTextView();

		if (view) doc = view->document();
		else return;
	}

	ConvertIO io(doc);
	ConvertEncToASCII conv = ConvertEncToASCII(doc->encoding(), &io);
	doc->setEncoding("ISO 8859-1");
	conv.convert();
}

void Kile::convertToEnc(KTextEditor::Document *doc)
{
	if (doc == 0)
	{
		KTextEditor::View *view = viewManager()->currentTextView();

		if (view) doc = view->document();
		else return;
	}

	if (sender())
	{
		ConvertIO io(doc);
		QString name = QString(sender()->name()).section('_', -1);
		ConvertASCIIToEnc conv = ConvertASCIIToEnc(name, &io);
		conv.convert();
		doc->setEncoding(ConvertMap::encodingNameFor(name));
	}
}

////////////////// GENERAL SLOTS //////////////
void Kile::newStatus(const QString & msg)
{
	statusBar()->changeItem(msg,ID_LINE_COLUMN);
}

int Kile::lineNumber()
{
	KTextEditor::View *view = viewManager()->currentTextView();

	int para = 0;

	if (view)
	{
		para = view->cursorPosition().line();
	}

	return para;
}

void Kile::newCaption()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		m_mainWindow->setCaption(getShortName( view->document()));
		if (m_bottomBar->currentPage()->inherits("KileWidget::Konsole")) {
			m_texKonsole->sync();
		}
	}
	else {
		m_mainWindow->setCaption("");
	}
}

void Kile::grepItemSelected(const QString &abs_filename, int line)
{
	KILE_DEBUG() << "Open file: "
		<< abs_filename << " (" << line << ")" << endl;
	docManager()->fileOpen(KUrl::fromPathOrUrl(abs_filename));
	setLine(QString::number(line));
}

void Kile::findInFiles()
{
	static QPointer<KileDialog::FindFilesDialog> dlg = 0;

	if (!dlg) {
		KILE_DEBUG() << "grep guard: create findInFiles dlg" << endl;
		dlg = new KileDialog::FindFilesDialog(mainWindow(), this, KileGrep::Directory);
		dlg->show();
		connect(dlg, SIGNAL(itemSelected(const QString &, int)),
		        this, SLOT(grepItemSelected(const QString &, int)));
	}
	else {
		KILE_DEBUG() << "grep guard: show findInFiles dlg" << endl;
		dlg->setActiveWindow();
		dlg->raise();
	}
}

void Kile::findInProjects()
{
	static QPointer<KileDialog::FindFilesDialog> project_dlg = NULL;

	if(!project_dlg) {
		KILE_DEBUG() << "grep guard: create findInProjects dlg" << endl;
		project_dlg = new KileDialog::FindFilesDialog(mainWindow(), this, KileGrep::Project);
		project_dlg->show();
		connect(project_dlg, SIGNAL(itemSelected(const QString &, int)),
		        this, SLOT(grepItemSelected(const QString &, int)));
	}
	else {
		KILE_DEBUG() << "grep guard: show findInProjects dlg" << endl;
		project_dlg->setActiveWindow();
		project_dlg->raise();
	}
}

/////////////////// PART & EDITOR WIDGET //////////
void Kile::showEditorWidget()
{
	if(!resetPart())
		return;
	m_mainWindow->setCentralWidget(m_topWidgetStack);
	m_topWidgetStack->show();
	m_horizontalSplitter->show();
	m_verticalSplitter->show();

	KTextEditor::View *view = viewManager()->currentTextView();
	if (view) view->setFocus();

	setupStatusBar();
	updateModeStatus();
	newCaption();
}


bool Kile::resetPart()
{
	KILE_DEBUG() << "==Kile::resetPart()=============================" << endl;
	KILE_DEBUG() << "\tcurrent state " << m_currentState << endl;
	KILE_DEBUG() << "\twant state " << m_wantState << endl;

	KParts::ReadOnlyPart *part = (KParts::ReadOnlyPart*)m_partManager->activePart();

	if (part && m_currentState != "Editor")
	{
		if(part->closeUrl())
		{
			m_partManager->removePart(part);
			m_topWidgetStack->removeWidget(part->widget());
			delete part;
		}
		else
			return false;
	}

    setupStatusBar();
    updateModeStatus();
    newCaption();

	m_currentState = "Editor";
	m_wantState = "Editor";
	//m_partManager->setActivePart( 0L);
	return true;
}

void Kile::activePartGUI(KParts::Part * part)
{
	KILE_DEBUG() << "==Kile::activePartGUI()=============================" << endl;
	KILE_DEBUG() << "\tcurrent state " << m_currentState << endl;
	KILE_DEBUG() << "\twant state " << m_wantState << endl;

	m_mainWindow->createGUI("kileui.rc");
	unplugActionList("list_quickies"); plugActionList("list_quickies", m_listQuickActions);
	unplugActionList("list_compilers"); plugActionList("list_compilers", m_listCompilerActions);
	unplugActionList("list_converters"); plugActionList("list_converters", m_listConverterActions);
	unplugActionList("list_viewers"); plugActionList("list_viewers", m_listViewerActions);
	unplugActionList("list_other"); plugActionList("list_other", m_listOtherActions);

	showToolBars(m_wantState);

	//manually plug the print action into the toolbar for
	//kghostview (which has the print action defined in
	//a KParts::BrowserExtension)
	KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject(part);
#ifdef __GNUC__
#warning metaObject()->slotNames() still needs to be ported!
#endif
//FIXME: port for KDE4
/*
	if (ext && ext->metaObject()->slotNames().contains( "print()" ) ) //part is a BrowserExtension, connect printAction()
	{
		connect(m_paPrint, SIGNAL(triggered()), ext, SLOT(print()));
		toolBar("mainToolBar")->addAction(m_paPrint); //plug this action into its default location
		m_paPrint->setEnabled(true);
	}
	else
	{
		if (m_paPrint->associatedWidgets().contains(toolBar("mainToolBar"))) {
			toolBar("mainToolBar")->removeAction(m_paPrint);
		}
		m_paPrint->setEnabled(false);
	}
*/
	//set the current state
	m_currentState = m_wantState;
	m_wantState = "Editor";
}

void Kile::showToolBars(const QString & wantState)
{
	// save state of all toolbars
	static bool mainToolBar = true;
	static bool buildToolBar = true;
	static bool errorToolBar = true;
	static bool toolsToolBar = true;
	static bool editToolBar = true;
	static bool mathToolBar = true;

	if(m_currentState == "Editor") {
		mainToolBar  = toolBar("mainToolBar")->isShown();
		buildToolBar = toolBar("buildToolBar")->isShown();
		errorToolBar = toolBar("errorToolBar")->isShown();
		toolsToolBar = toolBar("toolsToolBar")->isShown();
		editToolBar  = toolBar("editToolBar")->isShown();
		mathToolBar  = toolBar("mathToolBar")->isShown();
	}

	if(wantState == "HTMLpreview") {
		m_mainWindow->slotStateChanged( "HTMLpreview");
		setViewerToolBars();
		enableKileGUI(false);
	}
	else if(wantState == "Viewer") {
		m_mainWindow->slotStateChanged( "Viewer" );
		setViewerToolBars();
		enableKileGUI(false);
	}
	else {
		m_mainWindow->slotStateChanged( "Editor" );
		m_wantState="Editor";
		m_topWidgetStack->setCurrentIndex(0);
		if ( ! mainToolBar  ) toolBar("mainToolBar")->hide();
		if ( buildToolBar ) toolBar("buildToolBar")->show();
		if ( errorToolBar ) toolBar("errorToolBar")->show();
		if ( toolsToolBar ) toolBar("toolsToolBar")->show();
		if ( editToolBar  ) toolBar("editToolBar")->show();
		if ( mathToolBar  ) toolBar("mathToolBar")->show();
		toolBar("extraToolBar")->hide();
		enableKileGUI(true);
	}
}

void Kile::setViewerToolBars()
{
	toolBar("mainToolBar")->show();
	toolBar("buildToolBar")->hide();
	toolBar("errorToolBar")->hide();
	toolBar("toolsToolBar")->hide();
	toolBar("editToolBar")->hide();
	toolBar("mathToolBar")->hide();
	toolBar("extraToolBar")->show();
}

void Kile::enableKileGUI(bool enable)
{
	int id;
	QString text;

	QMenuBar *menubar = menuBar();
#ifdef __GNUC__
#warning Commenting out the popup menu stuff!
#endif
//FIXME: port for KDE4
/*
	for (uint i=0; i<menubar->count(); ++i) {
		id = menubar->idAt(i);
		Q3PopupMenu *popup = menubar->findItem(id)->popup();
		if ( popup ) {
			text = popup->name();
			if ( text == "menu_build"   ||
				  text == "menu_project" ||
				  text == "menu_latex"   ||
				  text == "wizard"       ||
				  text == "tools"
			   )
			   menubar->setItemEnabled(id, enable);
		}
	}
*/
	// enable or disable userhelp entries 
	m_help->enableUserhelpEntries(enable);
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
	   << "goto" << "complete" << "bullet" << "select"
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
	   << "WatchFile" << "ViewLog" << "PreviousError" << "NextError" << "PreviousWarning"
	   << "NextWarning" << "PreviousBadBox" << "NextBadBox" << "CleanAll"
	   // latex
	   << "tag_documentclass" << "tag_usepackage" << "tag_amspackages" << "tag_env_document"
	   << "tag_author" << "tag_title" << "tag_maketitle" << "tag_titlepage" << "tag_env_abstract"
	   << "tag_tableofcontents" << "tag_listoffigures" << "tag_listoftables"
	   << "tag_makeindex" << "tag_printindex" << "tag_makeglossary" << "tag_env_thebibliography"
	   << "tag_part" << "tag_chapter" << "tag_section" << "tag_subsection" << "tag_subsubsection"
	   << "tag_paragraph" << "tag_subparagraph" << "tag_label"
	   << "tag_ref" << "tag_pageref" << "tag_index" << "tag_footnote" << "tag_cite" << "citeViewBib"
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
	   << "tag_bibliographystyle" << "tag_bibliography" << "tag_bib_article" << "tag_bib_inproc"
	   << "tag_bib_incol" << "tag_bib_inbook" << "tag_bib_proceedings" << "tag_bib_book"
	   << "tag_bib_booklet" << "tag_bib_phdthesis" << "tag_bib_masterthesis" << "tag_bib_techreport"
	   << "tag_bib_manual" << "tag_bib_unpublished" << "tag_bib_misc" << "CleanBib"
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
	   << "menuUserTags"
	   // wizard
	   << "wizard_tabular" << "wizard_array" << "wizard_tabbing"
	   << "wizard_float" << "wizard_mathenv"
	   // settings
	   << "Mode" << "settings_keys"
	   // help
	   << "help_context"
	   // action lists
	   << "structure_list" << "size_list" << "other_list"
	   << "left_list" << "right_list"
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
	KILE_DEBUG() << "==Kile::updateKileMenu()====================" << endl;
	QAction *a;
	QMap<QString,bool>::Iterator it;

	// update project menus
	m_actRecentProjects->setEnabled( m_actRecentProjects->items().count() > 0 );
	bool project_open = ( docManager()->isProjectOpen() ) ;

	for ( it=m_dictMenuProject.begin(); it!=m_dictMenuProject.end(); ++it ) {
		a = actionCollection()->action(it.key().ascii());
		if ( a )
			a->setEnabled(project_open);
	}

	// project_show is only enabled, when more than 1 project is opened
	a = actionCollection()->action("project_show");
	if(a) {
		a->setEnabled(project_open && docManager()->projects().count() > 1);
	}

	// update file menus
	m_actRecentFiles->setEnabled( m_actRecentFiles->items().count() > 0 );
	bool file_open = ( viewManager()->currentTextView() );
	KILE_DEBUG() << "\tprojectopen=" << project_open << " fileopen=" << file_open << endl;

	QMenuBar *menubar = menuBar();
#ifdef __GNUC__
#warning Commenting out the popup menu stuff!
#endif
//FIXME: port for KDE4
/*
	for ( uint i=0; i<menubar->count(); ++i ) {
		int menu_id = menubar->idAt(i);
		Q3PopupMenu *menu = menubar->findItem(menu_id)->popup();
		if ( menu ) {
			QString menu_name = menu->name();
			for ( uint j=0; j<menu->count(); ++j ) {
				int sub_id = menu->idAt(j);
				Q3PopupMenu *submenu = menu->findItem(sub_id)->popup();
				if ( submenu ) {
					QString submenu_name = submenu->name();
					if ( m_dictMenuFile.contains(submenu_name) ) {
//					if ( m_menuFileList.findIndex( submenu_name ) >= 0 ) {
						menu->setItemEnabled(sub_id, file_open);
					}
				}
			}
		}
	}
*/
	// update action lists
	QList<QAction *> actions = actionCollection()->actions();
	for(QList<QAction *>::iterator itact = actions.begin(); itact != actions.end(); ++itact) {
		if (m_dictMenuAction.contains((*itact)->name())) {
			(*itact)->setEnabled(file_open);
		}
	}

	updateActionList(m_listQuickActions,file_open);
	updateActionList(m_listCompilerActions,file_open);
	updateActionList(m_listConverterActions,file_open);
	updateActionList(m_listViewerActions,file_open);
	updateActionList(m_listOtherActions,file_open);

}

void Kile::updateActionList(const QList<QAction*>& list, bool state)
{
	for (QList<QAction*>::const_iterator i = list.begin(); i != list.end(); ++i) {
		(*i)->setEnabled(state);
	}
}

//TODO: move to KileView::Manager
void Kile::prepareForPart(const QString & state)
{
	KILE_DEBUG() << "==Kile::prepareForPart====================";

	if(m_currentState == "Editor" && state == "Editor") {
		return;
	}

	resetPart();
	m_wantState = state;

	//deactivate kateparts
	for(int i = 0; i < viewManager()->textViews().count(); ++i) {
		m_mainWindow->guiFactory()->removeClient(viewManager()->textView(i));
#ifdef __GNUC__
#warning Commenting Kate::View->setActive() out!
#endif
//FIXME:
// 		viewManager()->textView(i)->setActive(false);
	}
}

void Kile::runTool()
{
	focusLog();
	QString name = sender()->name();
	name.replace(QRegExp("^.*tool_"), "");
	m_manager->run(name);
}

void Kile::cleanAll(KileDocument::TextInfo *docinfo)
{
	static QString noactivedoc = i18n("There is no active document or it is not saved.");
	if (docinfo == 0)
	{
		KTextEditor::Document *doc = activeTextDocument();
		if (doc)
			docinfo = docManager()->textInfoFor(doc);
		else
		{
			m_logWidget->printMsg(KileTool::Error, noactivedoc, i18n("Clean"));
			return;
		}
	}

	if (docinfo)
		docManager()->cleanUpTempFiles(docinfo->url(), false);
}

void Kile::refreshStructure()
{
	viewManager()->updateStructure(true);
}

void Kile::insertTag(const KileAction::TagData& data)
{
	logWidget()->clear();

	if ( data.description.length() > 0 )
	{
		outputView()->showPage(logWidget());
		setLogPresent(false);
		logWidget()->append(data.description);
	}

	KTextEditor::View *view = viewManager()->currentTextView();

	if ( !view ) return;

	view->setFocus();

	editorExtension()->insertTag(data, view);
}

void Kile::insertTag(const QString& tagB, const QString& tagE, int dx, int dy)
{
	insertTag(KileAction::TagData(QString::null,tagB,tagE,dx,dy));
}

void Kile::insertAmsTag(const KileAction::TagData& data)
{
	insertTag(data, QStringList("amsmath"));
}

void Kile::insertTag(const KileAction::TagData& data,const QStringList &pkgs)
{
	KILE_DEBUG() << "void Kile::insertTag(const KileAction::TagData& data,const QStringList " << pkgs.join(",") << ")" << endl;
	insertTag(data);

	KileDocument::Info *docinfo = docManager()->textInfoFor(getCompileName());
	if(docinfo) {
		const QStringList *packagelist = allPackages(docinfo);
		QStringList::const_iterator it;
		QStringList warnPkgs;
		
		for ( it = pkgs.begin() ; it != pkgs.end() ; it++ )
			if( !(*packagelist).contains(*it) )
				warnPkgs.append(*it);

		if(warnPkgs.count() > 0) {
			if(warnPkgs.count() == 1) {
				 m_logWidget->printMsg(KileTool::Error, i18n("You have to include the package %1.", warnPkgs.join(",")), i18n("Insert text"));
			}
			else {
				m_logWidget->printMsg(KileTool::Error, i18n("You have to include the packages %1.", warnPkgs.join(",")), i18n("Insert text"));
			}
		}
	}
}

void Kile::insertText(const QString &text)
{
	insertTag( KileAction::TagData(QString::null,text,"%C",0,0) );
}

void Kile::insertText(const QString &text, const QStringList &pkgs)
{
	insertTag( KileAction::TagData(QString::null,text,"%C",0,0),pkgs );
}

void Kile::quickDocument()
{
	KileDialog::QuickDocument *dlg = new KileDialog::QuickDocument(m_config.data(), m_mainWindow, "Quick Start", i18n("Quick Start"));

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

	KileDialog::TabularDialog *dlg = new KileDialog::TabularDialog(m_mainWindow, m_config.data(), m_latexCommands, tabularenv);
	if(dlg->exec()) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickTabbing()
{
	if(!viewManager()->currentTextView()) {
		return;
	}
	KileDialog::QuickTabbing *dlg = new KileDialog::QuickTabbing(m_config.data(), this, m_mainWindow, "Tabbing", i18n("Tabbing"));
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

	KileDialog::FloatEnvironmentDialog *dlg = new KileDialog::FloatEnvironmentDialog(m_config.data(), this, m_mainWindow);
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

	KileDialog::MathEnvironmentDialog *dlg = new KileDialog::MathEnvironmentDialog(m_mainWindow, m_config.data(), this, m_latexCommands);
	if(dlg->exec()) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickPostscript()
{
	QString startdir = QDir::homePath();
	QString texfilename = QString::null;

	KTextEditor::View *view = viewManager()->currentTextView();
	if(view) {
		startdir = QFileInfo(view->document()->url().path()).path();
		texfilename = getCompileName();
	}

	KileDialog::PostscriptDialog *dlg = new KileDialog::PostscriptDialog(m_mainWindow, texfilename, startdir, m_extensions->latexDocuments(), m_logWidget, m_outputWidget);
	dlg->exec();
	delete dlg;
}

void Kile::helpLaTex()
{
	QString loc = KStandardDirs::locate("html","en/kile/latexhelp.html");
	KileTool::ViewHTML *tool = new KileTool::ViewHTML("ViewHTML", m_manager, false);
	tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
	tool->setSource(loc);
	tool->setTargetPath(loc);
	tool->prepareToRun();
	m_manager->run(tool);
}

void Kile::editUserMenu()
{
	KileDialog::UserTags *dlg = new KileDialog::UserTags(m_listUserTags, m_mainWindow, "Edit User Tags", i18n("Edit User Tags"));

	if(dlg->exec()) {
		//remove all actions
		uint len = m_listUserTagsActions.count();
		for (uint j=0; j< len; ++j) {
			QAction *menuItem = m_listUserTagsActions.last();
//FIXME: port for KDE4
// 			m_menuUserTags->remove(menuItem);
			m_listUserTagsActions.removeLast();
			delete menuItem;
		}
#ifdef __GNUC__
#warning Port m_menuUserTags->remove(m_actionEditSeparator)!
#endif
//FIXME: port for KDE4
/*
		if ( len > 0 )
			m_menuUserTags->remove(m_actionEditSeparator);
*/
//FIXME: port for KDE4
//		m_menuUserTags->remove(m_actionEditTag);

		m_listUserTags = dlg->result();
		setupUserTagActions();
	}

	delete dlg;
}

void Kile::readGUISettings()
{
	m_horSplitLeft = KileConfig::horizontalSplitterLeft();
	m_horSplitRight = KileConfig::horizontalSplitterRight();
	m_verSplitTop = KileConfig::verticalSplitterTop();
	m_verSplitBottom = KileConfig::verticalSplitterBottom();
}

void Kile::readUserSettings()
{
	//test for old kilerc
	int version = KileConfig::rCVersion();

	//if the kilerc file is old some of the configuration
	//data must be set by kile, even if the keys are present
	//in the kilerc file
	if ( version < 4 )
	{
		KileTool::Factory *factory = new KileTool::Factory(0, m_config.data());
		KILE_DEBUG() << "WRITING STD TOOL CONFIG" << endl;
		factory->writeStdConfig();
		delete factory;
	}

	//delete old editor key
	if (m_config->hasGroup("Editor") )
	{
		m_config->deleteGroup("Editor");
	}

	KConfigGroup userGroup = m_config->group("User");
	int len = userGroup.readEntry("nUserTags", 0);
	for (int i = 0; i < len; ++i)
	{
		m_listUserTags.append(KileDialog::UserTags::splitTag(userGroup.readEntry("userTagName" + QString::number(i), i18n("no name")) , userGroup.readEntry("userTag" + QString::number(i), "")));
	}

	//convert user tools to new KileTool classes
	userItem tempItem;
	len = userGroup.readEntry("nUserTools", 0);
	for (int i=0; i< len; ++i)
	{
		tempItem.name = userGroup.readEntry("userToolName" + QString::number(i), i18n("no name"));
		tempItem.tag = userGroup.readEntry("userTool" + QString::number(i), "");
		m_listUserTools.append(tempItem);
	}
	if ( len > 0 )
	{
 		//move the tools
		userGroup.writeEntry("nUserTools", 0);
		for ( int i = 0; i < len; ++i)
		{
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

			if ( i < 10 )
			{
				KConfigGroup shortcutGroup = m_config->group("Shortcuts");
				shortcutGroup.writeEntry("tool_" + tempItem.name, "Alt+Shift+" + QString::number(i + 1) ); //should be alt+shift+
			}
		}
	}

	// check autocompletion modes: if KTextEditor-plugin for document word 
	// completion is active, both autocompletion modes of Kile must be disabled
	if ( kateCompletionPlugin() )
	{
		KileConfig::setCompleteAuto(false);
		KileConfig::setCompleteAutoText(false);
	}
}

void Kile::readRecentFileSettings()
{
	KConfigGroup group = m_config->group("FilesOpenOnStart");
	int n = group.readEntry("NoDOOS", 0);
	for (int i=0; i < n; ++i)
		m_listDocsOpenOnStart.append(group.readPathEntry("DocsOpenOnStart" + QString::number(i), ""));

	n = group.readEntry("NoPOOS", 0);
	for (int i=0; i < n; ++i)
		m_listProjectsOpenOnStart.append(group.readPathEntry("ProjectsOpenOnStart" + QString::number(i), ""));
}

void Kile::readConfig()
{
	enableAutosave(KileConfig::autosave());
	m_edit->complete()->readConfig(m_config.data());
	//m_edit->initDoubleQuotes();
	m_edit->readConfig();
	docManager()->updateInfos();
	m_jScriptManager->readConfig();
	m_sideBar->setPageVisible(m_scriptsManagementWidget, KileConfig::scriptingEnabled());
	m_sideBar->setPageVisible(m_kileAbbrevView, KileConfig::completeShowAbbrev());
	
	if( KileConfig::displayMFUS() )
		enableSymbolViewMFUS();
	else
		disableSymbolViewMFUS();
}

void Kile::saveSettings()
{
	showEditorWidget();

	m_fileBrowserWidget->writeConfig();
	m_symbolViewMFUS->writeConfig();

	KileConfig::setInputEncoding(m_fileBrowserWidget->comboEncoding()->lineEdit()->text());

	// Store recent files
	m_actRecentFiles->saveEntries(m_config->group("Recent Files"));
	m_actRecentProjects->saveEntries(m_config->group("Projects"));

	m_config->deleteGroup("FilesOpenOnStart");
	if (KileConfig::restore())
	{
		KConfigGroup configGroup = m_config->group("FilesOpenOnStart");
		configGroup.writeEntry("NoDOOS", m_listDocsOpenOnStart.count());
		for (int i = 0; i < m_listDocsOpenOnStart.count(); ++i) {
			configGroup.writePathEntry("DocsOpenOnStart"+QString::number(i), m_listDocsOpenOnStart[i]);
		}

		configGroup.writeEntry("NoPOOS", m_listProjectsOpenOnStart.count());
		for (int i = 0; i < m_listProjectsOpenOnStart.count(); ++i) {
			configGroup.writePathEntry("ProjectsOpenOnStart"+QString::number(i), m_listProjectsOpenOnStart[i]);
		}

		if (!m_singlemode)
			KileConfig::setMaster(m_masterName);
		else
			KileConfig::setMaster("");
	}

	KConfigGroup userGroup = m_config->group("User");

	userGroup.writeEntry("nUserTags", static_cast<int>(m_listUserTags.size()));
	for (int i = 0; i < m_listUserTags.size(); ++i) {
		KileAction::TagData td( m_listUserTags[i]);
		userGroup.writeEntry( "userTagName"+QString::number(i),  td.text );
		userGroup.writeEntry( "userTag"+QString::number(i), KileDialog::UserTags::completeTag(td) );
	}

	actionCollection()->writeSettings();
	m_mainWindow->saveMainWindowSettings(m_config->group("KileMainWindow"));

	scriptManager()->writeConfig();
	m_edit->complete()->saveLocalChanges();

	KileConfig::setRCVersion(KILERC_VERSION);
	KileConfig::setMainwindowWidth(m_mainWindow->width());
	KileConfig::setMainwindowHeight(m_mainWindow->height());

	QList<int> sizes;
	QList<int>::Iterator it;
	sizes = m_horizontalSplitter->sizes();
	it = sizes.begin();
	m_horSplitLeft=*it;
	++it;
	m_horSplitRight=*it;
	sizes.clear();
	sizes = m_verticalSplitter->sizes();
	it = sizes.begin();
	m_verSplitTop=*it;
	++it;
	m_verSplitBottom=*it;

#ifdef __GNUC__
#warning Restoring of the side bar's sizes and states doesn't work perfectly yet!
#endif
	// sync vertical splitter and size of bottom bar
	int sizeBottomBar = m_bottomBar->directionalSize();
	if(m_bottomBar->isVisible()) {
		sizeBottomBar = m_verSplitBottom;
	}
	else {
		m_verSplitBottom = sizeBottomBar;
	}

	KileConfig::setHorizontalSplitterLeft(m_horSplitLeft);
	KileConfig::setHorizontalSplitterRight(m_horSplitRight);
	KileConfig::setVerticalSplitterTop(m_verSplitTop);
	KileConfig::setVerticalSplitterBottom(m_verSplitBottom);

	KileConfig::setSideBar(m_sideBar->isVisible());
	KileConfig::setSideBarSize(m_sideBar->directionalSize());
	KileConfig::setBottomBar(m_bottomBar->isVisible());
	KileConfig::setBottomBarSize(sizeBottomBar);

	KileConfig::setSelectedLeftView(m_sideBar->currentTab());

	KileConfig::self()->writeConfig();
	m_config->sync();
}

/////////////////  OPTIONS ////////////////////
void Kile::toggleMode()
{
	if (!m_singlemode) {
		ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
		ModeAction->setChecked(false);
		m_logPresent=false;
		m_singlemode=true;
		m_masterName=QString::null;
	}
	else if (m_singlemode && viewManager()->currentTextView()) {
		m_masterName=getName();
		if ( KileUntitled::isUntitled(m_masterName) || m_masterName.isEmpty()) {
			ModeAction->setChecked(false);
			KMessageBox::error(m_mainWindow, i18n("In order to define the current document as a master document, it has to be saved first."));
			m_masterName = "";
			return;
		}

		QString shortName = m_masterName;
		int pos;
		while ( (pos = (int)shortName.indexOf('/')) != -1 )
			shortName.remove(0,pos+1);

		ModeAction->setText(i18n("Normal mode (current master document: %1)", shortName));
		ModeAction->setChecked(true);
		m_singlemode=false;
	}
	else {
		ModeAction->setChecked(false);
	}

	updateModeStatus();
	KILE_DEBUG() << "SETTING master to " << m_masterName << " singlemode = " << m_singlemode << endl;
}

void Kile::toggleWatchFile()
{
	m_bWatchFile=!m_bWatchFile;

	if (m_bWatchFile)
		WatchFileAction->setChecked(true);
	else
		WatchFileAction->setChecked(false);
}

// execute configuration dialog

void Kile::generalOptions()
{
	m_edit->complete()->saveLocalChanges();
	
	KileDialog::Config *dlg = new KileDialog::Config(m_config.data(), this, m_mainWindow);

	if (dlg->exec())
	{
		// check new Kate settings
		checkKateSettings();

		// update new settings
		readConfig();
		setupTools();
		m_help->update();

		emit configChanged();

		//stop/restart LyX server if necessary
		if (KileConfig::runLyxServer() && !m_lyxserver->isRunning())
			m_lyxserver->start();

		if (!KileConfig::runLyxServer() && m_lyxserver->isRunning())
			m_lyxserver->stop();
	}

	delete dlg;
}

// read kate plugin configuration
bool Kile::kateCompletionPlugin()
{
	return m_config->group("Kate Document Defaults").readEntry("KTextEditor Plugin ktexteditor_docwordcompletion",false);
}

void Kile::checkKateSettings()
{
	// editor settings were only available with an opened document
	KTextEditor::View *view = viewManager()->currentTextView();
	if (view) {
		// remove menu entry to config Kate
		viewManager()->unplugTextEditorPartMenu(view);
	}
}

void Kile::slotPerformCheck()
{
	if(!m_singlemode) {
		m_logWidget->printMsg(KileTool::Error, i18n("Please turn off the \'Master Document\' mode before performing the System Check."), i18n("System Check"));
		return;
	}
	KileDialog::ConfigChecker *dlg = new KileDialog::ConfigChecker(m_mainWindow);
	dlg->exec();
	delete dlg;
}

/////////////// KEYS - TOOLBARS CONFIGURATION ////////////////
void Kile::configureKeys()
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, m_mainWindow);
	QList<KXMLGUIClient*> clients = m_mainWindow->guiFactory()->clients();
	for(QList<KXMLGUIClient*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		dlg.addCollection((*it)->actionCollection());
	}
	dlg.configure(false); // don't save the settings automatically
	KConfigGroup shortcutsGroup = m_config->group("Shortcuts");
	actionCollection()->writeSettings(&shortcutsGroup);
}

void Kile::configureToolbars()
{
	m_mainWindow->saveMainWindowSettings(m_config->group("KileMainWindow"));
	KEditToolBar dlg(m_mainWindow->factory());
	dlg.exec();

	m_mainWindow->applyMainWindowSettings(m_config->group("KileMainWindow"));
 	showToolBars(m_currentState);
}

void Kile::changeInputEncoding()
{
	KTextEditor::Document *doc = activeTextDocument();
	if ( doc )
	{
		KileDocument::TextInfo *textInfo = docManager()->textInfoFor(doc);
		bool modified = doc->isModified();
		
		if( !textInfo )
			return;

		QString encoding = m_fileBrowserWidget->comboEncoding()->lineEdit()->text();

		if(!encoding.isNull())
			doc->setEncoding(encoding);
        	QString highlightingMode = doc->highlightingMode(); //remember the highlighting mode

		//reload the document so that the new encoding takes effect
		doc->openUrl(doc->url());
        	doc->setHighlightingMode(highlightingMode);
		doc->setModified(modified);
		
		viewManager()->updateStructure(true,textInfo); //reparse doc to get a correct structure view
	}
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
	while(i < view->document()->lines())
	{
		s = view->document()->line(i);

		// do we have a line that starts with ALT or OPT?
		if ( reOptional.search( s ) >= 0 )
		{
				// yes! capture type and entry
				QString type = reOptional.cap( 2 );
				QString entry = reOptional.cap( 3 );
				view->document()->removeLine( i );
				view->document()->setModified(true);
				if ( reNonEmptyEntry.search( entry ) >= 0 )
				{
					type.append( " = " );
					type.append( entry );
					view->document()->insertLine( i, type );
					++i;
				}
		}
		else
			++i;
	}
	int j = 0;
	for (i = 0; i < view->document()->lines(); ++i) {
		j = i+1;
		if ( j < view->document()->lines()  && view->document()->line(j).contains( QRegExp("^\\s*\\}\\s*$") ) )
			{
				s =  view->document()->line( i );
				view->document()->removeLine( i );
				s.remove( QRegExp(",\\s*$") );
				view->document()->setModified( true );
				view->document()->insertLine( i, s);
			}
	}
}

void Kile::includeGraphics()
{
	KTextEditor::View *view = viewManager()->currentTextView();
	if ( !view ) return;

	QFileInfo fi( view->document()->url().path() );
	KileDialog::IncludeGraphics *dialog = new KileDialog::IncludeGraphics(m_mainWindow, fi.path(), this);

	if ( dialog->exec() == QDialog::Accepted )
	{
		insertTag(dialog->getTemplate(), "%C", 0,0);
		docManager()->projectAddFile( dialog->getFilename(),true );
	}

	delete dialog;
}

void Kile::slotToggleFullScreen()
{
	//FIXME for Qt 3.3.x we can do: setWindowState(windowState() ^ WindowFullScreen);
	if (!m_pFullScreen->isChecked()) {
		m_mainWindow->showNormal();
	}
	else {
		m_mainWindow->showFullScreen();
	}
}

/////////////// QuickPreview (dani) ////////////////

// all calls of QuickPreview will get here, so we can decide what to do
// rewritten Sep 05 2006 to work together with preview in the bottom bar

void Kile::slotQuickPreview(int type)
{
	KILE_DEBUG() << "==Kile::slotQuickPreview()=========================="  << endl;

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

#ifdef __GNUC__
#warning Port the citeViewBib function!
#endif
void Kile::citeViewBib()
{
/*
	KILE_DEBUG()  << "===void Kile::citeViewBib()===" << endl;

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
		m_logWidget->printMsg(KileTool::Warning,
		i18n("No ViewBib tool running, trying to start it now"),
		i18n("ViewBib Citation"));
		uint ret = runWith("ViewBib","KBib");
		if( ret == 0 )
			m_logWidget->printMsg(KileTool::Info,
				i18n("Please select the desired bibliographies and re-execute this command"),
				i18n("ViewBib Citation"));
		return;	
	}

	remoteObjs = client->remoteObjects(viewBibApp);
	if( !remoteObjs.contains(viewBibObj) )
	{
		m_logWidget->printMsg(KileTool::Warning,
				      i18n("The ViewBib tool does not have the correct interface"),
				      i18n("ViewBib Citation"));
		return;
	}

	functions = client->remoteFunctions(viewBibApp,viewBibObj);
	if( !functions.contains(viewBibFncDef) )
	{
		m_logWidget->printMsg(KileTool::Warning,
					i18n("The ViewBib tool does not have the correct definition of the cite function"),
					i18n("ViewBib Citation"));
		return;
	}

	if ( !client->call(viewBibApp, viewBibObj, viewBibFnc, params, replyType, replyData) )
	{
		// we should never get here
		kWarning() << "internal error in viewbib citation" << endl;
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
				m_logWidget->printMsg(KileTool::Warning,
						i18n("No reference selected.\nPlease select a reference first!"),
						i18n("ViewBib Citation"));
			}
			else
			{
				insertTag(KileAction::TagData(i18n("ViewBib Citation"), result, QString::null, result.length()));

			}
		}
	}
*/
}

#include "kile.moc"
