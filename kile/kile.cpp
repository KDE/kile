/***************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                :  Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kile.h"

#include <qtooltip.h>
#include <qguardedptr.h>

#include <kaction.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <krun.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstandarddirs.h>
#include <kmultitabbar.h>
#include <ktabwidget.h>
#include <ktip.h>
#include <ktexteditor/configinterface.h>

#include "kileapplication.h"
#include "kiledocumentinfo.h"
#include "kileactions.h"
#include "kilestdactions.h"
#include "usermenudialog.h"
#include "kileconfigdialog.h"
#include "kileproject.h"
#include "kileprojectview.h"
#include "kileprojectdlgs.h"
#include "kilelistselector.h"
#include "kilelyxserver.h"
#include "kilegrepdialog.h"
#include "kiletool_enums.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "kilelogwidget.h"
#include "kileoutputwidget.h"
#include "kilekonsolewidget.h"
#include "quickdocumentdialog.h"
#include "tabbingdialog.h"
#include "kilestructurewidget.h"
#include "convert.h"
#include "includegraphicsdialog.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "kileeventfilter.h"
#include "kileconfig.h"
#include "kileerrorhandler.h"
#include "configcheckerdlg.h"
#include "kilespell.h"
//#include "kilespell2.h"
#include "kilesidebar.h"
#include "symbolview.h"
#include "floatdialog.h"
#include "mathenvdialog.h"
#include "tabulardialog.h"
#include "postscriptdialog.h"
#include "latexcmd.h"
#include "kileuntitled.h"
#include "kilestatsdlg.h"
#include "scriptsmanagementwidget.h"
#include "kilejscript.h"
#include "previewwidget.h"

Kile::Kile( bool allowRestore, QWidget *parent, const char *name ) :
	DCOPObject( "Kile" ),
	KParts::MainWindow( parent, name),
	KileInfo(this),
	m_paPrint(0L),
	m_bShowUserMovedMessage(false)
{
    m_focusWidget = this;

	m_config = KGlobal::config();
	readUserSettings();
	readRecentFileSettings();

	m_jScriptManager = new KileJScript::Manager(this, m_config, actionCollection(), parent, "KileJScript::Manager");

    setStandardToolBarMenuEnabled(true);

	m_masterName = KileConfig::master();
	m_singlemode = (m_masterName.isEmpty());

	m_AutosaveTimer = new QTimer();
	connect(m_AutosaveTimer,SIGNAL(timeout()),this,SLOT(autoSaveAll()));

	m_latexCommands = new KileDocument::LatexCommands(m_config,this);  // at first (dani)
	m_edit = new KileDocument::EditorExtension(this);
	m_help = new KileHelp::Help(m_edit);
	m_partManager = new KParts::PartManager( this );
	m_eventFilter = new KileEventFilter(m_edit);
	m_errorHandler = new KileErrorHandler(this, this);
	m_spell = new KileSpell(this, this, "kilespell");
	m_quickPreview = new KileTool::QuickPreview(this);

	connect( m_partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(activePartGUI ( KParts::Part * ) ) );
	connect(this,SIGNAL(configChanged()), m_eventFilter, SLOT(readConfig()));

	readGUISettings();

 KGlobal::dirs()->addResourceType( "app_symbols",KStandardDirs::kde_default("data") + "kile/mathsymbols/"); // needed for Symbolview


	setXMLFile( "kileui.rc" );

	// do initializations first
	m_currentState = m_wantState = "Editor";
	m_bWatchFile = m_logPresent = false;

	viewManager()->setClient(this, this);

	setupStatusBar();

	m_topWidgetStack = new QWidgetStack( this );
	m_topWidgetStack->setFocusPolicy(QWidget::NoFocus);

	m_horizontalSplitter = new QSplitter(QSplitter::Horizontal,m_topWidgetStack, "horizontalSplitter" );

	setupSideBar();

	m_verticalSplitter=new QSplitter(QSplitter::Vertical, m_horizontalSplitter, "verticalSplitter");
	viewManager()->createTabs(m_verticalSplitter);

	connect(viewManager(), SIGNAL(activateView(QWidget*, bool)), this, SLOT(activateView(QWidget*, bool)));
	connect(viewManager(), SIGNAL(prepareForPart(const QString& )), this, SLOT(prepareForPart(const QString& )));
	connect(viewManager(), SIGNAL(startQuickPreview(int)), this, SLOT(slotQuickPreview(int)) );

	setupBottomBar();
	setupGraphicTools();
	setupPreviewTools();
	setupActions();
	setupTools();

	QValueList<int> sizes;
	sizes << m_verSplitTop << m_verSplitBottom;
	m_verticalSplitter->setSizes( sizes );
	sizes.clear();
	sizes << m_horSplitLeft << m_horSplitRight;
	m_horizontalSplitter->setSizes( sizes );
	if ( ! KileConfig::bottomBar() )
	{
		m_actionMessageView->activate();
		m_bottomBar->setSize(KileConfig::bottomBarSize());
	}

	m_topWidgetStack->addWidget(m_horizontalSplitter , 0);
	setCentralWidget(m_topWidgetStack);
	newCaption();

	m_partManager->setActivePart( 0L );

	m_lyxserver = new KileLyxServer(KileConfig::runLyxServer());
	connect(m_lyxserver, SIGNAL(insert(const KileAction::TagData &)), this, SLOT(insertTag(const KileAction::TagData &)));

	applyMainWindowSettings(m_config, "KileMainWindow" );

	m_manager  = new KileTool::Manager(this, m_config, m_logWidget, m_outputWidget, m_partManager, m_topWidgetStack, m_paStop, 10000); //FIXME make timeout configurable
	connect(m_manager, SIGNAL(requestGUIState(const QString &)), this, SLOT(prepareForPart(const QString &)));
	connect(m_manager, SIGNAL(requestSaveAll(bool, bool)), docManager(), SLOT(fileSaveAll(bool, bool)));
	connect(m_manager, SIGNAL(jumpToFirstError()), m_errorHandler, SLOT(jumpToFirstError()));
	connect(m_manager, SIGNAL(toolStarted()), m_errorHandler, SLOT(reset()));
	connect(m_manager, SIGNAL(previewDone()), this, SLOT(focusPreview()));

	m_toolFactory = new KileTool::Factory(m_manager, m_config);
	m_manager->setFactory(m_toolFactory);
	m_help->setUserhelp(m_manager,menuBar());     // kile user help (dani)

	connect(docManager(), SIGNAL(updateModeStatus()), this, SLOT(updateModeStatus()));
	connect(docManager(), SIGNAL(updateStructure(bool, KileDocument::Info*)), viewManager(), SLOT(updateStructure(bool, KileDocument::Info*)));
	connect(docManager(), SIGNAL(closingDocument(KileDocument::Info* )), m_kwStructure, SLOT(closeDocumentInfo(KileDocument::Info *)));
	connect(docManager(), SIGNAL(documentInfoCreated(KileDocument::Info* )), m_kwStructure, SLOT(addDocumentInfo(KileDocument::Info* )));
	connect(docManager(), SIGNAL(updateReferences(KileDocument::Info *)), m_kwStructure, SLOT(updateReferences(KileDocument::Info *)));

	readConfig();

	KileApplication::closeSplash();

	resize(KileConfig::mainwindowWidth(), KileConfig::mainwindowHeight());
	show();

	if ( m_listUserTools.count() > 0 )
	{
		KMessageBox::information(0, i18n("You have defined some tools in the User menu. From now on these tools will be available from the Build->Other menu and can be configured in the configuration dialog (go to the Settings menu and choose Configure Kile). This has some advantages; your own tools can now be used in a QuickBuild command if you wish."), i18n("User Tools Detected"));
		m_listUserTools.clear();
	}

	if (m_bShowUserMovedMessage)
	{
		KMessageBox::information(0, i18n("Please note that the 'User' menu, which holds the (La)TeX tags you have defined, is moved to the LaTeX menu."));
	}

	KTipDialog::showTip(this, "kile/tips");

	restoreFilesAndProjects(allowRestore);
	initMenu();
	updateModeStatus();

	setFocus();
actionCollection()->readShortcutSettings("Shortcuts", m_config);
}

Kile::~Kile()
{
	kdDebug() << "cleaning up..." << endl;
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

void Kile::showEvent(QShowEvent *)
{
	newCaption();
	m_focusWidget->setFocus();
}

void Kile::hideEvent(QHideEvent *)
{
	Kate::View *view = viewManager()->currentTextView();
	if (view)
	{
		setCaption( getShortName(view->getDoc()) );
	}
	m_focusWidget = focusWidget();
}

void Kile::setupStatusBar()
{
    statusBar()->removeItem(ID_LINE_COLUMN);
    statusBar()->removeItem(ID_HINTTEXT);

	statusBar()->insertItem(i18n("Line: 1 Col: 1"), ID_LINE_COLUMN, 0, true);
	statusBar()->setItemAlignment( ID_LINE_COLUMN, AlignLeft|AlignVCenter );
	statusBar()->insertItem(i18n("Normal Mode"), ID_HINTTEXT,10);
	statusBar()->setItemAlignment( ID_HINTTEXT, AlignLeft|AlignVCenter );
}

void Kile::setupSideBar()
{
	m_sideBar = new KileSideBar(KileConfig::sideBarSize(), m_horizontalSplitter);

	m_fileSelector= new KileFileSelect(m_sideBar,"File Selector");
	m_sideBar->addTab(m_fileSelector, SmallIcon("fileopen"), i18n("Open File"));
	connect(m_fileSelector,SIGNAL(fileSelected(const KFileItem*)), docManager(), SLOT(fileSelected(const KFileItem*)));
	connect(m_fileSelector->comboEncoding(), SIGNAL(activated(int)),this,SLOT(changeInputEncoding()));
	m_fileSelector->comboEncoding()->lineEdit()->setText(KileConfig::defaultEncoding());
	m_fileSelector->readConfig();

	setupProjectView();
	setupStructureView();
	setupSymbolViews();
	setupScriptsManagementView();

	m_sideBar->showTab(KileConfig::selectedLeftView());
	m_sideBar->setVisible(KileConfig::sideBar());
    m_sideBar->setSize(KileConfig::sideBarSize());
}

void Kile::setupProjectView()
{
	KileProjectView *projectview = new KileProjectView(m_sideBar, this);
// 	viewManager()->setProjectView(projectview);
	m_sideBar->addTab(projectview, SmallIcon("editcopy"), i18n("Files and Projects"));
	connect(projectview, SIGNAL(fileSelected(const KileProjectItem *)), docManager(), SLOT(fileSelected(const KileProjectItem *)));
	connect(projectview, SIGNAL(fileSelected(const KURL &)), docManager(), SLOT(fileSelected(const KURL &)));
	connect(projectview, SIGNAL(closeURL(const KURL&)), docManager(), SLOT(fileClose(const KURL&)));
	connect(projectview, SIGNAL(closeProject(const KURL&)), docManager(), SLOT(projectClose(const KURL&)));
	connect(projectview, SIGNAL(projectOptions(const KURL&)), docManager(), SLOT(projectOptions(const KURL&)));
	connect(projectview, SIGNAL(projectArchive(const KURL&)), this, SLOT(runArchiveTool(const KURL&)));
	connect(projectview, SIGNAL(removeFromProject(const KileProjectItem *)), docManager(), SLOT(removeFromProject(const KileProjectItem *)));
	connect(projectview, SIGNAL(addFiles(const KURL &)), docManager(), SLOT(projectAddFiles(const KURL &)));
	connect(projectview, SIGNAL(openAllFiles(const KURL &)), docManager(), SLOT(projectOpenAllFiles(const KURL &)));
	connect(projectview, SIGNAL(toggleArchive(KileProjectItem *)), docManager(), SLOT(toggleArchive(KileProjectItem *)));
	connect(projectview, SIGNAL(addToProject(const KURL &)), docManager(), SLOT(addToProject(const KURL &)));
	connect(projectview, SIGNAL(saveURL(const KURL &)), docManager(), SLOT(saveURL(const KURL &)));
	connect(projectview, SIGNAL(buildProjectTree(const KURL &)), docManager(), SLOT(buildProjectTree(const KURL &)));
	connect(docManager(), SIGNAL(projectTreeChanged(const KileProject *)), projectview, SLOT(refreshProjectTree(const KileProject *)));
	connect(docManager(), SIGNAL(removeFromProjectView(const KURL &)),projectview,SLOT(remove(const KURL &)));
	connect(docManager(), SIGNAL(removeFromProjectView(const KileProject *)),projectview,SLOT(remove(const KileProject *)));
	connect(docManager(), SIGNAL(addToProjectView(const KURL &)),projectview,SLOT(add(const KURL &)));
	connect(docManager(), SIGNAL(addToProjectView(const KileProject *)),projectview,SLOT(add(const KileProject *)));
	connect(docManager(),SIGNAL(removeItemFromProjectView(const KileProjectItem *, bool)),projectview,SLOT(removeItem(const KileProjectItem *, bool)));
	connect(docManager(),SIGNAL(addToProjectView(KileProjectItem *)),projectview,SLOT(add(KileProjectItem *)));
}

void Kile::setupStructureView()
{
	m_kwStructure = new KileWidget::Structure(this, m_sideBar);
	m_sideBar->addTab(m_kwStructure, SmallIcon("structure"), i18n("Structure"));
	m_kwStructure->setFocusPolicy(QWidget::ClickFocus);
	connect(this, SIGNAL(configChanged()), m_kwStructure, SIGNAL(configChanged()));
	connect(m_kwStructure, SIGNAL(setCursor(const KURL &,int,int)), this, SLOT(setCursor(const KURL &,int,int)));
	connect(m_kwStructure, SIGNAL(fileOpen(const KURL&, const QString & )), docManager(), SLOT(fileOpen(const KURL&, const QString& )));
	connect(m_kwStructure, SIGNAL(fileNew(const KURL&)), docManager(), SLOT(fileNew(const KURL&)));
	connect(m_kwStructure, SIGNAL(sendText(const QString &)), this, SLOT(insertText(const QString &)));
}

void Kile::setupScriptsManagementView()
{
	m_scriptsManagementWidget = new KileWidget::ScriptsManagement(this, m_sideBar);
	connect((QObject*)editorKeySequenceManager(), SIGNAL(watchedKeySequencesChanged()), m_scriptsManagementWidget, SLOT(updateListView()));
	m_sideBar->addTab(m_scriptsManagementWidget, SmallIcon("exec"), i18n("Scripts"));
}

void Kile::setupSymbolViews()
{
	m_toolBox = new QToolBox(m_sideBar);
	m_sideBar->addTab(m_toolBox,SmallIcon("math0"),i18n("Symbols"));

	m_symbolViewRelation = new SymbolView(m_toolBox,"relation",SymbolView::Relation);
	m_toolBox->addItem(m_symbolViewRelation,SmallIcon("math1"),i18n("Relation"));
	connect(m_symbolViewRelation, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));
		
	m_symbolViewOperators = new SymbolView(m_toolBox,"operators",SymbolView::Operator);
	m_toolBox->addItem(m_symbolViewOperators,SmallIcon("math2"),i18n("Operators"));
	connect(m_symbolViewOperators, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewArrows = new SymbolView(m_toolBox,"arrow",SymbolView::Arrow);
	m_toolBox->addItem(m_symbolViewArrows,SmallIcon("math3"),i18n("Arrows"));
	connect(m_symbolViewArrows, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewMiscMath = new SymbolView(m_toolBox,"miscmath",SymbolView::MiscMath);
	m_toolBox->addItem(m_symbolViewMiscMath,SmallIcon("math4"),i18n("Miscellaneous Math"));
	connect(m_symbolViewMiscMath, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewMiscText = new SymbolView(m_toolBox,"misctext",SymbolView::MiscText);
	m_toolBox->addItem(m_symbolViewMiscText,SmallIcon("math5"),i18n("Miscellaneous Text"));
	connect(m_symbolViewMiscText, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewDelimiters= new SymbolView(m_toolBox,"delimiters",SymbolView::Delimiters);
	m_toolBox->addItem(m_symbolViewDelimiters,SmallIcon("math6"),i18n("Delimiters"));
	connect(m_symbolViewDelimiters, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewGreek = new SymbolView(m_toolBox,"greek",SymbolView::Greek);
	m_toolBox->addItem(m_symbolViewGreek,SmallIcon("math7"),i18n("Greek"));
	connect(m_symbolViewGreek, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewSpecial = new SymbolView(m_toolBox,"special",SymbolView::Special);
	m_toolBox->addItem(m_symbolViewSpecial,SmallIcon("math8"),i18n("Special Characters"));
	connect(m_symbolViewSpecial, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewCyrillic = new SymbolView(m_toolBox,"cyrillic",SymbolView::Cyrillic);
	m_toolBox->addItem(m_symbolViewCyrillic,SmallIcon("math10"),i18n("Cyrillic Characters"));
	connect(m_symbolViewCyrillic, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	m_symbolViewUser = new SymbolView(m_toolBox,"user",SymbolView::User);
	m_toolBox->addItem(m_symbolViewUser,SmallIcon("math9"),i18n("User Defined"));
	connect(m_symbolViewUser, SIGNAL(insertText(const QString& )), this, SLOT(insertText(const QString& )));

	for (int i=0; i< m_toolBox->count(); i++)
		m_toolBox->setItemToolTip(i, i18n("Move the mouse over the icons to see the corresponding LaTeX commands.\nClick on the images to insert the command, additionally pressing SHIFT inserts it in math mode, presssing CTRL in curly brackets."));

	m_mpview = new metapostview( m_sideBar );
	m_sideBar->addTab(m_mpview, SmallIcon("metapost"), i18n("MetaPost"));
	connect(m_mpview, SIGNAL(clicked(QListBoxItem *)), SLOT(insertMetaPost(QListBoxItem *)));
}

void Kile::setupBottomBar()
{
	m_bottomBar = new KileBottomBar(KileConfig::bottomBarSize(), m_verticalSplitter);
	m_bottomBar->setFocusPolicy(QWidget::ClickFocus);

	m_logWidget = new KileWidget::LogMsg( this, m_bottomBar );
	connect(m_logWidget, SIGNAL(showingErrorMessage(QWidget* )), m_bottomBar, SLOT(showPage(QWidget* )));
	connect(m_logWidget, SIGNAL(fileOpen(const KURL&, const QString & )), docManager(), SLOT(fileOpen(const KURL&, const QString& )));
	connect(m_logWidget, SIGNAL(setLine(const QString& )), this, SLOT(setLine(const QString& )));
	connect(m_docManager,SIGNAL(printMsg(int, const QString &, const QString &)),m_logWidget,SLOT(printMsg(int, const QString &, const QString &)));

	m_logWidget->setFocusPolicy(QWidget::ClickFocus);
	m_logWidget->setMinimumHeight(40);
	m_logWidget->setReadOnly(true);
	m_bottomBar->addTab(m_logWidget, SmallIcon("viewlog"), i18n("Log and Messages"));

	m_outputWidget = new KileWidget::Output(m_bottomBar);
	m_outputWidget->setFocusPolicy(QWidget::ClickFocus);
	m_outputWidget->setMinimumHeight(40);
	m_outputWidget->setReadOnly(true);
	m_bottomBar->addTab(m_outputWidget, SmallIcon("output_win"), i18n("Output"));

	m_outputInfo=new LatexOutputInfoArray();
	m_outputFilter=new LatexOutputFilter(m_outputInfo);
	connect(m_outputFilter, SIGNAL(problem(int, const QString& )), m_logWidget, SLOT(printProblem(int, const QString& )));

	m_texKonsole=new KileWidget::Konsole(this, m_bottomBar,"konsole");
	m_bottomBar->addTab(m_texKonsole, SmallIcon("konsole"),i18n("Konsole"));
	connect(viewManager()->tabs(), SIGNAL( currentChanged( QWidget * ) ), m_texKonsole, SLOT(sync()));

	m_previewView = new QScrollView (m_bottomBar);
	m_previewWidget = new KileWidget::PreviewWidget (this, m_previewView);
	m_previewView->viewport()->setPaletteBackgroundColor (QColor (0xff, 0xff, 0xff));
	m_previewView->addChild(m_previewWidget, 0, 0); 
	m_bottomBar->addTab (m_previewView, SmallIcon ("edu_mathematics"), i18n ("Preview"));

	m_bottomBar->setVisible(true);
	m_bottomBar->setSize(KileConfig::bottomBarSize());
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

void Kile::setupActions()
{
	m_paPrint = KStdAction::print(0,0, actionCollection(), "file_print");
	(void) KStdAction::openNew(docManager(), SLOT(fileNew()), actionCollection(), "file_new" );
	(void) KStdAction::open(docManager(), SLOT(fileOpen()), actionCollection(),"file_open" );
	m_actRecentFiles = KStdAction::openRecent(docManager(), SLOT(fileOpen(const KURL&)), actionCollection(), "file_open_recent");
	connect(docManager(), SIGNAL(addToRecentFiles(const KURL& )), m_actRecentFiles, SLOT(addURL(const KURL& )));
	m_actRecentFiles->loadEntries(m_config, "Recent Files");

	(void) KStdAction::save(docManager(), SLOT(fileSave()), actionCollection(),"kile_file_save" );
	(void) KStdAction::saveAs(docManager(), SLOT(fileSaveAs()), actionCollection(),"kile_file_save_as" );

	(void) new KAction(i18n("Save All"),"save_all", 0, docManager(), SLOT(fileSaveAll()), actionCollection(),"file_save_all" );
	(void) new KAction(i18n("Create Template From Document..."), 0, docManager(), SLOT(createTemplate()), actionCollection(),"template_create");
	(void) new KAction(i18n("&Remove Template..."),0, docManager(), SLOT(removeTemplate()), actionCollection(), "template_remove");
	(void) KStdAction::close(docManager(), SLOT(fileClose()), actionCollection(),"file_close" );
	(void) new KAction(i18n("Close All"), 0, docManager(), SLOT(fileCloseAll()), actionCollection(),"file_close_all" );
	(void) new KAction(i18n("Close All Ot&hers"), 0, docManager(), SLOT(fileCloseAllOthers()), actionCollection(),"file_close_all_others" );
	(void) new KAction(i18n("S&tatistics"), 0, this, SLOT(showDocInfo()), actionCollection(), "Statistics" );
	(void) new KAction(i18n("&ASCII"), 0, this, SLOT(convertToASCII()), actionCollection(), "file_export_ascii" );
	(void) new KAction(i18n("Latin-&1 (iso 8859-1)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_latin1" );
	(void) new KAction(i18n("Latin-&2 (iso 8859-2)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_latin2" );
	(void) new KAction(i18n("Latin-&3 (iso 8859-3)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_latin3" );
	(void) new KAction(i18n("Latin-&4 (iso 8859-4)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_latin4" );
	(void) new KAction(i18n("Latin-&5 (iso 8859-5)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_latin5" );
	(void) new KAction(i18n("Latin-&9 (iso 8859-9)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_latin9" );
	(void) new KAction(i18n("&Central European (cp-1250)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_cp1250" );
	(void) new KAction(i18n("&Western European (cp-1252)"), 0, this, SLOT(convertToEnc()), actionCollection(), "file_export_cp1252" );
	(void) KStdAction::quit(this, SLOT(close()), actionCollection(),"file_quit" );

	(void) KStdAction::gotoLine(m_edit, SLOT(gotoLine()), actionCollection(),"edit_goto_line" );
	(void) new KAction(i18n("Next section"), "nextsection", ALT+Key_Down, m_edit, SLOT(gotoNextSectioning()), actionCollection(),"edit_next_section" );
	(void) new KAction(i18n("Prev section"), "prevsection", ALT+Key_Up, m_edit, SLOT(gotoPrevSectioning()), actionCollection(),"edit_prev_section" );
	(void) new KAction(i18n("Next paragraph"), "nextparagraph", ALT+SHIFT+Key_Down, m_edit, SLOT(gotoNextParagraph()), actionCollection(),"edit_next_paragraph" );
	(void) new KAction(i18n("Prev paragraph"), "prevparagraph", ALT+SHIFT+Key_Up, m_edit, SLOT(gotoPrevParagraph()), actionCollection(),"edit_prev_paragraph" );

	(void) new KAction(i18n("Find &in Files..."), ALT+SHIFT+Key_F, this, SLOT(findInFiles()), actionCollection(),"FindInFiles" );

	kdDebug() << "CONNECTING SPELLCHECKER" << endl;
	connect ( viewManager(), SIGNAL(startSpellCheck()), m_spell, SLOT(spellcheck()) );

	(void) new KAction(i18n("Refresh Str&ucture"), "view_tree", Key_F12, this, SLOT(refreshStructure()), actionCollection(),"RefreshStructure" );

	//project actions
	(void) new KAction(i18n("&New Project..."), "window_new", 0, docManager(), SLOT(projectNew()), actionCollection(), "project_new");
	(void) new KAction(i18n("&Open Project..."), "project_open", 0, docManager(), SLOT(projectOpen()), actionCollection(), "project_open");
	m_actRecentProjects =  new KRecentFilesAction(i18n("Open &Recent Project"),  0, docManager(), SLOT(projectOpen(const KURL &)), actionCollection(), "project_openrecent");
	connect(docManager(), SIGNAL(removeFromRecentProjects(const KURL& )), m_actRecentProjects, SLOT(removeURL(const KURL& )));
	connect(docManager(), SIGNAL(addToRecentProjects(const KURL& )), m_actRecentProjects, SLOT(addURL(const KURL& )));
	m_actRecentProjects->loadEntries(m_config, "Projects");

	(void) new KAction(i18n("A&dd Files to Project..."),"project_add", 0, docManager(), SLOT(projectAddFiles()), actionCollection(), "project_add");
	(void) new KAction(i18n("Refresh Project &Tree"), "relation", 0, docManager(), SLOT(buildProjectTree()), actionCollection(), "project_buildtree");
 	(void) new KAction(i18n("&Archive"), "package", 0, this, SLOT(runArchiveTool()), actionCollection(), "project_archive");
	(void) new KAction(i18n("Project &Options"), "configure", 0, docManager(), SLOT(projectOptions()), actionCollection(), "project_options");
	(void) new KAction(i18n("&Close Project"), "fileclose", 0, docManager(), SLOT(projectClose()), actionCollection(), "project_close");

	// new project actions (dani)
	(void) new KAction(i18n("&Show Projects..."), 0, docManager(), SLOT(projectShow()), actionCollection(), "project_show");
	(void) new KAction(i18n("Re&move Files From Project..."),"project_remove", 0, docManager(), SLOT(projectRemoveFiles()), actionCollection(), "project_remove");
	(void) new KAction(i18n("Show Project &Files..."),"project_show", 0, docManager(), SLOT(projectShowFiles()), actionCollection(), "project_showfiles");
	// tbraun
	(void) new KAction(i18n("Open All &Project Files"), 0, docManager(), SLOT(projectOpenAllFiles()), actionCollection(), "project_openallfiles");
	(void) new KAction(i18n("Find in &Project..."), 0, this, SLOT(findInProjects()), actionCollection(),"project_findfiles" );

	//build actions
	(void) new KAction(i18n("Clean"),"trashcan_full",0 , this, SLOT(cleanAll()), actionCollection(),"CleanAll" );
	(void) new KAction(i18n("View Log File"),"viewlog", ALT+Key_0, m_errorHandler, SLOT(ViewLog()), actionCollection(),"ViewLog" );
	(void) new KAction(i18n("Previous LaTeX Error"),"errorprev", 0, m_errorHandler, SLOT(PreviousError()), actionCollection(),"PreviousError" );
	(void) new KAction(i18n("Next LaTeX Error"),"errornext", 0, m_errorHandler, SLOT(NextError()), actionCollection(),"NextError" );
	(void) new KAction(i18n("Previous LaTeX Warning"),"warnprev", 0, m_errorHandler, SLOT(PreviousWarning()), actionCollection(),"PreviousWarning" );
	(void) new KAction(i18n("Next LaTeX Warning"),"warnnext", 0, m_errorHandler, SLOT(NextWarning()), actionCollection(),"NextWarning" );
	(void) new KAction(i18n("Previous LaTeX BadBox"),"bboxprev", 0, m_errorHandler, SLOT(PreviousBadBox()), actionCollection(),"PreviousBadBox" );
	(void) new KAction(i18n("Next LaTeX BadBox"),"bboxnext", 0, m_errorHandler, SLOT(NextBadBox()), actionCollection(),"NextBadBox" );
	m_paStop = new KAction(i18n("&Stop"),"stop",Key_Escape,0,0,actionCollection(),"Stop");
	m_paStop->setEnabled(false);

	(void) new KAction(i18n("Editor View"),"edit",CTRL+Key_E , this, SLOT(showEditorWidget()), actionCollection(),"EditorView" );
	(void) new KAction(i18n("Next Document"),"forward",ALT+Key_Right, viewManager(), SLOT(gotoNextView()), actionCollection(), "gotoNextDocument" );
	(void) new KAction(i18n("Previous Document"),"back",ALT+Key_Left, viewManager(), SLOT(gotoPrevView()), actionCollection(), "gotoPrevDocument" );
	(void) new KAction(i18n("Focus Log/Messages View"), CTRL+ALT+Key_M, this, SLOT(focusLog()), actionCollection(), "focus_log");
	(void) new KAction(i18n("Focus Output View"), CTRL+ALT+Key_O, this, SLOT(focusOutput()), actionCollection(), "focus_output");
	(void) new KAction(i18n("Focus Konsole View"), CTRL+ALT+Key_K, this, SLOT(focusKonsole()), actionCollection(), "focus_konsole");
	(void) new KAction(i18n("Focus Editor View"), CTRL+ALT+Key_E, this, SLOT(focusEditor()), actionCollection(), "focus_editor");

 // CodeCompletion (dani)
	(void) new KAction(i18n("(La)TeX Command"),"complete1",CTRL+Key_Space, m_edit, SLOT(completeWord()), actionCollection(), "edit_complete_word");
	(void) new KAction(i18n("Environment"),"complete2",ALT+Key_Space, m_edit, SLOT(completeEnvironment()), actionCollection(), "edit_complete_env");
	(void) new KAction(i18n("Abbreviation"),"complete3",CTRL+ALT+Key_Space, m_edit, SLOT(completeAbbreviation()), actionCollection(), "edit_complete_abbrev");

	(void) new KAction(i18n("Next Bullet"),"nextbullet",CTRL+ALT+Key_Right, m_edit, SLOT(nextBullet()), actionCollection(), "edit_next_bullet");
	(void) new KAction(i18n("Prev Bullet"),"prevbullet",CTRL+ALT+Key_Left, m_edit, SLOT(prevBullet()), actionCollection(), "edit_prev_bullet");

 // advanced editor (dani)
	(void) new KAction(i18n("Environment (inside)"),"selenv_i",KShortcut("CTRL+Alt+S,E"), m_edit, SLOT(selectEnvInside()), actionCollection(), "edit_select_inside_env");
	(void) new KAction(i18n("Environment (outside)"),"selenv_o",KShortcut("CTRL+Alt+S,F"), m_edit, SLOT(selectEnvOutside()), actionCollection(), "edit_select_outside_env");
	(void) new KAction(i18n("TeX Group (inside)"),"selgroup_i",KShortcut("CTRL+Alt+S,T"), m_edit, SLOT(selectTexgroupInside()), actionCollection(), "edit_select_inside_group");
	(void) new KAction(i18n("TeX Group (outside)"), "selgroup_o",KShortcut("CTRL+Alt+S,U"),m_edit, SLOT(selectTexgroupOutside()), actionCollection(), "edit_select_outside_group");
	(void) new KAction(i18n("Math Group"), "selmath",KShortcut("CTRL+Alt+S,M"),m_edit, SLOT(selectMathgroup()), actionCollection(), "edit_select_mathgroup");
	(void) new KAction(i18n("Paragraph"),"selpar",KShortcut("CTRL+Alt+S,P"),m_edit, SLOT(selectParagraph()), actionCollection(), "edit_select_paragraph");
	(void) new KAction(i18n("Line"),"selline",KShortcut("CTRL+Alt+S,L"),m_edit, SLOT(selectLine()), actionCollection(), "edit_select_line");
	(void) new KAction(i18n("TeX Word"),"selword",KShortcut("CTRL+Alt+S,W"),m_edit, SLOT(selectWord()), actionCollection(), "edit_select_word");

	(void) new KAction(i18n("Environment (inside)"),"delenv_i",KShortcut("CTRL+Alt+T,E"), m_edit, SLOT(deleteEnvInside()), actionCollection(), "edit_delete_inside_env");
	(void) new KAction(i18n("Environment (outside)"),"delenv_o",KShortcut("CTRL+Alt+T,F"),m_edit, SLOT(deleteEnvOutside()), actionCollection(), "edit_delete_outside_env");
	(void) new KAction(i18n("TeX Group (inside)"),"delgroup_i",KShortcut("CTRL+Alt+T,T"), m_edit, SLOT(deleteTexgroupInside()), actionCollection(),"edit_delete_inside_group");
	(void) new KAction(i18n("TeX Group (outside)"),"delgroup_o",KShortcut("CTRL+Alt+T,U"),m_edit, SLOT(deleteTexgroupInside()), actionCollection(), "edit_delete_outside_group");
	(void) new KAction(i18n("Math Group"),"delmath",KShortcut("CTRL+Alt+T,M"),m_edit, SLOT(deleteMathgroup()), actionCollection(), "edit_delete_mathgroup");
	(void) new KAction(i18n("Paragraph"),"delpar",KShortcut("CTRL+Alt+T,P"),m_edit, SLOT(deleteParagraph()), actionCollection(), "edit_delete_paragraph");
	(void) new KAction(i18n("TeX Word"),"delword",KShortcut("CTRL+Alt+T,W"),m_edit, SLOT(deleteWord()), actionCollection(), "edit_delete_word");

	(void) new KAction(i18n("Goto Begin"),"gotobeginenv",KShortcut("CTRL+Alt+E,B"), m_edit, SLOT(gotoBeginEnv()), actionCollection(), "edit_begin_env");
	(void) new KAction(i18n("Goto End"),"gotoendenv",KShortcut("CTRL+Alt+E,E"), m_edit, SLOT(gotoEndEnv()), actionCollection(), "edit_end_env");
	(void) new KAction(i18n("Match"),"matchenv",KShortcut("CTRL+Alt+E,M"), m_edit, SLOT(matchEnv()), actionCollection(), "edit_match_env");
	(void) new KAction(i18n("Close"),"closeenv",KShortcut("CTRL+Alt+E,C"), m_edit, SLOT(closeEnv()), actionCollection(), "edit_close_env");
	(void) new KAction(i18n("Close All"),"closeallenv",KShortcut("CTRL+Alt+E,A"), m_edit, SLOT(closeAllEnv()), actionCollection(), "edit_closeall_env");

	(void) new KAction(i18n("Goto Begin"),"gotobegingroup",KShortcut("CTRL+Alt+G,B"), m_edit, SLOT(gotoBeginTexgroup()), actionCollection(), "edit_begin_group");
	(void) new KAction(i18n("Goto End"),"gotoendgroup",KShortcut("CTRL+Alt+G,E"), m_edit, SLOT(gotoEndTexgroup()), actionCollection(), "edit_end_group");
	(void) new KAction(i18n("Match"),"matchgroup",KShortcut("CTRL+Alt+G,M"), m_edit, SLOT(matchTexgroup()), actionCollection(), "edit_match_group");
	(void) new KAction(i18n("Close"),"closegroup",KShortcut("CTRL+Alt+G,C"), m_edit, SLOT(closeTexgroup()), actionCollection(), "edit_close_group");

	(void) new KAction(i18n("Selection"),"preview_sel",KShortcut("CTRL+Alt+P,S"), this, SLOT(quickPreviewSelection()), actionCollection(),"quickpreview_selection" );
	(void) new KAction(i18n("Environment"),"preview_env",KShortcut("CTRL+Alt+P,E"), this, SLOT(quickPreviewEnvironment()), actionCollection(),"quickpreview_environment" );
	(void) new KAction(i18n("Subdocument"),"preview_subdoc",KShortcut("CTRL+Alt+P,D"), this, SLOT(quickPreviewSubdocument()), actionCollection(),"quickpreview_subdocument" );
	(void) new KAction (i18n ("Mathgroup"), "edu_mathematics", KShortcut("CTRL+Alt+P,M"), this, SLOT(quickPreviewMathgroup()), actionCollection(), "quickpreview_math");

	KileStdActions::setupStdTags(this,this);
	KileStdActions::setupMathTags(this);
	KileStdActions::setupBibTags(this);

	(void) new KAction(i18n("Quick Start"),"quickwizard",0 , this, SLOT(quickDocument()), actionCollection(),"wizard_document" );
	connect(docManager(), SIGNAL(startWizard()), this, SLOT(quickDocument()));
	(void) new KAction(i18n("Tabular"),"wizard_tabular",0 , this, SLOT(quickTabular()), actionCollection(),"wizard_tabular" );
	(void) new KAction(i18n("Array"),"wizard_array",0 , this, SLOT(quickArray()), actionCollection(),"wizard_array" );
	(void) new KAction(i18n("Tabbing"),"wizard_tabbing",0 , this, SLOT(quickTabbing()), actionCollection(),"wizard_tabbing" );
	(void) new KAction(i18n("Floats"),"wizard_float",0, this, SLOT(quickFloat()), actionCollection(),"wizard_float" );
	(void) new KAction(i18n("Math"),"wizard_math",0, this, SLOT(quickMathenv()), actionCollection(),"wizard_mathenv" );
	(void) new KAction(i18n("Postscript Tools"),"wizard_pstools",0 , this, SLOT(quickPostscript()), actionCollection(),"wizard_postscript" );

	(void) new KAction(i18n("Clean"),0 , this, SLOT(cleanBib()), actionCollection(),"CleanBib" );

	ModeAction=new KToggleAction(i18n("Define Current Document as '&Master Document'"),"master",0 , this, SLOT(toggleMode()), actionCollection(),"Mode" );

	KToggleAction *tact = new KToggleAction(i18n("Show S&ide Bar"), 0, 0, 0, actionCollection(),"StructureView" );
	tact->setChecked(KileConfig::sideBar());
	connect(tact, SIGNAL(toggled(bool)), m_sideBar, SLOT(setVisible(bool)));
	connect(m_sideBar, SIGNAL(visibilityChanged(bool )), tact, SLOT(setChecked(bool)));
    connect(m_sideBar, SIGNAL(visibilityChanged(bool )), this, SLOT(sideOrBottomBarChanged(bool)));

	m_actionMessageView = new KToggleAction(i18n("Show Mess&ages Bar"), 0, 0, 0, actionCollection(),"MessageView" );
	m_actionMessageView->setChecked(true);
	connect(m_actionMessageView, SIGNAL(toggled(bool)), m_bottomBar, SLOT(setVisible(bool)));
	connect(m_bottomBar, SIGNAL(visibilityChanged(bool )), m_actionMessageView, SLOT(setChecked(bool)));
    connect(m_bottomBar, SIGNAL(visibilityChanged(bool )), this, SLOT(sideOrBottomBarChanged(bool)));

	if (m_singlemode) {ModeAction->setChecked(false);}
	else {ModeAction->setChecked(true);}

	WatchFileAction=new KToggleAction(i18n("Watch File Mode"),"watchfile",0 , this, SLOT(toggleWatchFile()), actionCollection(), "WatchFile");
	if (m_bWatchFile) {WatchFileAction->setChecked(true);}
	else {WatchFileAction->setChecked(false);}

	setHelpMenuEnabled(false);
	const KAboutData *aboutData = KGlobal::instance()->aboutData();
	KHelpMenu *help_menu = new KHelpMenu( this, aboutData);

	KStdAction::tipOfDay(this, SLOT(showTip()), actionCollection(), "help_tipofday");

	(void) new KAction(i18n("teTeX Guide"),KShortcut("CTRL+Alt+H,G"), m_help, SLOT(helpTetexGuide()), actionCollection(), "help_tetex_guide");
	(void) new KAction(i18n("teTeX Doc"),KShortcut("CTRL+Alt+H,D"), m_help, SLOT(helpTetexDoc()), actionCollection(), "help_tetex_doc");
	(void) new KAction(i18n("LaTeX"),KShortcut("CTRL+Alt+H,L"), m_help, SLOT(helpLatexIndex()), actionCollection(), "help_latex_index");
	(void) new KAction(i18n("LaTeX Command"),KShortcut("CTRL+Alt+H,C"), m_help, SLOT(helpLatexCommand()), actionCollection(), "help_latex_command");
	(void) new KAction(i18n("LaTeX Subject"),KShortcut("CTRL+Alt+H,S"), m_help, SLOT(helpLatexSubject()), actionCollection(), "help_latex_subject");
	(void) new KAction(i18n("LaTeX Env"),KShortcut("CTRL+Alt+H,E"), m_help, SLOT(helpLatexEnvironment()), actionCollection(), "help_latex_env");
	(void) new KAction(i18n("Context Help"),KShortcut("CTRL+Alt+H,K"), m_help, SLOT(helpKeyword()), actionCollection(), "help_context");
	(void) new KAction(i18n("Documentation Browser"),KShortcut("CTRL+Alt+H,B"), m_help, SLOT(helpDocBrowser()), actionCollection(), "help_docbrowser");

	(void) new KAction(i18n("LaTeX Reference"),"help",0 , this, SLOT(helpLaTex()), actionCollection(),"help_latex_reference" );
	(void) KStdAction::helpContents(help_menu, SLOT(appHelpActivated()), actionCollection(), "help_handbook");
	(void) KStdAction::reportBug (help_menu, SLOT(reportBug()), actionCollection(), "report_bug");
	(void) KStdAction::aboutApp(help_menu, SLOT(aboutApplication()), actionCollection(),"help_aboutKile" );
	(void) KStdAction::aboutKDE(help_menu, SLOT(aboutKDE()), actionCollection(),"help_aboutKDE" );
	KAction *kileconfig = KStdAction::preferences(this, SLOT(generalOptions()), actionCollection(),"settings_configure" );
	kileconfig->setIcon("configure_kile");

	(void) KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection(),"settings_keys" );
	(void) KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection(),"settings_toolbars" );
	new KAction(i18n("&System Check..."), 0, this, SLOT(slotPerformCheck()), actionCollection(), "settings_perform_check");

	m_menuUserTags = new KActionMenu(i18n("User Tags"), SmallIcon("label"), actionCollection(),"menuUserTags");
	m_menuUserTags->setDelayed(false);
	setupUserTagActions();

	actionCollection()->readShortcutSettings();

	m_pFullScreen = KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()), actionCollection(), this);
}

void Kile::setupTools()
{
	kdDebug() << "==Kile::setupTools()===================" << endl;
	QStringList tools = KileTool::toolList(m_config);
	QString toolMenu;
	QPtrList<KAction> *pl;

	unplugActionList("list_compilers");
	unplugActionList("list_converters");
	unplugActionList("list_quickies");
	unplugActionList("list_viewers");
	unplugActionList("list_other");

	for ( uint i = 0; i < tools.count(); ++i)
	{
		QString grp = KileTool::groupFor(tools[i], m_config);
		kdDebug() << tools[i] << " is using group: " << grp << endl;
		m_config->setGroup(KileTool::groupFor(tools[i], m_config));
		toolMenu = KileTool::menuFor(tools[i], m_config);

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

		kdDebug() << "\tadding " << tools[i] << " " << toolMenu << " #" << pl->count() << endl;

		if ( action(QString("tool_"+tools[i]).ascii()) == 0L )
		{
			KAction *act = new KAction(tools[i], KileTool::iconFor(tools[i], m_config), KShortcut(), this, SLOT(runTool()), actionCollection(), QString("tool_"+tools[i]).ascii());
			pl->append(act);
		}
	}

	cleanUpActionList(m_listCompilerActions, tools);
	cleanUpActionList(m_listViewerActions, tools);
	cleanUpActionList(m_listConverterActions, tools);
	cleanUpActionList(m_listQuickActions, tools);
	cleanUpActionList(m_listOtherActions, tools);

	plugActionList("list_compilers", m_listCompilerActions);
	plugActionList("list_viewers", m_listViewerActions);
	plugActionList("list_converters", m_listConverterActions);
	plugActionList("list_quickies", m_listQuickActions);
	plugActionList("list_other", m_listOtherActions);

	actionCollection()->readShortcutSettings("Shortcuts", m_config);
}

void Kile::cleanUpActionList(QPtrList<KAction> &list, const QStringList & tools)
{
	for ( KAction *act = list.first(); act; act = list.next() )
	{
		if ( action(act->name()) != 0L && !tools.contains(QString(act->name()).mid(5)) )
		{
			list.remove(act);
			if ( act->isPlugged(toolBar("toolsToolBar")) ) act->unplug(toolBar("toolsToolBar"));
		}
	}
}

void Kile::setupUserTagActions()
{
	KShortcut tagaccels[10] = {CTRL+SHIFT+Key_1, CTRL+SHIFT+Key_2,CTRL+SHIFT+Key_3,CTRL+SHIFT+Key_4,CTRL+SHIFT+Key_5,CTRL+SHIFT+Key_6,CTRL+SHIFT+Key_7,
		CTRL+SHIFT+Key_8,CTRL+SHIFT+Key_9,CTRL+SHIFT+Key_0};

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

	actionCollection()->readShortcutSettings("Shortcuts", m_config);
}

void Kile::restoreFilesAndProjects(bool allowRestore)
{
	if (! (allowRestore && KileConfig::restore()) )
	  return;

	QFileInfo fi;

	KURL url;
	for (uint i=0; i < m_listProjectsOpenOnStart.count(); ++i)
	{
		fi.setFile(m_listProjectsOpenOnStart[i]);
		if (fi.isReadable()) docManager()->projectOpen(KURL::fromPathOrURL(m_listProjectsOpenOnStart[i]), i, m_listProjectsOpenOnStart.count());
	}

	for (uint i=0; i < m_listDocsOpenOnStart.count(); ++i)
	{
		fi.setFile(m_listDocsOpenOnStart[i]);
		if (fi.isReadable())
			docManager()->fileOpen(KURL::fromPathOrURL(m_listDocsOpenOnStart[i]));
	}

	if (ModeAction) ModeAction->setChecked(!m_singlemode);
	updateModeStatus();

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();

    kdDebug() << "lastDocument=" << KileConfig::lastDocument() << endl;
	Kate::Document *doc = docManager()->docFor(KURL::fromPathOrURL(KileConfig::lastDocument()));
	if (doc) viewManager()->switchToTextView(doc->url());
}

void Kile::setActive()
{
	kdDebug() << "ACTIVATING" << endl;
	kapp->mainWidget()->raise();
	kapp->mainWidget()->setActiveWindow();
}

void Kile::showTip()
{
    KTipDialog::showTip(this, "kile/tips", true);
}

void Kile::setLine( const QString &line )
{
	bool ok;
	uint l=line.toUInt(&ok,10);
	Kate::View *view = viewManager()->currentTextView();
	if (view && ok)
  	{
		this->show();
		this->raise();
		view->setFocus();
		view->gotoLineNumber(l-1);

		showEditorWidget();
		newStatus();
  	}
}

void Kile::setCursor(const KURL &url, int parag, int index)
{
	Kate::Document *doc = docManager()->docFor(url);
	if (doc)
	{
		Kate::View *view = (Kate::View*)doc->views().first();
		if (view)
		{
			view->setCursorPositionReal(parag, index);
			view->setFocus();
		}
	}
}

void Kile::runArchiveTool()
{
	this->run("Archive");
}

void Kile::runArchiveTool(const KURL &url)
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
	//kdDebug() << "==Kile::activateView==========================" << endl;
	if (!w->inherits("Kate::View"))
		return;

	//disable gui updates to avoid flickering of toolbars
	setUpdatesEnabled(false);
	Kate::View* view = (Kate::View*)w;
	if (view->isActive()) return;

	for (uint i=0; i< viewManager()->textViews().count(); ++i)
	{
		guiFactory()->removeClient(viewManager()->textView(i));
		viewManager()->textView(i)->setActive(false);
	}

	guiFactory()->addClient(view);
	view->setActive( true );

	// remove menu entry to config Kate
	checkKateSettings();

	setUpdatesEnabled(true);

	if (updateStruct) viewManager()->updateStructure();
}

void Kile::updateModeStatus()
{
	kdDebug() << "==Kile::updateModeStatus()==========" << endl;
	KileProject *project = docManager()->activeProject();
	QString shortName = m_masterName;
	int pos = shortName.findRev('/');
	shortName.remove(0,pos+1);

	if (project)
	{
		if (m_singlemode)
			statusBar()->changeItem(i18n("Project: %1").arg(project->name()), ID_HINTTEXT);
		else
			statusBar()->changeItem(i18n("Project: %1 (Master document: %2)").arg(project->name()).arg(shortName), ID_HINTTEXT);
	}
	else
	{
		if (m_singlemode)
			statusBar()->changeItem(i18n("Normal mode"), ID_HINTTEXT);
		else
			statusBar()->changeItem(i18n("Master document: %1").arg(shortName), ID_HINTTEXT);
	}

	if (m_singlemode)
	{
		ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
		ModeAction->setChecked(false);
	}
	else
	{
		ModeAction->setText(i18n("Normal mode (current master document: %1)").arg(shortName));
		ModeAction->setChecked(true);
	}

	// enable or disable entries in Kile'S menu
	updateMenu();
}

void Kile::openDocument(const QString & url)
{
	kdDebug() << "==Kile::openDocument(" << url << ")==========" << endl;
	docManager()->fileSelected(KURL::fromPathOrURL(url));
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
	docManager()->projectOpen(KURL::fromPathOrURL(proj));
}

void Kile::focusPreview()
{
	m_bottomBar->showPage(m_previewView);
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
	Kate::View *view = viewManager()->currentTextView();
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
    Kate::View *view = viewManager()->currentTextView();
    if (view) KileConfig::setLastDocument(view->getDoc()->url().path());
    else KileConfig::setLastDocument("");

	//don't close Kile if embedded viewers are present
	kdDebug() << "==bool Kile::queryClose(" << m_currentState << ")==========" << endl;
	if ( m_currentState != "Editor" )
	{
		resetPart();
		return false;
	}

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();

	kdDebug() << "#projects = " << docManager()->projects()->count() << endl;
	for (uint i=0; i < docManager()->projects()->count(); ++i)
	{
		m_listProjectsOpenOnStart.append(docManager()->projects()->at(i)->url().path());
	}

	bool stage1 = docManager()->projectCloseAll();
	bool stage2 = true;

	if (stage1)
	{
		for (uint i=0; i < viewManager()->textViews().count(); ++i)
		{
			m_listDocsOpenOnStart.append(viewManager()->textView(i)->getDoc()->url().path());
		}
		stage2 =docManager()->fileCloseAll();
	}

	return stage1 && stage2;
}

void Kile::showDocInfo(Kate::Document *doc)
{
	if (doc == 0L)
	{
		Kate::View *view = viewManager()->currentTextView();

		if (view)
			doc = view->getDoc();
		else
			return;
	}

	KileDocument::TextInfo *docinfo = docManager()->textInfoFor(doc);
	KileProject *project = KileInfo::docManager()->activeProject();
	if (docinfo) // we have to ensure that we always get a _valid_ docinfo object
	{
		KileStatsDlg *dlg = new KileStatsDlg( project,docinfo, this, 0, "");
		dlg->exec();
		delete dlg;
	}
	else
		kdWarning() << "There is no KileDocument::Info object belonging to this document!" << endl;
}

void Kile::convertToASCII(Kate::Document *doc)
{
	if (doc == 0)
	{
		Kate::View *view = viewManager()->currentTextView();

		if (view) doc = view->getDoc();
		else return;
	}

	ConvertIO io(doc);
	ConvertEncToASCII conv = ConvertEncToASCII(doc->encoding(), &io);
	doc->setEncoding("ISO 8859-1");
	conv.convert();
}

void Kile::convertToEnc(Kate::Document *doc)
{
	if (doc == 0)
	{
		Kate::View *view = viewManager()->currentTextView();

		if (view) doc = view->getDoc();
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
	Kate::View *view = viewManager()->currentTextView();

	int para = 0;

	if (view)
	{
		para = view->cursorLine();
	}

	return para;
}

void Kile::newCaption()
{
	Kate::View *view = viewManager()->currentTextView();
	if (view)
	{
		setCaption(i18n("Document: %1").arg(getName(view->getDoc())));
		if (m_bottomBar->currentPage()->inherits("KileWidget::Konsole")) m_texKonsole->sync();
	}
	else
		setCaption("");
}

void Kile::grepItemSelected(const QString &abs_filename, int line)
{
	kdDebug() << "Open file: "
		<< abs_filename << " (" << line << ")" << endl;
	docManager()->fileOpen(KURL::fromPathOrURL(abs_filename));
	setLine(QString::number(line));
}

void Kile::findInFiles()
{
	static QGuardedPtr<KileGrepDialog> dlg = 0;

	if ( ! dlg )
	{
		kdDebug() << "grep guard: create findInFiles dlg" << endl;
		dlg = new KileGrepDialog(this,this,KileGrep::Directory);
		dlg->show();
		connect(dlg, SIGNAL(itemSelected(const QString &, int)),
		        this, SLOT(grepItemSelected(const QString &, int)));
	}
	else
	{
		kdDebug() << "grep guard: show findInFiles dlg" << endl;
		dlg->setActiveWindow();
	}
}

void Kile::findInProjects()
{
	static QGuardedPtr<KileGrepDialog> project_dlg = 0;

	if ( ! project_dlg )
	{
		kdDebug() << "grep guard: create findInProjects dlg" << endl;
		project_dlg = new KileGrepDialog(this,this,KileGrep::Project);
		project_dlg->show();
		connect(project_dlg, SIGNAL(itemSelected(const QString &, int)),
		        this, SLOT(grepItemSelected(const QString &, int)));
	}
	else
	{
		kdDebug() << "grep guard: show findInProjects dlg" << endl;
		project_dlg->setActiveWindow();
	}
}

/////////////////// PART & EDITOR WIDGET //////////
void Kile::showEditorWidget()
{
	if(!resetPart())
		return;
	setCentralWidget(m_topWidgetStack);
	m_topWidgetStack->show();
	m_horizontalSplitter->show();
	m_verticalSplitter->show();

	Kate::View *view = viewManager()->currentTextView();
	if (view) view->setFocus();

    setupStatusBar();
	updateModeStatus();
	newCaption();
}


bool Kile::resetPart()
{
	kdDebug() << "==Kile::resetPart()=============================" << endl;
	kdDebug() << "\tcurrent state " << m_currentState << endl;
	kdDebug() << "\twant state " << m_wantState << endl;

	KParts::ReadOnlyPart *part = (KParts::ReadOnlyPart*)m_partManager->activePart();

	if (part && m_currentState != "Editor")
	{
		if(part->closeURL())
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
	m_partManager->setActivePart( 0L);
	return true;
}

void Kile::activePartGUI(KParts::Part * part)
{
	kdDebug() << "==Kile::activePartGUI()=============================" << endl;
	kdDebug() << "\tcurrent state " << m_currentState << endl;
	kdDebug() << "\twant state " << m_wantState << endl;

	createGUI(part);
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
	if (ext && ext->metaObject()->slotNames().contains( "print()" ) ) //part is a BrowserExtension, connect printAction()
	{
		connect(m_paPrint, SIGNAL(activated()), ext, SLOT(print()));
		m_paPrint->plug(toolBar("mainToolBar"),3); //plug this action into its default location
		m_paPrint->setEnabled(true);
	}
	else
	{
		if (m_paPrint->isPlugged(toolBar("mainToolBar")))
			m_paPrint->unplug(toolBar("mainToolBar"));

		m_paPrint->setEnabled(false);
	}

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

	if ( m_currentState == "Editor" )
	{
		mainToolBar  = toolBar("mainToolBar")->isShown();
		buildToolBar = toolBar("buildToolBar")->isShown();
		errorToolBar = toolBar("errorToolBar")->isShown();
		toolsToolBar = toolBar("toolsToolBar")->isShown();
		editToolBar  = toolBar("editToolBar")->isShown();
		mathToolBar  = toolBar("mathToolBar")->isShown();
	}

	if ( wantState == "HTMLpreview" )
	{
		stateChanged( "HTMLpreview");
		setViewerToolBars();
		enableKileGUI(false);
	}
	else if ( wantState == "Viewer" )
	{
		stateChanged( "Viewer" );
		setViewerToolBars();
		enableKileGUI(false);
	}
	else
	{
		stateChanged( "Editor" );
		m_wantState="Editor";
		m_topWidgetStack->raiseWidget(0);
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
	for (uint i=0; i<menubar->count(); ++i) {
		id = menubar->idAt(i);
		QPopupMenu *popup = menubar->findItem(id)->popup();
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
	   << "file_save_all" << "template_create" << "Statistics"
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
	   << "tag_env_eqnarray" << "tag_env_eqnarray*" << "tag_env_array"
	   << "tag_env_multline" << "tag_env_multline*" << "tag_env_split"
	   << "tag_env_gather" << "tag_env_gather*" << "tag_env_align" << "tag_env_align*"
	   << "tag_env_flalign" << "tag_env_flalign*" << "tag_env_alignat" << "tag_env_alignat*"
	   << "tag_env_aligned" << "tag_env_gathered" << "tag_env_alignedat" << "tag_env_cases"
	   << "tag_bibliographystyle" << "tag_bibliography" << "tag_bib_article" << "tag_bib_inproc"
	   << "tag_bib_incol" << "tag_bib_inbook" << "tag_bib_proceedings" << "tag_bib_book"
	   << "tag_bib_booklet" << "tag_bib_phdthesis" << "tag_bib_masterthesis" << "tag_bib_techreport"
	   << "tag_bib_manual" << "tag_bib_unpublished" << "tag_bib_misc" << "CleanBib"
	   << "tag_textit" << "tag_textsl" << "tag_textbf" << "tag_underline"
	   << "tag_texttt" << "tag_textsc" << "tag_emph"
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
	kdDebug() << "==Kile::updateKileMenu()====================" << endl;
	KAction *a;
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
	if ( a )
		a->setEnabled( project_open && docManager()->projects()->count()>1 );

	// update file menus
	m_actRecentFiles->setEnabled( m_actRecentFiles->items().count() > 0 );
	bool file_open = ( viewManager()->currentTextView() );
	kdDebug() << "\tprojectopen=" << project_open << " fileopen=" << file_open << endl;

	QMenuBar *menubar = menuBar();
	for ( uint i=0; i<menubar->count(); ++i ) {
		int menu_id = menubar->idAt(i);
		QPopupMenu *menu = menubar->findItem(menu_id)->popup();
		if ( menu ) {
			QString menu_name = menu->name();
			for ( uint j=0; j<menu->count(); ++j ) {
				int sub_id = menu->idAt(j);
				QPopupMenu *submenu = menu->findItem(sub_id)->popup();
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

	// update action lists
	KActionPtrList actions = actionCollection()->actions();
	KActionPtrList::Iterator itact;
	for ( itact=actions.begin(); itact!=actions.end(); ++itact )
	{
		if ( m_dictMenuAction.contains( (*itact)->name() ) )
			(*itact)->setEnabled(file_open);
	}

	updateActionList(&m_listQuickActions,file_open);
	updateActionList(&m_listCompilerActions,file_open);
	updateActionList(&m_listConverterActions,file_open);
	updateActionList(&m_listViewerActions,file_open);
	updateActionList(&m_listOtherActions,file_open);

}

void Kile::updateActionList(QPtrList<KAction> *list, bool state)
{
	for ( KAction *a=list->first(); a; a=list->next() ) {
		a->setEnabled(state);
	}
}

//TODO: move to KileView::Manager
void Kile::prepareForPart(const QString & state)
{
	kdDebug() << "==Kile::prepareForPart====================" << endl;

	if ( m_currentState == "Editor" && state == "Editor" ) return;

	resetPart();
	m_wantState = state;

	//deactivate kateparts
	for (uint i=0; i<viewManager()->textViews().count(); ++i)
	{
		guiFactory()->removeClient(viewManager()->textView(i));
		viewManager()->textView(i)->setActive(false);
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
		Kate::Document *doc = activeTextDocument();
		if (doc)
			docinfo = docManager()->textInfoFor(doc);
		else
		{
			m_logWidget->printMsg(KileTool::Error, noactivedoc, i18n("Clean"));
			return;
		}
	}

	if (docinfo) docManager()->cleanUpTempFiles(docinfo, false);
}

void Kile::refreshStructure()
{
	viewManager()->updateStructure(true);
}


/////////////////////// LATEX TAGS ///////////////////
void Kile::insertTag(const KileAction::TagData& data)
{
	logWidget()->clear();

	if ( data.description.length() > 0 )
	{
		outputView()->showPage(logWidget());
		setLogPresent(false);
		logWidget()->append(data.description);
	}

	Kate::View *view = viewManager()->currentTextView();

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
	// insert AMS tag
	insertTag(data);

	// check if \usepackage{amsmath} was found in the main document
	bool amsmath = false;
	KileDocument::Info *docinfo = docManager()->textInfoFor(getCompileName());
	if ( docinfo ) {
		const QStringList *packagelist = allPackages(docinfo);
		for (uint i=0; i<packagelist->count(); ++i) {
			if ( (*packagelist)[i] == "amsmath" ) {
				amsmath = true;
				break;
			}
		}
	}

	// if this command was not found, because it was not found or the
	// main file is not opened, we will once give a warning
	if ( ! amsmath  ) {
		KMessageBox::information(0,"<center>"+i18n("You must include '\\usepackage{amsmath}' to use an AMS command like this.")+"</center>",i18n("AMS Information"),i18n("amsmath package warning"));
	}
}

void Kile::insertText(const QString &text)
{
	insertTag( KileAction::TagData(QString::null,text,"%C",0,0) );
}

void Kile::quickDocument()
{
	KileDialog::QuickDocument *dlg = new KileDialog::QuickDocument(m_config, this,"Quick Start",i18n("Quick Start"));

	if ( dlg->exec() )
	{
		if (!viewManager()->currentTextView())
		{
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
	if ( !viewManager()->currentTextView() ) return;

	KileDialog::TabularDialog *dlg = new KileDialog::TabularDialog(this,m_config,m_latexCommands,tabularenv);
	if ( dlg->exec() ) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickTabbing()
{
	if ( !viewManager()->currentTextView() ) return;
	KileDialog::QuickTabbing *dlg = new KileDialog::QuickTabbing(m_config,this,this,"Tabbing", i18n("Tabbing"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickFloat()
{
	if ( !viewManager()->currentTextView() ) return;

	KileDialog::FloatEnvironmentDialog *dlg = new KileDialog::FloatEnvironmentDialog(m_config,this,this);
	if ( dlg->exec() ) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickMathenv()
{
	if ( !viewManager()->currentTextView() ) return;

	KileDialog::MathEnvironmentDialog *dlg = new KileDialog::MathEnvironmentDialog(this,m_config,this,m_latexCommands);
	if ( dlg->exec() ) {
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickPostscript()
{
	QString startdir = QDir::homeDirPath();
	QString texfilename = QString::null;

	Kate::View *view = viewManager()->currentTextView();
	if ( view ) {
		startdir = QFileInfo(view->getDoc()->url().path()).dirPath();
		texfilename = getCompileName();
	}

	KileDialog::PostscriptDialog *dlg = new KileDialog::PostscriptDialog(this,texfilename,startdir,m_logWidget,m_outputWidget);
	dlg->exec();
	delete dlg;
}

void Kile::insertMetaPost(QListBoxItem *)
{
	QString mpcode = m_mpview->currentText();
	if (mpcode!="----------") insertTag(mpcode,QString::null,mpcode.length(),0);
}

void Kile::helpLaTex()
{
	QString loc = locate("html","en/kile/latexhelp.html");
	KileTool::ViewHTML *tool = new KileTool::ViewHTML("ViewHTML", m_manager, false);
	tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
	tool->setSource(loc);
	tool->setTargetPath(loc);
	tool->prepareToRun();
	m_manager->run(tool);
}

void Kile::editUserMenu()
{
	KileDialog::UserTags *dlg = new KileDialog::UserTags(m_listUserTags, this, "Edit User Tags", i18n("Edit User Tags"));

	if ( dlg->exec() )
	{
		//remove all actions
		uint len = m_listUserTagsActions.count();
		for (uint j=0; j< len; ++j)
		{
			KAction *menuItem = m_listUserTagsActions.getLast();
			m_menuUserTags->remove(menuItem);
			m_listUserTagsActions.removeLast();
			delete menuItem;
		}
		if ( len > 0 )
			m_menuUserTags->remove(m_actionEditSeparator);
		m_menuUserTags->remove(m_actionEditTag);

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
	bool old=false;

	m_bShowUserMovedMessage = (version < 5);

	//if the kilerc file is old some of the configuration
	//date must be set by kile, even if the keys are present
	//in the kilerc file
	if ( version < KILERC_VERSION ) old=true;

	if ( version < 4 )
	{
		KileTool::Factory *factory = new KileTool::Factory(0, m_config);
		kdDebug() << "WRITING STD TOOL CONFIG" << endl;
		factory->writeStdConfig();
		delete factory;
	}

	//delete old editor key
	if (m_config->hasGroup("Editor") )
	{
		m_config->deleteGroup("Editor");
	}

	m_config->setGroup( "User" );
	int len = m_config->readNumEntry("nUserTags",0);
	for (int i = 0; i < len; ++i)
	{
		m_listUserTags.append(KileDialog::UserTags::splitTag(m_config->readEntry("userTagName"+QString::number(i),i18n("no name")) , m_config->readEntry("userTag"+QString::number(i),"") ));
	}

	//convert user tools to new KileTool classes
	userItem tempItem;
	len= m_config->readNumEntry("nUserTools",0);
	for (int i=0; i< len; ++i)
	{
		tempItem.name=m_config->readEntry("userToolName"+QString::number(i),i18n("no name"));
		tempItem.tag =m_config->readEntry("userTool"+QString::number(i),"");
		m_listUserTools.append(tempItem);
	}
	if ( len > 0 )
	{
 		//move the tools
		m_config->writeEntry("nUserTools", 0);
		for ( int i = 0; i < len; ++i)
		{
			tempItem = m_listUserTools[i];
			m_config->setGroup("Tools");
			m_config->writeEntry(tempItem.name, "Default");

			KileTool::setGUIOptions(tempItem.name, "Other", "gear", m_config);

			m_config->setGroup(KileTool::groupFor(tempItem.name, "Default"));
			QString bin = KRun::binaryName(tempItem.tag, false);
			m_config->writeEntry("command", bin);
			m_config->writeEntry("options", tempItem.tag.mid(bin.length()));
			m_config->writeEntry("class", "Base");
			m_config->writeEntry("type", "Process");
			m_config->writeEntry("from", "");
			m_config->writeEntry("to", "");

			if ( i < 10 )
			{
				m_config->setGroup("Shortcuts");
				m_config->writeEntry("tool_" + tempItem.name, "Alt+Shift+" + QString::number(i + 1) ); //should be alt+shift+
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
	m_config->setGroup("FilesOpenOnStart");
	int n = m_config->readNumEntry("NoDOOS", 0);
	for (int i=0; i < n; ++i)
		m_listDocsOpenOnStart.append(m_config->readPathEntry("DocsOpenOnStart"+QString::number(i), ""));

	n = m_config->readNumEntry("NoPOOS", 0);
	for (int i=0; i < n; ++i)
		m_listProjectsOpenOnStart.append(m_config->readPathEntry("ProjectsOpenOnStart"+QString::number(i), ""));
}

void Kile::readConfig()
{
	enableAutosave(KileConfig::autosave());
	m_edit->complete()->readConfig(m_config);
	//m_edit->initDoubleQuotes();
	m_edit->readConfig();
	docManager()->updateInfos();
	m_jScriptManager->readConfig();
	m_sideBar->setPageVisible(m_scriptsManagementWidget, KileConfig::scriptingEnabled());
}

void Kile::saveSettings()
{
	showEditorWidget();

	m_fileSelector->writeConfig();

	KileConfig::setInputEncoding(m_fileSelector->comboEncoding()->lineEdit()->text());

	// Store recent files
	m_actRecentFiles->saveEntries(m_config, "Recent Files");
	m_actRecentProjects->saveEntries(m_config, "Projects");

	m_config->deleteGroup("FilesOpenOnStart");
	if (KileConfig::restore())
	{
		m_config->setGroup("FilesOpenOnStart");
		m_config->writeEntry("NoDOOS", m_listDocsOpenOnStart.count());
		for (uint i=0; i < m_listDocsOpenOnStart.count(); ++i)
			m_config->writePathEntry("DocsOpenOnStart"+QString::number(i), m_listDocsOpenOnStart[i]);

		m_config->writeEntry("NoPOOS", m_listProjectsOpenOnStart.count());
		for (uint i=0; i < m_listProjectsOpenOnStart.count(); ++i)
			m_config->writePathEntry("ProjectsOpenOnStart"+QString::number(i), m_listProjectsOpenOnStart[i]);

		if (!m_singlemode)
			KileConfig::setMaster(m_masterName);
		else
			KileConfig::setMaster("");
	}

	m_config->setGroup( "User" );

	m_config->writeEntry("nUserTags",static_cast<int>(m_listUserTags.size()));
	for (uint i=0; i < m_listUserTags.size(); ++i)
	{
		KileAction::TagData td( m_listUserTags[i]);
		m_config->writeEntry( "userTagName"+QString::number(i),  td.text );
		m_config->writeEntry( "userTag"+QString::number(i), KileDialog::UserTags::completeTag(td) );
	}

	actionCollection()->writeShortcutSettings();
	saveMainWindowSettings(m_config, "KileMainWindow" );

	scriptManager()->writeConfig();

	KileConfig::setRCVersion(KILERC_VERSION);
	KileConfig::setMainwindowWidth(width());
	KileConfig::setMainwindowHeight(height());

	QValueList<int> sizes;
	QValueList<int>::Iterator it;
	sizes=m_horizontalSplitter->sizes();
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

	// sync vertical splitter and size of bottom bar
	int sizeBottomBar = m_bottomBar->size();
	if ( m_bottomBar->isVisible() )
		sizeBottomBar = m_verSplitBottom;
	else
		m_verSplitBottom = sizeBottomBar;

	KileConfig::setHorizontalSplitterLeft(m_horSplitLeft);
	KileConfig::setHorizontalSplitterRight(m_horSplitRight);
	KileConfig::setVerticalSplitterTop(m_verSplitTop);
	KileConfig::setVerticalSplitterBottom(m_verSplitBottom);

	KileConfig::setSideBar(m_sideBar->isVisible());
	KileConfig::setSideBarSize(m_sideBar->size());
	KileConfig::setBottomBar(m_bottomBar->isVisible());
	KileConfig::setBottomBarSize(sizeBottomBar);

	if(m_sideBar->isVisible())
		KileConfig::setSelectedLeftView(m_sideBar->currentTab());
	else
		KileConfig::setSelectedLeftView(-1);
	KileConfig::writeConfig();
	m_config->sync();
}

/////////////////  OPTIONS ////////////////////
void Kile::toggleMode()
{
	if (!m_singlemode)
	{
		ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
		ModeAction->setChecked(false);
		m_logPresent=false;
		m_singlemode=true;
		m_masterName=QString::null;
	}
	else if (m_singlemode && viewManager()->currentTextView())
	{
		m_masterName=getName();
		if ( KileUntitled::isUntitled(m_masterName) || m_masterName.isEmpty())
		{
			ModeAction->setChecked(false);
			KMessageBox::error(this, i18n("In order to define the current document as a master document, it has to be saved first."));
			m_masterName="";
			return;
		}

		QString shortName = m_masterName;
		int pos;
		while ( (pos = (int)shortName.find('/')) != -1 )
			shortName.remove(0,pos+1);

		ModeAction->setText(i18n("Normal mode (current master document: %1)").arg(shortName));
		ModeAction->setChecked(true);
		m_singlemode=false;
	}
	else
		ModeAction->setChecked(false);

	updateModeStatus();
	kdDebug() << "SETTING master to " << m_masterName << " singlemode = " << m_singlemode << endl;
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
	KileDialog::Config *dlg = new KileDialog::Config(m_config,this,this);

	if (dlg->exec())
	{
		// check new Kate settings
		checkKateSettings();

		// update new settings
		readConfig();
		setupTools();

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
	m_config->setGroup("Kate Document Defaults");
	return m_config->readBoolEntry("KTextEditor Plugin ktexteditor_docwordcompletion",false);
}

void Kile::checkKateSettings()
{
	// editor settings were only available with an opened document
	Kate::View *view = viewManager()->currentTextView();
	if ( view )
	{
		// remove menu entry to config Kate
		viewManager()->unplugKatePartMenu(view);
	}
}

void Kile::slotPerformCheck()
{
	KileDialog::ConfigChecker *dlg = new KileDialog::ConfigChecker(this);
	dlg->exec();
	delete dlg;
}

/////////////// KEYS - TOOLBARS CONFIGURATION ////////////////
void Kile::configureKeys()
{
	KKeyDialog dlg( false, this );
	QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
	for( QPtrListIterator<KXMLGUIClient> it( clients );	it.current(); ++it )
	{
		dlg.insert( (*it)->actionCollection() );
	}
	dlg.configure();
	actionCollection()->writeShortcutSettings("Shortcuts", m_config);
}

void Kile::configureToolbars()
{
	saveMainWindowSettings(m_config, "KileMainWindow" );
	KEditToolbar dlg(factory());
	dlg.exec();

	applyMainWindowSettings(m_config, "KileMainWindow" );
 	showToolBars(m_currentState);
}

void Kile::changeInputEncoding()
{
	Kate::View *view = viewManager()->currentTextView();
	if (view)
	{
		bool modified = view->getDoc()->isModified();

		QString encoding=m_fileSelector->comboEncoding()->lineEdit()->text();
		QString text = view->getDoc()->text();

		view->getDoc()->setEncoding(encoding);
        unsigned int mode = view->getDoc()->hlMode(); //remember the highlighting mode

		//reload the document so that the new encoding takes effect
		view->getDoc()->openURL(view->getDoc()->url());
        view->getDoc()->setHlMode(mode);
		view->getDoc()->setModified(modified);
	}
}

//////////////////// CLEAN BIB /////////////////////
void Kile::cleanBib()
{
	Kate::View *view = viewManager()->currentTextView();
	if ( ! view )
		return;

	QRegExp reOptional( "(ALT|OPT)(\\w+)\\s*=\\s*(\\S.*)" );
	QRegExp reNonEmptyEntry( ".*\\w.*" );

	QString s;
	uint i=0;
	while(i < view->getDoc()->numLines())
	{
		s = view->getDoc()->textLine(i);

		// do we have a line that starts with ALT or OPT?
		if ( reOptional.search( s ) >= 0 )
		{
				// yes! capture type and entry
				QString type = reOptional.cap( 2 );
				QString entry = reOptional.cap( 3 );
				view->getDoc()->removeLine( i );
				view->getDoc()->setModified(true);
				if ( reNonEmptyEntry.search( entry ) >= 0 )
				{
					type.append( " = " );
					type.append( entry );
					view->getDoc()->insertLine( i, type );
					++i;
				}
		}
		else
			++i;
	}
	uint j=0;
	for ( i=0; i < view->getDoc()->numLines() ; i++ )
	{
		j = i+1;
		if ( j < view->getDoc()->numLines()  && view->getDoc()->textLine(j).contains( QRegExp("^\\s*\\}\\s*$") ) )
			{
				s =  view->getDoc()->textLine( i );
				view->getDoc()->removeLine( i );
				s.remove( QRegExp(",\\s*$") );
				view->getDoc()->setModified( true );
				view->getDoc()->insertLine( i, s);
			}
	}
}

void Kile::includeGraphics()
{
	Kate::View *view = viewManager()->currentTextView();
	if ( !view ) return;

	QFileInfo fi( view->getDoc()->url().path() );
	KileDialog::IncludeGraphics *dialog = new KileDialog::IncludeGraphics(this, fi.dirPath(), this);

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
	if (!m_pFullScreen->isChecked())
		showNormal();
	else
		showFullScreen();
}

/////////////// QuickPreview (dani) ////////////////

// all calls of QuickPreview will get here, so we can decide what to do
// rewritten Sep 05 2006 to work together with preview in the bottom bar

void Kile::slotQuickPreview(int type)
{
	kdDebug() << "==Kile::slotQuickPreview()=========================="  << endl;

	Kate::View *view = viewManager()->currentTextView();
	if ( ! view) return;

	Kate::Document *doc = view->getDoc();
	if ( ! doc )
		return;
 
	switch ( type )
	{
		case KileTool::qpSelection:   m_quickPreview->previewSelection(doc);   break;
		case KileTool::qpEnvironment: m_quickPreview->previewEnvironment(doc); break;
		case KileTool::qpSubdocument: m_quickPreview->previewSubdocument(doc); break;
		case KileTool::qpMathgroup:   m_quickPreview->previewMathgroup(doc);   break;
	}
}	

#include "kile.moc"
