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

#include <ktexteditor/editorchooser.h>
#include <ktexteditor/encodinginterface.h>

#include <kdebug.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kileapplication.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kspell.h>
#include <ksconfig.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <krun.h>
#include <khtmlview.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <kprinter.h>
#include <kwin.h>
#include <kparts/browserextension.h>
#include <kaccel.h>
#include <knuminput.h>
#include <klistview.h>

#include <qfileinfo.h>
#include <qregexp.h>
#include <qiconset.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qheader.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qmetaobject.h>
#include <qvaluelist.h>
#include <qtextstream.h>
#include <qsignalmapper.h>

#include "templates.h"
#include "newfilewizard.h"
#include "managetemplatesdialog.h"
#include "kilestdactions.h"
#include "usermenudialog.h"
#include "kileconfigdialog.h"
#include "kileproject.h"
#include "kileprojectview.h"
#include "kileprojectdlgs.h"
#include "kilelistselector.h"
#include "kilelyxserver.h"

Kile::Kile( QWidget *, const char *name ) :
	DCOPObject( "Kile" ),
	KParts::MainWindow( name, WDestructiveClose),
	KileInfo(),
	m_bQuick(false),
	m_activeView(0)
{
	m_docList.setAutoDelete(false);
	m_infoList.setAutoDelete(false);

	partManager = new KParts::PartManager( this );
	connect( partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(ActivePartGUI ( KParts::Part * ) ) );

	m_AutosaveTimer= new QTimer();
	connect(m_AutosaveTimer,SIGNAL(timeout()),this,SLOT(autoSaveAll()));

	m_eventFilter = new KileEventFilter();
	connect(this,SIGNAL(configChanged()), m_eventFilter, SLOT(readConfig()));

	config = KGlobal::config();

	//workaround for kdvi crash when started with Tooltips
	config->setGroup("TipOfDay");
	config->writeEntry( "RunOnStart",false);
	setXMLFile( "kileui.rc" );

	htmlpresent=false;
	pspresent=false;
	dvipresent=false;
	watchfile=false;
	htmlpart=0L;
	pspart=0L;
	dvipart=0L;
	m_bNewErrorlist=true;
	m_bCheckForLaTeXErrors=false;
	m_bBlockWindowActivateEvents=false;
	kspell = 0;

	ReadSettings();
	setupActions();

	// Read Settings should be after setupActions() because fileOpenRecentAction needs to be
	// initialized before calling ReadSettnigs().
	ReadRecentFileSettings();

	statusBar()->insertItem( i18n("Line:000000 Col: 000"), ID_LINE_COLUMN,0,true );
	statusBar()->setItemAlignment( ID_LINE_COLUMN, AlignLeft|AlignVCenter );
	statusBar()->changeItem( i18n("Line: 1 Col: 1"), ID_LINE_COLUMN );
	statusBar()->insertItem(i18n("Normal mode"), ID_HINTTEXT,10);
	statusBar()->setItemAlignment( ID_HINTTEXT, AlignLeft|AlignVCenter );
	topWidgetStack = new QWidgetStack( this );
	topWidgetStack->setFocusPolicy(QWidget::NoFocus);
	splitter1=new QSplitter(QSplitter::Horizontal,topWidgetStack, "splitter1" );

	Structview_layout=0;
	Structview=new QFrame(splitter1);
	Structview->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	Structview->setLineWidth( 2 );
	Structview_layout=0;
	ButtonBar=new KMultiVertTabBar(Structview);

	ButtonBar->insertTab(SmallIcon("fileopen"),0,i18n("Open File"));
	connect(ButtonBar->getTab(0),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	KileFS= new KileFileSelect(Structview,"File Selector");
	connect(KileFS,SIGNAL(fileSelected(const KFileItem*)),this,SLOT(fileSelected(const KFileItem*)));
	connect(KileFS->comboEncoding, SIGNAL(activated(int)),this,SLOT(changeInputEncoding()));
	QString currentDir=QDir::currentDirPath();
	if (!lastDocument.isEmpty())
	{
		QFileInfo fi(lastDocument);
		if (fi.exists() && fi.isReadable()) currentDir=fi.dirPath();
	}
	KileFS->setDir(KURL(currentDir));
	KileFS->comboEncoding->lineEdit()->setText(input_encoding);

	m_projectview = new KileProjectView(Structview, this);
	ButtonBar->insertTab( SmallIcon("editcopy"),9,i18n("Files and Projects"));
	connect(ButtonBar->getTab(9),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	connect(m_projectview, SIGNAL(fileSelected(const KURL&)), this, SLOT(fileOpen(const KURL&)));
	connect(m_projectview, SIGNAL(closeURL(const KURL&)), this, SLOT(fileClose(const KURL&)));
	connect(m_projectview, SIGNAL(closeProject(const KURL&)), this, SLOT(projectClose(const KURL&)));
	connect(m_projectview, SIGNAL(projectOptions(const KURL&)), this, SLOT(projectOptions(const KURL&)));
	connect(m_projectview, SIGNAL(projectArchive(const KURL&)), this, SLOT(projectArchive(const KURL&)));
	connect(m_projectview, SIGNAL(removeFromProject(const KURL &,const KURL &)), this, SLOT(removeFromProject(const KURL &,const KURL &)));
	connect(m_projectview, SIGNAL(addFiles(const KURL &)), this, SLOT(projectAddFiles(const KURL &)));
	connect(m_projectview, SIGNAL(toggleArchive(const KURL &)), this, SLOT(toggleArchive(const KURL &)));
	connect(m_projectview, SIGNAL(addToProject(const KURL &)), this, SLOT(addToProject(const KURL &)));
	connect(m_projectview, SIGNAL(saveURL(const KURL &)), this, SLOT(saveURL(const KURL &)));
	connect(m_projectview, SIGNAL(buildProjectTree(const KURL &)), this, SLOT(buildProjectTree(const KURL &)));
	connect(this, SIGNAL(projectTreeChanged(const KileProject *)),m_projectview, SLOT(refreshProjectTree(const KileProject *)));

	ButtonBar->insertTab( UserIcon("structure"),1,i18n("Structure"));
	connect(ButtonBar->getTab(1),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	outstruct = new KListView( Structview );
	outstruct->setFocusPolicy(QWidget::ClickFocus);
	outstruct->header()->hide();
	outstruct->addColumn(i18n("Structure"),-1);
	outstruct->setSorting(-1,true);
	connect( outstruct, SIGNAL(clicked(QListViewItem *)), SLOT(ClickedOnStructure(QListViewItem *)));
	connect( outstruct, SIGNAL(doubleClicked(QListViewItem *)), SLOT(DoubleClickedOnStructure(QListViewItem *)));
	QToolTip::add(outstruct, i18n("Click to jump to the line"));
	mpview = new metapostview( Structview );
	connect(mpview, SIGNAL(clicked(QListBoxItem *)), SLOT(InsertMetaPost(QListBoxItem *)));

	symbol_present=false;
	ButtonBar->insertTab(UserIcon("math1"),2,i18n("Relation Symbols"));
	connect(ButtonBar->getTab(2),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(UserIcon("math2"),3,i18n("Arrow Symbols"));
	connect(ButtonBar->getTab(3),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(UserIcon("math3"),4,i18n("Miscellaneous Symbols"));
	connect(ButtonBar->getTab(4),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(UserIcon("math4"),5,i18n("Delimiters"));
	connect(ButtonBar->getTab(5),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(UserIcon("math5"),6,i18n("Greek Letters"));
	connect(ButtonBar->getTab(6),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(UserIcon("math6"),7,i18n("Foreign characters"));
	connect(ButtonBar->getTab(7),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(UserIcon("metapost"),8,i18n("MetaPost Commands"));
	connect(ButtonBar->getTab(8),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));

	splitter2=new QSplitter(QSplitter::Vertical, splitter1, "splitter2");
	tabWidget=new QTabWidget(splitter2);
	tabWidget->setFocusPolicy(QWidget::ClickFocus);
	tabWidget->setFocus();
	connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(newCaption()) );
	connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(activateView( QWidget * )) );
	connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(updateModeStatus()) );

	//Log/Messages/KShell widgets
	Outputview=new QTabWidget(splitter2);
	Outputview->setFocusPolicy(QWidget::ClickFocus);

	LogWidget = new MessageWidget( Outputview );
	LogWidget->setFocusPolicy(QWidget::ClickFocus);
	LogWidget->setMinimumHeight(40);
	LogWidget->setReadOnly(true);
	Outputview->addTab(LogWidget,UserIcon("viewlog"), i18n("Log/Messages"));

	OutputWidget = new MessageWidget( Outputview );
	OutputWidget->setFocusPolicy(QWidget::ClickFocus);
	OutputWidget->setMinimumHeight(40);
	OutputWidget->setReadOnly(true);
	Outputview->addTab(OutputWidget,UserIcon("output_win"), i18n("Output"));

	logpresent=false;
	errorlist=new QStrList();
	warnlist=new QStrList();
	connect(LogWidget, SIGNAL(clicked(int,int)),this,SLOT(ClickedOnOutput(int,int)));

	texkonsole=new TexKonsoleWidget(Outputview,"konsole");
	Outputview->addTab(texkonsole,SmallIcon("konsole"),i18n("Konsole"));

	QValueList<int> sizes;
	sizes << split2_top << split2_bottom;
	splitter2->setSizes( sizes );
	sizes.clear();
	sizes << split1_left << split1_right;
	splitter1->setSizes( sizes );

	topWidgetStack->addWidget(splitter1 , 0);
	setCentralWidget(topWidgetStack);
	ShowOutputView(false);
	ShowStructView(false);
	Outputview->showPage(LogWidget);
	lastvtab=1;
	newCaption();
	showVertPage(0);
	m_singlemode=true;
	m_masterName=getName();

	partManager->setActivePart( 0L );

	showmaintoolbar=!showmaintoolbar;ToggleShowMainToolbar();
	showtoolstoolbar=!showtoolstoolbar;ToggleShowToolsToolbar();
	showedittoolbar=!showedittoolbar;ToggleShowEditToolbar();
	showmathtoolbar=!showmathtoolbar;ToggleShowMathToolbar();

	m_lyxserver = new KileLyxServer(m_runlyxserver);
	connect(m_lyxserver, SIGNAL(insertCite(const QString&)), this, SLOT(insertCite(const QString& )));
	connect(m_lyxserver, SIGNAL(insertBibTeX(const QString&)), this, SLOT(insertBibTeX(const QString& )));
	connect(m_lyxserver, SIGNAL(insertBibTeXDatabaseAdd(const QString&)), this, SLOT(insertBibTeXDatabaseAdd(const QString& )));

	KileApplication::closeSplash();
	show();
	ToggleAccels();
	connect(Outputview, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(RunTerminal(QWidget * )) );

	applyMainWindowSettings(config, "KileMainWindow" );

	restore();
}

Kile::~Kile()
{
	kdDebug() << "cleaning up..." << endl;
	delete errorlist;
	delete m_AutosaveTimer;
}

void Kile::setupActions()
{
	(void) KStdAction::openNew(this, SLOT(fileNew()), actionCollection(), "New" );
	(void) KStdAction::open(this, SLOT(fileOpen()), actionCollection(),"Open" );
	fileOpenRecentAction = KStdAction::openRecent(this, SLOT(fileOpen(const KURL&)), actionCollection(), "Recent");
	(void) new KAction(i18n("Save All"),"save_all", 0, this, SLOT(fileSaveAll()), actionCollection(),"SaveAll" );
	(void) new KAction(i18n("Create Template From Document..."),0,this,SLOT(createTemplate()), actionCollection(),"CreateTemplate");
	(void) KStdAction::close(this, SLOT(fileClose()), actionCollection(),"Close" );
	(void) new KAction(i18n("Close All"),0, this, SLOT(fileCloseAll()), actionCollection(),"CloseAll" );
	(void) new KAction(i18n("Statistics"), 0, this, SLOT(showDocInfo()), actionCollection(), "Statistics" );
	(void) KStdAction::quit(this, SLOT(close()), actionCollection(),"Exit" );

	(void) KStdAction::spelling(this, SLOT(spellcheck()), actionCollection(),"Spell" );
	(void) new KAction(i18n("Refresh Structure"),"structure",0 , this, SLOT(RefreshStructure()), actionCollection(),"RefreshStructure" );

	//project actions
	(void) new KAction(i18n("&New Project..."), "filenew", 0, this, SLOT(projectNew()), actionCollection(), "project_new");
	(void) new KAction(i18n("&Open Project..."), "fileopen", 0, this, SLOT(projectOpen()), actionCollection(), "project_open");
	m_actRecentProjects =  new KRecentFilesAction(i18n("Open &Recent Project..."),  0, this, SLOT(projectOpen(const KURL &)), actionCollection(), "project_openrecent");
	(void) new KAction(i18n("A&dd files to project..."), 0, this, SLOT(projectAddFiles()), actionCollection(), "project_add");
	(void) new KAction(i18n("Build Project &Tree"), "relation", 0, this, SLOT(buildProjectTree()), actionCollection(), "project_buildtree");
	(void) new KAction(i18n("&Archive"), "package", 0, this, SLOT(projectArchive()), actionCollection(), "project_archive");
	(void) new KAction(i18n("Project &Options..."), "configure", 0, this, SLOT(projectOptions()), actionCollection(), "project_options");
	(void) new KAction(i18n("&Close Project"), "fileclose", 0, this, SLOT(projectClose()), actionCollection(), "project_close");

	//build actions
	(void) new KAction(i18n("Quick Build"),"quick", ALT+Key_1, this, SLOT(QuickBuild()), actionCollection(),"QuickBuild" );
	(void) new KAction(i18n("View Log File"),"viewlog", ALT+Key_0, this, SLOT(ViewLog()), actionCollection(),"ViewLog" );
	(void) new KAction(i18n("Previous LaTeX Error"),"errorprev", 0, this, SLOT(PreviousError()), actionCollection(),"PreviousError" );
	(void) new KAction(i18n("Next LaTeX Error"),"errornext", 0, this, SLOT(NextError()), actionCollection(),"NextError" );
	(void) new KAction(i18n("Previous LaTeX Warning"),"warnprev", 0, this, SLOT(PreviousWarning()), actionCollection(),"PreviousWarning" );
	(void) new KAction(i18n("Next LaTeX Warning"),"warnnext", 0, this, SLOT(NextWarning()), actionCollection(),"NextWarning" );
	StopAction = new KAction(i18n("&Stop"),"stop",Key_Escape,this,SIGNAL(stopProcess()),actionCollection(),"Stop");
	StopAction->setEnabled(false);
	(void) new KAction("LaTeX","latex", ALT+Key_2, this, SLOT(Latex()), actionCollection(),"Latex" );
	(void) new KAction(i18n("View Dvi"),"viewdvi", ALT+Key_3, this, SLOT(ViewDvi()), actionCollection(),"ViewDvi" );
	(void) new KAction(i18n("Dvi to PS"),"dvips", ALT+Key_4, this, SLOT(DviToPS()), actionCollection(),"DvitoPS" );
	(void) new KAction(i18n("View PS"),"viewps", ALT+Key_5, this, SLOT(ViewPS()), actionCollection(),"ViewPS" );
	(void) new KAction(i18n("PDFLaTeX"),"latex", ALT+Key_6, this, SLOT(PDFLatex()), actionCollection(),"PDFLatex" );
	(void) new KAction(i18n("View PDF"),"viewpdf", ALT+Key_7, this, SLOT(ViewPDF()), actionCollection(),"ViewPDF" );
	(void) new KAction(i18n("PS to PDF"),"ps2pdf", ALT+Key_8, this, SLOT(PStoPDF()), actionCollection(),"PStoPDF" );
	(void) new KAction(i18n("DVI to PDF"),"dvipdf",ALT+Key_9, this, SLOT(DVItoPDF()), actionCollection(),"DVItoPDF" );
	(void) new KAction(i18n("BibTeX"),ALT+Key_Minus, this, SLOT(MakeBib()), actionCollection(),"MakeBib" );
	(void) new KAction(i18n("Make Index"),ALT+Key_Equal, this, SLOT(MakeIndex()), actionCollection(),"MakeIndex" );
	(void) new KAction(i18n("LaTeX to HTML"),"l2h",0, this, SLOT(LatexToHtml()), actionCollection(),"LaTeXtoHtml" );
	(void) new KAction(i18n("View HTML"),"viewhtml", 0, this, SLOT(HtmlPreview()), actionCollection(),"HtmlPreview" );
	(void) new KAction(i18n("View Bibtex"),0 , this, SLOT(Bibtexeditor()), actionCollection(),"Bibtexeditor" );
	(void) new KAction(i18n("Kdvi Forward Search"),"dvisearch",0, this, SLOT(KdviForwardSearch()), actionCollection(),"KdviForwardSearch" );
	(void) new KAction(i18n("Clean"),0 , this, SLOT(CleanAll()), actionCollection(),"CleanAll" );
	(void) new KAction(i18n("Mpost"),0 , this, SLOT(MetaPost()), actionCollection(),"MetaPost" );
	(void) new KAction(i18n("Editor View"),"edit",CTRL+Key_E , this, SLOT(ShowEditorWidget()), actionCollection(),"EditorView" );
	(void) new KAction(i18n("Next Document"),"forward",ALT+Key_Right, this, SLOT(gotoNextDocument()), actionCollection(), "gotoNextDocument" );
	(void) new KAction(i18n("Previous Document"),"back",ALT+Key_Left, this, SLOT(gotoPrevDocument()), actionCollection(), "gotoPrevDocument" );
	(void) new KAction(i18n("Focus Log/Messages view"), CTRL+ALT+Key_M, this, SLOT(focusLog()), actionCollection(), "focus_log");
	(void) new KAction(i18n("Focus Output view"), CTRL+ALT+Key_O, this, SLOT(focusOutput()), actionCollection(), "focus_output");
	(void) new KAction(i18n("Focus Konsole view"), CTRL+ALT+Key_K, this, SLOT(focusKonsole()), actionCollection(), "focus_konsole");
	(void) new KAction(i18n("Focus Editor view"), CTRL+ALT+Key_E, this, SLOT(focusEditor()), actionCollection(), "focus_editor");

	BackAction = KStdAction::back(this, SLOT(BrowserBack()), actionCollection(),"Back" );
	ForwardAction = KStdAction::forward(this, SLOT(BrowserForward()), actionCollection(),"Forward" );
	HomeAction = KStdAction::home(this, SLOT(BrowserHome()), actionCollection(),"Home" );

	QPtrList<KAction> alt_list;
	KileStdActions::setupStdTags(this,this, &alt_list);
	altI_action = alt_list.at(0);
	altA_action = alt_list.at(1);
	altB_action = alt_list.at(2);
	altT_action = alt_list.at(3);
	altC_action = alt_list.at(4);
	altH_action = alt_list.at(5);

	alt_list.clear();
	KileStdActions::setupMathTags(this, &alt_list);

	altM_action = alt_list.at(0);
	altE_action = alt_list.at(1);
 	altD_action = alt_list.at(2);
	altU_action = alt_list.at(3);
	altF_action = alt_list.at(4);
	altQ_action = alt_list.at(5);
	altS_action = alt_list.at(6);
	altL_action = alt_list.at(7);
	altR_action = alt_list.at(8);

	KileStdActions::setupBibTags(this);

  (void) new KAction(i18n("Quick Start"),"wizard",0 , this, SLOT(QuickDocument()), actionCollection(),"127" );
  (void) new KAction(i18n("Letter"),"wizard",0 , this, SLOT(QuickLetter()), actionCollection(),"128" );
  (void) new KAction(i18n("Tabular"),"wizard",0 , this, SLOT(QuickTabular()), actionCollection(),"129" );
  (void) new KAction(i18n("Tabbing"),"wizard",0 , this, SLOT(QuickTabbing()), actionCollection(),"149" );
  (void) new KAction(i18n("Array"),"wizard",0 , this, SLOT(QuickArray()), actionCollection(),"130" );


  (void) new KAction(i18n("Clean"),0 , this, SLOT(CleanBib()), actionCollection(),"CleanBib" );

  (void) new KAction("Xfig","xfig",0 , this, SLOT(RunXfig()), actionCollection(),"144" );
  (void) new KAction(i18n("Gnuplot Front End"),"xgfe",0 , this, SLOT(RunGfe()), actionCollection(),"145" );

  ModeAction=new KToggleAction(i18n("Define Current Document as 'Master Document'"),"master",0 , this, SLOT(ToggleMode()), actionCollection(),"Mode" );

  MenuAccelsAction = new KToggleAction(i18n("Standard Menu Shortcuts"), 0, this,SLOT(ToggleAccels()),actionCollection(),"MenuAccels" );
  MenuAccelsAction->setChecked(m_menuaccels);

  (void) KStdAction::preferences(this, SLOT(GeneralOptions()), actionCollection(),"settings_configure" );
  (void) KStdAction::keyBindings(this, SLOT(ConfigureKeys()), actionCollection(),"147" );
  (void) KStdAction::configureToolbars(this, SLOT(ConfigureToolbars()), actionCollection(),"148" );

  StructureAction=new KToggleAction(i18n("Show Structure View"),0 , this, SLOT(ToggleStructView()), actionCollection(),"StructureView" );
  MessageAction=new KToggleAction(i18n("Show Messages View"),0 , this, SLOT(ToggleOutputView()), actionCollection(),"MessageView" );

  ShowMainToolbarAction=new KToggleAction(i18n("Show Main Toolbar"),0 , this, SLOT(ToggleShowMainToolbar()), actionCollection(),"ShowMainToolbar" );
  ShowMainToolbarAction->setChecked(showmaintoolbar);

  ShowToolsToolbarAction=new KToggleAction(i18n("Show Tools Toolbar"),0 , this, SLOT(ToggleShowToolsToolbar()), actionCollection(),"ShowToolsToolbar" );
  ShowToolsToolbarAction->setChecked(showtoolstoolbar);

  ShowEditToolbarAction=new KToggleAction(i18n("Show Edit Toolbar"),0 , this, SLOT(ToggleShowEditToolbar()), actionCollection(),"ShowEditToolbar" );
  ShowEditToolbarAction->setChecked(showedittoolbar);

  ShowMathToolbarAction=new KToggleAction(i18n("Show Math Toolbar"),0 , this, SLOT(ToggleShowMathToolbar()), actionCollection(),"ShowMathToolbar" );
  ShowMathToolbarAction->setChecked(showmathtoolbar);

  if (m_singlemode) {ModeAction->setChecked(false);}
  else {ModeAction->setChecked(true);}
  if (showstructview) {StructureAction->setChecked(true);}
  else {StructureAction->setChecked(false);}
  if (showoutputview) {MessageAction->setChecked(true);}
  else {MessageAction->setChecked(false);}

  (void) new KAction(i18n("Remove Template..."),0,this,SLOT(removeTemplate()),actionCollection(),"removetemplates");

  WatchFileAction=new KToggleAction(i18n("Watch File Mode"),"watchfile",0 , this, SLOT(ToggleWatchFile()), actionCollection(),"WatchFile" );
  if (watchfile) {WatchFileAction->setChecked(true);}
  else {WatchFileAction->setChecked(false);}

	const KAboutData *aboutData = KGlobal::instance()->aboutData();
	help_menu = new KHelpMenu( this, aboutData);
	(void) new KAction(i18n("LaTeX Reference"),"help",0 , this, SLOT(LatexHelp()), actionCollection(),"help1" );
	(void) new KAction(i18n("Kile Handbook"),"contents",0 , this, SLOT(invokeHelp()), actionCollection(),"help2" );

	(void) KStdAction::reportBug (help_menu, SLOT(reportBug()), actionCollection(), "report_bug");
	(void) KStdAction::aboutApp(help_menu, SLOT(aboutApplication()), actionCollection(),"help4" );
	(void) KStdAction::aboutKDE(help_menu, SLOT(aboutKDE()), actionCollection(),"help5" );

	m_menuUserTags = new KActionMenu(i18n("User Tags"), UserIcon("usertag"), actionCollection(),"menuUserTags");
	m_menuUserTags->setDelayed(false);
	m_mapUserTagSignals = new QSignalMapper(this,"mapUserTagSignals");
	setupUserTagActions();
	connect(m_mapUserTagSignals,SIGNAL(mapped(int)),this,SLOT(insertUserTag(int)));

	m_menuUserTools = new KActionMenu(i18n("User Tools"), UserIcon("usertool"), actionCollection(), "menuUserTools");
	m_menuUserTools->setDelayed(false);
	m_mapUserToolsSignals = new QSignalMapper(this,"mapUserToolsSignals");
	setupUserToolActions();
	connect(m_mapUserToolsSignals,SIGNAL(mapped(int)), this, SLOT(execUserTool(int)));


  	actionCollection()->readShortcutSettings();

  	setHelpMenuEnabled(false);
}

void Kile::setupUserTagActions()
{
	KShortcut sc=0;
	QString name;
	KAction *menuItem;

	KShortcut tagaccels[10] = {CTRL+SHIFT+Key_1, CTRL+SHIFT+Key_2,CTRL+SHIFT+Key_3,CTRL+SHIFT+Key_4,CTRL+SHIFT+Key_5,CTRL+SHIFT+Key_6,CTRL+SHIFT+Key_7,
		CTRL+SHIFT+Key_8,CTRL+SHIFT+Key_9,CTRL+SHIFT+Key_0};

	m_actionEditTag = new KAction(i18n("Edit User Tags"),0 , this, SLOT(EditUserMenu()), m_menuUserTags,"EditUserMenu" );
	m_menuUserTags->insert(m_actionEditTag);
	for (uint i=0; i<m_listUserTags.size(); i++)
	{
		if (i<10) sc = tagaccels[i];
		else sc=0;
		name=QString::number(i+1)+": "+m_listUserTags[i].name;
		menuItem = new KAction(name,sc,m_mapUserTagSignals,SLOT(map()), m_menuUserTags, name.ascii());
		m_listUserTagsActions.append(menuItem);
		m_menuUserTags->insert(menuItem);
		m_mapUserTagSignals->setMapping(menuItem,i);
	}
}

void Kile::setupUserToolActions()
{
	KShortcut sc=0;
	QString name;
	KAction *menuItem;

	KShortcut toolaccels[10] = {SHIFT+ALT+Key_1, SHIFT+ALT+Key_2,SHIFT+ALT+Key_3,SHIFT+ALT+Key_4,SHIFT+ALT+Key_5,SHIFT+ALT+Key_6,SHIFT+ALT+Key_7,
		SHIFT+ALT+Key_8,SHIFT+ALT+Key_9,SHIFT+ALT+Key_0};

	m_actionEditTool = new KAction(i18n("Edit User Tools"),0 , this, SLOT(EditUserTool()), actionCollection(),"EditUserTool" );
	m_menuUserTools->insert(m_actionEditTool);
	for (uint i=0; i<m_listUserTools.size(); i++)
	{
		if (i<10) sc = toolaccels[i];
		else sc=0;
		name=QString::number(i+1)+": "+m_listUserTools[i].name;
		menuItem = new KAction(name,sc,m_mapUserToolsSignals,SLOT(map()), m_menuUserTools, name.ascii());
		m_listUserToolsActions.append(menuItem);
		m_menuUserTools->insert(menuItem);
		m_mapUserToolsSignals->setMapping(menuItem,i);
	}
}

void Kile::restore()
{
	if (!m_bRestore) return;

	QFileInfo fi;

	for (uint i=0; i < m_listProjectsOpenOnStart.count(); i++)
	{
		kdDebug() << "restoring " << m_listProjectsOpenOnStart[i] << endl;
		fi.setFile(m_listProjectsOpenOnStart[i]);
		if (fi.isReadable()) projectOpen(KURL::fromPathOrURL(m_listProjectsOpenOnStart[i]));
	}

	for (uint i=0; i < m_listDocsOpenOnStart.count(); i++)
	{
		kdDebug() << "restoring " << m_listDocsOpenOnStart[i] << endl;
		fi.setFile(m_listDocsOpenOnStart[i]);
		if (fi.isReadable()) fileOpen(KURL::fromPathOrURL(m_listDocsOpenOnStart[i]));
	}

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();
}

////////////////////////////// FILE /////////////////////////////
Kate::View* Kile::load( const KURL &url , const QString & encoding, bool create, const QString & highlight, bool load)
{
	QString hl = highlight;

	kdDebug() << "==Kile::load==========================" << endl;
	if ( url.path() != i18n("Untitled") && isOpen(url))
	{
		kdDebug() << "\talready opened " << url.path() << endl;
		Kate::View *view = static_cast<Kate::View*>(docFor(url)->views().first());
		//bring up the view to the already opened doc
		tabWidget->showPage(view);

		UpdateStructure(true);

		//return this view
		return view;
	}

	kdDebug() << QString("\tload(%1,%2,%3, %4)").arg(url.path()).arg(encoding).arg(create).arg(create) << endl;

	//create a new document
	Kate::Document *doc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");
	m_docList.append(doc);

	//set the default encoding
	QString enc = encoding.isNull() ? QString::fromLatin1(QTextCodec::codecForLocale()->name()) : encoding;
	KileFS->comboEncoding->lineEdit()->setText(enc);
	doc->setEncoding(enc);

	//load the contents into the doc and set the docname (we can't always use doc::url() since this returns "" for untitled documents)
	if (load) doc->openURL(url);
	//TODO: connect to completed() signal, now updatestructure is called before loading is completed

	if ( !url.isEmpty() )
	{
		doc->setDocName(url.path());
		fileOpenRecentAction->addURL(url);
	}
	else
	{
		doc->setDocName(i18n("Untitled"));
	}



	KileDocumentInfo *docinfo = 0;

	//see if this file belongs to an opened project
	//if so, make the project class aware
	KileProjectItem *item = itemFor(url);
	if (item)
	{
		kdDebug() << "\t" << url.path() <<" belongs to the project " << item->project()->name()  << endl;
		//decorate the doc with the KileProjectItem
		docinfo = infoFor(item);
		if (docinfo)
			docinfo->setDoc(doc);

		hl = item->highlight();
		item->setOpenState(create);
	}

	//no docinfo generated before, reasons
	// 1. file does not belong to a project, so a docinfo has not been created
	// 2. file does belong to a project and a "Close Project" has been aborted
	if (docinfo == 0)
	{
		//install a documentinfo class for this doc
		docinfo = new KileDocumentInfo(doc);
		//decorate the document with the KileDocumentInfo class
		docinfo->setListView(outstruct);
		docinfo->setURL(url);
		m_infoList.append(docinfo);
	}
	mapInfo(doc, docinfo);

	setHighlightMode(doc, hl);

	//handle changes of the document
	connect(doc, SIGNAL(nameChanged(Kate::Document *)), docinfo, SLOT(emitNameChanged(Kate::Document *)));
	//why not connect doc->nameChanged directly ot this->slotNameChanged ? : the function emitNameChanged
	//updates the docinfo, on which all decisions are bases in slotNameChanged
	connect(docinfo,SIGNAL(nameChanged(Kate::Document*)), this, SLOT(slotNameChanged(Kate::Document*)));
	connect(docinfo, SIGNAL(nameChanged(Kate::Document *)), this, SLOT(newCaption()));
	connect(doc, SIGNAL(modStateChanged(Kate::Document*)), this, SLOT(newDocumentStatus(Kate::Document*)));

	if (create) return createView(doc);
	else return 0;
}

Kate::View * Kile::createView(Kate::Document *doc)
{
	kdDebug() << "==Kile::createView==========================" << endl;
	kdDebug() << "\t"<< doc->docName() << endl;
	Kate::View *view;
	view = (Kate::View*) doc->createView (tabWidget, 0L);

	//install event filter on the view
	view->installEventFilter(m_eventFilter);

	//insert the view in the tab widget
	tabWidget->addTab( view, getShortName(doc) );
	tabWidget->showPage( view );
	m_viewList.append(view);

	connect(view, SIGNAL(viewStatusMsg(const QString&)), this, SLOT(newStatus(const QString&)));
	connect(view, SIGNAL(newStatus()), this, SLOT(newCaption()));

	//activate the newly created view
	activateView(view, false, false);

	newStatus();
	newCaption();

	view->setFocusPolicy(QWidget::StrongFocus);
	view->setFocus();

	return view;
}

void Kile::slotNameChanged(Kate::Document * doc)
{
	kdDebug() << "==Kile::slotNameChagned==========================" << endl;
	//set the doc name so we can use the docname to set the caption
	//(we want the caption to be untitled for an new document not ""
	//doc->setDocName(doc->url().path());
	QPtrList<KTextEditor::View> list = doc->views();
	for (uint i=0; i < list.count(); i++)
	{
		tabWidget->setTabLabel((Kate::View*) list.at(i), getShortName(doc));
	}

	KileDocumentInfo *docinfo = infoFor(doc);

	//add to project view if doc was Untitled before
	if (docinfo->oldURL().isEmpty())
	{
		kdDebug() << "\tadding URL to projectview " << doc->url().path() << endl;
		m_projectview->add(doc->url());
	}
}

Kate::View *Kile::currentView() const
{
	if ( 	tabWidget->currentPage() &&
		tabWidget->currentPage()->inherits( "Kate::View" ) )
	{
		return (Kate::View*)tabWidget->currentPage();
	}

	return 0;
}

void Kile::setLine( const QString &line )
{
	bool ok;
	uint l=line.toUInt(&ok,10), para, col;
	Kate::View *view = currentView();
	if (view && ok)
  	{
		view->cursorPosition(&para,&col);
  		view->setCursorPosition(l,0);

		if ( l > para)
		{
			view->down();view->down();view->down();
			view->up();view->up();view->up();
		}

		if ( l < para)
		{
			view->up();view->up();view->up();
			view->down();view->down();view->down();
		}

		this->show();
		this->raise();
		view->setFocus();
		view->gotoLineNumber(l);
		ShowEditorWidget();
		newStatus();
  	}
}

void Kile::setHighlightMode(Kate::Document * doc, const QString &highlight)
{
	int c = doc->hlModeCount();
	bool found = false;
	int i;

	QString hl = highlight;
	QString ext = doc->url().fileName().right(4);

	if ( hl == QString::null && ext == ".bib" ) hl = "BibTeX-Kile";

	if ( (hl != QString::null) || doc->url().isEmpty() || ext == ".tex" || ext == ".ltx"  || ext == ".dtx" || ext == ".sty" || ext == ".cls" )
	{
		if (hl == QString::null) hl = "LaTeX-Kile";
		for (i = 0; i < c; i++)
		{
			if (doc->hlModeName(i) == hl) { found = true; break; }
		}

		if (found)
		{
			doc->setHlMode(i);
		}
		else
		{
			//doc->setHlMode(0);
			kdWarning() << "could not find the LaTeX2 highlighting definitions" << endl;
		}
	}
}

void Kile::fileNew()
{
    NewFileWizard *nfw = new NewFileWizard(this);

    if (nfw->exec()) {
		loadTemplate(nfw->getSelection());
    }
}

Kate::View* Kile::loadTemplate(TemplateItem *sel)
{
	Kate::View *view = load(KURL());

	if (sel->name() != DEFAULT_EMPTY_CAPTION)
	{
		//create a new document to open the template in
		Kate::Document *tempdoc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");

		if (!tempdoc->openURL(sel->path()))
		{
			KMessageBox::error(this, i18n("Couldn't find template: %1").arg(sel->name()),i18n("File Not Found!"));
		}
		else
		{
			//substitute templates variables
			QString text = tempdoc->text();
			delete tempdoc;
			replaceTemplateVariables(text);
			view->getDoc()->setText(text);
			view->getDoc()->setModified(false);

			//set the highlight mode (this was already done in load, but somehow after setText this is forgotten)
			setHighlightMode(view->getDoc());
		}
	}

	return view;
}
//TODO: connect to modifiedondisc() when using KDE 3.2
bool Kile::eventFilter(QObject* o, QEvent* e)
{
	if ( (!m_bBlockWindowActivateEvents) && e->type() == QEvent::WindowActivate && o == this )
	{
		//block windowactivate events since the popup window (isModOnHD) takes focus
		//away from the mainwindow
		m_bBlockWindowActivateEvents = true;

		for (uint i=0; i < m_viewList.count(); i++)
		{
			m_viewList.at(i)->getDoc()->isModOnHD();
		}

		m_bBlockWindowActivateEvents = false;
	}

	return QWidget::eventFilter(o,e);
}


void Kile::activateView(QWidget* w ,bool checkModified /*= true*/, bool updateStruct /* = true */  )  //Needs to be QWidget because of QTabWidget::currentChanged
{
	kdDebug() << "==Kile::activateView==========================" << endl;
	if (!w->inherits("Kate::View"))
		return;

	Kate::View* view = (Kate::View*)w;

	for (uint i=0; i<m_viewList.count(); i++)
	{
		if (m_viewList.at(i)==0)
			kdDebug() << "\tNULL pointer in m_viewList" << endl;
		else
			kdDebug() << "\tremoving client from guiFactory " << m_viewList.at(i)->getDoc()->docName() << endl;

		guiFactory()->removeClient(m_viewList.at(i));
		m_viewList.at(i)->setActive(false);
	}

	toolBar ()->setUpdatesEnabled (false);

	guiFactory()->addClient(view);
	view->setActive( true );

	KParts::GUIActivateEvent ev( true );
   	QApplication::sendEvent( view, &ev );

	if( checkModified )
		if (view) view->getDoc()->isModOnHD();

	if (updateStruct) UpdateStructure();

	toolBar ()->setUpdatesEnabled (true);
}

void Kile::updateModeStatus()
{
	KileProject *project = activeProject();

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

void Kile::replaceTemplateVariables(QString &line)
{
	line=line.replace("$$AUTHOR$$",templAuthor);
	line=line.replace("$$DOCUMENTCLASSOPTIONS$$",templDocClassOpt);
	if (templEncoding != "") { line=line.replace("$$INPUTENCODING$$", "\\input["+templEncoding+"]{inputenc}");}
	else { line = line.replace("$$INPUTENCODING$$","");}
}

void Kile::fileOpen()
{
	//determine the starting dir for the file dialog
	QString currentDir=KileFS->dirOperator()->url().path();
	QFileInfo fi;
	if (currentView())
	{
		fi.setFile(currentView()->getDoc()->url().path());
		if (fi.exists()) currentDir= fi.dirPath();
	}

	//get the URLs
	KURL::List urls = KFileDialog::getOpenURLs( currentDir, i18n("*.ltx *.tex *.dtx *.bib *.sty *.cls *.mp|TeX files\n*|All files"), this,i18n("Open File(s)") );

	//open them
	for (uint i=0; i < urls.count(); i++)
	{
		fileOpen(urls[i]);
	}
}


void Kile::fileOpen(const KURL& url, const QString & encoding)
{
	kdDebug() << "==Kile::fileOpen==========================" << endl;
	kdDebug() << "\t" << url.fileName() << endl;
	bool isopen = isOpen(url);

	Kate::View *view = load(url, encoding);

	//URL wasn't open before loading, add it to the project view
	if (!isopen) m_projectview->add(url);

	if (view)
		infoFor(view->getDoc())->updateStruct(m_defaultLevel);
	updateModeStatus();
}


bool Kile::isOpen(const KURL & url)
{
	Kate::Document *doc = docFor(url);
	if ( doc == 0)
		return false;
	else
	{
		return true;
	}
}

void Kile::fileSaveAll(bool amAutoSaving)
{
	Kate::View *view;
	QFileInfo fi;

	kdDebug() << "==Kile::fileSaveAll=================" << endl;
	kdDebug() << "\tautosaving = " << amAutoSaving << endl;

	for (uint i = 0; i < m_viewList.count(); i++)
	{
		view = m_viewList.at(i);

		if (view && view->getDoc()->isModified())
		{
			fi.setFile(view->getDoc()->url().path());
			//don't save unwritable and untitled documents when autosaving
			if (
			      (!amAutoSaving) ||
				  (amAutoSaving && (!view->getDoc()->url().isEmpty() ) && fi.isWritable() )
			   )
			{
				kdDebug() << "\tsaving: " << view->getDoc()->url().path() << endl;

				if (amAutoSaving)
				{
					//make a backup
					KURL url = view->getDoc()->url();
					KileAutoSaveJob *job = new KileAutoSaveJob(url);

					//save the current file if job is finished succesfully
					connect(job, SIGNAL(success()), view, SLOT(save()));
				}
				else
					view->save();

				//TODO: make it save to a different location (.backup)
			}
		}
	}
}

void Kile::autoSaveAll()
{
	fileSaveAll(true);
	/*if (m_singlemode)
	{
		statusBar()->changeItem(i18n("Normal mode"), ID_HINTTEXT);
	}
	else
	{
		QString shortName = m_masterName;
      		int pos = shortName.findRev('/');
      		shortName.remove(0,pos+1);
		statusBar()->changeItem(i18n("Master document: %1").arg(shortName), ID_HINTTEXT);
	}*/
}

void Kile::enableAutosave(bool as)
{
	autosave=as;
	if (as) m_AutosaveTimer->start(autosaveinterval);
	else m_AutosaveTimer->stop();
}

void Kile::buildProjectTree(const KURL & url)
{
	KileProject * project = projectFor(url);

	if (project)
		buildProjectTree(project);
}

void Kile::buildProjectTree(KileProject *project)
{
	if (project == 0)
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Build project tree..."));

	if (project)
	{
		//TODO: update structure for all docs
		project->buildProjectTree();
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(this, i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to build the tree for, then choose Build Project Tree again."),i18n( "Could not build project tree."));
}

void Kile::projectNew()
{
	KileNewProjectDlg *dlg = new KileNewProjectDlg(this);

	if (dlg->exec())
	{
		kdDebug() << "==Kile::projectNew==========================" << endl;
		kdDebug() << "\t" << dlg->name() << " " << dlg->location() << endl;

		KileProject *project = new KileProject(dlg->name(), dlg->location());


		//add the project file to the project
		//TODO: shell expand the filename
		KileProjectItem *item = new KileProjectItem(project, project->url());
		item->setOpenState(false);
		projectOpenItem(item);

		if (dlg->createNewFile())
		{
			//create the new document and fill it with the template
			//TODO: shell expand the filename
			Kate::View *view = loadTemplate(dlg->getSelection());

			//derive the URL from the base url of the project
			KURL url = project->baseURL();
			url.addPath(dlg->file());

			KileDocumentInfo *docinfo = infoFor(view->getDoc());
			docinfo->setURL(url);

			//save the new file
			view->getDoc()->saveAs(url);

			//add this file to the project
			item = new KileProjectItem(project, url);
			//project->add(item);
			mapItem(docinfo, item);

			docinfo->updateStruct(m_defaultLevel);
		}

		project->setArchiveCommand(dlg->archiveCommand());
		project->setExtIsRegExp(dlg->useRegExp());
		project->setExtensions(dlg->extensions());
		project->buildProjectTree();

		//project->save();
		addProject(project);
	}
}

void Kile::addProject(const KileProject *project)
{
	m_projects.append(project);
	m_projectview->add(project);
	connect(project, SIGNAL(projectTreeChanged(const KileProject *)), this, SIGNAL(projectTreeChanged(const KileProject *)));
}

KileProject* Kile::selectProject(const QString& caption)
{
	QStringList list;
	QPtrListIterator<KileProject> it(m_projects);
	while (it.current())
	{
		list.append((*it)->name());
		++it;
	}

	KileProject *project = 0;
	QString name = QString::null;
	if (list.count() > 1)
	{
		KileListSelector *dlg  = new KileListSelector(list, caption, i18n("project"), this);
		if (dlg->exec())
		{
			name = list[dlg->currentItem()];
		}
	}
	else if (list.count() == 0)
	{
		return 0;
	}
	else
		name = m_projects.first()->name();

	project = projectFor(name);

	return project;
}

void Kile::addToProject(const KURL & url)
{
	kdDebug() << "==Kile::addToProject==========================" << endl;
	kdDebug() << "\t" <<  url.fileName() << endl;

	KileProject *project = selectProject(i18n("Add to project.."));

	if (project)
	{
		addToProject(project, url);
	}
}

void Kile::addToProject(KileProject* project, const KURL & url)
{
	KileProjectItem *item = new KileProjectItem(project, url);
	item->setOpenState(isOpen(url));
	projectOpenItem(item);
	m_projectview->add(item);
	buildProjectTree(project);
}

void Kile::removeFromProject(const KURL & projecturl, const KURL & url)
{
	kdDebug() << "==Kile::removeFromProject==========================" << endl;
	KileProject *project = projectFor(projecturl);

	if (project)
	{
		KileProjectItem *item = project->item(url);
		if (item)
		{
			kdDebug() << "\tprojecturl = " << projecturl.path() << ", url = " << item->url().path() << endl;

			if (project->url() == item->url())
			{
				KMessageBox::error(this, i18n("This file is the project file, it holds all the information about your project. Therefore it is not allowed to remove this file from its project."), i18n("Cannot remove file from project"));
				return;
			}

			removeMap(infoFor(item), item);
			project->remove(item);

			//move projectviewitem to a place outside of this project tree
			m_projectview->removeItem(url);
			if (isOpen(url)) m_projectview->add(url);

			buildProjectTree(project);
		}
	}
}

void Kile::projectOpenItem(KileProjectItem *item)
{
	kdDebug() << "==Kile::projectOpenItem==========================" << endl;
	kdDebug() << "\titem:" << item->url().path() << endl;

	KileDocumentInfo *docinfo;

	Kate::View *view = 0;
	if (isOpen(item->url())) //remove item from projectview (this file was opened before as a normal file)
	{
		m_projectview->remove(item->url());
	}

	view = load(item->url(),item->encoding(), item->isOpen(), item->highlight());

	if (view) //there is a view for this projectitem, get docinfo by doc
	{
		docinfo = infoFor(view->getDoc());
	}
	else //there is no view for this item, get docinfo by path of this file
	{
		docinfo = infoFor(item->url().path());
	}

	mapItem(docinfo, item);
	docinfo->updateStruct(m_defaultLevel);

	if ((!item->isOpen()) && (view != 0)) //oops, doc apparently was open while the project settings wants it closed, don't trash it the doc, update openstate instead
	{
			item->setOpenState(true);
	}

	if ( (!item->isOpen()) && ( view ==0)) //doc shouldn't be displayed, trash the doc
	{
		//since we've parsed it, trash the document
		trash(docinfo->getDoc());
	}

	//workaround: remove structure of this doc from structureview (shouldn't appear there in the first place)
	outstruct->takeItem(outstruct->firstChild());
}

void Kile::projectOpen(const KURL &url)
{
	kdDebug() << "==Kile::projectOpen==========================" << endl;
	kdDebug() << "\tfilename: " << url.fileName() << endl;
	if (projectIsOpen(url))
	{
		KMessageBox::information(this, i18n("The project you tried to open is already opened. If you wanted to reload the project, close the project before you re-open it."),i18n("Project already open"));
		return;
	}

	QFileInfo fi(url.path());
	if ( ! fi.isReadable() )
	{
		if (KMessageBox::warningYesNo(this, i18n("The project file for this project does not exists or is not readable. Remove this project from the recent projects list?"),i18n("Could not load the project file"))  == KMessageBox::Yes)
			m_actRecentProjects->removeURL(url);

		return;
	}

	KileProject *kp = new KileProject(url);

	m_actRecentProjects->addURL(url);

	KileProjectItemList *list = kp->items();

	kdDebug() << "\t" << list->count() << " items" << endl;

	KileProjectItem *item;
	for ( uint i=0; i < list->count(); i++)
	{
		item = list->at(i);
		projectOpenItem(item);
	}

	kp->buildProjectTree();
	addProject(kp);

	UpdateStructure();
	updateModeStatus();
}

void Kile::sanityCheck()
{
	kdDebug() << "===Kile::sanityCheck()=begin===============" << endl;
	KileProject *project = 0;//= activeProject();

	if (project==0)
	{
		if (m_projects.count() == 0)
			kdError() << "\tNO PROJECTS" << endl;
		for (uint i=0; i < m_projects.count(); i++)
		{
			project=m_projects.at(i);
			if (project)
				kdDebug() << "\tproject: " << project->name() << endl;
			else
				kdError() << "\tZERO" << endl;
		}
	}

	KileDocumentInfo *docinfo;
	Kate::Document *doc;
	for (uint i=0; i< m_infoList.count(); i++)
	{
		docinfo=0;
		doc=0;

		docinfo = m_infoList.at(i);
		doc = docinfo->getDoc();
		kdDebug() << "\tdocinfo " << docinfo->url().fileName() << " has doc " << (doc != 0) << endl;
		if (doc)
		{
			docinfo = infoFor(doc);
			kdDebug() << "\tdoc " << doc->url().fileName() << " has docinfo " << (docinfo !=0 ) << endl;
		}
	}

	if (project==0)
		return;

	//do a sanity check
	KileProjectItemList *list = project->items();

	KileProjectItem *item, *pi = 0;
	KileDocumentInfo *di = 0;
	Kate::Document *kdoc;
	for ( uint i=0; i < list->count(); i++)
	{
		item = list->at(i);

		di = infoFor(item);
		if (di==0) kdError() << "\tNO DOCINFO for item " << item->url().fileName() << endl;
		else
		{
			pi = itemFor(di);
			if (pi==0) kdError() << "\tNO ITEM for docinfo " << item->url().fileName() << endl;

			kdoc =di->getDoc();
			if (kdoc !=0)
			{
				di = infoFor(kdoc);
				if (di==0) kdError() << "\tNO DOCINFO for doc " << item->url().fileName() << endl;
			}
		}
		if (di !=0 && pi != 0) kdDebug() << "\tSANE " << item->url().fileName() << endl;
		di=0;pi=0;
	}
	kdDebug() << "===Kile::sanityCheck()=end===============" << endl;
}

void Kile::projectOpen()
{
	kdDebug() << "==Kile::projectOpen==========================" << endl;
	KURL url = KFileDialog::getOpenURL( "", i18n("*.kilepr|Kile Project files\n*|All files"), this,i18n("Open Project") );

	if (!url.isEmpty())
		projectOpen(url);
}

void Kile::storeProjectItem(KileProjectItem *item, Kate::Document *doc)
{
	kdDebug() << "===Kile::storeProjectItem==============" << endl;
	item->setEncoding( doc->encoding());
	item->setHighlight( doc->hlModeName(doc->hlMode()));

	kdDebug() << "\t" << item->encoding() << " " << item->highlight() << " should be " << doc->hlModeName(doc->hlMode()) << endl;
}

void Kile::projectSave(KileProject *project /* = 0 */)
{
	kdDebug() << "==Kile::projectSave==========================" << endl;
	if (project == 0)
	{
		//find the project that corresponds to the active doc
		project= activeProject();
	}

	if (project == 0 )
		project = selectProject(i18n("Save project..."));

	if (project)
	{
		KileProjectItemList *list = project->items();
		Kate::Document *doc = 0;

		KileProjectItem *item;
		KileDocumentInfo *docinfo;
		//update the open-state of the items
		for (uint i=0; i < list->count(); i++)
		{
			item =list->at(i);
			kdDebug() << "\tsetOpenState(" << item->url().path() << ") to " << isOpen(item->url()) << endl;
			item->setOpenState(isOpen(item->url()));
			docinfo = infoFor(item);
			if (docinfo)
			{
				doc = docinfo->getDoc();
			}

			if (doc)
			{
				storeProjectItem(item, doc);
			}
		}

		project->save();
	}
	else
		KMessageBox::error(this, i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to save, then choose Save Project again."),i18n( "Could determine active project."));
}

void Kile::projectAddFiles(const KURL & url)
{
	KileProject *project = projectFor(url);

	if (project)
		projectAddFiles(project);
}

void Kile::projectAddFiles(KileProject *project)
{
	kdDebug() << "==Kile::projectAddFiles()==========================" << endl;
 	if (project == 0 )
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Add files to project..."));

	if (project)
	{
		//determine the starting dir for the file dialog
		QString currentDir=KileFS->dirOperator()->url().path();
		QFileInfo fi;
		if (currentView())
		{
			fi.setFile(currentView()->getDoc()->url().path());
			if (fi.exists()) currentDir= fi.dirPath();
		}

		KURL::List urls = KFileDialog::getOpenURLs( currentDir, i18n("*|All files"), this,i18n("Add File(s)") );

		//open them
		for (uint i=0; i < urls.count(); i++)
		{
			addToProject(project, urls[i]);
		}
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(this, i18n("There are no projects opened. Please open the project you want to add files to, then choose Add Files again."),i18n( "Could not determine active project."));
}

void Kile::toggleArchive(const KURL & url)
{
	KileProjectItem *item = itemFor(url);
	if (item)
		item->setArchive(!item->archive());
}

bool Kile::projectArchive(const KURL & url)
{
	KileProject *project = projectFor(url);

	if (project)
		return projectArchive(project);
	else
		return false;
}

bool Kile::projectArchive(KileProject *project /* = 0*/)
{
	if (project == 0)
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Archive project..."));

	if (project)
	{
		//TODO: this should be in the KileProject class
		QString command = project->archiveCommand();
		QString files, path;
		QPtrListIterator<KileProjectItem> it(*project->items());
		while (it.current())
		{
			if ((*it)->archive())
			{
				path = (*it)->path();
				KRun::shellQuote(path);
				files += path+" ";
			}
			++it;
		}

		command.replace("%F", files);
		command.replace("%S", project->url().fileName().replace(".kilepr",""));

		QFileInfo fic(project->url().path());

		CommandProcess *proc=execCommand(command,fic,true, true);
 		connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

		if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
		{
			KMessageBox::error( this,i18n("Could not start the archive command, check the archive command in Project Options for any mistakes."));
		}
		else
		{
			OutputWidget->clear();
			LogWidget->clear();
			logpresent=false;
			LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
		}
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(this, i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to archive, then choose Archive again."),i18n( "Could not determine active project."));

	return true;
}

void Kile::projectOptions(const KURL & url)
{
	KileProject *project = projectFor(url);

	if (project)
		projectOptions(project);
}

void Kile::projectOptions(KileProject *project /* = 0*/)
{
	kdDebug() << "==Kile::projectOptions==========================" << endl;
	if (project ==0 )
		project = activeProject();

	if (project == 0 )
		project = selectProject(i18n("Project options for..."));

	if (project)
	{
		kdDebug() << "\t" << project->name() << endl;
		KileProjectOptionsDlg *dlg = new KileProjectOptionsDlg(project, this);
		dlg->exec();
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(this, i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to modify, then choose Project Options again."),i18n( "Could not determine active project."));
}

bool Kile::projectCloseAll()
{
	kdDebug() << "==Kile::projectCloseAll==========================" << endl;
	bool close = true;

	//copy the list, since projectClose() changes the list
	QPtrList<KileProject> list = m_projects;
	QPtrListIterator<KileProject> it(list);
	while ( it.current() )
	{
		close = close && projectClose((*it)->url());
		++it;
	}

	return close;
}

bool Kile::projectClose(const KURL & url)
{
	kdDebug() << "==Kile::projectClose==========================" << endl;
	KileProject *project = 0;

	if (url.isEmpty())
	{
		 project = activeProject();

		 if (project == 0 )
			project = selectProject(i18n("Close project..."));
	}
	else
	{
		project = projectFor(url);
	}

 	if (project)
	{
		kdDebug() << "\tclosing:" << project->name() << endl;

		//close the project file first, projectSave, changes this file
		Kate::Document *doc = docFor(project->url());
		if (doc)
		{
			doc->save();
			fileClose(doc);
		}
		projectSave(project);

		KileProjectItemList *list = project->items();

		bool close = true;
		KileDocumentInfo *docinfo;
		for (uint i =0; i < list->count(); i++)
		{
			docinfo = infoFor(list->at(i));
			if (docinfo) doc = docinfo->getDoc();
			if (doc)
			{
				kdDebug() << "\t\tclosing item " << doc->url().path() << endl;
				close = close && fileClose(doc, true);
			}
		}

		if (close)
		{
			m_projects.remove(project);
			m_projectview->remove(project);
			delete project;
			return true;
		}
		else
			return false;
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(this, i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to close, then choose Close Project again."),i18n( "Could not close project."));

	return true;
}

void Kile::createTemplate() {
   if (currentView()){
      if (currentView()->getDoc()->isModified() ) {
      KMessageBox::information(this,i18n("Please save the file first!"));
      return;
      }
   } else {
      KMessageBox::information(this,i18n("Open/create a document first!"));
      return;
   }

   QFileInfo fi(currentView()->getDoc()->url().path());
   ManageTemplatesDialog mtd(&fi,i18n("Create Template From Document"));
   mtd.exec();
}

void Kile::removeTemplate() {
	ManageTemplatesDialog mtd(i18n("Remove a template."));
	mtd.exec();
}

void Kile::removeView(Kate::View *view)
{
	guiFactory()->removeClient( view );
	tabWidget->removePage(view);
	m_viewList.remove(view);
	delete view;

	//if viewlist is empty, no currentChanged() signal is emitted
	//call UpdateStructure such that the structure view is emptied
	if (m_viewList.isEmpty()) UpdateStructure();
}

void Kile::focusLog()
{
	Outputview->showPage(LogWidget);
}

void Kile::focusOutput()
{
	Outputview->showPage(OutputWidget);
}

void Kile::focusKonsole()
{
	Outputview->showPage(texkonsole);
}

void Kile::focusEditor()
{
	Kate::View *view = currentView();
	if (view) view->setFocus();
}

void Kile::saveURL(const KURL & url)
{
	Kate::Document *doc = docFor(url);

	if (doc)
	{
		doc->save();
	}
}

bool Kile::fileClose(const KURL & url, bool delDocinfo /* = false */ )
{
	QPtrListIterator<Kate::Document> it(m_docList);
	while ( it.current())
	{
		if ((*it)->url() == url )
			return fileClose((*it), delDocinfo);

		++it;
	}

	return true;
}

bool Kile::fileClose(Kate::Document *doc /* = 0*/, bool delDocinfo /* = false */)
{
	Kate::View *view = currentView();

	if (doc == 0)
		doc = activeDocument();

	if (doc)
		view = static_cast<Kate::View*>(doc->views().first());
	else
		return true;

	//TODO: remove from docinfo map, remove from dirwatch

	if (view)
	{
		kdDebug() << "==Kile::fileClose==========================" << endl;
		kdDebug() << "\t" << view->getDoc()->docName() << endl;

		KURL url = view->getDoc()->url();

		KileDocumentInfo *docinfo= infoFor(doc);
		KileProjectItem *item = itemFor(docinfo);

		if (item && doc)
		{
			storeProjectItem(item,doc);
		}

		if (view->getDoc()->closeURL() )
		{
			//KMessageBox::information(this,"closing "+url.path());
			kdDebug() << "\tclosed" << endl;
			removeView(view);
			//remove the decorations

			if ( (item == 0) || delDocinfo)
			{
				//doc doesn't belong to a project, get rid of the docinfo
				//or we're closing the project itself (delDocinfo is true)
				kdDebug() << "ABOUT TO REMOVE DOCINFO (" << (item==0) << "," << delDocinfo << " )" << endl;
				m_infoList.remove(docinfo);
				delete docinfo;
			}

			//remove entry in projectview
			m_projectview->remove(url);

			trash(doc);
		}
		else
			return false;
	}

	kdDebug() << "\t" << m_docList.count() << " documents open." << endl;
	if (m_docList.count() == 0)
		showVertPage(0);

	return true;
}

bool Kile::fileCloseAll()
{
	Kate::View * view = currentView();

	if (view)
	{
		lastDocument = view->getDoc()->url().path();
	}

	//assumes one view per doc here
	while( ! m_viewList.isEmpty() )
    {
		view = m_viewList.first();

		fileClose(view->getDoc());
    }

	return true;
}

bool Kile::queryExit()
{
	SaveSettings();
	return true;
}

bool Kile::queryClose()
{
	for (uint i=0; i < m_projects.count(); i++)
	{
		m_listProjectsOpenOnStart.append(m_projects.at(i)->url().path());
	}

	bool stage1 = projectCloseAll();
	bool stage2 = true;

	if (stage1)
	{
		//KMessageBox::information(this,QString::number(m_viewList.count()));
		for (uint i=0; i < m_viewList.count(); i++)
		{
			//KMessageBox::information(this, "adding "+m_viewList.at(i)->getDoc()->url().path());
			m_listDocsOpenOnStart.append(m_viewList.at(i)->getDoc()->url().path());
		}
		stage2 = fileCloseAll();
	}

	return stage1 && stage2;
}

void Kile::fileSelected(const KFileItem *file)
{
    QString encoding =KileFS->comboEncoding->lineEdit()->text();
	kdDebug() << "==Kile::fileSelected==========================" << endl;
	kdDebug() << "\t" << file->url().fileName() << endl;
	fileOpen(file->url(), encoding);
}

void Kile::showDocInfo(Kate::Document *doc)
{
	if (doc == 0)
	{
		Kate::View *view = currentView();

		if (view) doc = view->getDoc();
		else return;
	}

	KileDocumentInfo *docinfo = infoFor(doc);

	if (docinfo)
	{
		KileDocInfoDlg *dlg = new KileDocInfoDlg(docinfo, this, 0, i18n("Summary for document : %1").arg(getShortName(doc)));
		dlg->exec();
	}
	else
		kdWarning() << "There is know KileDocumentInfo object belonging to this document!" << endl;
}

////////////////// GENERAL SLOTS //////////////
void Kile::newStatus(const QString & msg)
{
	statusBar()->changeItem(msg,ID_LINE_COLUMN);
}

void Kile::newDocumentStatus(Kate::Document *doc)
{
	if (doc)
	{
		kdDebug() << "==Kile::newDocumentStatus==========================" << endl;
		kdDebug() << "\t" << doc->docName() << endl;
		QPtrList<KTextEditor::View> list = doc->views();

		KIconLoader *loader = KGlobal::iconLoader();
		QPixmap icon = doc->isModified() ? loader->loadIcon("modified", KIcon::User, KIcon::SizeSmall, KIcon::DefaultState, 0, true) : QPixmap();

		//QString icon = doc->isModified() ? "modified" : "empty";

		for (uint i=0; i < list.count(); i++)
		{
			//tabWidget->changeTab( list.at(i),UserIcon(icon), getShortName(doc) );
			tabWidget->changeTab( list.at(i), icon, getShortName(doc) );
		}

		//updatestructure if active document changed from modified to unmodified (typically after a save)
		if (doc == activeDocument() && !doc->isModified())
			UpdateStructure(true);
	}
}

const QStringList* Kile::retrieveList(const QStringList* (KileDocumentInfo::*getit)() const)
{
	m_listTemp.clear();

	KileDocumentInfo *docinfo = getInfo();
	KileProjectItem *item = itemFor(docinfo);

	kdDebug() << "Kile::retrieveList()" << endl;
	if (item)
	{
		const KileProject *project = item->project();
		const KileProjectItem *root = project->rootItem(item);
		if (root)
		{
			kdDebug() << "\tusing root item " << root->url().fileName() << endl;

			QPtrList<KileProjectItem> children;
			children.append(root);
			root->allChildren(&children);

			const QStringList *list;

			for (uint i=0; i < children.count(); i++)
			{
				kdDebug() << "\t" << children.at(i)->url().fileName() << endl;
				list = (children.at(i)->getInfo()->*getit)();
				if (list)
				{
					for (uint i=0; i < list->count(); i++)
						m_listTemp << (*list)[i];
				}
			}

			return &m_listTemp;
		}
		else
			return &m_listTemp;
	}
	else	if (docinfo)
	{
		m_listTemp = *((docinfo->*getit)());
		return &m_listTemp;
	}
	else
		return &m_listTemp;
}

const QStringList* Kile::labels()
{
	kdDebug() << "Kile::labels()" << endl;
	const QStringList* (KileDocumentInfo::*p)() const=&KileDocumentInfo::labels;
	const QStringList* list = retrieveList(p);
	return list;
}

const QStringList* Kile::bibItems()
{
	kdDebug() << "Kile::bibItems()" << endl;
	const QStringList* (KileDocumentInfo::*p)() const=&KileDocumentInfo::bibItems;
	const QStringList* list = retrieveList(p);
	return list;
}

const QStringList* Kile::bibliographies()
{
	kdDebug() << "Kile::bibliographies()" << endl;
	const QStringList* (KileDocumentInfo::*p)() const=&KileDocumentInfo::bibliographies;
	const QStringList* list = retrieveList(p);
	return list;
}

void Kile::newCaption()
{
	Kate::View *view = currentView();
	if (view)
	{
		setCaption(i18n("Document: %1").arg(getName(view->getDoc())));
		if (Outputview->currentPage()->inherits("TexKonsoleWidget")) syncTerminal();
	}
}

void Kile::gotoNextDocument()
{
  if ( tabWidget->count() < 2 )
    return;

  int cPage = tabWidget->currentPageIndex() + 1;
  if ( cPage >= tabWidget->count() )
    tabWidget->setCurrentPage( 0 );
  else
    tabWidget->setCurrentPage( cPage );
}

void Kile::gotoPrevDocument()
{
  if ( tabWidget->count() < 2 )
    return;

  int cPage = tabWidget->currentPageIndex() - 1;
  if ( cPage < 0 )
    tabWidget->setCurrentPage( tabWidget->count() - 1 );
  else
    tabWidget->setCurrentPage( cPage );
}

/////////////////// PART & EDITOR WIDGET //////////
void Kile::ShowEditorWidget()
{
	ResetPart();
	setCentralWidget(topWidgetStack);
	topWidgetStack->show();
	splitter1->show();
	splitter2->show();
	if (showstructview)  Structview->show();
	if (showoutputview)   Outputview->show();

	Kate::View *view=currentView();
	if (view) view->setFocus();

	newStatus();
	newCaption();
}


void Kile::ResetPart()
{
KParts::BrowserExtension::ActionSlotMap * actionSlotMap = KParts::BrowserExtension::actionSlotMapPtr();
KParts::BrowserExtension::ActionSlotMap::ConstIterator it = actionSlotMap->begin();
KParts::BrowserExtension::ActionSlotMap::ConstIterator itEnd = actionSlotMap->end();
KParts::BrowserExtension *ext =0L;
if (dvipresent && dvipart) ext = KParts::BrowserExtension::childObject(dvipart);
if (pspresent && pspart) ext = KParts::BrowserExtension::childObject(pspart);
if (ext)
{
  QStrList slotNames =  ext->metaObject()->slotNames();
  for ( ; it != itEnd ; ++it )
  {
    KAction * act = actionCollection()->action( it.key() );
    if ( act && slotNames.contains( it.key()+"()" ) )
    {
        act->disconnect( ext );
    }
  }
}

if (htmlpresent  && htmlpart)
 {
   htmlpart->closeURL();
   partManager->removePart(htmlpart) ;
   topWidgetStack->removeWidget(htmlpart->widget());
   delete htmlpart;
   htmlpart=0L;
 }
else if (dvipresent && dvipart)
 {
   dvipart->closeURL();
   partManager->removePart(dvipart) ;
   topWidgetStack->removeWidget(dvipart->widget());
   delete dvipart;
   dvipart=0L;
 }
else if (pspresent && pspart)
 {
   pspart->closeURL();
   partManager->removePart(pspart) ;
   topWidgetStack->removeWidget(pspart->widget());
   delete pspart;
   pspart=0L;
 }

pspresent=false;
htmlpresent=false;
dvipresent=false;
partManager->setActivePart( 0L);
}

void Kile::ActivePartGUI(KParts::Part * the_part)
{
KParts::BrowserExtension::ActionSlotMap * actionSlotMap = KParts::BrowserExtension::actionSlotMapPtr();
KParts::BrowserExtension::ActionSlotMap::ConstIterator it = actionSlotMap->begin();
KParts::BrowserExtension::ActionSlotMap::ConstIterator itEnd = actionSlotMap->end();
KParts::BrowserExtension *ext =0L;
if (dvipresent && dvipart) ext = KParts::BrowserExtension::childObject(dvipart);
if (pspresent && pspart) ext = KParts::BrowserExtension::childObject(pspart);

if (ext)
{
    QStrList slotNames = ext->metaObject()->slotNames();
    for ( ; it != itEnd ; ++it )
    {
    KAction * act = actionCollection()->action( it.key() );
    if ( act )
    {
      if ( slotNames.contains( it.key()+"()" ) )
      {
          connect( act, SIGNAL( activated() ), ext, it.data() /* SLOT(slot name) */ );
          act->setEnabled( ext->isActionEnabled( it.key() ) );
      } else
          act->setEnabled(false);
    }
  }
}
else
{
    for ( ; it != itEnd ; ++it )
    {
      KAction * act = actionCollection()->action( it.key() );
      if (act) act->setEnabled( false );
    }
}
    createGUI( the_part );
    if (htmlpresent && htmlpart)
    {
    stateChanged( "State1" );
    toolBar("mainToolBar")->hide();
    toolBar("ToolBar2")->hide();
    toolBar("Extra")->show();
    toolBar("ToolBar4")->hide();
    toolBar("ToolBar5")->hide();
    }
    else if (pspresent && pspart)
    {
    stateChanged( "State2" );
    toolBar("mainToolBar")->hide();
    toolBar("ToolBar2")->hide();
    toolBar("Extra")->show();
    toolBar("ToolBar4")->hide();
    toolBar("ToolBar5")->hide();
    }
    else if (dvipresent && dvipart)
    {
    stateChanged( "State3" );
    toolBar("mainToolBar")->hide();
    toolBar("ToolBar2")->hide();
    toolBar("Extra")->show();
    toolBar("ToolBar4")->hide();
    toolBar("ToolBar5")->hide();
    }
    else
    {
    stateChanged( "State4" );
    topWidgetStack->raiseWidget(0);
    if (showmaintoolbar) {toolBar("mainToolBar")->show();}
    if (showtoolstoolbar) {toolBar("ToolBar2")->show();}
    if (showedittoolbar) {toolBar("ToolBar4")->show();}
    if (showmathtoolbar) {toolBar("ToolBar5")->show();}
    toolBar("Extra")->hide();
    }

}

void Kile::BrowserBack()
{
if (htmlpresent)
 {
 ((docpart *)partManager->activePart())->back();
 }
}

void Kile::BrowserForward()
{
if (htmlpresent)
 {
  ((docpart *)partManager->activePart())->forward();
 }
}

void Kile::BrowserHome()
{
if (htmlpresent)
 {
 ((docpart *)partManager->activePart())->home();
 }
}


/////////////////// QUICK /////////////////////////
void Kile::QuickBuild()
{
   m_bQuick = true;
   QStringList command;
   QString compile_command;
   if (quickmode==4) {compile_command = pdflatex_command;} else {compile_command = latex_command;}

   QString finame;
   if ( (finame = prepareForCompile(compile_command) ) == QString::null ) return;

   QFileInfo fic(finame);

   command << compile_command;
   CommandProcess* proc= execCommand(command,fic,true);
   connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(EndQuickCompile()));

   if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
   {
      KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.")
                          .arg(compile_command));
   }
   else
   {
      OutputWidget->clear();
      LogWidget->clear();
      logpresent=false;
      LogWidget->insertLine(i18n("Quick build..."));
      LogWidget->insertLine(i18n("Compilation..."));
      LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
   }

   //newStatus();
}

void Kile::EndQuickCompile()
{
QuickLatexError();
LogWidget->insertLine(i18n("Viewing..."));
switch (quickmode)
 {
  case 1:
    {
    if (errorlist->isEmpty()) {QuickDviToPS();}
    else {NextError();}
    }break;
  case 2:
    {
    if (errorlist->isEmpty() && !watchfile) {ViewDvi();}
    else {NextError();}
    }break;
 case 3:
    {
    if (errorlist->isEmpty()) {KdviForwardSearch();}
    else {NextError();}
    }break;
 case 4:
    {
    if (errorlist->isEmpty() && !watchfile) {ViewPDF();}
    else {NextError();}
    }break;
 case 5:
    {
    if (errorlist->isEmpty()) {QuickDviPDF();}
    else {NextError();}
    }break;
 case 6:
    {
    if (errorlist->isEmpty()) {QuickDviToPS();}
    else {NextError();}
    }break;
 }
}

void Kile::QuickDviToPS()
{
  QStringList files;
  if ( (files = prepareForConversion("DviPs","dvi", "ps")).size() == 0) return;

  QString dviname=files[0];
  QString psname=files[1];
  QFileInfo fic(dviname);

  QStringList command; command << dvips_command;
  CommandProcess *proc=execCommand(command,fic,true);

  if (quickmode==1)
  {
     if (watchfile) connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
     else connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(ViewPS()));
  }
  else
  {
     connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(QuickPS2PDF()));
  }


  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
      KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(dvips_command));
  }
  else
  {
      logpresent=false;
      LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
  }

  //newStatus();
}

void Kile::QuickDviPDF()
{
  QStringList files;
  if ( (files=prepareForConversion("DVItoPDF","dvi","pdf")).size() ==0) return;

  QString dviname=files[0];
  QString pdfname=files[1];

  QFileInfo fic(dviname);

  QStringList command; command << dvipdf_command;
  CommandProcess *proc=execCommand(command,fic,true);

  if (watchfile) connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
  else connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(ViewPDF()));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
     KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(dvipdf_command));
  }
  else
  {
     logpresent=false;
     LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
  }

}

void Kile::QuickPS2PDF()
{
  QStringList files;
  if ( (files=prepareForConversion("PStoPDF","ps","pdf")).size() ==0) return;

  QString psname=files[0];
  QString pdfname=files[1];

  QFileInfo fic(psname);

  QStringList command; command << ps2pdf_command;
  CommandProcess *proc=execCommand(command,fic,true);

  if (watchfile) connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
  else connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(ViewPDF()));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
     KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(ps2pdf_command));
  }
  else
  {
	  logpresent=false;
	  LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
  }
}


/////////////////// TOOLS /////////////////////////

//command : a list representing the command to be started
//          i.e. latex %.tex -interactionmode=nonstop
//          is represented by the list (latex,%S.tex,-interactionmode=nonstop)
//file    : the file to be passed as an argument to the command, %S is substituted
//          by the basename of this file
//enablestop : whether or not this process can be stopped by pressing the STOP button
CommandProcess* Kile::execCommand(const QStringList &command, const QFileInfo &file, bool enablestop,bool runonfile) {
 //substitute %S for the basename of the file
 QStringList cmmnd = command;
 QString dir = file.dirPath();
 QString name = file.baseName(TRUE);

 m_nErrors=m_nWarnings=0;

 CommandProcess* proc = new CommandProcess();
 currentProcess=proc;
 proc->clearArguments();

 if (runonfile)
 {
 	KRun::shellQuote(const_cast<QString&>(dir));
 	(*proc) << "cd " << dir << "&&";
 }

 for ( QValueListIterator<QString> i = cmmnd.begin(); i != cmmnd.end(); i++) {
   if (runonfile) (*i).replace("%S",name);
   (*proc) << *i;
 }


 connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
 connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
 connect(this, SIGNAL( stopProcess() ), proc, SLOT(terminate()));

 if (enablestop) {
    connect(proc, SIGNAL(processExited(KProcess*)), this, SLOT(slotDisableStop()));
    StopAction->setEnabled(true);
 }

 LogWidget->clear();
 return proc;
}

//This function prepares files for compiling by the command <command>.
// - untitled document -> warn user that he needs to save the file
// - save the file (if untitled a file save dialog is opened)
// - determine the file to be compile (this file could be a child of the master document)
// -
// - return the name of the file to be compiled (master document)
QString Kile::prepareForCompile(const QString & command) {
  Kate::View *view = currentView();

  //warn if there is no active view
  if (m_singlemode && !view)
  {
     KMessageBox::error( this,i18n("Could not start the %1 command, because there is no file to run %1 on.\n"
                                   "Make sure you have the file you want to compile open and saved.")
                         .arg(command).arg(command));
     return QString::null;
  }

  //QString finame = getShortName();
	QString finame = getCompileName(true);
	if (finame == QString::null)
	{
		KileProject *project = activeProject();
		if (project)
		{
			KMessageBox::error(this, i18n("The active project does not have a master file. Therefore Kile cannot determine which file to run %1 on. Please define a master file.").arg(command), i18n("No master file defined."));
			return QString::null;
		}
	}

  if ( finame == i18n("Untitled") || finame == "")
  {
  	   if (KMessageBox::warningYesNo(this,i18n("You need to save an untitled document before you run %1 on it.\n"
                                             "Do you want to save it? Click Yes to save and No to abort.").arg(command),
                                   i18n("File Needs to be Saved!"))
         == KMessageBox::No) return QString::null;
  }

  //save the file before doing anything
  //attempting to save an untitled document will result in a file-save dialog pop-up
  if (view) view->save();

  	finame = getCompileName();
	bool isRoot = true;
	KileDocumentInfo *docinfo = infoFor(finame);
	if (docinfo) isRoot = m_bCheckForRoot ? docinfo->isLaTeXRoot() : true;

	if (view && ! isRoot )
	{
		if (KMessageBox::warningYesNo(this,i18n("This document doesn't contain a LaTeX header.\nIt should probably be used with a master document.\nContinue anyway?"))
			== KMessageBox::No)
			return QString::null;
	}

  QFileInfo fic(finame);

  if (!fic.exists() )
  {
     KMessageBox::error(this,i18n("The file %1 does not exist. Are you working with a master document which is accidently deleted?")
                        .arg(fic.absFilePath()));
     return QString::null;
  }

  if (!fic.isReadable() )
  {
     KMessageBox::error(this, i18n("You do not have read permission for the file: %1").arg(fic.absFilePath()));
     return QString::null;
  }

  return fic.absFilePath();
}

QStringList Kile::prepareForConversion(const QString &command, const QString &from, const QString &to)
{
   Kate::View *view = currentView();
   QStringList list,empty;
   QString finame, fromName, toName;

   //warn if there is no active view
   if (m_singlemode && !view)
   {
     KMessageBox::error( this,i18n("Could not start the %1 command, because there is no file to run %1 on. "
                                   "Make sure you have the source file of the file you want to convert open and saved.")
                         .arg(command).arg(command));
     return empty;
   }

	finame = getCompileName();
	if (finame == QString::null)
	{
		KileProject *project = activeProject();
		if (project)
		{
			KMessageBox::error(this, i18n("The active project does not have a master file. Therefore Kile cannot determine which file to run %1 on. Please define a master file.").arg(command), i18n("No master file defined."));
			return empty;
		}
	}

   if ( finame == i18n("Untitled") || finame == "") {
      KMessageBox::error(this,i18n("You need to save an untitled document and make a %1 "
                                   "file out of it. After you have done this, you can turn it into a %2 file.")
                                   .arg(from.upper()).arg(to.upper()),
                         i18n("File needs to be saved and compiled!"));
      return empty;
   }

   QFileInfo fic(finame);
   fromName = fic.dirPath() + "/" +fic.baseName(TRUE) + "." + from;
   toName = fic.dirPath() + "/" +fic.baseName(TRUE) + "." + to;

   fic.setFile(fromName);
   if (!(fic.exists() && fic.isReadable()))
   {
      KMessageBox::error(this, i18n("The %1 file does not exist or you do not have read permission. "
                                    "Did you forget to compile to source file to turn it into a %1 file?").arg(from.upper()).arg(from.upper()));
   }

   list.append(fromName);
   list.append(toName);

   return list;
}

QString Kile::prepareForViewing(const QString & command, const QString &ext, const QString &target /*= QString::null*/)
{
	kdDebug() << "==Kile::prepareForViewing==========================" << endl;
	Kate::View *view = currentView();

   QString finame;
   finame = getCompileName();

   //warn if there is no active view
   if (m_singlemode && !view)
   {
     KMessageBox::error( this, i18n("Unable to determine which %1 file to show. Please open the source file of the %1 file to want to view.")
                         .arg(ext.upper()).arg(ext.upper()));
     return QString::null;
   }

   	if (finame == QString::null)
	{
		KileProject *project = activeProject();
		if (project)
		{
			KMessageBox::error(this, i18n("The active project does not have a master file. Therefore Kile cannot determine which file to run %1 on. Please define a master file.").arg(command), i18n("No master file defined."));
			return QString::null;
		}
	}

   if ( finame == i18n("Untitled") || finame == "") {
      KMessageBox::error(this,i18n("You need to save an untitled document and make a %1 "
                                   "file out of it. After you have done this, you can view the %1 file.")
                                   .arg(ext.upper()).arg(ext.upper()),
                         i18n("File needs to be saved and compiled!"));
      return QString::null;
   }

   QFileInfo fic(finame);
   finame = fic.dirPath() + "/";
   if (!target.isNull())
   {
   		kdDebug() << "\t using target " << target << endl;
		finame += target;
		finame = finame.replace("%S",fic.baseName(TRUE));
		if (finame.right(ext.length()+1) != "."+ext)
			finame += "."+ext;
			
		kdDebug() << "\t resulting in " << finame << endl;
   }
   else
   {
		finame += fic.baseName(TRUE) + "." + ext;
   }

   fic.setFile(finame);

   if ( ! ( fic.isReadable() ) )
   {
      KMessageBox::error(this,i18n("The %1 file does not exist or you do not have read permission. "
                                   "Maybe you forgot to create the %1 file?")
                         .arg(ext.upper()).arg(ext.upper()));
      return QString::null;
   }

   return finame;
}

void Kile::Latex()
{
  QString finame;
  if ( (finame=prepareForCompile("LaTeX")) == QString::null)  return;

  m_bCheckForLaTeXErrors=true;

  QFileInfo fic(finame);
  QStringList command;
  command << latex_command;
  CommandProcess *proc=execCommand(command,fic,true);
  connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
     KMessageBox::error( this,i18n("Could not start LaTeX, make sure you have installed the LaTeX package on your system."));
  }
  else
  {
     OutputWidget->clear();
     LogWidget->clear();
     logpresent=false;
     LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
  }

  //newStatus();
}


void Kile::ViewDvi()
{
  QString finame;
  if ( (finame=prepareForViewing("ViewDvi","dvi")) == QString::null) return;

  QFileInfo fic(finame);

  if (viewdvi_command=="Embedded Viewer")
  {
   ResetPart();
   KLibFactory *dvifactory;
   dvifactory = KLibLoader::self()->factory("kviewerpart");
   if (!dvifactory)    {
      KMessageBox::error(this, i18n("Couldn't find the DVI embedded viewer! Please install kviewshell."));
      return;
   }
   dvipart =(KParts::ReadOnlyPart *)dvifactory->create(topWidgetStack, "kviewerpart", "KViewPart", "dvi");
   dvipresent=true;
   topWidgetStack->addWidget(dvipart->widget() , 1 );
   topWidgetStack->raiseWidget(1);
   dvipart->openURL(finame);
   partManager->addPart(dvipart, true);
   partManager->setActivePart( dvipart);
  }
  else {
    QStringList command; command << viewdvi_command;
    CommandProcess *proc=execCommand(command,fic,false);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::Stdout) )
    {
       KMessageBox::error( this,i18n("Could not start %1. Make sure you have this package installed.").arg(viewdvi_command) );
    }
    else
    {
         OutputWidget->clear();
         LogWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
     }
  }

  //newStatus();
}

void Kile::KdviForwardSearch()
{
	QString finame;
	if ( (finame = prepareForViewing("KDVIForwardSearch","dvi")) == QString::null) return;

	LogWidget->clear();
	logpresent=false;
	//LogWidget->insertLine(i18n("You must be in 'Normal mode' to use this command."));
	LogWidget->insertLine(i18n("If you do not have a TeX-binary which includes inverse search information natively :"));
	LogWidget->insertLine(i18n("- copy the files srcltx.sty and srctex.sty to the directory where your TeX-file resides."));
	LogWidget->insertLine(i18n("- add the line \\usepackage[active]{srcltx} to the preamble of your TeX-file."));
	LogWidget->insertLine(i18n("(see the kdvi handbook for more details)"));

	//this is the DVI file
	QFileInfo fic(finame);
	QString dviname=finame;

	//this is the current file, forward search is done for this file (in the DVI file finame)
	QString texname = getName();
	QFileInfo fic_cur(texname);

	int para=0;
	int index=0;

	Kate::View *view = currentView();

	if (view)
	{
		view->setFocus();
		para = view->cursorLine();
		index = view->cursorColumn();
	}

  if (viewdvi_command=="Embedded Viewer")
  {
   ResetPart();
   KLibFactory *dvifactory;
   dvifactory = KLibLoader::self()->factory("kviewerpart");
   if (!dvifactory)
      {
      KMessageBox::error(this, i18n("Couldn't find the DVI embedded viewer! Please install kviewshell."));
      return;
      }
   dvipart =(KParts::ReadOnlyPart *)dvifactory->create(topWidgetStack, "kviewerpart", "KViewPart", "dvi");
   dvipresent=true;
   topWidgetStack->addWidget(dvipart->widget() , 1 );
   topWidgetStack->raiseWidget(1);
   partManager->addPart(dvipart, true);
   partManager->setActivePart( dvipart);

   dvipart->openURL("file:"+finame+"#src:"+QString::number(para+1)+texname);
   }
   else
   {
    //QStringList command; command << "kdvi" <<"--unique" <<"file:./%S.dvi#src:"+QString::number(para + 1)+"./"+ texname;
	QStringList command; command << "kdvi" <<"--unique" <<"file:"+finame+"#src:"+QString::number(para + 1)+  texname;
    CommandProcess *proc=execCommand(command,fic,false);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::Stdout) )
    {
       KMessageBox::error( this,i18n("Could not start KDVI. Are you sure KDVI is installed on your system?"));
    }
    else
    {
       LogWidget->insertLine(i18n("Launched: %1").arg("kdvi"));
    }
   }


   //newStatus();
}

void Kile::DviToPS()
{
  QStringList files;
  if ( (files = prepareForConversion("DviPs","dvi", "ps")).size() == 0) return;

  QString dviname=files[0];
  QString psname=files[1];
  QFileInfo fic(dviname);

  QStringList command; command << dvips_command;
  CommandProcess *proc=execCommand(command,fic,true);
  connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
      KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(dvips_command));
  }
  else
  {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));

  }

  //newStatus();
}

void Kile::ViewPS()
{
  QString finame;
  if ( (finame=prepareForViewing("ViewPS","ps")) == QString::null) return;

  QFileInfo fic(finame);

   if (viewps_command=="Embedded Viewer")
   {
   ResetPart();
   KLibFactory *psfactory;
   psfactory = KLibLoader::self()->factory("libkghostviewpart");
   if (!psfactory)
      {
      KMessageBox::error(this, i18n("Couldn't find the embedded PostScript viewer! Install kviewshell."));
      return;
      }
   pspart =(KParts::ReadOnlyPart *)psfactory->create(topWidgetStack, "kgvpart", "KParts::ReadOnlyPart" );
   pspresent=true;
   topWidgetStack->addWidget(pspart->widget() , 1 );
   topWidgetStack->raiseWidget(1);
   pspart->openURL(finame);
   partManager->addPart(pspart, true);
   partManager->setActivePart( pspart);
   }
   else
    {
    QStringList command; command << viewps_command;
    CommandProcess *proc=execCommand(command,fic,false);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::Stdout) )
    {
       KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(viewps_command));
    }
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
         }
    }

    //newStatus();
}

void Kile::PDFLatex()
{
  QString finame;
  if ( (finame= prepareForCompile("PDFLaTeX")) == QString::null) return;

  m_bCheckForLaTeXErrors=true;

  QFileInfo fic(finame);

  QStringList command; command << pdflatex_command;
  CommandProcess *proc=execCommand(command,fic,true);
  connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
     KMessageBox::error( this,i18n("Could not start PDFLaTeX. Make sure you have this package installed on your system."));
  }
  else
  {
     OutputWidget->clear();
     logpresent=false;
     LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
  }

  //newStatus();
}

void Kile::ViewPDF()
{
  QString finame;
  if ( (finame = prepareForViewing("ViewPDF","pdf")) == QString::null ) return;

  QFileInfo fic(finame);
   if (viewpdf_command=="Embedded Viewer")
   {
   ResetPart();
   KLibFactory *psfactory;
   psfactory = KLibLoader::self()->factory("libkghostviewpart");
   if (!psfactory)
      {
      KMessageBox::error(this, i18n("Couldn't find the embedded PDF viewer! Install kviewshell."));
      return;
      }
   pspart =(KParts::ReadOnlyPart *)psfactory->create(topWidgetStack, "kgvpart", "KParts::ReadOnlyPart" );
   pspresent=true;
   topWidgetStack->addWidget(pspart->widget() , 1 );
   topWidgetStack->raiseWidget(1);
   pspart->openURL(finame);
   partManager->addPart(pspart, true);
   partManager->setActivePart( pspart);
   }
   else
   {
    QStringList command; command << viewpdf_command;
    CommandProcess *proc=execCommand(command,fic,false);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::Stdout) )
    {
       KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(viewpdf_command));
    }
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
         }
    }


 //newStatus();
}

void Kile::MakeBib()
{
  Kate::View *view = currentView();
  if (!view) return;

  QString finame = getShortName();
  if (finame == i18n("Untitled")) {
     KMessageBox::error(this,i18n("You need to save this file first. Then run LaTeX to create an AUX file which is required to run %1").arg(bibtex_command),
                        i18n("File needs to be saved!"));
     return;
  }

  finame = getCompileName();

  //we need to check for finame==i18n("Untitled") etc. because the user could have
  //escaped the file save dialog
  if ((m_singlemode && !currentView()) || finame=="")
  {
     KMessageBox::error( this,i18n("Unable to determine on which file to run %1. Make sure you have the source file "
                                   "of the file you want to run %1 on open and saved.")
                         .arg(bibtex_command).arg(bibtex_command));
     return;
  }

  QFileInfo fic(finame);
  finame = fic.dirPath()+"/"+fic.baseName(TRUE)+".aux";
  fic.setFile(finame);

  if (!(fic.exists() && fic.isReadable()) )
  {
     KMessageBox::error(this,i18n("The file %1 does not exist or you do not have read permission. "
                                  "You need to run LaTeX to create this file.").arg(finame));
     return;
  }

    QStringList command; command << bibtex_command;
    CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
    {
       KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.")
                           .arg(bibtex_command));
    }
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
         }


 //newStatus();
}

void Kile::MakeIndex()
{
  //TODO: figure out how makeindex works ;-))
  //I'm just guessing here

  Kate::View *view = currentView();
  if (!view) return;

  QString finame = getShortName();
  if (finame == i18n("Untitled")) {
     KMessageBox::error(this,i18n("You need to save this file first. Then run LaTeX to create an idx file "
                                  "which is required to run %1.").arg(makeindex_command),
                        i18n("File needs to be saved!"));
     return;
  }

  if (m_singlemode) {finame=getName();}
  else {
     finame=m_masterName; //FIXME: MasterFile does not get saved if it is modified
  }

  //we need to check for finame==i18n("Untitled") etc. because the user could have
  //escaped the file save dialog
  if ((m_singlemode && !currentView()) || finame=="")
  {
     KMessageBox::error(this,i18n("Unable to determine on which file to run %1. "
                                  "Make sure you have the source file of the file you want to run %1 on open and saved.")
                        .arg(makeindex_command).arg(makeindex_command));
     return;
  }

  QFileInfo fic(finame);
  finame = fic.dirPath()+"/"+fic.baseName(TRUE)+".idx";
  fic.setFile(finame);

  if (!(fic.exists() && fic.isReadable()) )
  {
     KMessageBox::error(this,i18n("The file %1 does not exist or you do not have read permission. "
                                  "You need to run LaTeX to create this file.").arg(finame));
     return;
  }


    QStringList command; command << makeindex_command;
    CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )  { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
         }

  //newStatus();
}

void Kile::PStoPDF()
{
  QStringList files;
  if ( (files=prepareForConversion("PStoPDF","ps","pdf")).size() ==0) return;

  QString psname=files[0];
  QString pdfname=files[1];

  QFileInfo fic(psname);

  QStringList command; command << ps2pdf_command;
  CommandProcess *proc=execCommand(command,fic,true);
  connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
    {
       KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(ps2pdf_command));
    }
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
         }
    //newStatus();
}

void Kile::DVItoPDF()
{
  QStringList files;
  if ( (files=prepareForConversion("DVItoPDF","dvi","pdf")).size() ==0) return;

  QString dviname=files[0];
  QString pdfname=files[1];

  QFileInfo fic(dviname);

    QStringList command; command << dvipdf_command;
    CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
    {
       KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(dvipdf_command));
    }
    else
    {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
    }

  //newStatus();
}

void Kile::MetaPost()
{/*
  //TODO: what the h*ll is MetaPost, how should we deal with the
  //error messages?

  QString finame;

  finame=getShortName();
  if (!currentView() ||finame==i18n("Untitled") || finame=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();

  if (m_singlemode) { finame= getName();}
  else { finame = m_masterName;}

  QFileInfo fi(finame);
  QString name=fi.dirPath()+"/"+fi.baseName(TRUE)+".mp";
  QString mpname=fi.baseName(TRUE)+".mp";
  QFileInfo fic(name);
  if (fic.exists() && fic.isReadable() )
  {
    QStringList command;
    command << "mpost" << "--interaction" << "nonstopmode" <<"%S.mp";
    CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )  { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg("mpost"));
         }
  }
 else
 {
  KMessageBox::error(this, i18n("MetaPost file not found!"));
 }
//newStatus();*/
}

void Kile::CleanAll()
{
  QString finame = getShortName();

  if ((m_singlemode && !currentView()) ||finame==i18n("Untitled") || finame=="")
  {
     KMessageBox::error( this,i18n("Unable to determine what to clean-up. Make sure you have the file opened and saved, then choose Clean All."));
     return;
  }

  finame=getName();

  QFileInfo fic(finame);
  if ( ! (fic.exists() && fic.isReadable() ) )
  {
        KMessageBox::sorry(this,i18n("The current document does not exists or is not readable. I'm not sure if it is ok to go ahead, bailing out."));
        return;
  }

  QString extlist[] = {".log",".aux",".dvi",".aux",".lof",".lot",".bit",".idx" ,".glo",".bbl",".ilg",".toc",".ind"};

    QStringList prettyList;
   QStringList command;

   command << "cd " << fic.dirPath() << "&&";

   for (int i=0; i< 13; i++) {
      prettyList.append(fic.baseName(TRUE)+extlist[i]);
      command << "rm -f" << fic.baseName(TRUE)+extlist[i];
      if (i<12) {command << "&&"; }
   }

   int query = KMessageBox::warningContinueCancelList( this,
            i18n( "Do you really want to delete these files?" ),
            prettyList,
            i18n( "Delete Files" ),
            i18n( "Delete" ));

   if (query==KMessageBox::Continue)
   {
     CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
    {
       KMessageBox::error( this,i18n("Could not start the command."));
    }
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Cleaning up..."));
         }
   }

   //newStatus();
}

void Kile::syncTerminal()
{
	Kate::View *view = currentView();

	if (view)
	{
		QString finame;
		if (m_singlemode) {finame=view->getDoc()->url().path();}
    		else {finame=m_masterName;}

		if (finame == "" || finame == i18n("Untitled") ) return;

    		QFileInfo fic(finame);
  		if ( fic.isReadable() )
    		{
    			texkonsole->SetDirectory(fic.dirPath());
    			texkonsole->activate();
		}

		view->setFocus();
	}
}

void Kile::RunTerminal(QWidget* w)
{
if (w->inherits ("TexKonsoleWidget")) syncTerminal();
}

void Kile::slotDisableStop() {
   StopAction->setEnabled(false);
}

void Kile::LatexToHtml()
{
  QString finame;
  if ( (finame=prepareForCompile("latex2html")) == QString::null ) return;

  QFileInfo fic(finame);

    l2hDlg = new l2hdialog(this,"LaTex2Html Options",i18n("LaTex2Html Options"));
    l2hDlg->options_edit->setText(l2h_options);
    if ( l2hDlg->exec() )
    {
    l2h_options=l2hDlg->options_edit->text();
    QStringList command; command <<  "konsole" << "-e" << "latex2html" << "%S.tex" << l2h_options;
    CommandProcess* proc = execCommand(command,fic,false);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotl2hExited(KProcess*)));

    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: %1").arg("latex2html"));
        }
    }
    delete (l2hDlg);

   //newStatus();
}

void Kile::slotProcessOutput(KProcess* /*proc*/,char* buffer,int buflen)
{
int row = (OutputWidget->paragraphs() == 0)? 0 : OutputWidget->paragraphs()-1;
int col = OutputWidget->paragraphLength(row);
QString s=QCString(buffer,buflen+1);
OutputWidget->setCursorPosition(row,col);
OutputWidget->insertAt(s, row, col);
}

void Kile::slotProcessExited(KProcess* proc)
{
if (m_bCheckForLaTeXErrors)
{
	LatexError();
	m_bCheckForLaTeXErrors=false;
}

QString result;
if (m_nErrors !=0 || m_nWarnings != 0)
{
	result = i18n("Process exited with %1 errors and %1 warnings.").arg(m_nErrors).arg(m_nWarnings);
}
else
if (proc->normalExit())
{
	result= ((proc->exitStatus()) ? i18n("Process failed.") : i18n("Process exited without errors."));
}
else
{
   result= i18n("Process exited unexpectedly.");
}
LogWidget->append(result);

delete proc;
currentProcess=0;
}

void Kile::slotl2hExited(KProcess* proc)
{
QString result;
if (proc->normalExit())
  {
  result= ((proc->exitStatus()) ? i18n("Process failed") : i18n("Process exited normally"));
  }
else
  {
   result= i18n("Process exited with error(s)");
  }
int row = (LogWidget->paragraphs() == 0)? 0 : LogWidget->paragraphs()-1;
int col = LogWidget->paragraphLength(row);
LogWidget->setCursorPosition(row,col);
LogWidget->insertAt(result, row, col);
//newStatus();
HtmlPreview();
}

void Kile::HtmlPreview()
{
	QString finame;
	if ( (finame = prepareForViewing("KHTML","html","%S/index.html") ) == QString::null ) return;

	LogWidget->clear();
	logpresent=false;

	QFileInfo fih(finame);

	ResetPart();
   htmlpart = new docpart(topWidgetStack,"help");
   connect(htmlpart,    SIGNAL(updateStatus(bool, bool)), SLOT(updateNavAction( bool, bool)));
   htmlpresent=true;
   htmlpart->openURL(finame);
   htmlpart->addToHistory(finame);
   topWidgetStack->addWidget(htmlpart->widget() , 1 );
   topWidgetStack->raiseWidget(1);
   partManager->addPart(htmlpart, true);
   partManager->setActivePart( htmlpart);
}

void Kile::Bibtexeditor()
{
	kdDebug() << "==Bibtexeditor()========================" << endl;
  //check if a file is opened
	Kate::View *view = currentView();
	if (m_singlemode && !view)
	{
     KMessageBox::error( this, i18n("Unable to determine which BibTeX file to show. Please open a source file that uses a BibTeX bibliography."));
     return;
   }

	QString finame_t = getCompileName();
	kdDebug() << "Compile name: " << finame_t <<endl;

	//get the referenced bibliograph files
	const QStringList *filesbib = bibliographies();
	int NumberOfBibtexFiles = filesbib->count();
	QString finame_a;

	kdDebug() << "\tno. of files: " << NumberOfBibtexFiles << endl;
	if (NumberOfBibtexFiles == 0)
	{
		KMessageBox::error(this, i18n("Could not find any bibliographies for this document (or any master documents for this document). Refreshing the structure view might help."), i18n("No bibliographies found!"));
		return;
	}

	//If there are more references, show dialog to choose
	if(NumberOfBibtexFiles > 1 )
	{
		kdDebug() << "Opening Bibtex dialog " << endl;
		bibtexdialog *BibtexDlg = new bibtexdialog( (*filesbib),"Bibtex file Selector",i18n("Bibtex"),this );
		if ( BibtexDlg->exec() )
		{
			finame_a = BibtexDlg->currentItem();
		}
		else
			return;
	}

  	QString currentDir = getName();
	QFileInfo fica(currentDir);
	QString path = fica.dirPath();

    QStringList::ConstIterator it = filesbib->begin();
	finame_a = *it;
  	QString finame = path+"/"+finame_a+".bib";
	QFileInfo fic(finame);

	if( (finame= prepareForViewing("ViewBibtex","bib", finame_a)) == QString::null) return;

	QStringList command; command << bibtexeditor_command;
	CommandProcess *proc=execCommand(command,fic,false);
  	connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));
	if ( ! proc->start(KProcess::NotifyOnExit, KProcess::Stdout) )
	{
    	KMessageBox::error( this,i18n("Could not start %1. Make sure this package is installed on your system.").arg(bibtexeditor_command));
	}
  	else
	{
		OutputWidget->clear();
		logpresent=false;
		LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
	}
}

void Kile::execUserTool(int i)
{
	Kate::View *view = currentView();
	QString finame;
	QString commandline=m_listUserTools[i].tag;
	QFileInfo fi;

	bool documentpresent=true;

	if (m_singlemode) {finame=getName();}
	else {finame=m_masterName;}
	if ((m_singlemode && !view) ||getShortName()==i18n("Untitled") || getShortName()=="")
	{
		documentpresent=false;
	}

	QStringList command;
	if (documentpresent)
	{
		fi.setFile(finame);
		view->save();
	}
	else
	{
		if (commandline.contains("%S"))
		{
			if (KMessageBox::warningContinueCancel(this,i18n("Please open or create a document before you execute this tool."))
				== KMessageBox::Cancel)
			{
				LogWidget->insertLine(i18n("Process cancelled by user."));
				return;
			}

		}
	}

	command << commandline;
	CommandProcess* proc = execCommand(command,fi,true, documentpresent);
	connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));

	if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
	{
		KMessageBox::error( this,i18n("Could not start the command."));
	}
	else
	{
		OutputWidget->clear();
		logpresent=false;
		LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
	}

	newStatus();
}

////////////////// STRUCTURE ///////////////////
void Kile::ShowStructure()
{
	showVertPage(1);
}

void Kile::RefreshStructure()
{
	showVertPage(1);
	UpdateStructure(true);
}


void Kile::UpdateStructure(bool parse /* = false */)
{
	kdDebug() << "==Kile::UpdateStructure==========================" << endl;

	KileDocumentInfo *docinfo = getInfo();

	kdDebug() << "\ttaking item" <<endl;
	outstruct->takeItem(outstruct->firstChild());

	if (docinfo)
	{
		QListViewItem *item = (QListViewItem*)docinfo->structViewItem();
		if ((item == 0) || parse) docinfo->updateStruct(m_defaultLevel);
		outstruct->insertItem(item);
	}

	Kate::View *view = currentView();
	if (view) {view->setFocus();}
}

void Kile::ClickedOnStructure(QListViewItem * itm)
{
	kdDebug() << "==Kile::ClickedOnStructure==========================" << endl;
	KileListViewItem *item = (KileListViewItem *)itm;
	//return if user didn't click on an item
	if (! item)
	{
		kdDebug() << "\t(empty)" << endl;
		return;
	}

	kdDebug() << "\t" << item->title() << endl;

	if (! (item->type() & (KileStruct::None | KileStruct::Input)))
	{
		Kate::View *view = currentView();
		view->setCursorPositionReal(item->line()-1, item->column());
		view->setFocus();
	}
}

void Kile::DoubleClickedOnStructure(QListViewItem * itm)
{
	kdDebug() << "==Kile::DoubleClickedOnStructure==========================" << endl;
	KileListViewItem *item = (KileListViewItem*)(itm);
	if (! item) return;
	if (! (item->type() & KileStruct::Input)) return;

	kdDebug() << "\t " << itm->text(0) << endl;

	Kate::View *view = currentView();
	if ( ! view ) return;

	QString fn = getCompileName();
	QString fname = item->title();
	if (fname.right(4)==".tex")
		fname =QFileInfo(fn).dirPath()+"/" + fname;
	else
		fname=QFileInfo(fn).dirPath()+"/" + fname + ".tex";

	QFileInfo fi(fname);
	kdDebug() << "\ttrying : " << fname << endl;
	if (fi.isReadable())
	{
		fileOpen(KURL::fromPathOrURL(fname));
	}
	else
		KMessageBox::error(this, i18n("Cannot find the included file. The file does not exists, is not readable or Kile is unable to determine the correct path to this file. The filename leading to this error was: %1").arg(fname), i18n("Cannot find file!"));
}

//////////////// MESSAGES - LOG FILE///////////////////////
void Kile::ViewLog()
{
	Outputview->showPage(LogWidget);
	logpresent=false;
	LatexError();

	if (tempLog != QString::null)
	{
		LogWidget->setText(tempLog);
		LogWidget->highlight();
		LogWidget->scrollToBottom();
		//newStatus();
		logpresent=true;
	}
	else
	{
		LogWidget->insertLine(i18n("Cannot open log file! Did you run LaTeX?"));
	}
	tempLog=QString::null;
}

void Kile::ClickedOnOutput(int parag, int /*index*/)
{

if ( !currentView() ) return;

 int Start, End;
 bool ok;
 QString s;
 QString line="";
 s = LogWidget->text(parag);
 //check for ! first
 //the line number where the error occurred is below the !
 if (s.find("!",0) == 0)
 {
	 int i=0;
	 //error found jump to the following lines (somewhere we hope to find the line-number)
	 //assumptions: line number occurs within 10 lines of the !
	 do {
		 s = LogWidget->text(++parag);  i++;
		 if ( (s.at(0) == '!') ||
			 (s.find("LaTeX Warning",0) == 0 ) ||
			 (parag >= LogWidget->paragraphs()) ||
			 (i>10) ) return;
	 } while (  s.find(QRegExp("l.[0-9]"),0) < 0 ) ;
 }

 //// l. ///

 Start=End=0;
 Start=s.find(QRegExp("l.[0-9]"), End);
 if (Start!=-1)
  {
  Start=Start+2;
  s=s.mid(Start,s.length());
  End=s.find(QRegExp("[ a-zA-Z.\\-]"),0);
  if (End!=-1)
    line=s.mid(0,End);
  else
    line=s.mid(0,s.length());
  };
 //// line ///
 s = LogWidget->text(parag);
 Start=End=0;
 Start=s.find(QRegExp("line [0-9]"), End);
 if (Start!=-1)
  {
  Start=Start+5;
  s=s.mid(Start,s.length());
  End=s.find(QRegExp("[ a-zA-Z.\\-]"),0);
  if (End!=-1)
    line=s.mid(0,End);
  else
    line=s.mid(0,s.length());
  };
 //// lines ///
 s = LogWidget->text(parag);
 Start=End=0;
 Start=s.find(QRegExp("lines [0-9]"), End);
 if (Start!=-1)
  {
  Start=Start+6;
  s=s.mid(Start,s.length());
  End=s.find(QRegExp("[ a-zA-Z.\\-]"),0);
  if (End!=-1)
    line=s.mid(0,End);
  else
    line=s.mid(0,s.length());
  };


uint l=line.toInt(&ok,10)-1;

if (ok && l<=currentView()->getDoc()->numLines())
 {
 currentView()->setFocus();
 currentView()->setCursorPosition(l, 0);
 //newStatus();
 }
}
////////////////////////// ERRORS /////////////////////////////
void Kile::LatexError(bool warnings)
{
	errorlist->clear();
	warnlist->clear();
	m_nErrors=m_nWarnings=0;
	m_bNewErrorlist=m_bNewWarninglist=true;
	int tagStart,i=0;
	QString s,num;
	tempLog=QString::null;

	QString finame;
	if (  (finame = prepareForViewing("ViewLog","log") ) == QString::null ) return;
	QFileInfo fic(finame);
	QFile f(finame);
	if ( f.open(IO_ReadOnly) )
	{
		QTextStream t( &f );
		while ( !t.eof() )
		{
			s=t.readLine()+"\n";
			tempLog += s;
   			//// ! ////
			tagStart=s.find("!");
			if (tagStart==0)
			{
				num = QString::number(i,10);
				m_nErrors++;
				errorlist->append(num.ascii());
			}
			//// latex warning ////
			if (warnings)
			{
				tagStart=s.find("LaTeX Warning");
				if (tagStart!=-1)
				{
					num = QString::number(i,10);
					m_nWarnings++;
					warnlist->append(num.ascii());
				}
			}
			i++;
		}
		f.close();
	}
}

void Kile::jumpToProblem(QStrList *list, bool &newlist, bool forward)
{
	QString line="";
	bool ok;
	if (!logpresent) {ViewLog();}

	if (logpresent && !list->isEmpty())
	{
		Outputview->showPage(LogWidget);
		uint id=list->findRef(list->current());
		if (newlist)
		{
			line=list->at(0);
		}
		else
		{
			if (forward)
			{
				if (id < (list->count()-1))
					line=list->at(id+1);
				else
					line=list->at(id);
			}
			else
			{
				if (id>0)
					line=list->at(id-1);
				else
					line=list->at(0);
			}
		}
		int l=line.toInt(&ok,10);
		if (ok && l<=LogWidget->paragraphs())
		{
			//LogWidget->setCursorPosition(0 , 0 );
			LogWidget->setCursorPosition(l+3 , 0);
			LogWidget->setSelection(l,0,l,LogWidget->paragraphLength(l));
		}
	}

	if (logpresent && list->isEmpty())
	{
		LogWidget->insertLine(i18n("No LaTeX errors detected!"));
	}

	newlist = false;
}

void Kile::NextError()
{
	jumpToProblem(errorlist, m_bNewErrorlist, true);
}

void Kile::PreviousError()
{
	jumpToProblem(errorlist, m_bNewErrorlist, false);
}

void Kile::NextWarning()
{
	jumpToProblem(warnlist, m_bNewWarninglist, true);
}

void Kile::PreviousWarning()
{
	jumpToProblem(warnlist, m_bNewWarninglist, false);
}

/////////////////////// LATEX TAGS ///////////////////
void Kile::insertTag(const KileAction::TagData& data)
{
	Kate::View *view = currentView();
	int para,index, para_end, para_begin, index_begin;

	if ( !view ) return;

	view->setFocus();

	//whether or not to wrap tag around selection
	bool wrap = (data.tagEnd != QString::null && view->getDoc()->hasSelection());

	//%C before or after the selection
	bool before = data.tagBegin.contains("%C");
	bool after = data.tagEnd.contains("%C");

	//save current cursor position
	para=para_begin=view->cursorLine();
	index=index_begin=view->cursorColumnReal();

	//if there is a selection act as if cursor is at the beginning of selection
	if (wrap)
	{
		index = view->getDoc()->selStartCol();
		para  = view->getDoc()->selStartLine();
		para_end = view->getDoc()->selEndLine();
	}

	QString ins = data.tagBegin;

	//cut the selected text
	if (wrap)
	{
		ins += view->getDoc()->selection();
		view->getDoc()->removeSelectedText();
	}

	ins += data.tagEnd;

	//do some replacements
	QFileInfo fi( view->getDoc()->url().path());
	ins.replace("%S", fi.baseName(true));

	//insert first part of tag at cursor position
	//kdDebug() << QString("insertTag: inserting %1 at (%2,%3)").arg(ins).arg(para).arg(index) << endl;
	view->getDoc()->insertText(para,index,ins);

	//move cursor to the new position
	if ( before || after )
	{
		kdDebug() << "before || after" << endl;
		int n = data.tagBegin.contains("\n")+ data.tagEnd.contains("\n");
		if (wrap) n += para_end > para ? para_end-para : para-para_end;
		for (int line = para_begin; line <= para_begin+n; line++)
		{
			if (view->getDoc()->textLine(line).contains("%C"))
			{
				int i=view->getDoc()->textLine(line).find("%C");
				view->setCursorPositionReal(line,i);
				view->getDoc()->removeText(line,i,line,i+2);
				break;
			}
			view->setCursorPositionReal(line,index);
		}
	}
	else
	{
		int py = para_begin, px = index_begin;
		if (wrap) //act as if cursor was at beginning of selected text (which is the point where the tagBegin is inserted)
		{
			py = para;
			px = index;
		}
		kdDebug() << "py = " << py << " px = " << px << endl;
		view->setCursorPositionReal(py+data.dy,px+data.dx);
	}

	view->getDoc()->clearSelection();

	LogWidget->clear();
	Outputview->showPage(LogWidget);
	logpresent=false;

	LogWidget->append(data.description);
}

void Kile::insertGraphic(const KileAction::TagData& data)
{
	insertTag(data);

	QFileInfo fi(data.tagBegin.mid(data.tagBegin.find('{')+1));
	kdDebug() << "insertGraphic : filename " << fi.fileName() << endl;

	if (fi.extension(false) =="eps")
	{
		LogWidget->insertLine("*************  ABOUT THIS IMAGE  *************");
		LogWidget->insertLine(DetectEpsSize(fi.absFilePath()));
	}
}

void Kile::insertTag(const QString& tagB, const QString& tagE, int dx, int dy)
{
	insertTag(KileAction::TagData(QString::null,tagB,tagE,dx,dy));
}

void Kile::QuickDocument()
{
QString opt="";
int li=3;
  if ( !currentView() )	return;
  QString tag=QString("\\documentclass[");
  startDlg = new quickdocumentdialog(this,"Quick Start",i18n("Quick Start"));
  startDlg->otherClassList=userClassList;
  startDlg->otherPaperList=userPaperList;
  startDlg->otherEncodingList=userEncodingList;
  startDlg->otherOptionsList=userOptionsList;
  startDlg->Init();
  startDlg->combo1->setCurrentText(document_class);
  startDlg->combo2->setCurrentText(typeface_size);
  startDlg->combo3->setCurrentText(paper_size);
  startDlg->combo4->setCurrentText(document_encoding);
  startDlg->checkbox1->setChecked(ams_packages);
  startDlg->checkbox2->setChecked(makeidx_package);
  startDlg->LineEdit1->setText(author);
  if ( startDlg->exec() )
  {
  tag+=startDlg->combo2->currentText()+QString(",");
  tag+=startDlg->combo3->currentText();
  for ( uint j=0;j<=startDlg->availableBox->count();j++)
      {
      if (startDlg->availableBox->isSelected(j)) opt+=QString(",")+startDlg->availableBox->item(j)->text();
      }
  tag+=opt+QString("]{");
  tag+=startDlg->combo1->currentText()+QString("}");
  tag+=QString("\n");
  if (startDlg->combo4->currentText()!="NONE") tag+=QString("\\usepackage[")+startDlg->combo4->currentText()+QString("]{inputenc}");
  tag+=QString("\n");
  if (startDlg->checkbox1->isChecked())
     {
     tag+=QString("\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n");
     li=li+3;
     }
  if (startDlg->checkbox2->isChecked())
     {
     tag+=QString("\\usepackage{makeidx}\n");
     li=li+1;
     }
  if (startDlg->LineEdit1->text()!="")
     {
     tag+="\\author{"+startDlg->LineEdit1->text()+"}\n";
     li=li+1;
     }
  if (startDlg->LineEdit2->text()!="")
     {
     tag+="\\title{"+startDlg->LineEdit2->text()+"}\n";
     li=li+1;
     }
  tag+= "\\begin{document}\n";
  QString tagE = "\n\\end{document}";
  insertTag(tag,tagE,0,li);
  document_class=startDlg->combo1->currentText();
  typeface_size=startDlg->combo2->currentText();
  paper_size=startDlg->combo3->currentText();
  document_encoding=startDlg->combo4->currentText();
  ams_packages=startDlg->checkbox1->isChecked();
  makeidx_package=startDlg->checkbox2->isChecked();
  author=startDlg->LineEdit1->text();
  userClassList=startDlg->otherClassList;
  userPaperList=startDlg->otherPaperList;
  userEncodingList=startDlg->otherEncodingList;
  userOptionsList=startDlg->otherOptionsList;
  }
  delete( startDlg);
}


void Kile::QuickTabular()
{
  if ( !currentView() )	return;
  QString al="";
  QString vs="";
  QString hs="";
	quickDlg = new tabdialog(this,"Tabular",i18n("Tabular"));
  if ( quickDlg->exec() )
  {
    int	y = quickDlg->spinBoxRows->value();
    int	x = quickDlg->spinBoxCollums->value();
    if  ((quickDlg->combo2->currentItem ())==0) vs=QString("|");
    if  ((quickDlg->combo2->currentItem ())==1) vs=QString("||");
    if  ((quickDlg->combo2->currentItem ())==2) vs=QString("");
    if  ((quickDlg->combo2->currentItem ())==3) vs=QString("@{}");
  	QString tag = QString("\\begin{tabular}{")+vs;
    if  ((quickDlg->combo1->currentItem ())==0) al=QString("c")+vs;
    if  ((quickDlg->combo1->currentItem ())==1) al=QString("l")+vs;
    if  ((quickDlg->combo1->currentItem ())==2) al=QString("r")+vs;
    if  ((quickDlg->combo1->currentItem ())==3) al=QString("p{}")+vs;
    if (quickDlg->checkbox1->isChecked()) hs=QString("\\hline ");
 		for ( int j=0;j<x;j++) {tag +=al;}
    tag +=QString("}\n");
 	  for ( int i=0;i<y;i++) {
 		  tag +=hs;
 		  for ( int j=0;j<x-1;j++)
 			  tag +=quickDlg->Table1->text(i,j)+ QString(" & ");
 		  tag +=quickDlg->Table1->text(i,x-1)+ QString(" \\\\ \n");
 	  }
 	  if (quickDlg->checkbox1->isChecked()) tag +=hs+QString("\n\\end{tabular} ");
    else tag +=QString("\\end{tabular} ");
  insertTag(tag,QString::null,0,0);
  }

  delete( quickDlg);

}

void Kile::QuickTabbing()
{
  if ( !currentView() )	return;
  tabDlg = new tabbingdialog(this,"Tabbing",i18n("Tabbing"));
  if ( tabDlg->exec() )
 {
  int	x = tabDlg->spinBoxCollums->value();
  int	y = tabDlg->spinBoxRows->value();
  QString s=tabDlg->LineEdit1->text();
  QString tag = QString("\\begin{tabbing}\n");
  for ( int j=1;j<x;j++) {tag +="\\hspace{"+s+"}\\=";}
  tag+="\\kill\n";
 	for ( int i=0;i<y-1;i++)
   {
   for ( int j=1;j<x;j++) {tag +=" \\> ";}
   tag+="\\\\ \n";
   }
   for ( int j=1;j<x;j++) {tag +=" \\> ";}

  insertTag(tag,"\n\\end{tabbing} ",0,2);
 }
  delete( tabDlg);
}

void Kile::QuickArray()
{
  if ( !currentView() )	return;
  QString al;
	arrayDlg = new arraydialog(this,"Array",i18n("Array"));
  if ( arrayDlg->exec() ) {
  	int y = arrayDlg->spinBoxRows->value();
  	int x = arrayDlg->spinBoxCollums->value();
    QString env=arrayDlg->combo2->currentText();
  	QString tag = QString("\\begin{")+env+"}";
    if (env=="array")
    {
      tag+="{";
      if  ((arrayDlg->combo->currentItem ())==0) al=QString("c");
      if  ((arrayDlg->combo->currentItem ())==1) al=QString("l");
      if  ((arrayDlg->combo->currentItem ())==2) al=QString("r");
   		for ( int j=0;j<x;j++) {tag +=al;}
      tag+="}";
    }
    tag +=QString("\n");
 	  for ( int i=0;i<y-1;i++) {
  		  for ( int j=0;j<x-1;j++)
 			  tag +=arrayDlg->Table1->text(i,j)+ QString(" & ");
 		  tag +=arrayDlg->Table1->text(i,x-1)+ QString(" \\\\ \n");
 	  }
  		  for ( int j=0;j<x-1;j++)
 			  tag +=arrayDlg->Table1->text(y-1,j)+ QString(" & ");
 	  tag +=arrayDlg->Table1->text(y-1,x-1)+ QString("\n\\end{")+env+"} ";
  insertTag(tag,QString::null,0,0);
  }
  delete( arrayDlg);

}

void Kile::QuickLetter()
{
  if ( !currentView() )	return;
  QString tag=QString("\\documentclass[");
	ltDlg = new letterdialog(this,"Letter",i18n("Letter"));
  if ( ltDlg->exec() )
  {
  tag+=ltDlg->combo2->currentText()+QString(",");
  tag+=ltDlg->combo3->currentText()+QString("]{letter}");
  tag+=QString("\n");
  if (ltDlg->combo4->currentText()!="NONE") tag+=QString("\\usepackage[")+ltDlg->combo4->currentText()+QString("]{inputenc}");
  tag+=QString("\n");
  if (ltDlg->checkbox1->isChecked()) tag+=QString("\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n");
  tag+="\\address{your name and address} \n";
  tag+="\\signature{your signature} \n";
  tag+="\\begin{document} \n";
  tag+="\\begin{letter}{name and address of the recipient} \n";
  tag+="\\opening{saying hello} \n \n";
  tag+="write your letter here \n \n";
  tag+="\\closing{saying goodbye} \n";
  tag+="%\\cc{Cclist} \n";
  tag+="%\\ps{adding a postscript} \n";
  tag+="%\\encl{list of enclosed material} \n";
  tag+="\\end{letter} \n";
  tag+="\\end{document}";
  if (ltDlg->checkbox1->isChecked()) {insertTag(tag,QString::null,9,5);}
  else {insertTag(tag,QString::null,9,2);}
  }
  delete( ltDlg);
}

//////////////////////////// MATHS TAGS/////////////////////////////////////
void Kile::insertSymbol(QIconViewItem *item)
{
	QString code_symbol= item->key();
	insertTag(code_symbol,QString::null,code_symbol.length(),0);
}

void Kile::InsertMetaPost(QListBoxItem *)
{
QString mpcode=mpview->currentText();
if (mpcode!="----------") insertTag(mpcode,QString::null,mpcode.length(),0);
}

////////////////////////// BIBLIOGRAPHY //////////////////////////

//////////////// USER //////////////////
void Kile::insertUserTag(int i)
{
	if (m_listUserTags[i].tag.left(1)=="%")
	{
		QString t=m_listUserTags[i].tag;
		t=t.remove(0,1);
		insertTag("\\begin{"+t+"}\n","\n\\end{"+t+"}\n",0,1);
	}
	else
	{
		QStringList parts = QStringList::split("%M",m_listUserTags[i].tag);
		insertTag(parts[0],parts[1],0,0);
	}
}

//////////////// HELP /////////////////
void Kile::LatexHelp()
{
QFileInfo fic(locate("html","en/kile/latexhelp.html"));
kdDebug() << "latexhelp: " << fic.absFilePath() << endl;
    if (fic.exists() && fic.isReadable() )
      {
      ResetPart();
      htmlpart = new docpart(topWidgetStack,"help");
      connect(htmlpart,    SIGNAL(updateStatus(bool, bool)), SLOT(updateNavAction( bool, bool)));
      htmlpresent=true;
      topWidgetStack->addWidget(htmlpart->widget() , 1 );
      topWidgetStack->raiseWidget(1);
      partManager->addPart(htmlpart, true);
      partManager->setActivePart( htmlpart);
      htmlpart->openURL(locate("html","en/kile/latexhelp.html"));
      htmlpart->addToHistory(locate("html","en/kile/latexhelp.html"));
      }
    else { KMessageBox::error( this,i18n("File not found"));}
}

void Kile::invokeHelp()
{
	kapp->invokeHelp();
}

///////////////////// USER ///////////////
void Kile::EditUserMenu()
{
	usermenudialog *umDlg = new usermenudialog(m_listUserTags,this,"Edit User Tags", i18n("Edit User Tags"));

	if ( umDlg->exec() )
	{
		//remove all actions
		KAction *menuItem;
		uint len = m_listUserTagsActions.count();
		for (uint j=0; j< len; j++)
		{
			menuItem = m_listUserTagsActions.getLast();
			m_mapUserTagSignals->removeMappings(menuItem);
			m_menuUserTags->remove(menuItem);
			m_listUserTagsActions.removeLast();
			delete menuItem;
		}
		m_menuUserTags->remove(m_actionEditTag);

		m_listUserTags = umDlg->result();
		setupUserTagActions();
	}

	delete umDlg;
}

void Kile::EditUserTool()
{
	usermenudialog *umDlg = new usermenudialog(m_listUserTools,this,"Edit User Tools", i18n("Edit User Tools"));

	if ( umDlg->exec() )
	{
		//remove all actions
		KAction *menuItem;
		uint len = m_listUserToolsActions.count();
		for (uint j=0; j< len; j++)
		{
			menuItem = m_listUserToolsActions.getLast();
			m_mapUserToolsSignals->removeMappings(menuItem);
			m_menuUserTools->remove(menuItem);
			m_listUserToolsActions.removeLast();
			delete menuItem;
		}
		m_menuUserTools->remove(m_actionEditTool);

		m_listUserTools = umDlg->result();
		setupUserToolActions();
	}

	delete umDlg;
}

/////////////// GRAF ////////////////////
void Kile::RunXfig()
{
KShellProcess* proc = new KShellProcess("/bin/sh");
proc->clearArguments();
(*proc) << "xfig" ;
connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start Xfig."));}
else
  {
  LogWidget->clear();
  Outputview->showPage(LogWidget);
  logpresent=false;
  LogWidget->insertLine(i18n("Launched: %1").arg("xfig"));
  }
}

void Kile::RunGfe()
{
  LogWidget->clear();
  Outputview->showPage(LogWidget);
  logpresent=false;
  //newStatus();
  if (!gfe_widget) gfe_widget=new Qplotmaker(0,"Gnuplot Front End");
  gfe_widget->setIcon(kapp->miniIcon());
  gfe_widget->raise();
  gfe_widget->show();
}
/////////////// CONFIG ////////////////////
void Kile::ReadSettings()
{
	//test for old kilerc
	config->setGroup("VersionInfo");
	int version = config->readNumEntry("RCVersion",0);
	bool old=false;

	//if the kilerc file is old some of the configuration
	//date must be set by kile, even if the keys are present
	//in the kilerc file
	if (version<KILERC_VERSION) old=true;

	m_singlemode=true;
	QRect screen = QApplication::desktop()->screenGeometry();
	config->setGroup( "Geometries" );
	int w= config->readNumEntry( "MainwindowWidth",screen.width()-100);
	int h= config->readNumEntry( "MainwindowHeight",screen.height()-100) ;
	resize(w,h);
	split1_left=config->readNumEntry("Splitter1_left",100);
	split1_right=config->readNumEntry("Splitter1_right",350);
	split2_top=config->readNumEntry("Splitter2_top",350);
	split2_bottom=config->readNumEntry("Splitter2_bottom",100);

	//delete old editor key
	if (config->hasGroup("Editor") )
	{
		config->deleteGroup("Editor");
	}

	config->setGroup( "Show" );
	showoutputview=config->readBoolEntry("Outputview",true);
	showstructview=config->readBoolEntry( "Structureview",true);
	showmaintoolbar=config->readBoolEntry("ShowMainToolbar",true);
	showtoolstoolbar=config->readBoolEntry("ShowToolsToolbar",true);
	showedittoolbar=config->readBoolEntry("ShowEditToolbar",true);
	showmathtoolbar=config->readBoolEntry("ShowMathToolbar",true);
	m_menuaccels=config->readBoolEntry("MenuAccels", true);

	config->setGroup( "Tools" );
	if (old)
	{
		latex_command="latex -interaction=nonstopmode '%S.tex'";
		viewdvi_command="Embedded Viewer";
		dvips_command="dvips -o '%S.ps' '%S.dvi'";
		viewps_command="Embedded Viewer";
		ps2pdf_command="ps2pdf '%S.ps' '%S.pdf'";
		makeindex_command="makeindex '%S.idx'";
		bibtex_command="bibtex '%S'";
		pdflatex_command="pdflatex -interaction=nonstopmode '%S.tex'";
		viewpdf_command="Embedded Viewer";
		dvipdf_command="dvipdfm '%S.dvi'";
		l2h_options="";
	}
	//new configuration scheme is read in readConfig()

	config->setGroup( "User" );
	userItem tempItem;
	int len = config->readNumEntry("nUserTags",0);
	for (int i = 0; i < len; i++)
	{
		tempItem.name=config->readEntry("userTagName"+QString::number(i),i18n("no name"));
		tempItem.tag =config->readEntry("userTag"+QString::number(i),"");
		m_listUserTags.append(tempItem);
	}

	len= config->readNumEntry("nUserTools",0);
	for (int i=0; i< len; i++)
	{
		tempItem.name=config->readEntry("userToolName"+QString::number(i),i18n("no name"));
		tempItem.tag =config->readEntry("userTool"+QString::number(i),"");
		m_listUserTools.append(tempItem);
	}

	config->setGroup( "Structure" );
	struct_level1=config->readEntry("Structure Level 1","part");
	struct_level2=config->readEntry("Structure Level 2","chapter");
	struct_level3=config->readEntry("Structure Level 3","section");
	struct_level4=config->readEntry("Structure Level 4","subsection");
	struct_level5=config->readEntry("Structure Level 5","subsubsection");

	config->setGroup( "Quick" );
	document_class=config->readEntry("Class","article");
	typeface_size=config->readEntry("Typeface","10pt");
	paper_size=config->readEntry("Papersize","a4paper");
	document_encoding=config->readEntry("Encoding","latin1");
	ams_packages=config->readBoolEntry( "AMS",true);
	makeidx_package=config->readBoolEntry( "MakeIndex",false);
	author=config->readEntry("Author","");

	readConfig();
}

void Kile::ReadRecentFileSettings()
{
	config->setGroup("FilesOpenOnStart");
	int n = config->readNumEntry("NoDOOS", 0);
	for (int i=0; i < n; i++)
		m_listDocsOpenOnStart.append(config->readPathEntry("DocsOpenOnStart"+QString::number(i), ""));

	n = config->readNumEntry("NoPOOS", 0);
	for (int i=0; i < n; i++)
		m_listProjectsOpenOnStart.append(config->readPathEntry("ProjectsOpenOnStart"+QString::number(i), ""));

	config->setGroup( "Files" );

	lastDocument=config->readPathEntry("Last Document","");
	input_encoding=config->readEntry("Input Encoding", QString::fromLatin1(QTextCodec::codecForLocale()->name()));

	// Load recent files from "Recent Files" group
	// using the KDE standard action for recent files
	fileOpenRecentAction->loadEntries(config,"Recent Files");

	// Now check if user is using an old rc file that has "Recent Files" under
	// the "Files" group
	if(config->hasKey("Recent Files"))
	{
		// If so, then read the entry in, add it to fileOpenRecentAction
		QStringList recentFilesList = config->readListEntry("Recent Files", ':');
		QStringList::ConstIterator it = recentFilesList.begin();
		for ( ; it != recentFilesList.end(); ++it )
		{
		fileOpenRecentAction->addURL(KURL::fromPathOrURL(*it));
		}
		// Now delete this recent files entry as we are now using a separate
		// group for recent files
		config->deleteEntry("Recent Files");
	}

	m_actRecentProjects->loadEntries(config,"Projects");
}

//reads options that can be set in the configuration dialog
void Kile::readConfig()
{
	config->setGroup( "Structure" );
	m_defaultLevel = config->readNumEntry("DefaultLevel", 1);

	config->setGroup( "Files" );
	m_bRestore=config->readBoolEntry("Restore",true);
	autosave=config->readBoolEntry("Autosave",true);
	autosaveinterval=config->readLongNumEntry("AutosaveInterval",600000);
	enableAutosave(autosave);
	setAutosaveInterval(autosaveinterval);

	config->setGroup( "User" );
	templAuthor=config->readEntry("Author","");
	templDocClassOpt=config->readEntry("DocumentClassOptions","a4paper,10pt");
	templEncoding=config->readEntry("Template Encoding","");

	config->setGroup("Tools");
	m_bCheckForRoot = config->readBoolEntry("CheckForRoot",true);
	quickmode=config->readNumEntry( "Quick Mode",1);
	latex_command=config->readEntry("Latex","latex -interaction=nonstopmode '%S.tex'");
	viewdvi_command=config->readEntry("Dvi","Embedded Viewer");
	dvips_command=config->readEntry("Dvips","dvips -o '%S.ps' '%S.dvi'");
	viewps_command=config->readEntry("Ps","Embedded Viewer");
	ps2pdf_command=config->readEntry("Ps2pdf","ps2pdf '%S.ps' '%S.pdf'");
	makeindex_command=config->readEntry("Makeindex","makeindex '%S.idx'");
	bibtex_command=config->readEntry("Bibtex","bibtex '%S'");
	pdflatex_command=config->readEntry("Pdflatex","pdflatex -interaction=nonstopmode '%S.tex'");
	viewpdf_command=config->readEntry("Pdf","Embedded Viewer");
	dvipdf_command=config->readEntry("Dvipdf","dvipdfm '%S.dvi'");
	l2h_options=config->readEntry("L2h Options","");
	bibtexeditor_command=config->readEntry("Bibtexeditor","gbib '%S.bib'");
	m_runlyxserver = config->readBoolEntry("RunLyxServer", true);
	userClassList=config->readListEntry("User Class", ':');
	userPaperList=config->readListEntry("User Paper", ':');
	userEncodingList=config->readListEntry("User Encoding", ':');
	userOptionsList=config->readListEntry("User Options", ':');
}

void Kile::SaveSettings()
{
ShowEditorWidget();
QValueList<int> sizes;
QValueList<int>::Iterator it;

config->setGroup("VersionInfo");
config->writeEntry("RCVersion",KILERC_VERSION);

config->setGroup( "Geometries" );
config->writeEntry("MainwindowWidth", width() );
config->writeEntry("MainwindowHeight", height() );
sizes=splitter1->sizes();
it = sizes.begin();
split1_left=*it;
++it;
split1_right=*it;
sizes.clear();
sizes=splitter2->sizes();
it = sizes.begin();
split2_top=*it;
++it;
split2_bottom=*it;
config->writeEntry("Splitter1_left", split1_left );
config->writeEntry("Splitter1_right", split1_right );
config->writeEntry("Splitter2_top", split2_top );
config->writeEntry("Splitter2_bottom", split2_bottom );

config->setGroup( "Show" );
config->writeEntry("Outputview",showoutputview);
config->writeEntry( "Structureview",showstructview);
config->writeEntry("ShowMainToolbar",showmaintoolbar);
config->writeEntry("ShowToolsToolbar",showtoolstoolbar);
config->writeEntry("ShowEditToolbar",showedittoolbar);
config->writeEntry("ShowMathToolbar",showmathtoolbar);
config->writeEntry("MenuAccels", m_menuaccels);

config->writeEntry("L2h Options",l2h_options);
config->writeEntry("User Class",userClassList, ':');
config->writeEntry("User Paper",userPaperList, ':');
config->writeEntry("User Encoding",userEncodingList, ':');
config->writeEntry("User Options",userOptionsList, ':');


	config->setGroup( "Files" );
	if (m_viewList.last()) lastDocument = m_viewList.last()->getDoc()->url().path();
	config->writeEntry("Last Document",lastDocument);
	input_encoding=KileFS->comboEncoding->lineEdit()->text();
	config->writeEntry("Input Encoding", input_encoding);

	// Store recent files
	fileOpenRecentAction->saveEntries(config,"Recent Files");
	m_actRecentProjects->saveEntries(config,"Projects");

	config->deleteGroup("FilesOpenOnStart");
	kdDebug() << "deleting FilesOpenOnStart" << endl;
	if (m_bRestore)
	{
		kdDebug() << "saving Restore info" << endl;
		config->setGroup("FilesOpenOnStart");
		config->writeEntry("NoDOOS", m_listDocsOpenOnStart.count());
		for (uint i=0; i < m_listDocsOpenOnStart.count(); i++)
			config->writePathEntry("DocsOpenOnStart"+QString::number(i), m_listDocsOpenOnStart[i]);

		config->writeEntry("NoPOOS", m_listProjectsOpenOnStart.count());
		for (uint i=0; i < m_listProjectsOpenOnStart.count(); i++)
			config->writePathEntry("ProjectsOpenOnStart"+QString::number(i), m_listProjectsOpenOnStart[i]);
	}

config->setGroup( "User" );

userItem tempItem;
config->writeEntry("nUserTags",static_cast<int>(m_listUserTags.size()));
for (uint i=0; i<m_listUserTags.size(); i++)
{
	tempItem = m_listUserTags[i];
	config->writeEntry("userTagName"+QString::number(i),tempItem.name);
	config->writeEntry("userTag"+QString::number(i),tempItem.tag);
}

config->writeEntry("nUserTools",static_cast<int>(m_listUserTools.size()));
for (uint i=0; i<m_listUserTools.size(); i++)
{
	tempItem = m_listUserTools[i];
	config->writeEntry("userToolName"+QString::number(i),tempItem.name);
	config->writeEntry("userTool"+QString::number(i),tempItem.tag);
}

config->setGroup( "Structure" );
config->writeEntry("Structure Level 1",struct_level1);
config->writeEntry("Structure Level 2",struct_level2);
config->writeEntry("Structure Level 3",struct_level3);
config->writeEntry("Structure Level 4",struct_level4);
config->writeEntry("Structure Level 5",struct_level5);

config->setGroup("Quick");
config->writeEntry( "Class",document_class);
config->writeEntry( "Typeface",typeface_size);
config->writeEntry( "Papersize",paper_size);
config->writeEntry( "Encoding",document_encoding);
config->writeEntry( "AMS",ams_packages);
config->writeEntry( "MakeIndex",makeidx_package);
config->writeEntry( "Author",author);

//config->setGroup( "Editor Ext" );
//config->writeEntry( "Complete Environment", m_bCompleteEnvironment );

actionCollection()->writeShortcutSettings();
saveMainWindowSettings(config, "KileMainWindow" );
config->sync();
}

/////////////////  OPTIONS ////////////////////
void Kile::ToggleMode()
{
	if (!m_singlemode)
	{
		ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
		ModeAction->setChecked(false);
		OutputWidget->clear();
		Outputview->showPage(OutputWidget);
		logpresent=false;
		m_singlemode=true;

		updateModeStatus();
		return;
	}
	if (m_singlemode && currentView())
	{
		m_masterName=getName();
		if (m_masterName==i18n("Untitled") || m_masterName=="")
		{
			ModeAction->setChecked(false);
			KMessageBox::error( this,i18n("Could not start the command."));
			return;
		}
		QString shortName = m_masterName;
		int pos;
		while ( (pos = (int)shortName.find('/')) != -1 )
		shortName.remove(0,pos+1);
		ModeAction->setText(i18n("Normal mode (current master document: %1)").arg(shortName));
		ModeAction->setChecked(true);
		m_singlemode=false;

		updateModeStatus();
		return;
	}
	ModeAction->setChecked(false);
}

void Kile::ToggleMenuShortcut(KMenuBar *bar, bool accelOn, const QString &accelText, const QString &noAccelText)
{
  QString from = (accelOn) ? noAccelText : accelText;
  QString to   = (accelOn) ? accelText   : noAccelText;

  for ( int i = 0; i < (int) bar->count(); i++ )
    if (bar->text( bar->idAt( i ) ) == from) {
      bar->changeItem( bar->idAt( i ), to );
      break;
    }
}

void Kile::ToggleKeyShortcut(KAction *action, bool addShiftModifier)
{
  KShortcut cut = action->shortcut();
  KKey key = cut.seq( 0 ).key( 0 );

  // Add SHIFT modifier to first key with only ALT modifier
  if (addShiftModifier && key.modFlags() == KKey::ALT) {
     KKey newKey( "SHIFT+" + key.toString() );
     KKeySequence newSeq( cut.seq( 0 ) );
     newSeq.setKey( 0, newKey );
     KShortcut newCut( cut );
     newCut.setSeq( 0, newSeq );
     action->setShortcut( newCut );
   }

   // Remove SHIFT modifier from first key with only SHIFT+ALT modifiers
   if (!addShiftModifier && key.modFlags() == KKey::SHIFT | KKey::ALT) {
     KKey newKey( key.toString().remove( key.modFlagLabel(KKey::SHIFT) + "+" ) );
     KKeySequence newSeq( cut.seq( 0 ) );
     newSeq.setKey( 0, newKey );
     KShortcut newCut( cut );
     newCut.setSeq( 0, newSeq );
     action->setShortcut( newCut );
   }
}

void Kile::ToggleAccels()
{
  KMenuBar *bar = menuBar();

  // Toggle KDE standard menu shortcuts or special Kile shortcuts
  m_menuaccels = MenuAccelsAction->isChecked();
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&File"),         i18n("File"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Edit"),         i18n("Edit"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Tools"),        i18n("Tools"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&LaTeX"),        i18n("LaTeX"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Math"),         i18n("Math"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Wizard"),       i18n("Wizard"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Bibliography"), i18n("Bibliography"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&User"),         i18n("User"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Graph"),        i18n("Graph"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&View"),         i18n("View"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Settings"),     i18n("Settings"));
  ToggleMenuShortcut(bar, m_menuaccels, i18n("&Help"),         i18n("Help"));

  ToggleKeyShortcut(altH_action, m_menuaccels);
  ToggleKeyShortcut(altI_action, m_menuaccels);
  ToggleKeyShortcut(altA_action, m_menuaccels);
  ToggleKeyShortcut(altB_action, m_menuaccels);
  ToggleKeyShortcut(altT_action, m_menuaccels);
  ToggleKeyShortcut(altC_action, m_menuaccels);
  ToggleKeyShortcut(altM_action, m_menuaccels);
  ToggleKeyShortcut(altE_action, m_menuaccels);
  ToggleKeyShortcut(altD_action, m_menuaccels);
  ToggleKeyShortcut(altU_action, m_menuaccels);
  ToggleKeyShortcut(altF_action, m_menuaccels);
  ToggleKeyShortcut(altQ_action, m_menuaccels);
  ToggleKeyShortcut(altS_action, m_menuaccels);
  ToggleKeyShortcut(altL_action, m_menuaccels);
  ToggleKeyShortcut(altR_action, m_menuaccels);
}

void Kile::ToggleOutputView()
{
ShowOutputView(true);
}

void Kile::ToggleStructView()
{
ShowStructView(true);
}

void Kile::ToggleShowMainToolbar() {
  showmaintoolbar = !showmaintoolbar;

  if (showmaintoolbar ) {
      toolBar("mainToolBar")->show();
  } else {
    toolBar("mainToolBar")->hide();
  }
}

void Kile::ToggleShowToolsToolbar() {
  showtoolstoolbar = !showtoolstoolbar;

  if (showtoolstoolbar ) {
      toolBar("ToolBar2")->show();
  } else {
    toolBar("ToolBar2")->hide();
  }
}

void Kile::ToggleShowEditToolbar() {
  showedittoolbar = !showedittoolbar;

  if (showedittoolbar ) {
      toolBar("ToolBar4")->show();
  } else {
    toolBar("ToolBar4")->hide();
  }
}

void Kile::ToggleShowMathToolbar() {
  showmathtoolbar = !showmathtoolbar;

  if (showmathtoolbar ) {
      toolBar("ToolBar5")->show();
  } else {
    toolBar("ToolBar5")->hide();
  }
}

void Kile::ToggleWatchFile()
{
watchfile=!watchfile;
if (watchfile) {WatchFileAction->setChecked(true);}
else {WatchFileAction->setChecked(false);}
}

void Kile::ShowOutputView(bool change)
{
if (change) showoutputview=!showoutputview;
if (showoutputview)
    {
    MessageAction->setChecked(true);
    Outputview->show();
    }
else
   {
   MessageAction->setChecked(false);
   Outputview->hide();
   }
}

void Kile::ShowStructView(bool change)
{
if (change) showstructview=!showstructview;
if (showstructview)
   {
   StructureAction->setChecked(true);
   Structview->show();
   }
else
   {
   StructureAction->setChecked(false);
   Structview->hide();
   }
}

void Kile::GeneralOptions()
{
	KileConfigDialog *dlg = new KileConfigDialog(config,this,"Configure Kile");

	if (dlg->exec())
	{
		dlg->ksc->writeGlobalSettings ();

		readConfig();
		emit(configChanged());

		//stop/restart LyX server if necessary
		if (m_runlyxserver && !m_lyxserver->isRunning())
			m_lyxserver->start();

		if (!m_runlyxserver && m_lyxserver->isRunning())
			m_lyxserver->stop();
	}

	delete dlg;
}
////////////// SPELL ///////////////
void Kile::spellcheck()
{
	if ( !currentView() ) return;

  	if ( kspell )
  	{
		kdDebug() << "kspell wasn't deleted before!" << endl;
		delete kspell;
	}

	#if KDE_VERSION >= KDE_MAKE_VERSION(3,1,90)
		kdDebug() << "KSPELL: using NEW kspell " << KDE_VERSION_STRING << endl;
		kspell = new KSpell(this, i18n("Spellcheck"), this,SLOT( spell_started(KSpell *)), 0, true, false, KSpell::TeX);
	#else
		kdDebug() << "KSPELL: using OLD kspell " << KDE_VERSION_STRING << endl;
		kspell = new KSpell(this, i18n("Spellcheck"), this,SLOT( spell_started(KSpell *)), 0, true, false);
	#endif
	ks_corrected=0;
	connect (kspell, SIGNAL ( death()),this, SLOT ( spell_finished( )));
	connect (kspell, SIGNAL (progress (unsigned int)),this, SLOT (spell_progress (unsigned int)));
	connect (kspell, SIGNAL (misspelling (const QString & , const QStringList & , unsigned int )),this, SLOT (misspelling (const QString & , const QStringList & , unsigned int )));
	connect (kspell, SIGNAL (corrected (const QString & , const QString & , unsigned int )),this, SLOT (corrected (const QString & , const QString & , unsigned int )));
	connect (kspell, SIGNAL (done(const QString&)), this, SLOT (spell_done(const QString&)));
}

void Kile::spell_started( KSpell *)
{
	kspell->setProgressResolution(2);
	Kate::View *view = currentView();

	if ( view->getDoc()->hasSelection() )
	{
		kspell->check(view->getDoc()->selection());
		par_start = view->getDoc()->selStartLine();
		par_end =  view->getDoc()->selEndLine();
		index_start =  view->getDoc()->selStartCol();
		index_end =  view->getDoc()->selEndCol();
	}
	else
	{
		kspell->check(view->getDoc()->text());
		par_start=0;
		par_end=view->getDoc()->numLines()-1;
		index_start=0;
		index_end=view->getDoc()->textLine(par_end).length();
	}
}

void Kile::spell_progress (unsigned int /*percent*/)
{
}

void Kile::spell_done(const QString& /*newtext*/)
{
  currentView()->getDoc()->clearSelection();
  kspell->cleanUp();
	KMessageBox::information(this,i18n("Corrected %1 words.").arg(ks_corrected),i18n("Spell checking done"));
}

void Kile::spell_finished( )
{
	//newStatus();
	KSpell::spellStatus status = kspell->status();

	delete kspell;
	kspell = 0;
	if (status == KSpell::Error)
  {
     KMessageBox::sorry(this, i18n("I(A)Spell could not be started."));
  }
	else if (status == KSpell::Crashed)
  {
     currentView()->getDoc()->clearSelection();
     KMessageBox::sorry(this, i18n("I(A)Spell seems to have crashed."));
  }
}

void Kile::misspelling (const QString & originalword, const QStringList & /*suggestions*/,unsigned int pos)
{
  int l=par_start;
  int cnt=0;
  int col=0;
  int p=pos+index_start;

  while ((cnt+currentView()->getDoc()->lineLength(l)<=p) && (l < par_end))
  {
  	cnt+=currentView()->getDoc()->lineLength(l)+1;
  	l++;
  }
  col=p-cnt;
  currentView()->setCursorPosition(l,col);
  currentView()->getDoc()->setSelection( l,col,l,col+originalword.length());
}


void Kile::corrected (const QString & originalword, const QString & newword, unsigned int pos)
{
  int l=par_start;
  int cnt=0;
  int col=0;
  int p=pos+index_start;
  if( newword != originalword )
  {
    while ((cnt+currentView()->getDoc()->lineLength(l)<=p) && (l < par_end))
    {
    cnt+=currentView()->getDoc()->lineLength(l)+1;
    l++;
    }
    col=p-cnt;
    currentView()->setCursorPosition(l,col);
    currentView()->getDoc()->setSelection( l,col,l,col+originalword.length());
    currentView()->getDoc()->removeSelectedText();
    currentView()->getDoc()->insertText( l,col,newword );
    currentView()->getDoc()->setModified( TRUE );
  }
  currentView()->getDoc()->clearSelection();

  ks_corrected++;
}

/////////////// KEYS - TOOLBARS CONFIGURATION ////////////////
void Kile::ConfigureKeys()
{
	KKeyDialog dlg( false, this );
	QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
	for( QPtrListIterator<KXMLGUIClient> it( clients );	it.current(); ++it )
	{
		dlg.insert( (*it)->actionCollection() );
	}
	dlg.configure();
}

void Kile::ConfigureToolbars()
{
 saveMainWindowSettings(config, "KileMainWindow" );
 KEditToolbar dlg(factory());
    if ( dlg.exec() )
    {
    partManager->setActivePart( 0L );
    applyMainWindowSettings(config, "KileMainWindow" );
    }
}

///////////// NAVIGATION - DOC ////////////////////
void Kile::updateNavAction(bool back, bool forward)
{
BackAction->setEnabled(back);
ForwardAction->setEnabled(forward);
}
////////////// VERTICAL TAB /////////////////
void Kile::showVertPage(int page)
{
ButtonBar->setTab(lastvtab,false);
ButtonBar->setTab(page,true);
lastvtab=page;
if (page==0)
   {
   m_projectview->hide();
   outstruct->hide();
   mpview->hide();
   if (symbol_view && symbol_present) delete symbol_view;
   symbol_present=false;
   if (Structview_layout) delete Structview_layout;
   Structview_layout=new QHBoxLayout(Structview);
   Structview_layout->add(KileFS);
   Structview_layout->add(ButtonBar);
   ButtonBar->setPosition(KMultiVertTabBar::Right);
   KileFS->show();
   }
else if (page==1)
   {
   //UpdateStructure();
   m_projectview->hide();
   KileFS->hide();
   mpview->hide();
   if (symbol_view && symbol_present) delete symbol_view;
   symbol_present=false;
   if (Structview_layout) delete Structview_layout;
   Structview_layout=new QHBoxLayout(Structview);
   Structview_layout->add(outstruct);
   Structview_layout->add(ButtonBar);
   ButtonBar->setPosition(KMultiVertTabBar::Right);
   outstruct->show();
   }
else if (page==8)
   {
   m_projectview->hide();
   KileFS->hide();
   outstruct->hide();
   if (symbol_view && symbol_present) delete symbol_view;
   symbol_present=false;
   if (Structview_layout) delete Structview_layout;
   Structview_layout=new QHBoxLayout(Structview);
   Structview_layout->add(mpview);
   Structview_layout->add(ButtonBar);
   ButtonBar->setPosition(KMultiVertTabBar::Right);
   mpview->show();
   }
else if (page==9)
{
	kdDebug() << "SHOWING PROJECTS VIEW" << endl;
	if (symbol_view && symbol_present) delete symbol_view;
   symbol_present=false;
	KileFS->hide();
    outstruct->hide();
    mpview->hide();
	delete Structview_layout;
	Structview_layout=new QHBoxLayout(Structview);
    Structview_layout->add(m_projectview);
    Structview_layout->add(ButtonBar);
	ButtonBar->setPosition(KMultiVertTabBar::Right);
	m_projectview->show();
}
else
   {
	m_projectview->hide();
      KileFS->hide();
      outstruct->hide();
      mpview->hide();
      if (symbol_view && symbol_present) delete symbol_view;
      if (Structview_layout) delete Structview_layout;
      Structview_layout=new QHBoxLayout(Structview);
      symbol_view = new SymbolView(page-1,Structview,"Symbols");
      connect(symbol_view, SIGNAL(executed(QIconViewItem*)), SLOT(insertSymbol(QIconViewItem*)));
      symbol_present=true;
      Structview_layout->add(symbol_view);
      Structview_layout->add(ButtonBar);
      ButtonBar->setPosition(KMultiVertTabBar::Right);
      symbol_view->show();
   }
}

void Kile::changeInputEncoding()
{
	Kate::View *view = currentView();
	if (view)
	{
		bool modified = view->getDoc()->isModified();

  		QString encoding=KileFS->comboEncoding->lineEdit()->text();
		QString text = view->getDoc()->text();

		view->getDoc()->setEncoding(encoding);
		//reload the document so that the new encoding takes effect
		view->getDoc()->openURL(view->getDoc()->url());

		setHighlightMode(view->getDoc());
		view->getDoc()->setModified(modified);
	}
}

////////////////// EPS SIZE ///////////////////
QString Kile::DetectEpsSize(const QString &epsfile)
{
int tagStart;
float  el, et, er, eb, w1, h1, w2, h2;
QString win, hin, wcm, hcm;
QString l="0";
QString t="0";
QString r="0";
QString b="0";
bool ok;
QFileInfo fic(epsfile);
if (fic.exists() && fic.isReadable() )
  {
  QFile f( epsfile );
  if ( !f.open( IO_ReadOnly ) )
  {
  return "";
  }
  QTextStream text(&f);
    while ( !text.eof() )
    {
    QString line = text.readLine();
    tagStart=0;
    tagStart=line.find("%%BoundingBox: ",0);
    if (tagStart!=-1)
       {
       line=line.right(line.length()-15);
       /// l ///
       tagStart=line.find(" ",0);
       if (tagStart!=-1)
           {
           l=line.left(tagStart);
           line=line.right(line.length()-tagStart-1);
           }
       /// t ///
       tagStart=line.find(" ",0);
       if (tagStart!=-1)
           {
           t=line.left(tagStart);
           line=line.right(line.length()-tagStart-1);
           }
       /// r ///
       tagStart=line.find(" ",0);
       if (tagStart!=-1)
           {
           r=line.left(tagStart);
           line=line.right(line.length()-tagStart-1);
           }
       /// b ///
       b=line;
       break;
       }
    }
 f.close();
 }
el = l.toFloat( &ok );
if (!ok) return "";
et = t.toFloat( &ok);
if (!ok) return "";
er = r.toFloat( &ok);
if (!ok) return "";
eb = b.toFloat( &ok);
if (!ok) return "";
if ((er-el>0) && (eb-et>0))
   {
     w1= (float) ((er-el)/72.27);
     h1= (float) ((eb-et)/72.27);
     w2= (float) (w1*2.54);
     h2= (float) (h1*2.54);
     win=win.setNum(w1,'f',2);
     hin=hin.setNum(h1,'f',2);
     wcm=wcm.setNum(w2,'f',2);
     hcm=hcm.setNum(h2,'f',2);
     return "Original size : [width="+win+"in,height="+hin+"in] or [width="+wcm+"cm,height="+hcm+"cm]";
    }
else return "";
}

/*
 * LyX server commands
 */

void Kile::insertCite(const QString &cite)
{
	insertTag(KileAction::TagData("cite", "\\cite{"+cite+"}", QString::null, 7+cite.length()));
}

void Kile::insertBibTeX(const QString& bib)
{
	insertTag(KileAction::TagData("bibliography", "\\bibliography{"+bib+"}", QString::null, 15+bib.length()));
}

void Kile::insertBibTeXDatabaseAdd(const QString& bib)
{
	insertTag(KileAction::TagData("bibliography", "\\bibliography{"+bib+"}", QString::null, 15+bib.length()));
}

//////////////////// CLEAN BIB /////////////////////
void Kile::CleanBib()
{
QString s;
if ( !currentView() )	return;
uint i=0;
while(i < currentView()->getDoc()->numLines())
   {
    s = currentView()->getDoc()->textLine(i);
    s=s.left(3);
    if (s=="OPT" || s=="ALT")
        {
        currentView()->getDoc()->removeLine(i );
        currentView()->getDoc()->setModified(true);
        }
    else i++;
   }
}

////////KileAutoSaveJob
KileAutoSaveJob::KileAutoSaveJob(const KURL &url)
{
	KIO::Job *job = KIO::file_copy(url,KURL(KURL::fromPathOrURL(url.path()+".backup")),-1,true,false,false);
	//let KIO show the error messages
    job->setAutoErrorHandlingEnabled(true);
	connect(job, SIGNAL(result(KIO::Job*)), this, SLOT(slotResult(KIO::Job*)));
}

KileAutoSaveJob::~KileAutoSaveJob()
{
	kdDebug() << "DELETING KileAutoSaveJob" << endl;
}

void KileAutoSaveJob::slotResult(KIO::Job *job)
{
	if (job->error() == 0)
	{
		emit(success());
	}
}

/////// editor extensions /////////////
KileEventFilter::KileEventFilter()
{
	m_bHandleEnter = true;
	//m_bCompleteEnvironment = false;
	m_regexpEnter  = QRegExp("(.*)(\\\\begin\\s*\\{[^\\{\\}]*\\})\\s*$");

	readConfig();
}

void KileEventFilter::readConfig()
{
	KConfig *config = kapp->config();
	config->setGroup( "Editor Ext" );
	m_bCompleteEnvironment = config->readBoolEntry( "Complete Environment", true);
}

bool KileEventFilter::eventFilter(QObject *o, QEvent *e)
{
	if ( e->type() == QEvent::AccelOverride)
	{
		QKeyEvent *ke = (QKeyEvent*) e;
		//kdDebug() << "eventFilter : AccelOverride : " << ke->key() << endl;
		//kdDebug() << "              type          : " << ke->type() << endl;
		//kdDebug() << "              state         : " << ke->state() << endl;

		if ( m_bCompleteEnvironment &&  ke->key() == Qt::Key_Return && ke->state() == 0)
		{
			if (m_bHandleEnter)
			{
				//kdDebug() << "              enter" << endl;
				Kate::View *view = (Kate::View*) o;

				QString line = view->getDoc()->textLine(view->cursorLine()).left(view->cursorColumnReal());
				int pos = m_regexpEnter.search(line);
				//kdDebug() << "              line     : " << line << endl;
				//kdDebug() << "              pos      : " << pos << endl;
				//kdDebug() << "              captured : " << m_regexpEnter.cap(1) << "+" << m_regexpEnter.cap(2) << endl;
				if (pos != -1 )
				{
					line = m_regexpEnter.cap(1);
					for (uint i=0; i < line.length(); i++)
						if ( ! line[i].isSpace() ) line[i] = ' ';

					line += m_regexpEnter.cap(2).replace("\\begin","\\end")+"\n";

					view->getDoc()->insertText(view->cursorLine()+1, 0, line);
				}

				m_bHandleEnter=false;

				return true;
			}
			else
				m_bHandleEnter = true;
		}

		m_bHandleEnter = true;
		return false;
	}

	//pass this event on
	return false;
}

KileListViewItem::KileListViewItem(QListViewItem * parent, QListViewItem * after, QString title, uint line, uint column, int type)
	: KListViewItem(parent,after), m_title(title), m_line(line), m_column(column), m_type(type)
{
	this->setText(0, m_title+" (line "+QString::number(m_line)+")");
}

#include "kile.moc"
