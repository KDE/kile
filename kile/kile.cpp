/***************************************************************************
                          kile.cpp  -  description
                             -------------------
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
#include "tabdialog.h"
#include "arraydialog.h"
#include "tabbingdialog.h"
#include "kilestructurewidget.h"
#include "convert.h"
#include "includegraphicsdialog.h"
#include "cleandialog.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "kileeventfilter.h"
#include "kileautosavejob.h"
#include "kileconfig.h"
#include "kileerrorhandler.h"
#include "configcheckerdlg.h"
#include "kilespell.h"
#include "kilespell2.h"
#include "kilesidebar.h"
#include "symbolview.h"

Kile::Kile( bool allowRestore, QWidget *parent, const char *name ) :
	DCOPObject( "Kile" ),
	KParts::MainWindow( parent, name),
	KileInfo(this),
	m_paPrint(0L),
	m_bShowUserMovedMessage(false)
{
	m_config = KGlobal::config();
	readUserSettings();
	readRecentFileSettings();

	m_AutosaveTimer = new QTimer();
	connect(m_AutosaveTimer,SIGNAL(timeout()),this,SLOT(autoSaveAll()));

	m_edit = new KileDocument::EditorExtension(this);
	m_help = new KileHelp::Help(m_edit);
	m_partManager = new KParts::PartManager( this );
	m_eventFilter = new KileEventFilter();
	m_errorHandler = new KileErrorHandler(this, this);
	m_spell = new KileSpell(this, this, "kilespell");

	connect( m_partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(activePartGUI ( KParts::Part * ) ) );
	connect(this,SIGNAL(configChanged()), m_eventFilter, SLOT(readConfig()));

	readGUISettings();

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

	setupBottomBar();

	// check requirements for IncludeGraphicsDialog (dani)
	KileConfig::setImagemagick(!(KStandardDirs::findExe("identify") == QString::null));

	//workaround for kdvi crash when started with Tooltips
	KileConfig::setRunOnStart(false);

	setupActions();
	setupTools();

	QValueList<int> sizes;
	sizes << m_verSplitTop << m_verSplitBottom;
	m_verticalSplitter->setSizes( sizes );
	sizes.clear();
	sizes << m_horSplitLeft << m_horSplitRight;
	m_horizontalSplitter->setSizes( sizes );

	m_topWidgetStack->addWidget(m_horizontalSplitter , 0);
	setCentralWidget(m_topWidgetStack);
	newCaption();

	m_partManager->setActivePart( 0L );

	m_lyxserver = new KileLyxServer(KileConfig::runLyxServer());
	connect(m_lyxserver, SIGNAL(insert(const KileAction::TagData &)), this, SLOT(insertTag(const KileAction::TagData &)));

	applyMainWindowSettings(m_config, "KileMainWindow" );

	m_manager  = new KileTool::Manager(this, m_config, m_logWidget, m_outputWidget, m_partManager, m_topWidgetStack, m_paStop, 10000); //FIXME make timeout configurable
	connect(m_manager, SIGNAL(requestGUIState(const QString &)), this, SLOT(prepareForPart(const QString &)));
	connect(m_manager, SIGNAL(requestSaveAll()), docManager(), SLOT(fileSaveAll()));
	connect(m_manager, SIGNAL(jumpToFirstError()), m_errorHandler, SLOT(jumpToFirstError()));
	connect(m_manager, SIGNAL(toolStarted()), m_errorHandler, SLOT(reset()));

	m_toolFactory = new KileTool::Factory(m_manager, m_config);
	m_manager->setFactory(m_toolFactory);
	m_help->setManager(m_manager);     // kile help (dani)

	connect(docManager(), SIGNAL(updateModeStatus()), this, SLOT(updateModeStatus()));
	connect(docManager(), SIGNAL(updateStructure(bool, KileDocument::Info*)), viewManager(), SLOT(updateStructure(bool, KileDocument::Info*)));
	connect(docManager(), SIGNAL(closingDocument(KileDocument::Info* )), m_kwStructure, SLOT(closeDocumentInfo(KileDocument::Info *)));
	connect(docManager(), SIGNAL(documentInfoCreated(KileDocument::Info* )), m_kwStructure, SLOT(addDocumentInfo(KileDocument::Info* )));

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

	m_singlemode = true;
	m_masterName = getName();
	restoreFilesAndProjects(allowRestore);
}

Kile::~Kile()
{
	kdDebug() << "cleaning up..." << endl;

	// CodeCompletion  and edvanced editor (dani)
	delete m_edit;
	delete m_AutosaveTimer;
}


void Kile::setupStatusBar()
{
	statusBar()->insertItem(i18n("Line: 1 Col: 1"), ID_LINE_COLUMN, 0, true);
	statusBar()->setItemAlignment( ID_LINE_COLUMN, AlignLeft|AlignVCenter );
	statusBar()->insertItem(i18n("Normal mode"), ID_HINTTEXT,10);
	statusBar()->setItemAlignment( ID_HINTTEXT, AlignLeft|AlignVCenter );
}

void Kile::setupSideBar()
{
	m_sideBar = new KileSideBar(m_horizontalSplitter);

	m_fileSelector= new KileFileSelect(m_sideBar,"File Selector");
	m_sideBar->addTab(m_fileSelector, SmallIcon("fileopen"), i18n("Open File"));
	connect(m_fileSelector,SIGNAL(fileSelected(const KFileItem*)), docManager(), SLOT(fileSelected(const KFileItem*)));
	connect(m_fileSelector->comboEncoding(), SIGNAL(activated(int)),this,SLOT(changeInputEncoding()));
	m_fileSelector->comboEncoding()->lineEdit()->setText(KileConfig::inputEncoding());
	m_fileSelector->readConfig();

	setupProjectView();
	setupStructureView();
	setupSymbolViews();

	m_sideBar->showTab(KileConfig::selectedLeftView());
	m_sideBar->setVisible(KileConfig::sideBar());
}

void Kile::setupProjectView()
{
	KileProjectView *projectview = new KileProjectView(m_sideBar, this);
	viewManager()->setProjectView(projectview);
	m_sideBar->addTab(projectview, SmallIcon("editcopy"), i18n("Files && Projects"));
	connect(projectview, SIGNAL(fileSelected(const KileProjectItem *)), docManager(), SLOT(fileSelected(const KileProjectItem *)));
	connect(projectview, SIGNAL(fileSelected(const KURL &)), docManager(), SLOT(fileSelected(const KURL &)));
	connect(projectview, SIGNAL(closeURL(const KURL&)), docManager(), SLOT(fileClose(const KURL&)));
	connect(projectview, SIGNAL(closeProject(const KURL&)), docManager(), SLOT(projectClose(const KURL&)));
	connect(projectview, SIGNAL(projectOptions(const KURL&)), docManager(), SLOT(projectOptions(const KURL&)));
	connect(projectview, SIGNAL(projectArchive(const KURL&)), docManager(), SLOT(projectArchive(const KURL&)));
	connect(projectview, SIGNAL(removeFromProject(const KileProjectItem *)), docManager(), SLOT(removeFromProject(const KileProjectItem *)));
	connect(projectview, SIGNAL(addFiles(const KURL &)), docManager(), SLOT(projectAddFiles(const KURL &)));
	connect(projectview, SIGNAL(toggleArchive(KileProjectItem *)), docManager(), SLOT(toggleArchive(KileProjectItem *)));
	connect(projectview, SIGNAL(addToProject(const KURL &)), docManager(), SLOT(addToProject(const KURL &)));
	connect(projectview, SIGNAL(saveURL(const KURL &)), docManager(), SLOT(saveURL(const KURL &)));
	connect(projectview, SIGNAL(buildProjectTree(const KURL &)), docManager(), SLOT(buildProjectTree(const KURL &)));
	connect(docManager(), SIGNAL(projectTreeChanged(const KileProject *)), projectview, SLOT(refreshProjectTree(const KileProject *)));
}

void Kile::setupStructureView()
{
	m_kwStructure = new KileWidget::Structure(this, m_sideBar);
	m_sideBar->addTab(m_kwStructure, SmallIcon("structure"), i18n("Structure"));
	m_kwStructure->setFocusPolicy(QWidget::ClickFocus);
	connect(m_kwStructure, SIGNAL(setCursor(const KURL &,int,int)), this, SLOT(setCursor(const KURL &,int,int)));
	connect(m_kwStructure, SIGNAL(fileOpen(const KURL&, const QString & )), docManager(), SLOT(fileOpen(const KURL&, const QString& )));
	connect(m_kwStructure, SIGNAL(fileNew(const KURL&)), docManager(), SLOT(fileNew(const KURL&)));

	QToolTip::add(m_kwStructure, i18n("Click to jump to the line"));
}

void Kile::setupSymbolViews()
{
	m_sideBar->addSymbolTab(SymbolView::Relation, SmallIcon("math1"), i18n("Relation Symbols"));
	m_sideBar->addSymbolTab(SymbolView::Arrow, SmallIcon("math2"), i18n("Arrow Symbols"));
	m_sideBar->addSymbolTab(SymbolView::Misc, SmallIcon("math3"), i18n("Miscellaneous Symbols"));
	m_sideBar->addSymbolTab(SymbolView::Delimiters, SmallIcon("math4"), i18n("Delimiters"));
	m_sideBar->addSymbolTab(SymbolView::Greek, SmallIcon("math5"), i18n("Greek Letters"));
	m_sideBar->addSymbolTab(SymbolView::Special, SmallIcon("math6"), i18n("Special Characters"));
	connect(m_sideBar->symbolView(), SIGNAL(executed(QIconViewItem*)), this, SLOT(insertSymbol(QIconViewItem*)));

	m_mpview = new metapostview( m_sideBar );
	m_sideBar->addTab(m_mpview, SmallIcon("metapost"), i18n("MetaPost Commands"));
	connect(m_mpview, SIGNAL(clicked(QListBoxItem *)), SLOT(insertMetaPost(QListBoxItem *)));
}

void Kile::setupBottomBar()
{
	m_bottomBar = new KileBottomBar(m_verticalSplitter);
	m_bottomBar->setFocusPolicy(QWidget::ClickFocus);

	m_logWidget = new KileWidget::LogMsg( this, m_bottomBar );
	connect(m_logWidget, SIGNAL(fileOpen(const KURL&, const QString & )), docManager(), SLOT(fileOpen(const KURL&, const QString& )));
	connect(m_logWidget, SIGNAL(setLine(const QString& )), this, SLOT(setLine(const QString& )));

	m_logWidget->setFocusPolicy(QWidget::ClickFocus);
	m_logWidget->setMinimumHeight(40);
	m_logWidget->setReadOnly(true);
	m_bottomBar->addTab(m_logWidget, SmallIcon("viewlog"), i18n(" Log && Messages "));

	m_outputWidget = new KileWidget::Output(m_bottomBar);
	m_outputWidget->setFocusPolicy(QWidget::ClickFocus);
	m_outputWidget->setMinimumHeight(40);
	m_outputWidget->setReadOnly(true);
	m_bottomBar->addTab(m_outputWidget, SmallIcon("output_win"), i18n(" Output "));

	m_outputInfo=new LatexOutputInfoArray();
	m_outputFilter=new LatexOutputFilter(m_outputInfo);
	connect(m_outputFilter, SIGNAL(problem(int, const QString& )), m_logWidget, SLOT(printProblem(int, const QString& )));

	m_texKonsole=new KileWidget::Konsole(this, m_bottomBar,"konsole");
	m_bottomBar->addTab(m_texKonsole, SmallIcon("konsole"),i18n(" Konsole "));
	connect(viewManager()->tabs(), SIGNAL( currentChanged( QWidget * ) ), m_texKonsole, SLOT(sync()));

	m_bottomBar->showPage(m_logWidget);
	m_bottomBar->setVisible(KileConfig::bottomBar());
}

void Kile::setupActions()
{
	m_paPrint = KStdAction::print(0,0, actionCollection(), "print");
	(void) KStdAction::openNew(docManager(), SLOT(fileNew()), actionCollection(), "file_new" );
	(void) KStdAction::open(docManager(), SLOT(fileOpen()), actionCollection(),"file_open" );
	m_actRecentFiles = KStdAction::openRecent(docManager(), SLOT(fileOpen(const KURL&)), actionCollection(), "file_open_recent");
	connect(docManager(), SIGNAL(addToRecentFiles(const KURL& )), m_actRecentFiles, SLOT(addURL(const KURL& )));
	m_actRecentFiles->loadEntries(m_config, "Recent Files");

	(void) new KAction(i18n("Save All"),"save_all", 0, docManager(), SLOT(fileSaveAll()), actionCollection(),"file_save_all" );
	(void) new KAction(i18n("Create Template From Document..."), 0, docManager(), SLOT(createTemplate()), actionCollection(),"CreateTemplate");
	(void) KStdAction::close(docManager(), SLOT(fileClose()), actionCollection(),"file_close" );
	(void) new KAction(i18n("Close All"), 0, docManager(), SLOT(fileCloseAll()), actionCollection(),"file_close_all" );
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

	(void) new KAction(i18n("Find &in Files..."), ALT+SHIFT+Key_F, this, SLOT(findInFiles()), actionCollection(),"FindInFiles" );

	kdDebug() << "CONNECTING SPELLCHECKER" << endl;
	connect ( viewManager(), SIGNAL(startSpellCheck()), m_spell, SLOT(spellcheck()) );

	(void) new KAction(i18n("Refresh Structure"), "structure", 0, this, SLOT(refreshStructure()), actionCollection(),"RefreshStructure" );

	//project actions
	(void) new KAction(i18n("&New Project..."), "filenew", 0, docManager(), SLOT(projectNew()), actionCollection(), "project_new");
	(void) new KAction(i18n("&Open Project..."), "fileopen", 0, docManager(), SLOT(projectOpen()), actionCollection(), "project_open");
	m_actRecentProjects =  new KRecentFilesAction(i18n("Open &Recent Project"),  0, docManager(), SLOT(projectOpen(const KURL &)), actionCollection(), "project_openrecent");
	connect(docManager(), SIGNAL(removeFromRecentProjects(const KURL& )), m_actRecentProjects, SLOT(removeURL(const KURL& )));
	connect(docManager(), SIGNAL(addToRecentProjects(const KURL& )), m_actRecentProjects, SLOT(addURL(const KURL& )));
	m_actRecentProjects->loadEntries(m_config, "Projects");

	(void) new KAction(i18n("A&dd Files to Project..."), 0, docManager(), SLOT(projectAddFiles()), actionCollection(), "project_add");
	(void) new KAction(i18n("Refresh Project &Tree"), "relation", 0, docManager(), SLOT(buildProjectTree()), actionCollection(), "project_buildtree");
	(void) new KAction(i18n("&Archive"), "package", 0, docManager(), SLOT(projectArchive()), actionCollection(), "project_archive");
	(void) new KAction(i18n("Project &Options"), "configure", 0, docManager(), SLOT(projectOptions()), actionCollection(), "project_options");
	(void) new KAction(i18n("&Close Project"), "fileclose", 0, docManager(), SLOT(projectClose()), actionCollection(), "project_close");

	//build actions
	(void) new KAction(i18n("Clean"),0 , this, SLOT(cleanAll()), actionCollection(),"CleanAll" );
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
	(void) new KAction(i18n("La(TeX) Command"),"complete1",CTRL+Key_Space, m_edit, SLOT(completeWord()), actionCollection(), "edit_complete_word");
	(void) new KAction(i18n("Environment"),"complete2",ALT+Key_Space, m_edit, SLOT(completeEnvironment()), actionCollection(), "edit_complete_env");
	(void) new KAction(i18n("Abbreviation"),"complete3",CTRL+ALT+Key_Space, m_edit, SLOT(completeAbbreviation()), actionCollection(), "edit_complete_abbrev");
	(void) new KAction(i18n("Next Bullet"),"nextbullet",CTRL+ALT+Key_Right, m_edit, SLOT(nextBullet()), actionCollection(), "edit_next_bullet");
	(void) new KAction(i18n("Prev Bullet"),"prevbullet",CTRL+ALT+Key_Left, m_edit, SLOT(prevBullet()), actionCollection(), "edit_prev_bullet");

 // advanced editor (dani)
	(void) new KAction(i18n("Environment (inside)"),KShortcut("CTRL+Alt+S,E"), m_edit, SLOT(selectEnvInside()), actionCollection(), "edit_select_inside_env");
	(void) new KAction(i18n("Environment (outside)"),KShortcut("CTRL+Alt+S,F"), m_edit, SLOT(selectEnvOutside()), actionCollection(), "edit_select_outside_env");
	(void) new KAction(i18n("TeX Group (inside)"),"selgroup_i",KShortcut("CTRL+Alt+S,T"), m_edit, SLOT(selectTexgroupInside()), actionCollection(), "edit_select_inside_group");
	(void) new KAction(i18n("TeX Group (outside)"),"selgroup_o",KShortcut("CTRL+Alt+S,U"),m_edit, SLOT(selectTexgroupOutside()), actionCollection(), "edit_select_outside_group");
	(void) new KAction(i18n("Paragraph"),KShortcut("CTRL+Alt+S,P"),m_edit, SLOT(selectParagraph()), actionCollection(), "edit_select_paragraph");
	(void) new KAction(i18n("Line"),KShortcut("CTRL+Alt+S,L"),m_edit, SLOT(selectLine()), actionCollection(), "edit_select_line");
	(void) new KAction(i18n("TeX Word"),KShortcut("CTRL+Alt+S,W"),m_edit, SLOT(selectWord()), actionCollection(), "edit_select_word");

	(void) new KAction(i18n("Environment (inside)"),KShortcut("CTRL+Alt+D,E"), m_edit, SLOT(deleteEnvInside()), actionCollection(), "edit_delete_inside_env");
	(void) new KAction(i18n("Environment (outside)"),KShortcut("CTRL+Alt+D,F"),m_edit, SLOT(deleteEnvOutside()), actionCollection(), "edit_delete_outside_env");
	(void) new KAction(i18n("TeX Group (inside)"),KShortcut("CTRL+Alt+D,T"), m_edit, SLOT(deleteTexgroupInside()), actionCollection(), "edit_delete_inside_group");
	(void) new KAction(i18n("TeX Group (outside)"),KShortcut("CTRL+Alt+D,U"),m_edit, SLOT(deleteTexgroupInside()), actionCollection(), "edit_delete_outside_group");
	(void) new KAction(i18n("Paragraph"),KShortcut("CTRL+Alt+D,P"),m_edit, SLOT(deleteParagraph()), actionCollection(), "edit_delete_paragraph");
	(void) new KAction(i18n("TeX Word"),KShortcut("CTRL+Alt+D,W"),m_edit, SLOT(deleteWord()), actionCollection(), "edit_delete_word");

	(void) new KAction(i18n("Goto Begin"),KShortcut("CTRL+Alt+E,B"), m_edit, SLOT(gotoBeginEnv()), actionCollection(), "edit_begin_env");
	(void) new KAction(i18n("Goto End"),KShortcut("CTRL+Alt+E,E"), m_edit, SLOT(gotoEndEnv()), actionCollection(), "edit_end_env");
	(void) new KAction(i18n("Match"),"matchenv",KShortcut("CTRL+Alt+E,M"), m_edit, SLOT(matchEnv()), actionCollection(), "edit_match_env");
	(void) new KAction(i18n("Close"),"closeenv",KShortcut("CTRL+Alt+E,C"), m_edit, SLOT(closeEnv()), actionCollection(), "edit_close_env");

	(void) new KAction(i18n("Goto Begin"),KShortcut("CTRL+Alt+G,B"), m_edit, SLOT(gotoBeginTexgroup()), actionCollection(), "edit_begin_group");
	(void) new KAction(i18n("Goto End"),KShortcut("CTRL+Alt+G,E"), m_edit, SLOT(gotoEndTexgroup()), actionCollection(), "edit_end_group");
	(void) new KAction(i18n("Match"),"matchgroup",KShortcut("CTRL+Alt+G,M"), m_edit, SLOT(matchTexgroup()), actionCollection(), "edit_match_group");
	(void) new KAction(i18n("Close"),"closegroup",KShortcut("CTRL+Alt+G,C"), m_edit, SLOT(closeTexgroup()), actionCollection(), "edit_close_group");

	(void) new KAction(i18n("teTeX Guide"),KShortcut("CTRL+Alt+H,T"), m_help, SLOT(helpTetexGuide()), actionCollection(), "help_tetex_guide");
	(void) new KAction(i18n("teTeX Doc"),KShortcut("CTRL+Alt+H,T"), m_help, SLOT(helpTetexDoc()), actionCollection(), "help_tetex_doc");
	(void) new KAction(i18n("LaTeX"),KShortcut("CTRL+Alt+H,L"), m_help, SLOT(helpLatexIndex()), actionCollection(), "help_latex_index");
	(void) new KAction(i18n("LaTeX Command"),KShortcut("CTRL+Alt+H,C"), m_help, SLOT(helpLatexCommand()), actionCollection(), "help_latex_command");
	(void) new KAction(i18n("LaTeX Subject"),KShortcut("CTRL+Alt+H,S"), m_help, SLOT(helpLatexSubject()), actionCollection(), "help_latex_subject");
	(void) new KAction(i18n("LaTeX Env"),KShortcut("CTRL+Alt+H,E"), m_help, SLOT(helpLatexEnvironment()), actionCollection(), "help_latex_env");
	(void) new KAction(i18n("Context Help"),KShortcut("CTRL+Alt+H,K"), m_help, SLOT(helpKeyword()), actionCollection(), "help_context");

	KileStdActions::setupStdTags(this,this);
	KileStdActions::setupMathTags(this);
	KileStdActions::setupBibTags(this);

	(void) new KAction(i18n("Quick Start"),"wizard",0 , this, SLOT(quickDocument()), actionCollection(),"127" );
	connect(docManager(), SIGNAL(startWizard()), this, SLOT(quickDocument()));
	(void) new KAction(i18n("Tabular"),"wizard",0 , this, SLOT(quickTabular()), actionCollection(),"129" );
	(void) new KAction(i18n("Tabbing"),"wizard",0 , this, SLOT(quickTabbing()), actionCollection(),"149" );
	(void) new KAction(i18n("Array"),"wizard",0 , this, SLOT(quickArray()), actionCollection(),"130" );

	(void) new KAction(i18n("Clean"),0 , this, SLOT(cleanBib()), actionCollection(),"CleanBib" );

	ModeAction=new KToggleAction(i18n("Define Current Document as '&Master Document'"),"master",0 , this, SLOT(toggleMode()), actionCollection(),"Mode" );

	KToggleAction *tact = new KToggleAction(i18n("Show S&ide Bar"), 0, 0, 0, actionCollection(),"StructureView" );
	tact->setChecked(KileConfig::sideBar());
	connect(tact, SIGNAL(toggled(bool)), m_sideBar, SLOT(setVisible(bool)));
	connect(m_sideBar, SIGNAL(visibilityChanged(bool )), tact, SLOT(setChecked(bool)));

	tact = new KToggleAction(i18n("Show Mess&ages Bar"), 0, 0, 0, actionCollection(),"MessageView" );
	tact->setChecked(KileConfig::bottomBar());
	connect(tact, SIGNAL(toggled(bool)), m_bottomBar, SLOT(setVisible(bool)));
	connect(m_bottomBar, SIGNAL(visibilityChanged(bool )), tact, SLOT(setChecked(bool)));

	//FIXME: obsolete for KDE 4
	m_paShowMainTB = new KToggleToolBarAction("mainToolBar", i18n("Main"), actionCollection(), "ShowMainToolbar");
	m_paShowToolsTB = new KToggleToolBarAction("toolsToolBar", i18n("Tools"), actionCollection(), "ShowToolsToolbar");
	m_paShowBuildTB = new KToggleToolBarAction("buildToolBar", i18n("Build"), actionCollection(), "ShowQuickToolbar");
	m_paShowErrorTB = new KToggleToolBarAction("errorToolBar", i18n("Error"), actionCollection(), "ShowErrorToolbar");
	m_paShowEditTB = new KToggleToolBarAction("editToolBar", i18n("Edit"), actionCollection(), "ShowEditToolbar");
	m_paShowMathTB = new KToggleToolBarAction("mathToolBar", i18n("Math"), actionCollection(), "ShowMathToolbar");

	//Save the toolbar settings, we need to know if the toolbars should be visible or not. We can't use KToggleToolBarAction->isChecked()
	//since this will return false if we hide the toolbar when switching to Viewer mode for example.
	m_bShowMainTB = m_paShowMainTB->isChecked();
	m_bShowToolsTB = m_paShowToolsTB->isChecked();
	m_bShowErrorTB = m_paShowErrorTB->isChecked();
	m_bShowBuildTB = m_paShowBuildTB->isChecked();
	m_bShowEditTB = m_paShowEditTB->isChecked();
	m_bShowMathTB = m_paShowMathTB->isChecked();

	if (m_singlemode) {ModeAction->setChecked(false);}
	else {ModeAction->setChecked(true);}

	(void) new KAction(i18n("&Remove Template..."),0, docManager(), SLOT(removeTemplate()), actionCollection(), "removetemplates");

	WatchFileAction=new KToggleAction(i18n("Watch File Mode"),"watchfile",0 , this, SLOT(toggleWatchFile()), actionCollection(), "WatchFile");
	if (m_bWatchFile) {WatchFileAction->setChecked(true);}
	else {WatchFileAction->setChecked(false);}

	setHelpMenuEnabled(false);
	const KAboutData *aboutData = KGlobal::instance()->aboutData();
	KHelpMenu *help_menu = new KHelpMenu( this, aboutData);
	(void) new KAction(i18n("LaTeX Reference"),"help",0 , this, SLOT(helpLaTex()), actionCollection(),"help_latex_reference" );
	(void) KStdAction::helpContents(help_menu, SLOT(appHelpActivated()), actionCollection(), "help_handbook");
	(void) KStdAction::reportBug (help_menu, SLOT(reportBug()), actionCollection(), "report_bug");
	(void) KStdAction::aboutApp(help_menu, SLOT(aboutApplication()), actionCollection(),"help_aboutKile" );
	(void) KStdAction::aboutKDE(help_menu, SLOT(aboutKDE()), actionCollection(),"help_aboutKDE" );
	(void) KStdAction::preferences(this, SLOT(generalOptions()), actionCollection(),"settings_configure" );
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

	for ( uint i = 0; i < tools.count(); i++)
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

	m_actionEditTag = new KAction(i18n("Edit User Tags"),0 , this, SLOT(editUserMenu()), m_menuUserTags,"EditUserMenu" );
	m_menuUserTags->insert(m_actionEditTag);
	for (uint i=0; i<m_listUserTags.size(); i++)
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
	for (uint i=0; i < m_listProjectsOpenOnStart.count(); i++)
	{
		fi.setFile(m_listProjectsOpenOnStart[i]);
		if (fi.isReadable()) docManager()->projectOpen(KURL::fromPathOrURL(m_listProjectsOpenOnStart[i]), i, m_listProjectsOpenOnStart.count());
	}

	for (uint i=0; i < m_listDocsOpenOnStart.count(); i++)
	{
		fi.setFile(m_listDocsOpenOnStart[i]);
		if (fi.isReadable())
			docManager()->fileOpen(KURL::fromPathOrURL(m_listDocsOpenOnStart[i]));
	}

	m_masterName = KileConfig::master();
	m_singlemode = (m_masterName == "");
	if (ModeAction) ModeAction->setChecked(!m_singlemode);
	updateModeStatus();

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();

	Kate::Document *doc = docManager()->docFor(KURL::fromPathOrURL(KileConfig::lastDocument()));
	if (doc) activateView(doc->views().first());
}

void Kile::setActive()
{
	kdDebug() << "ACTIVATING" << endl;
	kapp->mainWidget()->raise();
	kapp->mainWidget()->setActiveWindow();
}

////////////////////////////// FILE /////////////////////////////

void Kile::setLine( const QString &line )
{
	bool ok;
	uint l=line.toUInt(&ok,10);
	Kate::View *view = viewManager()->currentView();
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

void Kile::load(const QString &path)
{
	docManager()->load(KURL::fromPathOrURL(path));
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

	Kate::View* view = (Kate::View*)w;

	for (uint i=0; i< viewManager()->views().count(); i++)
	{
		guiFactory()->removeClient(viewManager()->view(i));
		viewManager()->view(i)->setActive(false);
	}

	toolBar ()->setUpdatesEnabled (false);

	guiFactory()->addClient(view);
	view->setActive( true );

	if (updateStruct) viewManager()->updateStructure();

	toolBar ()->setUpdatesEnabled (true);
}

void Kile::updateModeStatus()
{
	KileProject *project = docManager()->activeProject();

	if (project)
	{
		statusBar()->changeItem(i18n("Project: %1").arg(project->name()), ID_HINTTEXT);
	}
	else
	{
		if (m_singlemode)
		{
			statusBar()->changeItem(i18n("Normal mode"), ID_HINTTEXT);
		}
		else
		{
			QString shortName = m_masterName;
			int pos = shortName.findRev('/');
			shortName.remove(0,pos+1);
			statusBar()->changeItem(i18n("Master document: %1").arg(shortName), ID_HINTTEXT);
		}
	}
}

void Kile::openDocument(const QString & url)
{
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
		if ( interval < 1 ) interval = 10;
		m_AutosaveTimer->start(interval * 60000);
	}
	else m_AutosaveTimer->stop();
}

void Kile::openProject(const QString& proj)
{
	docManager()->projectOpen(KURL::fromPathOrURL(proj));
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
	Kate::View *view = viewManager()->currentView();
	if (view) view->setFocus();
}

bool Kile::queryExit()
{
	saveSettings();
	return true;
}

bool Kile::queryClose()
{
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
	for (uint i=0; i < docManager()->projects()->count(); i++)
	{
		m_listProjectsOpenOnStart.append(docManager()->projects()->at(i)->url().path());
	}

	bool stage1 = docManager()->projectCloseAll();
	bool stage2 = true;

	if (stage1)
	{
		for (uint i=0; i < viewManager()->views().count(); i++)
		{
			m_listDocsOpenOnStart.append(viewManager()->view(i)->getDoc()->url().path());
		}
		stage2 =docManager()->fileCloseAll();
	}

	return stage1 && stage2;
}

void Kile::showDocInfo(Kate::Document *doc)
{
	if (doc == 0)
	{
		Kate::View *view = viewManager()->currentView();

		if (view) doc = view->getDoc();
		else return;
	}

	KileDocument::Info *docinfo = docManager()->infoFor(doc);

	if (docinfo)
	{
		KileDocInfoDlg *dlg = new KileDocInfoDlg(docinfo, this, 0, i18n("Summary for Document: %1").arg(getShortName(doc)));
		dlg->exec();
	}
	else
		kdWarning() << "There is know KileDocument::Info object belonging to this document!" << endl;
}

void Kile::convertToASCII(Kate::Document *doc)
{
	if (doc == 0)
	{
		Kate::View *view = viewManager()->currentView();

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
		Kate::View *view = viewManager()->currentView();

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
	Kate::View *view = viewManager()->currentView();

	int para = 0;

	if (view)
	{
		para = view->cursorLine();
	}

	return para;
}

void Kile::newCaption()
{
	Kate::View *view = viewManager()->currentView();
	if (view)
	{
		setCaption(i18n("Document: %1").arg(getName(view->getDoc())));
		if (m_bottomBar->currentPage()->inherits("KileWidget::Konsole")) m_texKonsole->sync();
	}
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
	static KileGrepDialog *dlg = 0;

	if (dlg != 0) {
		if (!dlg->isVisible())
			dlg->setDirName((docManager()->activeProject() != 0)
				? docManager()->activeProject()->baseURL().path()
				: QDir::home().absPath() + "/");

		dlg->show();
		return;
	}

	dlg = new KileGrepDialog
		((docManager()->activeProject() != 0)
		? docManager()->activeProject()->baseURL().path()
		: QDir::home().absPath() + "/");

	QString filter(SOURCE_EXTENSIONS);
	filter.append(" ");
	filter.append(PACKAGE_EXTENSIONS);
	filter.replace(".", "*.");
	filter.replace(" ", ",");
	filter.append("|");
	filter.append(i18n("TeX Files"));
	filter.append("\n*|");
	filter.append(i18n("All Files"));
	dlg->setFilter(filter);

	dlg->show();

	connect(dlg, SIGNAL(itemSelected(const QString &, int)),
		this, SLOT(grepItemSelected(const QString &, int)));
}

/////////////////// PART & EDITOR WIDGET //////////
void Kile::showEditorWidget()
{
	resetPart();
	setCentralWidget(m_topWidgetStack);
	m_topWidgetStack->show();
	m_horizontalSplitter->show();
	m_verticalSplitter->show();

	Kate::View *view = viewManager()->currentView();
	if (view) view->setFocus();

	newStatus();
	newCaption();
}


void Kile::resetPart()
{
	kdDebug() << "==Kile::resetPart()=============================" << endl;
	kdDebug() << "\tcurrent state " << m_currentState << endl;
	kdDebug() << "\twant state " << m_wantState << endl;

	KParts::ReadOnlyPart *part = (KParts::ReadOnlyPart*)m_partManager->activePart();

	if (part && m_currentState != "Editor")
	{
		kdDebug() << "\tclosing current part" << endl;
		part->closeURL();
		m_partManager->removePart(part) ;
		m_topWidgetStack->removeWidget(part->widget());
		delete part;
	}

	m_currentState = "Editor";
	m_wantState = "Editor";
	m_partManager->setActivePart( 0L);
}

void Kile::activePartGUI(KParts::Part * part)
{
	kdDebug() << "==Kile::activePartGUI()=============================" << endl;
	kdDebug() << "\tcurrent state " << m_currentState << endl;
	kdDebug() << "\twant state " << m_wantState << endl;

	//save the toolbar state
	if ( m_wantState == "HTMLpreview" || m_wantState == "Viewer" )
	{
		kdDebug() << "\tsaving toolbar status" << endl;
		m_bShowMainTB = m_paShowMainTB->isChecked();
		m_bShowToolsTB = m_paShowToolsTB->isChecked();
		m_bShowBuildTB = m_paShowBuildTB->isChecked();
		m_bShowErrorTB = m_paShowErrorTB->isChecked();
		m_bShowEditTB = m_paShowEditTB->isChecked();
		m_bShowMathTB = m_paShowMathTB->isChecked();
	}

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
	if ( wantState == "HTMLpreview" )
	{
		stateChanged( "HTMLpreview");
		toolBar("mainToolBar")->hide();
		toolBar("toolsToolBar")->hide();
		toolBar("buildToolBar")->hide();
		toolBar("errorToolBar")->hide();
		toolBar("editToolBar")->hide();
		toolBar("mathToolBar")->hide();
		toolBar("extraToolBar")->show();
		enableKileGUI(false);
	}
	else if ( wantState == "Viewer" )
	{
		stateChanged( "Viewer" );
		toolBar("mainToolBar")->show();
		toolBar("toolsToolBar")->hide();
		toolBar("buildToolBar")->hide();
		toolBar("errorToolBar")->hide();
		toolBar("mathToolBar")->hide();
		toolBar("extraToolBar")->show();
		toolBar("editToolBar")->hide();
		enableKileGUI(false);
	}
	else
	{
		stateChanged( "Editor" );
		m_wantState="Editor";
		m_topWidgetStack->raiseWidget(0);
		if (m_bShowMainTB) toolBar("mainToolBar")->show();
		if (m_bShowEditTB) toolBar("editToolBar")->show();
		if (m_bShowToolsTB) toolBar("toolsToolBar")->show();
		if (m_bShowBuildTB) toolBar("buildToolBar")->show();
		if (m_bShowErrorTB) toolBar("errorToolBar")->show();
		if (m_bShowMathTB) toolBar("mathToolBar")->show();
		toolBar("extraToolBar")->hide();
		enableKileGUI(true);
	}
}

void Kile::enableKileGUI(bool enable)
{
	int id;
	QString text;
	for (uint i=0; i < menuBar()->count(); i++)
	{
		id = menuBar()->idAt(i);
		text = menuBar()->text(id);
		if (
			text == i18n("&Build") ||
			text == i18n("&Project") ||
			text == i18n("&LaTeX") ||
			text == i18n("&Wizard") ||
			text == i18n("&User") ||
			text == i18n("&Graph") ||
			text == i18n("&Tools")
		)
			menuBar()->setItemEnabled(id, enable);
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
	for (uint i=0; i<viewManager()->views().count(); i++)
	{
		guiFactory()->removeClient(viewManager()->view(i));
		viewManager()->view(i)->setActive(false);
	}
}

void Kile::runTool()
{
	kdDebug() << "==void Kile::runTool()===" << endl;
	QString name = sender()->name();
	kdDebug() << "\tname: " << name << endl;
	name.replace(QRegExp("^.*tool_"), "");
	kdDebug() << "\ttool: " << name << endl;
	m_manager->run(name);
}

void Kile::cleanAll(KileDocument::Info *docinfo, bool silent)
{
	static QString noactivedoc = i18n("There is no active document or it is not saved.");
	if (docinfo == 0)
	{
		Kate::Document *doc = activeDocument();
		if (doc)
			docinfo = docManager()->infoFor(doc);
		else
		{
			m_logWidget->printMsg(KileTool::Error, noactivedoc, i18n("Clean"));
			return;
		}
	}

	if (docinfo)
	{
		QStringList extlist;
		QStringList templist = QStringList::split(" ", KileConfig::cleanUpFileExtensions());
		QString str;
		QFileInfo file(docinfo->url().path()), fi;
		for (uint i=0; i <  templist.count(); i++)
		{
			str = file.dirPath(true) + "/" + file.baseName(true) + templist[i];
			fi.setFile(str);
			if ( fi.exists() )
				extlist.append(templist[i]);
		}

		str = file.fileName();
		if (!silent &&  (str==i18n("Untitled") || str == "" ) )
		{
			m_logWidget->printMsg(KileTool::Error, noactivedoc, i18n("Clean"));
			return;
		}

		if (!silent && extlist.count() > 0 )
		{
			kdDebug() << "\tnot silent" << endl;
			KileDialog::Clean *dialog = new KileDialog::Clean(this, str, extlist);
			if ( dialog->exec() )
				extlist = dialog->getCleanlist();
			else
			{
				delete dialog;
				return;
			}

			delete dialog;
		}

		if ( extlist.count() == 0 )
		{
			m_logWidget->printMsg(KileTool::Warning, i18n("Nothing to clean for %1").arg(str), i18n("Clean"));
			return;
		}

		m_logWidget->printMsg(KileTool::Info, i18n("cleaning %1 : %2").arg(str).arg(extlist.join(" ")), i18n("Clean"));

		docinfo->cleanTempFiles(extlist);
	}
}

void Kile::refreshStructure()
{
	viewManager()->updateStructure(true);
}


/////////////////////// LATEX TAGS ///////////////////
void Kile::insertTag(const KileAction::TagData& data)
{
	logWidget()->clear();
	outputView()->showPage(logWidget());
	setLogPresent(false);

	logWidget()->append(data.description);

	Kate::View *view = viewManager()->currentView();

	if ( !view ) return;

	view->setFocus();

	editorExtension()->insertTag(data, view);
}

void Kile::insertTag(const QString& tagB, const QString& tagE, int dx, int dy)
{
	insertTag(KileAction::TagData(QString::null,tagB,tagE,dx,dy));
}

void Kile::quickDocument()
{
	KileDialog::QuickDocument *dlg = new KileDialog::QuickDocument(m_config, this,"Quick Start",i18n("Quick Start"));

	if ( dlg->exec() )
	{
		if ( !viewManager()->currentView() && ( docManager()->createDocumentWithText(QString::null) == 0L ) )
			return;

		insertTag( dlg->tagData() );
	}
	delete dlg;
}

void Kile::quickTabular()
{
	if ( !viewManager()->currentView() ) return;
	KileDialog::QuickTabular *dlg = new KileDialog::QuickTabular(m_config, this,"Tabular", i18n("Tabular"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickTabbing()
{
	if ( !viewManager()->currentView() ) return;
	KileDialog::QuickTabbing *dlg = new KileDialog::QuickTabbing(m_config, this,"Tabbing", i18n("Tabbing"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::quickArray()
{
	if ( !viewManager()->currentView() ) return;
	KileDialog::QuickArray *dlg = new KileDialog::QuickArray(m_config, this,"Array", i18n("Array"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

//////////////////////////// MATHS TAGS/////////////////////////////////////
void Kile::insertSymbol(QIconViewItem *item)
{
	QString code_symbol = item->key();
	insertTag(code_symbol,QString::null,code_symbol.length(),0);
}

void Kile::insertMetaPost(QListBoxItem *)
{
	QString mpcode = m_mpview->currentText();
	if (mpcode!="----------") insertTag(mpcode,QString::null,mpcode.length(),0);
}

//////////////// HELP /////////////////
void Kile::helpLaTex()
{
	QString loc = locate("html","en/kile/latexhelp.html");
	KileTool::ViewHTML *tool = dynamic_cast<KileTool::ViewHTML*>(m_toolFactory->create("ViewHTML"));
	tool->setSource(loc);
	tool->setRelativeBaseDir("");
	tool->setTarget("latexhelp.html");
	m_manager->run(tool);
}

///////////////////// USER ///////////////
void Kile::editUserMenu()
{
	KileDialog::UserTags *dlg = new KileDialog::UserTags(m_listUserTags, this, "Edit User Tags", i18n("Edit User Tags"));

	if ( dlg->exec() )
	{
		//remove all actions
		uint len = m_listUserTagsActions.count();
		for (uint j=0; j< len; j++)
		{
			KAction *menuItem = m_listUserTagsActions.getLast();
			m_menuUserTags->remove(menuItem);
			m_listUserTagsActions.removeLast();
			delete menuItem;
		}
		m_menuUserTags->remove(m_actionEditTag);

		m_listUserTags = dlg->result();
		setupUserTagActions();
	}

	delete dlg;
}

/////////////// CONFIG ////////////////////
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
	for (int i = 0; i < len; i++)
	{
		m_listUserTags.append(KileDialog::UserTags::splitTag(m_config->readEntry("userTagName"+QString::number(i),i18n("no name")) , m_config->readEntry("userTag"+QString::number(i),"") ));
	}

	//convert user tools to new KileTool classes
	userItem tempItem;
	len= m_config->readNumEntry("nUserTools",0);
	for (int i=0; i< len; i++)
	{
		tempItem.name=m_config->readEntry("userToolName"+QString::number(i),i18n("no name"));
		tempItem.tag =m_config->readEntry("userTool"+QString::number(i),"");
		m_listUserTools.append(tempItem);
	}
	if ( len > 0 )
	{
 		//move the tools
		m_config->writeEntry("nUserTools", 0);
		for ( int i = 0; i < len; i++)
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
}

void Kile::readRecentFileSettings()
{
	m_config->setGroup("FilesOpenOnStart");
	int n = m_config->readNumEntry("NoDOOS", 0);
	for (int i=0; i < n; i++)
		m_listDocsOpenOnStart.append(m_config->readPathEntry("DocsOpenOnStart"+QString::number(i), ""));

	n = m_config->readNumEntry("NoPOOS", 0);
	for (int i=0; i < n; i++)
		m_listProjectsOpenOnStart.append(m_config->readPathEntry("ProjectsOpenOnStart"+QString::number(i), ""));
}

void Kile::readConfig()
{
	enableAutosave(KileConfig::autosave());
	m_edit->complete()->readConfig();
}

void Kile::saveSettings()
{
	showEditorWidget();

	m_fileSelector->writeConfig();

	Kate::View *view = viewManager()->currentView();
	if (view) KileConfig::setLastDocument(view->getDoc()->url().path());
	else KileConfig::setLastDocument("");

	KileConfig::setInputEncoding(m_fileSelector->comboEncoding()->lineEdit()->text());

	// Store recent files
	m_actRecentFiles->saveEntries(m_config, "Recent Files");
	m_actRecentProjects->saveEntries(m_config, "Projects");

	m_config->deleteGroup("FilesOpenOnStart");
	if (KileConfig::restore())
	{
		m_config->setGroup("FilesOpenOnStart");
		m_config->writeEntry("NoDOOS", m_listDocsOpenOnStart.count());
		for (uint i=0; i < m_listDocsOpenOnStart.count(); i++)
			m_config->writePathEntry("DocsOpenOnStart"+QString::number(i), m_listDocsOpenOnStart[i]);

		m_config->writeEntry("NoPOOS", m_listProjectsOpenOnStart.count());
		for (uint i=0; i < m_listProjectsOpenOnStart.count(); i++)
			m_config->writePathEntry("ProjectsOpenOnStart"+QString::number(i), m_listProjectsOpenOnStart[i]);

		if (!m_singlemode)
			KileConfig::setMaster(m_masterName);
		else
			KileConfig::setMaster("");
	}

	m_config->setGroup( "User" );

	m_config->writeEntry("nUserTags",static_cast<int>(m_listUserTags.size()));
	for (uint i=0; i < m_listUserTags.size(); i++)
	{
		KileAction::TagData td( m_listUserTags[i]);
		m_config->writeEntry( "userTagName"+QString::number(i),  td.text );
		m_config->writeEntry( "userTag"+QString::number(i), KileDialog::UserTags::completeTag(td) );
	}

	actionCollection()->writeShortcutSettings();
	saveMainWindowSettings(m_config, "KileMainWindow" );

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

	KileConfig::setHorizontalSplitterLeft(m_horSplitLeft);
	KileConfig::setHorizontalSplitterRight(m_horSplitRight);
	KileConfig::setVerticalSplitterTop(m_verSplitTop);
	KileConfig::setVerticalSplitterBottom(m_verSplitBottom);

	KileConfig::setSideBar(m_sideBar->isVisible());
	KileConfig::setBottomBar(m_bottomBar->isVisible());

	KileConfig::setSelectedLeftView(m_sideBar->currentTab());

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
	else if (m_singlemode && viewManager()->currentView())
	{
		m_masterName=getName();
		if (m_masterName==i18n("Untitled") || m_masterName=="")
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

void Kile::generalOptions()
{
	KileDialog::Config *dlg = new KileDialog::Config(m_config, m_manager, this);

	if (dlg->exec())
	{
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

	showToolBars(m_currentState);
}

void Kile::changeInputEncoding()
{
	Kate::View *view = viewManager()->currentView();
	if (view)
	{
		bool modified = view->getDoc()->isModified();

		QString encoding=m_fileSelector->comboEncoding()->lineEdit()->text();
		QString text = view->getDoc()->text();

		view->getDoc()->setEncoding(encoding);
		//reload the document so that the new encoding takes effect
		view->getDoc()->openURL(view->getDoc()->url());

		docManager()->setHighlightMode(view->getDoc());
		view->getDoc()->setModified(modified);
	}
}

//////////////////// CLEAN BIB /////////////////////
void Kile::cleanBib()
{
	Kate::View *view = viewManager()->currentView();
	if ( ! view )
		return;

	uint i=0;
	QString s;

	while(i < view->getDoc()->numLines())
	{
		s = view->getDoc()->textLine(i).left(3);
		if (s == "OPT" || s == "ALT")
		{
			view->getDoc()->removeLine(i);
			view->getDoc()->setModified(true);
		}
		else
			i++;
	}
}

void Kile::includeGraphics()
{
	Kate::View *view = viewManager()->currentView();
	if ( !view ) return;

	QFileInfo fi( view->getDoc()->url().path() );
	KileDialog::IncludeGraphics *dialog = new KileDialog::IncludeGraphics(this, fi.dirPath(), false);

	if ( dialog->exec() == QDialog::Accepted )
		insertTag(dialog->getTemplate(), "%C", 0,0);

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

#include "kile.moc"
