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

// last change: 24.01.2004 (dani)

#include "kile.h"

#include <ktexteditor/editorchooser.h>
#include <ktexteditor/encodinginterface.h>
#include <ktexteditor/codecompletioninterface.h>    
#include <ktexteditor/searchinterface.h>
#include <kparts/componentfactory.h>

#include <kdebug.h>
#include <kaboutdata.h>
#include <kileapplication.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <kiconloader.h>
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
#include <kprogress.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <ktoolbarbutton.h>

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

#include "kiledocumentinfo.h"
#include "kileactions.h"
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
#include "letterdialog.h"
#include "arraydialog.h"
#include "tabbingdialog.h"
#include "kilestructurewidget.h"
#include "convert.h"
#include "includegraphicsdialog.h"                  // new dialog (dani)
#include "cleandialog.h"                            // clean dialog (dani)
#include "kiletoolcapability.h"

Kile::Kile( bool rest, QWidget *parent, const char *name ) :
	DCOPObject( "Kile" ),
	KParts::MainWindow( parent, name),
	KileInfo(),
	m_paPrint(0L),
	m_bQuick(false),
	m_activeView(0),
	m_nCurrentError(-1),
	m_bShowUserMovedMessage(false)
{
	// do initializations first
	m_currentState=m_wantState="Editor";
	m_bWatchFile=false;
	m_bNewInfolist=true;
	m_bCheckForLaTeXErrors=false;
	m_bBlockWindowActivateEvents=false;
	kspell = 0;
	symbol_view = 0L;
	symbol_present=false;

	m_docList.setAutoDelete(false);
	m_infoList.setAutoDelete(false);

	partManager = new KParts::PartManager( this );
	connect( partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(ActivePartGUI ( KParts::Part * ) ) );

	m_AutosaveTimer= new QTimer();
	connect(m_AutosaveTimer,SIGNAL(timeout()),this,SLOT(autoSaveAll()));

	m_eventFilter = new KileEventFilter();
	connect(this,SIGNAL(configChanged()), m_eventFilter, SLOT(readConfig()));

	statusBar()->insertItem(i18n("Line: 1 Col: 1"), ID_LINE_COLUMN, 0, true);
	statusBar()->setItemAlignment( ID_LINE_COLUMN, AlignLeft|AlignVCenter );
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
	KileFS->comboEncoding->lineEdit()->setText(input_encoding);

	m_projectview = new KileProjectView(Structview, this);
	ButtonBar->insertTab( SmallIcon("editcopy"),9,i18n("Files and Projects"));
	connect(ButtonBar->getTab(9),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	connect(m_projectview, SIGNAL(fileSelected(const KileProjectItem *)), this, SLOT(fileSelected(const KileProjectItem *)));
	connect(m_projectview, SIGNAL(fileSelected(const KURL &)), this, SLOT(fileSelected(const KURL &)));
	connect(m_projectview, SIGNAL(closeURL(const KURL&)), this, SLOT(fileClose(const KURL&)));
	connect(m_projectview, SIGNAL(closeProject(const KURL&)), this, SLOT(projectClose(const KURL&)));
	connect(m_projectview, SIGNAL(projectOptions(const KURL&)), this, SLOT(projectOptions(const KURL&)));
	connect(m_projectview, SIGNAL(projectArchive(const KURL&)), this, SLOT(projectArchive(const KURL&)));
	connect(m_projectview, SIGNAL(removeFromProject(const KileProjectItem *)), this, SLOT(removeFromProject(const KileProjectItem *)));
	connect(m_projectview, SIGNAL(addFiles(const KURL &)), this, SLOT(projectAddFiles(const KURL &)));
	connect(m_projectview, SIGNAL(toggleArchive(KileProjectItem *)), this, SLOT(toggleArchive(KileProjectItem *)));
	connect(m_projectview, SIGNAL(addToProject(const KURL &)), this, SLOT(addToProject(const KURL &)));
	connect(m_projectview, SIGNAL(saveURL(const KURL &)), this, SLOT(saveURL(const KURL &)));
	connect(m_projectview, SIGNAL(buildProjectTree(const KURL &)), this, SLOT(buildProjectTree(const KURL &)));
	connect(this, SIGNAL(projectTreeChanged(const KileProject *)),m_projectview, SLOT(refreshProjectTree(const KileProject *)));

	ButtonBar->insertTab( SmallIcon("structure"),1,i18n("Structure"));
	connect(ButtonBar->getTab(1),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	m_kwStructure = new KileWidget::Structure(this, Structview);
	m_kwStructure->setFocusPolicy(QWidget::ClickFocus);
	m_kwStructure->header()->hide();
	m_kwStructure->addColumn(i18n("Structure"),-1);
	m_kwStructure->setSorting(-1,true);
	connect(m_kwStructure, SIGNAL(setCursor(int,int)), this, SLOT(setCursor(int,int)));
	connect(m_kwStructure, SIGNAL(fileOpen(const KURL&, const QString & )), this, SLOT(fileOpen(const KURL&, const QString& )));
	connect(m_kwStructure, SIGNAL(fileNew(const KURL&)), this, SLOT(fileNew(const KURL&)));
	connect(this, SIGNAL(closingDocument(KileDocumentInfo* )), m_kwStructure, SLOT(closeDocument(KileDocumentInfo *)));
	QToolTip::add(m_kwStructure, i18n("Click to jump to the line"));

	mpview = new metapostview( Structview );
	connect(mpview, SIGNAL(clicked(QListBoxItem *)), SLOT(InsertMetaPost(QListBoxItem *)));

	// new features
	m_complete = new CodeCompletion();                 // code completion (dani)
	m_edit = new KileEdit();                           // advanced editor (dani)
	m_help = new KileHelp::Help(m_edit);     // kile help (dani)

	config = KGlobal::config();

  // check requirements for IncludeGraphicsDialog (dani)
	config->setGroup("IncludeGraphics");
	config->writeEntry("imagemagick", ! ( KStandardDirs::findExe("identify")==QString::null ) );
  
	//workaround for kdvi crash when started with Tooltips
	config->setGroup("TipOfDay");
	config->writeEntry( "RunOnStart",false);

	KileFS->readConfig();

	setXMLFile( "kileui.rc" );

	ReadSettings();

	setupActions();

	// ReadRecentFileSettings should be after setupActions() because fileOpenRecentAction needs to be
	// initialized before calling ReadSettnigs().
	ReadRecentFileSettings();

	
	ButtonBar->insertTab(SmallIcon("math1"),2,i18n("Relation Symbols"));
	connect(ButtonBar->getTab(2),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(SmallIcon("math2"),3,i18n("Arrow Symbols"));
	connect(ButtonBar->getTab(3),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(SmallIcon("math3"),4,i18n("Miscellaneous Symbols"));
	connect(ButtonBar->getTab(4),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(SmallIcon("math4"),5,i18n("Delimiters"));
	connect(ButtonBar->getTab(5),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(SmallIcon("math5"),6,i18n("Greek Letters"));
	connect(ButtonBar->getTab(6),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(SmallIcon("math6"),7,i18n("Foreign characters"));
	connect(ButtonBar->getTab(7),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
	ButtonBar->insertTab(SmallIcon("metapost"),8,i18n("MetaPost Commands"));
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

	//LogWidget = new MessageWidget( Outputview );
	LogWidget = new KileWidget::LogMsg( this, Outputview );
	connect(LogWidget, SIGNAL(fileOpen(const KURL&, const QString & )), this, SLOT(fileOpen(const KURL&, const QString& )));
	connect(LogWidget, SIGNAL(setLine(const QString& )), this, SLOT(setLine(const QString& )));

	LogWidget->setFocusPolicy(QWidget::ClickFocus);
	LogWidget->setMinimumHeight(40);
	LogWidget->setReadOnly(true);
	Outputview->addTab(LogWidget,SmallIcon("viewlog"), i18n("Log/Messages"));

	OutputWidget = new KileWidget::Output(Outputview);
	OutputWidget->setFocusPolicy(QWidget::ClickFocus);
	OutputWidget->setMinimumHeight(40);
	OutputWidget->setReadOnly(true);
	Outputview->addTab(OutputWidget,SmallIcon("output_win"), i18n("Output"));

	logpresent=false;
	m_outputInfo=new LatexOutputInfoArray();
	m_outputFilter=new LatexOutputFilter(m_outputInfo);
	connect(m_outputFilter, SIGNAL(problem(int, const QString& )), LogWidget, SLOT(printProblem(int, const QString& )));

	texkonsole=new KileWidget::Konsole(this, Outputview,"konsole");
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

	m_lyxserver = new KileLyxServer(m_runlyxserver);
	connect(m_lyxserver, SIGNAL(insert(const KileAction::TagData &)), this, SLOT(insertTag(const KileAction::TagData &)));

	KileApplication::closeSplash();
	show();

	connect(Outputview, SIGNAL( currentChanged( QWidget * ) ), texkonsole, SLOT(sync()));

	applyMainWindowSettings(config, "KileMainWindow" );

	m_manager  = new KileTool::Manager(this, config, LogWidget, OutputWidget, partManager, topWidgetStack, m_paStop, 10000); //FIXME make timeout configurable
	connect(m_manager, SIGNAL(requestGUIState(const QString &)), this, SLOT(prepareForPart(const QString &)));
	connect(m_manager, SIGNAL(requestSaveAll()), this, SLOT(fileSaveAll()));

	m_toolFactory = new KileTool::Factory(m_manager, config);
	m_manager->setFactory(m_toolFactory);
	m_help->setManager(m_manager);     // kile help (dani)

	if ( m_listUserTools.count() > 0 )
	{
		KMessageBox::information(0, i18n("You have defined some tools in the User menu. From now on these tools will be available from the Build->Other menu and can be configured in the configuration dialog (go to the Settings menu and choose Configure Kile). This has some advantages; your own tools can now be used in a QuickBuild command if you wish."), i18n("User Tools Detected"));
		m_listUserTools.clear();
	}

	if (m_bShowUserMovedMessage)
	{
		KMessageBox::information(0, i18n("Please note that the 'User' menu, which holds the (La)TeX tags you have defined, is moved to the LaTeX menu."));
	}

	if (rest) restore();
}

Kile::~Kile()
{
	kdDebug() << "cleaning up..." << endl;

	// CodeCompletion  and edvanced editor (dani)
	delete m_complete;
	delete m_edit;

	delete m_AutosaveTimer;
}

void Kile::setupActions()
{
	m_paPrint = KStdAction::print(0,0, actionCollection(), "print");
	(void) KStdAction::openNew(this, SLOT(fileNew()), actionCollection(), "file_new" );
	(void) KStdAction::open(this, SLOT(fileOpen()), actionCollection(),"file_open" );
	fileOpenRecentAction = KStdAction::openRecent(this, SLOT(fileOpen(const KURL&)), actionCollection(), "file_open_recent");
	(void) new KAction(i18n("Save All"),"save_all", 0, this, SLOT(fileSaveAll()), actionCollection(),"file_save_all" );
	(void) new KAction(i18n("Create Template From Document..."), 0, this, SLOT(createTemplate()), actionCollection(),"CreateTemplate");
	(void) KStdAction::close(this, SLOT(fileClose()), actionCollection(),"file_close" );
	(void) new KAction(i18n("Close All"), 0, this, SLOT(fileCloseAll()), actionCollection(),"file_close_all" );
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

	(void) new KAction(i18n("Find &in files..."), ALT+SHIFT+Key_F, this, SLOT(FindInFiles()), actionCollection(),"FindInFiles" );

	#if KDE_VERSION <= KDE_MAKE_VERSION(3,3,0)
		(void) KStdAction::spelling(this, SLOT(spellcheck()), actionCollection(), "Spell" );
	#endif

	(void) new KAction(i18n("Refresh Structure"), "structure", 0, this, SLOT(RefreshStructure()), actionCollection(),"RefreshStructure" );

	//project actions
	(void) new KAction(i18n("&New Project..."), "filenew", 0, this, SLOT(projectNew()), actionCollection(), "project_new");
	(void) new KAction(i18n("&Open Project..."), "fileopen", 0, this, SLOT(projectOpen()), actionCollection(), "project_open");
	m_actRecentProjects =  new KRecentFilesAction(i18n("Open &Recent Project..."),  0, this, SLOT(projectOpen(const KURL &)), actionCollection(), "project_openrecent");
	(void) new KAction(i18n("A&dd files to project..."), 0, this, SLOT(projectAddFiles()), actionCollection(), "project_add");
	(void) new KAction(i18n("Refresh Project &Tree"), "relation", 0, this, SLOT(buildProjectTree()), actionCollection(), "project_buildtree");
	(void) new KAction(i18n("&Archive"), "package", 0, this, SLOT(projectArchive()), actionCollection(), "project_archive");
	(void) new KAction(i18n("Project &Options..."), "configure", 0, this, SLOT(projectOptions()), actionCollection(), "project_options");
	(void) new KAction(i18n("&Close Project"), "fileclose", 0, this, SLOT(projectClose()), actionCollection(), "project_close");

	//build actions
	(void) new KAction(i18n("Clean"),0 , this, SLOT(CleanAll()), actionCollection(),"CleanAll" );
	(void) new KAction(i18n("View Log File"),"viewlog", ALT+Key_0, this, SLOT(ViewLog()), actionCollection(),"ViewLog" );
	(void) new KAction(i18n("Previous LaTeX Error"),"errorprev", 0, this, SLOT(PreviousError()), actionCollection(),"PreviousError" );
	(void) new KAction(i18n("Next LaTeX Error"),"errornext", 0, this, SLOT(NextError()), actionCollection(),"NextError" );
	(void) new KAction(i18n("Previous LaTeX Warning"),"warnprev", 0, this, SLOT(PreviousWarning()), actionCollection(),"PreviousWarning" );
	(void) new KAction(i18n("Next LaTeX Warning"),"warnnext", 0, this, SLOT(NextWarning()), actionCollection(),"NextWarning" );
	(void) new KAction(i18n("Previous LaTeX BadBox"),"bboxprev", 0, this, SLOT(PreviousBadBox()), actionCollection(),"PreviousBadBox" );
	(void) new KAction(i18n("Next LaTeX BadBox"),"bboxnext", 0, this, SLOT(NextBadBox()), actionCollection(),"NextBadBox" );
	m_paStop = new KAction(i18n("&Stop"),"stop",Key_Escape,0,0,actionCollection(),"Stop");
	m_paStop->setEnabled(false);

// 	m_toolsToolBar = new KToolBar(this, Qt::DockTop, false, "toolsToolBar");
// 	m_toolsToolBar->setXMLGUIClient(this);
//	connect(toolBar("toolsToolBar"), SIGNAL(clicked(int)), this, SLOT(test(int)));
	setupTools();

	(void) new KAction(i18n("Editor View"),"edit",CTRL+Key_E , this, SLOT(ShowEditorWidget()), actionCollection(),"EditorView" );
	(void) new KAction(i18n("Next Document"),"forward",ALT+Key_Right, this, SLOT(gotoNextDocument()), actionCollection(), "gotoNextDocument" );
	(void) new KAction(i18n("Previous Document"),"back",ALT+Key_Left, this, SLOT(gotoPrevDocument()), actionCollection(), "gotoPrevDocument" );
	(void) new KAction(i18n("Focus Log/Messages view"), CTRL+ALT+Key_M, this, SLOT(focusLog()), actionCollection(), "focus_log");
	(void) new KAction(i18n("Focus Output view"), CTRL+ALT+Key_O, this, SLOT(focusOutput()), actionCollection(), "focus_output");
	(void) new KAction(i18n("Focus Konsole view"), CTRL+ALT+Key_K, this, SLOT(focusKonsole()), actionCollection(), "focus_konsole");
	(void) new KAction(i18n("Focus Editor view"), CTRL+ALT+Key_E, this, SLOT(focusEditor()), actionCollection(), "focus_editor");

 // CodeCompletion (dani)
	(void) new KAction(i18n("La(TeX) Command"),"complete1",CTRL+Key_Space, this, SLOT(editCompleteWord()), actionCollection(), "edit_complete_word");
	(void) new KAction(i18n("Environment"),"complete2",ALT+Key_Space, this, SLOT(editCompleteEnvironment()), actionCollection(), "edit_complete_env");
	(void) new KAction(i18n("Abbreviation"),"complete3",CTRL+ALT+Key_Space, this, SLOT(editCompleteAbbreviation()), actionCollection(), "edit_complete_abbrev");
	(void) new KAction(i18n("Next Bullet"),"nextbullet",CTRL+ALT+Key_Right, this, SLOT(editNextBullet()), actionCollection(), "edit_next_bullet");
	(void) new KAction(i18n("Prev Bullet"),"prevbullet",CTRL+ALT+Key_Left, this, SLOT(editPrevBullet()), actionCollection(), "edit_prev_bullet");

 // advanced editor (dani)
	(void) new KAction(i18n("Environment (inside)"),KShortcut("CTRL+Alt+S,E"), this, SLOT(selectEnvInside()), actionCollection(), "edit_select_inside_env");
	(void) new KAction(i18n("Environment (outside)"),KShortcut("CTRL+Alt+S,F"),this, SLOT(selectEnvOutside()), actionCollection(), "edit_select_outside_env");
	(void) new KAction(i18n("TeX Group (inside)"),"selgroup_i",KShortcut("CTRL+Alt+S,T"), this, SLOT(selectTexgroupInside()), actionCollection(), "edit_select_inside_group");
	(void) new KAction(i18n("TeX Group (outside)"),"selgroup_o",KShortcut("CTRL+Alt+S,U"),this, SLOT(selectTexgroupOutside()), actionCollection(), "edit_select_outside_group");
	(void) new KAction(i18n("Paragraph"),KShortcut("CTRL+Alt+S,P"),this, SLOT(selectParagraph()), actionCollection(), "edit_select_paragraph");
	(void) new KAction(i18n("Line"),KShortcut("CTRL+Alt+S,L"),this, SLOT(selectLine()), actionCollection(), "edit_select_line");
	(void) new KAction(i18n("TeX word"),KShortcut("CTRL+Alt+S,W"),this, SLOT(selectWord()), actionCollection(), "edit_select_word");

	(void) new KAction(i18n("Environment (inside)"),KShortcut("CTRL+Alt+D,E"), this, SLOT(deleteEnvInside()), actionCollection(), "edit_delete_inside_env");
	(void) new KAction(i18n("Environment (outside)"),KShortcut("CTRL+Alt+D,F"),this, SLOT(deleteEnvOutside()), actionCollection(), "edit_delete_outside_env");
	(void) new KAction(i18n("TeX Group (inside)"),KShortcut("CTRL+Alt+D,T"), this, SLOT(deleteTexgroupInside()), actionCollection(), "edit_delete_inside_group");
	(void) new KAction(i18n("TeX Group (outside)"),KShortcut("CTRL+Alt+D,U"),this, SLOT(deleteTexgroupInside()), actionCollection(), "edit_delete_outside_group");
	(void) new KAction(i18n("Paragraph"),KShortcut("CTRL+Alt+D,P"),this, SLOT(deleteParagraph()), actionCollection(), "edit_delete_paragraph");
	(void) new KAction(i18n("TeX word"),KShortcut("CTRL+Alt+D,W"),this, SLOT(deleteWord()), actionCollection(), "edit_delete_word");

	(void) new KAction(i18n("Goto Begin"),KShortcut("CTRL+Alt+E,B"), this, SLOT(gotoBeginEnv()), actionCollection(), "edit_begin_env");
	(void) new KAction(i18n("Goto End"),KShortcut("CTRL+Alt+E,E"), this, SLOT(gotoEndEnv()), actionCollection(), "edit_end_env");
	(void) new KAction(i18n("Match"),"matchenv",KShortcut("CTRL+Alt+E,M"), this, SLOT(matchEnv()), actionCollection(), "edit_match_env");
	(void) new KAction(i18n("Close"),"closeenv",KShortcut("CTRL+Alt+E,C"), this, SLOT(closeEnv()), actionCollection(), "edit_close_env");

	(void) new KAction(i18n("Goto Begin"),KShortcut("CTRL+Alt+G,B"), this, SLOT(gotoBeginTexgroup()), actionCollection(), "edit_begin_group");
	(void) new KAction(i18n("Goto End"),KShortcut("CTRL+Alt+G,E"), this, SLOT(gotoEndTexgroup()), actionCollection(), "edit_end_group");
	(void) new KAction(i18n("Match"),"matchgroup",KShortcut("CTRL+Alt+G,M"), this, SLOT(matchTexgroup()), actionCollection(), "edit_match_group");
	(void) new KAction(i18n("Close"),"closegroup",KShortcut("CTRL+Alt+G,C"), this, SLOT(closeTexgroup()), actionCollection(), "edit_close_group");

	(void) new KAction(i18n("teTeX Guide"),KShortcut("CTRL+Alt+H,T"), this, SLOT(helpTetexGuide()), actionCollection(), "edit_help_tetex_guide");
	(void) new KAction(i18n("teTeX Doc"),KShortcut("CTRL+Alt+H,T"), this, SLOT(helpTetexDoc()), actionCollection(), "edit_help_tetex_doc");
	(void) new KAction(i18n("LaTeX"),KShortcut("CTRL+Alt+H,L"), this, SLOT(helpLatexIndex()), actionCollection(), "edit_help_latex_index");
	(void) new KAction(i18n("LaTeX Command"),KShortcut("CTRL+Alt+H,C"), this, SLOT(helpLatexCommand()), actionCollection(), "edit_help_latex_command");
	(void) new KAction(i18n("LaTeX Subject"),KShortcut("CTRL+Alt+H,S"), this, SLOT(helpLatexSubject()), actionCollection(), "edit_help_latex_subject");
	(void) new KAction(i18n("LaTeX Env"),KShortcut("CTRL+Alt+H,E"), this, SLOT(helpLatexEnvironment()), actionCollection(), "edit_help_latex_env");
	(void) new KAction(i18n("Context Help"),KShortcut("CTRL+Alt+H,K"), this, SLOT(helpKeyword()), actionCollection(), "edit_help_context");

	KileStdActions::setupStdTags(this,this);
	KileStdActions::setupMathTags(this);
	KileStdActions::setupBibTags(this);

	(void) new KAction(i18n("Quick Start"),"wizard",0 , this, SLOT(QuickDocument()), actionCollection(),"127" );
	(void) new KAction(i18n("Letter"),"wizard",0 , this, SLOT(QuickLetter()), actionCollection(),"128" );
	(void) new KAction(i18n("Tabular"),"wizard",0 , this, SLOT(QuickTabular()), actionCollection(),"129" );
	(void) new KAction(i18n("Tabbing"),"wizard",0 , this, SLOT(QuickTabbing()), actionCollection(),"149" );
	(void) new KAction(i18n("Array"),"wizard",0 , this, SLOT(QuickArray()), actionCollection(),"130" );


	(void) new KAction(i18n("Clean"),0 , this, SLOT(CleanBib()), actionCollection(),"CleanBib" );

	ModeAction=new KToggleAction(i18n("Define Current Document as 'Master Document'"),"master",0 , this, SLOT(ToggleMode()), actionCollection(),"Mode" );

	StructureAction=new KToggleAction(i18n("Show Structure View"),0 , this, SLOT(ToggleStructView()), actionCollection(),"StructureView" );
	MessageAction=new KToggleAction(i18n("Show Messages View"),0 , this, SLOT(ToggleOutputView()), actionCollection(),"MessageView" );

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
	if (showstructview) {StructureAction->setChecked(true);}
	else {StructureAction->setChecked(false);}
	if (showoutputview) {MessageAction->setChecked(true);}
	else {MessageAction->setChecked(false);}

	(void) new KAction(i18n("Remove Template..."),0,this,SLOT(removeTemplate()),actionCollection(),"removetemplates");

	WatchFileAction=new KToggleAction(i18n("Watch File Mode"),"watchfile",0 , this, SLOT(ToggleWatchFile()), actionCollection(),"WatchFile" );
	if (m_bWatchFile) {WatchFileAction->setChecked(true);}
	else {WatchFileAction->setChecked(false);}

	setHelpMenuEnabled(false);
	const KAboutData *aboutData = KGlobal::instance()->aboutData();
	KHelpMenu *help_menu = new KHelpMenu( this, aboutData);
	(void) new KAction(i18n("LaTeX Reference"),"help",0 , this, SLOT(LatexHelp()), actionCollection(),"help1" );
	(void) KStdAction::helpContents(help_menu, SLOT(appHelpActivated()), actionCollection(), "help2");
	(void) KStdAction::reportBug (help_menu, SLOT(reportBug()), actionCollection(), "report_bug");
	(void) KStdAction::aboutApp(help_menu, SLOT(aboutApplication()), actionCollection(),"help4" );
	(void) KStdAction::aboutKDE(help_menu, SLOT(aboutKDE()), actionCollection(),"help5" );
	(void) KStdAction::preferences(this, SLOT(GeneralOptions()), actionCollection(),"settings_configure" );
	(void) KStdAction::keyBindings(this, SLOT(ConfigureKeys()), actionCollection(),"147" );
	(void) KStdAction::configureToolbars(this, SLOT(ConfigureToolbars()), actionCollection(),"148" );

	m_menuUserTags = new KActionMenu(i18n("User Tags"), SmallIcon("usertag"), actionCollection(),"menuUserTags");
	m_menuUserTags->setDelayed(false);
	m_mapUserTagSignals = new QSignalMapper(this,"mapUserTagSignals");
	setupUserTagActions();
	connect(m_mapUserTagSignals,SIGNAL(mapped(int)),this,SLOT(insertUserTag(int)));

	actionCollection()->readShortcutSettings();
}

void Kile::setupTools()
{
	kdDebug() << "==Kile::setupTools()===================" << endl;
	QStringList tools = KileTool::toolList(config);
	QString toolMenu;
	QPtrList<KAction> *pl;

	unplugActionList("list_compilers");
	unplugActionList("list_converters");
	unplugActionList("list_quickies");
	unplugActionList("list_viewers");
	unplugActionList("list_other");

	for ( uint i = 0; i < tools.count(); i++)
	{
		QString grp = KileTool::groupFor(tools[i], config);
		kdDebug() << tools[i] << " is using group: " << grp << endl;
		config->setGroup(KileTool::groupFor(tools[i], config));
		toolMenu = KileTool::menuFor(tools[i], config);

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
			KAction *act = new KAction(tools[i], KileTool::iconFor(tools[i], config), KShortcut(), this, SLOT(runTool()), actionCollection(), QString("tool_"+tools[i]).ascii());
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

	actionCollection()->readShortcutSettings("Shortcuts", config);
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

void Kile::restore()
{
	if (!m_bRestore) return;

	QFileInfo fi;

	for (uint i=0; i < m_listProjectsOpenOnStart.count(); i++)
	{
		kdDebug() << "restoring " << m_listProjectsOpenOnStart[i] << endl;
		fi.setFile(m_listProjectsOpenOnStart[i]);
		if (fi.isReadable())
			projectOpen(KURL::fromPathOrURL(m_listProjectsOpenOnStart[i]),
				i, m_listProjectsOpenOnStart.count());
	}

	for (uint i=0; i < m_listDocsOpenOnStart.count(); i++)
	{
		kdDebug() << "restoring " << m_listDocsOpenOnStart[i] << endl;
		fi.setFile(m_listDocsOpenOnStart[i]);
		if (fi.isReadable())
			fileOpen(KURL::fromPathOrURL(m_listDocsOpenOnStart[i]));
	}

	config->setGroup("FilesOpenOnStart");
	m_masterName = config->readEntry("Master", "");
	m_singlemode = (m_masterName == "");
	if (ModeAction) ModeAction->setChecked(!m_singlemode);
	updateModeStatus();

	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();
}

void Kile::setActive()
{
	kdDebug() << "ACTIVATING" << endl;
	kapp->mainWidget()->raise();
	kapp->mainWidget()->setActiveWindow();
}

Kate::View* Kile::load( const KURL &url , const QString & encoding, bool create, const QString & highlight, bool load, const QString &text)
{
	QString hl = highlight;

	kdDebug() << "==Kile::load==========================" << endl;
	//if doc already opened, update the structure view and return the view
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

	kdDebug() << QString("\tload(%1,%2,%3, %4)").arg(url.path()).arg(encoding).arg(create).arg(load) << endl;

	Kate::Document *doc = 0;
	
	//create a new document
	if (load)
	{
		kdDebug() << "load == true ==> creating document " << url.path() << endl;
		doc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");
		m_docList.append(doc);
	
		//set the default encoding
		QString enc = encoding.isNull() ? QString::fromLatin1(QTextCodec::codecForLocale()->name()) : encoding;
		KileFS->comboEncoding->lineEdit()->setText(enc);
		doc->setEncoding(enc);
	
		//load the contents into the doc and set the docname (we can't always use doc::url() since this returns "" for untitled documents)
		doc->openURL(url);
		//TODO: connect to completed() signal, now updatestructure is called before loading is completed
	
		if ( !url.isEmpty() ) 
		{
			doc->setDocName(url.path());
			fileOpenRecentAction->addURL(url);
		}
		else 
		{
			doc->setDocName(i18n("Untitled"));
			if (text != QString::null)
				doc->insertText(0,0,text);
		}
	}

	kdDebug() << "1 doc " << doc << endl;

	KileDocumentInfo *docinfo = 0;

	//see if this file belongs to an opened project
	//if so, make the project class aware
	KileProjectItem *item = itemFor(url);
	if (item)
	{
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
		if (doc == 0) docinfo->setURL(url);
		
		//decorate the document with the KileDocumentInfo class
		docinfo->setListView(m_kwStructure);
		docinfo->setURL(url);
		m_infoList.append(docinfo);
	}

	if (docinfo == 0)
		kdWarning() << "no docinfo for " << url.path() << endl;

	if (doc) 
	{
		mapInfo(doc, docinfo);
		setHighlightMode(doc, hl);

		//handle changes of the document
		connect(doc, SIGNAL(nameChanged(Kate::Document *)), docinfo, SLOT(emitNameChanged(Kate::Document *)));
		//why not connect doc->nameChanged directly ot this->slotNameChanged ? : the function emitNameChanged
		//updates the docinfo, on which all decisions are bases in slotNameChanged
		connect(docinfo,SIGNAL(nameChanged(Kate::Document*)), this, SLOT(slotNameChanged(Kate::Document*)));
		connect(docinfo, SIGNAL(nameChanged(Kate::Document *)), this, SLOT(newCaption()));
		connect(doc, SIGNAL(modStateChanged(Kate::Document*)), this, SLOT(newDocumentStatus(Kate::Document*)));

		if (create)
			return createView(doc);
	}

	return 0;
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

	// code completion (dani)
	connect( doc,  SIGNAL(charactersInteractivelyInserted (int,int,const QString&)), this,  SLOT(slotCharactersInserted(int,int,const QString&)) );
	connect( view, SIGNAL(completionDone()), this,  SLOT( slotCompletionDone()) );
	connect( view, SIGNAL(completionAborted()), this,  SLOT( slotCompletionAborted()) );
	connect( view, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)), this,  SLOT( slotFilterCompletion(KTextEditor::CompletionEntry*,QString *)) );

	// install a working kate part popup dialog thingy
	if (static_cast<Kate::View*>(view->qt_cast("Kate::View")))
		static_cast<Kate::View*>(view->qt_cast("Kate::View"))->installPopup((QPopupMenu*)(factory()->container("ktexteditor_popup", this)) );

	//activate the newly created view
	activateView(view, false, false);

	newStatus();
	newCaption();

	view->setFocusPolicy(QWidget::StrongFocus);
	view->setFocus();

	if ( m_currentState != "Editor" ) prepareForPart("Editor");

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
	uint l=line.toUInt(&ok,10);
	Kate::View *view = currentView();
	if (view && ok)
  	{
		this->show();
		this->raise();
		view->setFocus();
		view->gotoLineNumber(l);

		ShowEditorWidget();
		newStatus();
  	}
}

void Kile::setCursor(int parag, int index)
{
	Kate::View *view = currentView();
	if (view)
	{
		view->setCursorPositionReal(parag, index);
		view->setFocus();
	}
}

void Kile::setHighlightMode(Kate::Document * doc, const QString &highlight)
{
	kdDebug() << "==Kile::setHighlightMode()==================" << endl;

	int c = doc->hlModeCount();
	bool found = false;
	int i;

	QString hl = highlight.lower();
	QString ext = doc->url().fileName().right(4);

	KMimeType::Ptr pMime = KMimeType::findByURL(doc->url(), 0, false, true);
	kdDebug() << "\tmimeType name: " << pMime->name() << endl;

	if ( hl == QString::null && ext == ".bib" ) hl = "bibtex-kile";

	if ( (hl != QString::null) || doc->url().isEmpty() || pMime->name() == "text/x-tex" || ext == ".tex" || ext == ".ltx" || ext == ".latex" || ext == ".dtx" || ext == ".sty" || ext == ".cls")
	{
		if (hl == QString::null) hl = "latex-kile";
		for (i = 0; i < c; i++)
		{
			kdDebug() << "\tCOMPARING " << doc->hlModeName(i).lower() << " with " << hl << endl;
			if (doc->hlModeName(i).lower() == hl) { found = true; break; }
		}

		if (found)
		{
			doc->setHlMode(i);
		}
		else
		{
			//doc->setHlMode(0);
			kdWarning() << "could not find the LaTeX-Kile highlighting definitions" << endl;
		}
	}
}

void Kile::fileNew()
{
	NewFileWizard *nfw = new NewFileWizard(this);
	if (nfw->exec()) 
	{
		loadTemplate(nfw->getSelection());
	}
}

void Kile::fileNew(const KURL & url)
{
	//create an empty file
	QFile file(url.path());
	file.open(IO_ReadWrite);
	file.close();

	fileOpen(url, QString::null);
}

Kate::View* Kile::loadTemplate(TemplateItem *sel)
{
	QString text = QString::null;

	if (sel && sel->name() != DEFAULT_EMPTY_CAPTION)
	{
		//create a new document to open the template in
		Kate::Document *tempdoc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");

		if (!tempdoc->openURL(KURL(sel->path())))
		{
			KMessageBox::error(this, i18n("Couldn't find template: %1").arg(sel->name()),i18n("File Not Found!"));
		}
		else
		{
			//substitute templates variables
			text = tempdoc->text();
			delete tempdoc;
			replaceTemplateVariables(text);
		}
	}
	
	return load(KURL(), QString::null, true, QString::null, true, text);
}

//FIXME: connect to modifiedondisc() when using KDE 3.2
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

	//KParts::GUIActivateEvent ev( true );
   	//QApplication::sendEvent( view, &ev );

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
	QString filter;
	QFileInfo fi;
	if (currentView())
	{
		fi.setFile(currentView()->getDoc()->url().path());
		if (fi.exists()) currentDir= fi.dirPath();
	}

	//get the URLs
	filter.append(SOURCE_EXTENSIONS);
	filter.append(" ");
	filter.append(PACKAGE_EXTENSIONS);
	filter.replace(".","*.");
	filter.append("|");
	filter.append(i18n("TeX files"));
	filter.append("\n*|");
	filter.append(i18n("All files"));
	kdDebug() << "using FILTER " << filter << endl;
	KURL::List urls = KFileDialog::getOpenURLs( currentDir,
		filter, this,i18n("Open File(s)") );

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

	if ( !isOpen(url) )
		load(url, encoding);

	//URL wasn't open before loading, add it to the project view
	if (!isopen && (itemFor(url) == 0) ) m_projectview->add(url);

	UpdateStructure(true);
	updateModeStatus();
}


bool Kile::isOpen(const KURL & url)
{
	for ( uint i = 0; i < m_viewList.count(); i++)
	{
// 		kdDebug() << "comparing " << url.path() << " with view " << i << endl;
		if ( url == m_viewList.at(i)->getDoc()->url() )
			return true;
	}

	return false;
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
			}
		}
	}
}

void Kile::autoSaveAll()
{
	fileSaveAll(true);
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
		project = selectProject(i18n("Refresh project tree..."));

	if (project)
	{
		//TODO: update structure for all docs
		project->buildProjectTree();
	}
	else if (m_projects.count() == 0)
		KMessageBox::error(this, i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to build the tree for, then choose Refresh Project Tree again."),i18n( "Could not refresh project tree."));
}

void Kile::projectNew()
{
	kdDebug() << "==Kile::projectNew==========================" << endl;
	KileNewProjectDlg *dlg = new KileNewProjectDlg(this);
	kdDebug()<< "\tdialog created" << endl;

	if (dlg->exec())
	{
		kdDebug()<< "\tdialog executed" << endl;
		kdDebug() << "\t" << dlg->name() << " " << dlg->location() << endl;

		KileProject *project = dlg->project();

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

			//docinfo->updateStruct(m_kwStructure->level());
			UpdateStructure(true, docinfo);
		}

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
	if (project->contains(url)) return;

	KileProjectItem *item = new KileProjectItem(project, url);
	item->setOpenState(isOpen(url));
	projectOpenItem(item);
	m_projectview->add(item);
	buildProjectTree(project);
}

void Kile::removeFromProject(const KileProjectItem *item)
{
	kdDebug() << "==Kile::removeFromProject==========================" << endl;

	if (item->project())
	{
		kdDebug() << "\tprojecturl = " << item->project()->url().path() << ", url = " << item->url().path() << endl;

		if (item->project()->url() == item->url())
		{
			KMessageBox::error(this, i18n("This file is the project file, it holds all the information about your project. Therefore it is not allowed to remove this file from its project."), i18n("Cannot remove file from project"));
			return;
		}

		m_projectview->removeItem(item, isOpen(item->url()));

		KileProject *project = item->project();
		item->project()->remove(item);

		project->buildProjectTree();
	}
}

void Kile::projectOpenItem(KileProjectItem *item)
{
	kdDebug() << "==Kile::projectOpenItem==========================" << endl;
	kdDebug() << "\titem:" << item->url().path() << endl;

	KileDocumentInfo *docinfo;

	Kate::View *view = 0;
	if (isOpen(item->url())) //remove item from projectview (this file was opened before as a normal file)
		m_projectview->remove(item->url());

	view = load(item->url(),item->encoding(), item->isOpen(), item->highlight(), (item->type() == KileProjectItem::Source) || (item->type() == KileProjectItem::ProjectFile) );

	if (view) //there is a view for this projectitem, get docinfo by doc
		docinfo = infoFor(view->getDoc());
	else //there is no view for this item, get docinfo by path of this file
		docinfo = infoFor(item->url().path());

	mapItem(docinfo, item);
	UpdateStructure(false, docinfo);

	if ((!item->isOpen()) && (view != 0)) //oops, doc apparently was open while the project settings wants it closed, don't trash it the doc, update openstate instead
		item->setOpenState(true);

	if ( (!item->isOpen()) && (view == 0) ) //doc shouldn't be displayed, trash the doc
	{
		//since we've parsed it, trash the document
		trash(docinfo->getDoc());
	}

	//FIXME: workaround: remove structure of this doc from structureview (shouldn't appear there in the first place)
	m_kwStructure->takeItem(m_kwStructure->firstChild());
}

void Kile::projectOpen(const KURL &url, int step, int max)
{
	static KProgressDialog *kpd = 0;

	kdDebug() << "==Kile::projectOpen==========================" << endl;
	kdDebug() << "\tfilename: " << url.fileName() << endl;

	if (projectIsOpen(url))
	{
		if (kpd != 0)
			kpd->cancel();
		KMessageBox::information(this, i18n("The project you tried to open is already opened. If you wanted to reload the project, close the project before you re-open it."),i18n("Project already open"));
		return;
	}

	QFileInfo fi(url.path());
	if ( ! fi.isReadable() )
	{
		if (kpd != 0)
			kpd->cancel();
		if (KMessageBox::warningYesNo(this, i18n("The project file for this project does not exists or is not readable. Remove this project from the recent projects list?"),i18n("Could not load the project file"))  == KMessageBox::Yes)
			m_actRecentProjects->removeURL(url);
		return;
	}

	if (kpd == 0) {
		kpd = new KProgressDialog
			(this, 0, i18n("Open Project..."),
			QString::null, true);
		kpd->showCancelButton(false);
		kpd->setLabel(i18n("Scanning project files..."));
		kpd->setAutoClose(true);
		kpd->setMinimumDuration(2000);
	}
	kpd->show();

	kapp->processEvents();

	KileProject *kp = new KileProject(url);

	m_actRecentProjects->addURL(url);

	KileProjectItemList *list = kp->items();

	int project_steps = list->count() + 1;
	kpd->progressBar()->setTotalSteps(project_steps * max);
	project_steps *= step;
	kpd->progressBar()->setValue(project_steps);

	kdDebug() << "\t" << list->count() << " items" << endl;

	KileProjectItem *item;
	uint i = 0;
	for ( ; i < list->count(); i++)
	{
		item = list->at(i);
		projectOpenItem(item);

		kpd->progressBar()->setValue(i + project_steps);
		kapp->processEvents();
	}

	kp->buildProjectTree();
	addProject(kp);

	kpd->progressBar()->setValue(i + project_steps);
	kapp->processEvents();

	UpdateStructure();
	updateModeStatus();

	if (step == (max - 1))
		kpd->cancel();
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
	kdDebug() << "\titem==0 " << (item==0) << "\tdoc==0 " << (doc==0) << endl;
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
			if (docinfo) doc = docinfo->getDoc();
			if (doc) storeProjectItem(item, doc);
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

void Kile::toggleArchive(KileProjectItem *item)
{
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
		//QString command = project->archiveCommand();
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

		KileTool::Base *tool = new KileTool::Base("Archive", m_manager);
		tool->setSource(project->url().path());
		tool->addDict("%F", files);
		m_manager->run(tool);
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
			docinfo = 0L; doc = 0L;
			docinfo = infoFor(list->at(i));
			if (docinfo) doc = docinfo->getDoc();
			if (doc)
			{
				kdDebug() << "\t\tclosing item " << doc->url().path() << endl;
				close = close && fileClose(doc, true);
			}
			else if (docinfo) 
			{
				m_infoList.remove(docinfo);
				delete docinfo;
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

void Kile::createTemplate() 
{
	if (currentView())
	{
		if (currentView()->getDoc()->isModified() ) 
		{
			KMessageBox::information(this,i18n("Please save the file first!"));
			return;
		}
	} 
	else 
	{
		KMessageBox::information(this,i18n("Open/create a document first!"));
		return;
	}
	
	QFileInfo fi(currentView()->getDoc()->url().path());
	ManageTemplatesDialog mtd(&fi,i18n("Create Template From Document"));
	mtd.exec();
}

void Kile::removeTemplate() 
{
	ManageTemplatesDialog mtd(i18n("Remove a template."));
	mtd.exec();
}

void Kile::removeView(Kate::View *view)
{
	if (view)
	{
		guiFactory()->removeClient( view );
		tabWidget->removePage(view);
		m_viewList.remove(view);
		delete view;
	
		//if viewlist is empty, no currentChanged() signal is emitted
		//call UpdateStructure such that the structure view is emptied
		if (m_viewList.isEmpty()) UpdateStructure();
	}
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
	if (doc == 0)
		doc = activeDocument();

	if (doc == 0)
		return true;

	//TODO: remove from docinfo map, remove from dirwatch
	if (doc)
	{
		kdDebug() << "==Kile::fileClose==========================" << endl;
		kdDebug() << "\t" << doc->docName() << endl;

		KURL url = doc->url();

		KileDocumentInfo *docinfo= infoFor(doc);
		KileProjectItemList *items = itemsFor(docinfo);

		while ( items->current() )
		{
			if (items->current() && doc) storeProjectItem(items->current(),doc);
			items->next();
		}

		if (doc->closeURL() )
		{
			//KMessageBox::information(this,"closing "+url.path());
			kdDebug() << "\tclosed" << endl;
			removeView((Kate::View*)doc->views().first());
			//remove the decorations

			config->setGroup( "Files" );
			if ( config->readBoolEntry("CleanUpAfterClose") ) CleanAll(docinfo, true);

			if ( (items->count() == 0) || delDocinfo)
			{
				//doc doesn't belong to a project, get rid of the docinfo
				//or we're closing the project itself (delDocinfo is true)
				kdDebug() << "ABOUT TO REMOVE DOCINFO (" << (items->count() ==0) << "," << delDocinfo << " )" << endl;
				m_infoList.remove(docinfo);
				delete docinfo;
			}

			//remove entry in projectview
			m_projectview->remove(url);

			emit(closingDocument(docinfo));
			trash(doc);
		}
		else
			return false;
	}

	kdDebug() << "\t" << m_docList.count() << " documents open." << endl;

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
		if (!fileClose(view->getDoc())) return false;
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
	//don't close Kile if embedded viewers are present
	if ( m_currentState != "Editor" )
	{
		ResetPart();
		return false;
	}
	
	m_listProjectsOpenOnStart.clear();
	m_listDocsOpenOnStart.clear();

	for (uint i=0; i < m_projects.count(); i++)
	{
		m_listProjectsOpenOnStart.append(m_projects.at(i)->url().path());
	}

	bool stage1 = projectCloseAll();
	bool stage2 = true;

	if (stage1)
	{
		for (uint i=0; i < m_viewList.count(); i++)
		{
			m_listDocsOpenOnStart.append(m_viewList.at(i)->getDoc()->url().path());
		}
		stage2 = fileCloseAll();
	}

	return stage1 && stage2;
}

void Kile::fileSelected(const KFileItem *file)
{
	fileSelected(file->url());
}

void Kile::fileSelected(const KileProjectItem * item)
{
	fileOpen(item->url(), item->encoding());
}

void Kile::fileSelected(const KURL & url)
{
	fileOpen(url, KileFS->comboEncoding->lineEdit()->text());
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

void Kile::convertToASCII(Kate::Document *doc)
{
	if (doc == 0)
	{
		Kate::View *view = currentView();

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
		Kate::View *view = currentView();

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

void Kile::newDocumentStatus(Kate::Document *doc)
{
	if (doc)
	{
		kdDebug() << "==Kile::newDocumentStatus==========================" << endl;
		kdDebug() << "\t" << doc->docName() << endl;

		//sync terminal
		texkonsole->sync();

		QPtrList<KTextEditor::View> list = doc->views();

		QPixmap icon = doc->isModified() ? SmallIcon("filesave") : QPixmap();

		for (uint i=0; i < list.count(); i++)
		{
			//tabWidget->changeTab( list.at(i),SmallIcon(icon), getShortName(doc) );
			tabWidget->changeTab( list.at(i), icon, getShortName(doc) );
		}

		//updatestructure if active document changed from modified to unmodified (typically after a save)
		if (doc == activeDocument() && !doc->isModified())
			UpdateStructure(true);
	}
}

const QStringList* Kile::retrieveList(const QStringList* (KileDocumentInfo::*getit)() const, KileDocumentInfo * docinfo /* = 0 */)
{
	m_listTemp.clear();

	if (docinfo == 0)
	{
		docinfo = getInfo();
	}
	KileProjectItem *item = itemFor(docinfo, activeProject());

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

const QStringList* Kile::labels(KileDocumentInfo * info)
{
	kdDebug() << "Kile::labels()" << endl;
	const QStringList* (KileDocumentInfo::*p)() const=&KileDocumentInfo::labels;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* Kile::bibItems(KileDocumentInfo * info)
{
	kdDebug() << "Kile::bibItems()" << endl;
	const QStringList* (KileDocumentInfo::*p)() const=&KileDocumentInfo::bibItems;
	const QStringList* list = retrieveList(p, info);
	return list;
}

const QStringList* Kile::bibliographies(KileDocumentInfo * info)
{
	kdDebug() << "Kile::bibliographies()" << endl;
	const QStringList* (KileDocumentInfo::*p)() const=&KileDocumentInfo::bibliographies;
	const QStringList* list = retrieveList(p, info);
	return list;
}

int Kile::lineNumber()
{
	Kate::View *view = currentView();

	int para = 0;

	if (view)
	{
		para = view->cursorLine();
	}
	
	return para;
}

void Kile::newCaption()
{
	Kate::View *view = currentView();
	if (view)
	{
		setCaption(i18n("Document: %1").arg(getName(view->getDoc())));
		if (Outputview->currentPage()->inherits("KileWidget::Konsole")) texkonsole->sync();
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

void Kile::GrepItemSelected(const QString &abs_filename, int line)
{
	kdDebug() << "Open file: "
		<< abs_filename << " (" << line << ")" << endl;
	fileOpen(KURL::fromPathOrURL(abs_filename));
	setLine(QString::number(line));
}

void Kile::FindInFiles()
{
	static KileGrepDialog *dlg = 0;

	if (dlg != 0) {
		if (!dlg->isVisible())
			dlg->setDirName((activeProject() != 0)
				? activeProject()->baseURL().path()
				: QDir::home().absPath() + "/");

		dlg->show();
		return;
	}

	dlg = new KileGrepDialog
		((activeProject() != 0)
		? activeProject()->baseURL().path()
		: QDir::home().absPath() + "/");

	QString filter(SOURCE_EXTENSIONS);
	filter.append(" ");
	filter.append(PACKAGE_EXTENSIONS);
	filter.replace(".", "*.");
	filter.replace(" ", ",");
	filter.append("|");
	filter.append(i18n("TeX files"));
	filter.append("\n*|");
	filter.append(i18n("All files"));
	dlg->setFilter(filter);

	dlg->show();

	connect(dlg, SIGNAL(itemSelected(const QString &, int)),
		this, SLOT(GrepItemSelected(const QString &, int)));
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
	kdDebug() << "==Kile::ResetPart()=============================" << endl;
	kdDebug() << "\tcurrent state " << m_currentState << endl;
	kdDebug() << "\twant state " << m_wantState << endl;
	
	KParts::ReadOnlyPart *part = (KParts::ReadOnlyPart*)partManager->activePart();

	if (part && m_currentState != "Editor")
	{
		kdDebug() << "\tclosing current part" << endl;
		part->closeURL();
		partManager->removePart(part) ;
		topWidgetStack->removeWidget(part->widget());
		delete part;
	}
	
	m_currentState = "Editor";
	m_wantState = "Editor";
	partManager->setActivePart( 0L);
}

void Kile::ActivePartGUI(KParts::Part * part)
{
	kdDebug() << "==Kile::ActivePartGUI()=============================" << endl;
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

	KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject(part);
	if (ext && ext->metaObject()->slotNames().contains( "print()" ) ) //part is a BrowserExtension, connect printAction()
	{
// 		kdDebug() << "HAS BrowserExtension + print" << endl;
		connect(m_paPrint, SIGNAL(activated()), ext, SLOT(print()));
		m_paPrint->plug(toolBar("mainToolBar"),3); //plug this action into its default location
		m_paPrint->setEnabled(true);
	}
	else
	{
// 		kdDebug() << "NO BrowserExtension + print" << endl;
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
// 		kdDebug() << "\tchanged to: HTMLpreview" << endl;
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
// 		kdDebug() << "\tchanged to: Viewer" << endl;
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
// 		kdDebug() << "\tchanged to: Editor" << endl;
		stateChanged( "Editor" );
		m_wantState="Editor";
		topWidgetStack->raiseWidget(0);
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

void Kile::prepareForPart(const QString & state)
{
// 	kdDebug() << "==Kile::prepareForPart====================" << endl;
	ResetPart();

	m_wantState = state;

	//deactivate kateparts
	for (uint i=0; i<m_viewList.count(); i++)
	{
		guiFactory()->removeClient(m_viewList.at(i));
		m_viewList.at(i)->setActive(false);
	}
}

void Kile::runTool()
{
// 	kdDebug() << "==Kile::runTool()============" << endl;
	QString name = sender()->name();
	kdDebug() << "\tname: " << name << endl;
	name.replace(QRegExp("^.*tool_"), "");
	kdDebug() << "\ttool: " << name << endl;
	m_manager->run(name);
}

// changed clean dialog with selectable items (dani)

void Kile::CleanAll(KileDocumentInfo *docinfo, bool silent)
{
	static QString noactivedoc = i18n("There is no active document or it is not saved.");
	if (docinfo == 0) 
	{
		Kate::Document *doc = activeDocument();
		if (doc) 
			docinfo = infoFor(doc);
		else
		{
			LogWidget->printMsg(KileTool::Error, noactivedoc, i18n("Clean"));
			return;
		}
	}

	if (docinfo)
	{
		config->setGroup( "Files" );
		QStringList extlist, templist = config->readListEntry("CleanUpFileExtensions");
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
			LogWidget->printMsg(KileTool::Error, noactivedoc, i18n("Clean"));
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
			LogWidget->printMsg(KileTool::Warning, i18n("Nothing to clean for %1").arg(str), i18n("Clean"));
			return;
		}

		LogWidget->printMsg(KileTool::Info, i18n("cleaning %1 : %2").arg(str).arg(extlist.join(" ")), i18n("Clean"));

		docinfo->cleanTempFiles(extlist);
	}
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


void Kile::UpdateStructure(bool parse /* = false */, KileDocumentInfo *docinfo /* = 0 */)
{
// 	kdDebug() << "==Kile::UpdateStructure==========================" << endl;

	if (docinfo == 0)
		docinfo = getInfo();

	if (docinfo)
		m_kwStructure->update(docinfo, parse);

	Kate::View *view = currentView();
	if (view) {view->setFocus();}
}

//////////////// MESSAGES - LOG FILE///////////////////////
void Kile::ViewLog()
{
	Outputview->showPage(LogWidget);
	logpresent=false;

	QString cn = getCompileName();
	if ( m_outputFilter->source() !=  cn )
	{
		m_outputFilter->setSource(cn);
		m_outputFilter->Run(cn.replace(QRegExp("\\..*$"),".log"));
	}

	QString log = m_outputFilter->log();

	if (log != QString::null)
	{
		LogWidget->setText(log);
		LogWidget->highlight();
		LogWidget->scrollToBottom();
		logpresent=true;
	}
	else
	{
		LogWidget->printProblem(KileTool::Error, i18n("Cannot open log file! Did you run LaTeX?"));
	}
}

////////////////////////// ERRORS /////////////////////////////
void Kile::jumpToProblem(int type, bool forward)
{
	static LatexOutputInfoArray::iterator it;

	if (!logpresent) {ViewLog();}

	if (logpresent && !m_outputInfo->isEmpty())
	{
		Outputview->showPage(LogWidget);

		int sz = m_outputInfo->size();
		int pl = forward ? 1 : -1;

		//look for next problem of type
		for (int i=m_nCurrentError+pl; (i < sz) && (i >= 0); i += pl )
		{
			if ( (*m_outputInfo)[i].type() == type )
			{
				m_nCurrentError = i;
				int l= (*m_outputInfo)[i].outputLine();
				LogWidget->setCursorPosition(l+pl * 3 , 0);
				LogWidget->setSelection(l,0,l,LogWidget->paragraphLength(l));

				break;
			}
		}
	}

	if (logpresent && m_outputInfo->isEmpty())
	{
		LogWidget->append(i18n("No LaTeX errors detected!"));
	}

	m_bNewInfolist = false;
}

void Kile::NextError()
{
	jumpToProblem(LatexOutputInfo::itmError, true);
}

void Kile::PreviousError()
{
	jumpToProblem(LatexOutputInfo::itmError, false);
}

void Kile::NextWarning()
{
	jumpToProblem(LatexOutputInfo::itmWarning, true);
}

void Kile::PreviousWarning()
{
	jumpToProblem(LatexOutputInfo::itmWarning, false);
}

void Kile::NextBadBox()
{
	jumpToProblem(LatexOutputInfo::itmBadBox, true);
}

void Kile::PreviousBadBox()
{
	jumpToProblem(LatexOutputInfo::itmBadBox, false);
}

/////////////////////// LATEX TAGS ///////////////////
void Kile::insertTag(const KileAction::TagData& data)
{
	Kate::View *view = currentView();
	int para,index, para_end=0, para_begin, index_begin;

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
// 		kdDebug() << "before || after" << endl;
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

void Kile::insertTag(const QString& tagB, const QString& tagE, int dx, int dy)
{
	insertTag(KileAction::TagData(QString::null,tagB,tagE,dx,dy));
}

void Kile::QuickDocument()
{
	if ( !currentView() ) return;
	KileDialog::QuickDocument *dlg = new KileDialog::QuickDocument(config, this,"Quick Start",i18n("Quick Start"));
	if ( dlg->exec() )
	{
		insertTag( dlg->tagData() );
	}
	delete dlg;
}

void Kile::QuickTabular()
{
	if ( !currentView() ) return;
	KileDialog::QuickTabular *dlg = new KileDialog::QuickTabular(config, this,"Tabular", i18n("Tabular"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::QuickTabbing()
{
	if ( !currentView() ) return;
	KileDialog::QuickTabbing *dlg = new KileDialog::QuickTabbing(config, this,"Tabbing", i18n("Tabbing"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::QuickArray()
{
	if ( !currentView() ) return;
	KileDialog::QuickArray *dlg = new KileDialog::QuickArray(config, this,"Array", i18n("Array"));
	if ( dlg->exec() )
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
}

void Kile::QuickLetter()
{
	if ( !currentView() ) return;
	KileDialog::QuickLetter *dlg = new KileDialog::QuickLetter(config, this, "Letter", i18n("Letter"));
	if (dlg->exec())
	{
		insertTag(dlg->tagData());
	}
	delete dlg;
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
	QString loc = locate("html","en/kile/latexhelp.html");
	if (viewlatexhelp_command == i18n("Embedded Viewer") )
	{
		KileTool::ViewHTML *tool = dynamic_cast<KileTool::ViewHTML*>(m_toolFactory->create("ViewHTML"));
		tool->setSource(loc);
		tool->setRelativeBaseDir("");
		tool->setTarget("latexhelp.html");
		m_manager->run(tool);
	}
	else if (viewlatexhelp_command == i18n("External Browser") )
	{
// 		kdDebug() << "HTML: " << loc << endl;
		kapp->invokeBrowser(loc);
	}
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

/////////////// CONFIG ////////////////////
void Kile::ReadSettings()
{
	//test for old kilerc
	config->setGroup("VersionInfo");
	int version = config->readNumEntry("RCVersion",0);
	bool old=false;

	//reads options that can be set in the configuration dialog
	readConfig();
	
	//now read the other config data
	
	//if the kilerc file is old some of the configuration
	//date must be set by kile, even if the keys are present
	//in the kilerc file
	if ( version < KILERC_VERSION ) old=true;
	
	if ( version < 4 )
	{
		KileTool::Factory *factory = new KileTool::Factory(0,config);
		kdDebug() << "WRITING STD TOOL CONFIG" << endl;
		factory->writeStdConfig();
	}

	m_bShowUserMovedMessage = (version < 4);

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
	if ( len > 0 )
	{
 		//move the tools
		config->writeEntry("nUserTools", 0);
		for ( int i = 0; i < len; i++)
		{
			tempItem = m_listUserTools[i];
			config->setGroup("Tools");
			config->writeEntry(tempItem.name, "Default");

			KileTool::setGUIOptions(tempItem.name, "Other", "gear", config);

			config->setGroup(KileTool::groupFor(tempItem.name, "Default"));
			QString bin = KRun::binaryName(tempItem.tag, false);
			config->writeEntry("command", bin);
			config->writeEntry("options", tempItem.tag.mid(bin.length()));
			config->writeEntry("class", "Base");
			config->writeEntry("type", "Process");
			config->writeEntry("from", "");
			config->writeEntry("to", "");

			if ( i < 10 )
			{
				config->setGroup("Shortcuts");
				config->writeEntry("tool_" + tempItem.name, "Alt+Shift+" + QString::number(i + 1) ); //should be alt+shift+
			}
		}
	}

	config->setGroup( "Structure" );
	struct_level1=config->readEntry("Structure Level 1","part");
	struct_level2=config->readEntry("Structure Level 2","chapter");
	struct_level3=config->readEntry("Structure Level 3","section");
	struct_level4=config->readEntry("Structure Level 4","subsection");
	struct_level5=config->readEntry("Structure Level 5","subsubsection");
	
	config->setGroup( "Files" );
	if ( !config->hasKey("CleanUpFileExtensions") ) // no rc file or old version
	{
		QStringList extensionList;// = new QStringList();
		extensionList.append(".log");
		extensionList.append(".aux");
		extensionList.append(".dvi");
		extensionList.append(".aux");
		extensionList.append(".lof");
		extensionList.append(".lot");
		extensionList.append(".bit");
		extensionList.append(".idx");
		extensionList.append(".glo");
		extensionList.append(".bbl");
		extensionList.append(".ilg");
		extensionList.append(".toc");
		extensionList.append(".ind");
		config->writeEntry("CleanUpFileExtensions", extensionList );
	}
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
// 	kdDebug() << "==Kile::readConfig()=======================" << endl;
	
	config->setGroup( "Structure" );
	m_kwStructure->setLevel(config->readNumEntry("DefaultLevel", 1));

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
	quickmode=config->readNumEntry( "Quick Mode",1);
	latex_command=config->readEntry("Latex","latex -interaction=nonstopmode '%S.tex'");
	viewdvi_command=config->readEntry("Dvi",i18n("Embedded Viewer"));
	viewlatexhelp_command=config->readEntry("LatexHelp",i18n("Embedded Viewer"));
	dvips_command=config->readEntry("Dvips","dvips -o '%S.ps' '%S.dvi'");
	viewps_command=config->readEntry("Ps",i18n("Embedded Viewer"));
	ps2pdf_command=config->readEntry("Ps2pdf","ps2pdf '%S.ps' '%S.pdf'");
	makeindex_command=config->readEntry("Makeindex","makeindex '%S.idx'");
	bibtex_command=config->readEntry("Bibtex","bibtex '%S'");
	pdflatex_command=config->readEntry("Pdflatex","pdflatex -interaction=nonstopmode '%S.tex'");
	viewpdf_command=config->readEntry("Pdf",i18n("Embedded Viewer"));
	dvipdf_command=config->readEntry("Dvipdf","dvipdfm '%S.dvi'");
	l2h_options=config->readEntry("L2h Options","");
	bibtexeditor_command=config->readEntry("Bibtexeditor","gbib '%S.bib'");
	m_runlyxserver = config->readBoolEntry("RunLyxServer", true);

//////////////////// code completion (dani) ////////////////////
	m_complete->readConfig(config);
	m_help->readConfig();
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

	KileFS->writeConfig();
	config->setGroup( "Files" );
	if (m_viewList.last()) lastDocument = m_viewList.last()->getDoc()->url().path();
	config->writePathEntry("Last Document",lastDocument);
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

		if (!m_singlemode)
			config->writeEntry("Master", m_masterName);
		else
			config->writeEntry("Master", "");
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

config->setGroup( "Structure" );
config->writeEntry("Structure Level 1",struct_level1);
config->writeEntry("Structure Level 2",struct_level2);
config->writeEntry("Structure Level 3",struct_level3);
config->writeEntry("Structure Level 4",struct_level4);
config->writeEntry("Structure Level 5",struct_level5);

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
		logpresent=false;
		m_singlemode=true;
	}
	else if (m_singlemode && currentView())
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
}

void Kile::ToggleOutputView()
{
ShowOutputView(true);
}

void Kile::ToggleStructView()
{
ShowStructView(true);
}

void Kile::ToggleWatchFile()
{
m_bWatchFile=!m_bWatchFile;
if (m_bWatchFile) {WatchFileAction->setChecked(true);}
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
	KileDialog::Config *dlg = new KileDialog::Config(config, m_manager, this, "Configure Kile");

	if (dlg->exec())
	{
		readConfig();
		setupTools();

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
	kdDebug() <<"==Kile::spellcheck()==============" << endl;

	if ( !currentView() ) return;

  	if ( kspell )
  	{
		kdDebug() << "kspell wasn't deleted before!" << endl;
		delete kspell;
	}

	#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,0)
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
	actionCollection()->writeShortcutSettings("Shortcuts", config);
}

void Kile::ConfigureToolbars()
{
	saveMainWindowSettings(config, "KileMainWindow" );
	KEditToolbar dlg(factory());
	dlg.exec();

	showToolBars(m_currentState);
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
   m_kwStructure->hide();
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
   Structview_layout->add(m_kwStructure);
   Structview_layout->add(ButtonBar);
   ButtonBar->setPosition(KMultiVertTabBar::Right);
   m_kwStructure->show();
   }
else if (page==8)
   {
   m_projectview->hide();
   KileFS->hide();
   m_kwStructure->hide();
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
    m_kwStructure->hide();
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
      m_kwStructure->hide();
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
	deleteLater();
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

//////////////////// code completion (dani) ////////////////////

void Kile::editCompleteWord()
{
   editComplete(CodeCompletion::cmLatex);
}

void Kile::editCompleteEnvironment()
{
    editComplete(CodeCompletion::cmEnvironment);
}

void Kile::editCompleteAbbreviation()
{
   editComplete(CodeCompletion::cmAbbreviation);
}

void Kile::editComplete(CodeCompletion::Mode mode)
{
   Kate::View *view = currentView();
   if ( !view || !m_complete || !m_complete->isActive() || m_complete->inProgress() ) return;

   QString word;
   CodeCompletion::Type type;
   if ( getCompleteWord( ( mode == CodeCompletion::cmLatex ) ? true : false, word,type ) ) {
      if ( mode==CodeCompletion::cmLatex && word.at(0)!='\\' ) {
         mode = CodeCompletion::cmDictionary;
      }
      kdDebug() << "=== code completion start ====================" << endl;
      kdDebug() << "   completion word: " << word << endl;
      if ( type == CodeCompletion::ctNone )
         m_complete->completeWord(view,word,mode);
      else
         editCompleteList(view,type);
   }
}

void Kile::editCompleteList(Kate::View *view, CodeCompletion::Type type)
{
   if ( type == CodeCompletion::ctReference )
      m_complete->completeFromList( view,labels() );
   else if ( type == CodeCompletion::ctCitation )
      m_complete->completeFromList( view,bibItems() );
   // else do nothing
}

//////////////////// slots for code completion ////////////////////

void Kile::slotCompletionDone()
{
   kdDebug() << "   completion done " << endl;
   m_complete->CompletionDone();

   if ( m_complete->getMode() == CodeCompletion::cmLatex ) {
        m_completetimer = new QTimer( this );
        connect( m_completetimer, SIGNAL(timeout()),
                 this, SLOT(slotCompleteValueList()) );
        m_completetimer->start( 0, false );
   }
}

void Kile::slotCompleteValueList()
{
	kdDebug() << "   completion restart (timerslot): " << endl;
	m_completetimer->stop();
	delete m_completetimer;

	editCompleteList( currentView(), m_complete->getType());
}

void Kile::slotCompletionAborted()
{
   kdDebug() << "   completion aborted" << endl;
   m_complete->CompletionAborted();
}

void Kile::slotFilterCompletion(KTextEditor::CompletionEntry* c,QString *s)
{
   kdDebug() << "   completion filter pre:  " << *s << endl;
   *s = m_complete->filterCompletionText(c->text,c->type);
   kdDebug() << "   completion filter post:  " << *s << endl;
}

void Kile::slotCharactersInserted(int,int,const QString& string)
{
  if ( !m_complete || !m_complete->isActive() ||
        !m_complete->autoComplete()  // || m_complete->inProgress()
      )
      return;

   QString word;
   CodeCompletion::Type type;
   if ( getCompleteWord(true,word,type) && word.at(0)=='\\' ) {
      kdDebug() << "   auto completion: word=" << word << endl;
      if ( string.at(0).isLetter() ) {
         m_complete->completeWord(currentView(),word,CodeCompletion::cmLatex);
      } else if ( string.at(0) == '{' ) {
         editCompleteList( currentView(),type);
      }
   }
}

//////////////////// testing characters (dani) ////////////////////

static bool isBackslash ( QChar ch )
{
  return (ch == '\\');
}

//////////////////// das vorangehende Wort lesen ////////////////////

// (Buchstaben, je nach Modus mit/ohne Backslash)

bool Kile::getCompleteWord(bool latexmode, QString &text, CodeCompletion::Type &type)
{
    uint row,col;
    QChar ch;

    // get current position
    currentView()->cursorPositionReal(&row,&col);

    // there must be et least one sign
    if ( col < 1) return "";

    // get current text line
    QString textline = currentView()->getDoc()->textLine(row);

    //
    int n = 0;                           // number of characters
    int index = col;                     // go back from here
    while ( --index >= 0 )
    {
       // get current character
       ch = textline.at(index);

       if ( ch.isLetter() || (latexmode && (index+1==(int)col) && ch=='{') )
          n++;                           // accept letters and '{' as first character in latexmode
       else
       {
          if ( latexmode && isBackslash(ch) && oddBackslashes(textline,index) )    // Backslash?
             n++;
          break;                         // stop when a backslash was found
       }
    }

    // select pattern and set type of match
    text = textline.mid(col-n,n);
    type = m_complete->getType(text);

    return !text.isEmpty();
}

//////////////////// counting backslashes (dani) ////////////////////

bool Kile::oddBackslashes(const QString& text, int index)
{
   uint n = 0;
   while ( index>=0 && isBackslash(text.at(index)) ) {
      n++;
      index--;
   }
   return ( n % 2 ) ? true  : false;
}

//////////////////// bullet movements (dani) ////////////////////

void Kile::editNextBullet()
{
   m_edit->gotoBullet(currentView(),m_complete->getBullet(),false);
}

void Kile::editPrevBullet()
{
   m_edit->gotoBullet(currentView(),m_complete->getBullet(),true);
}

//////////////////// include graphics (dani) ////////////////////

void Kile::includeGraphics()
{
	Kate::View *view = currentView();
	if ( !view ) return;
	
	QFileInfo fi( view->getDoc()->url().path() );
	IncludegraphicsDialog *dialog = new IncludegraphicsDialog(this,config,fi.dirPath(),false);
	
	if ( dialog->exec() == QDialog::Accepted ) {
	insertTag( dialog->getTemplate(),"%C",0,0 );
	// updateStructure();
	}
	
	delete dialog;
}

//////////////////// environment commands (dani) ////////////////////

void Kile::selectEnvInside()
{
	m_edit->selectEnvironment( currentView(),true );
}

void Kile::selectEnvOutside()
{
	m_edit->selectEnvironment( currentView(),false );
}

void Kile::deleteEnvInside()
{
	m_edit->deleteEnvironment( currentView(),true );
}

void Kile::deleteEnvOutside()
{
	m_edit->deleteEnvironment( currentView(),false );
}

void Kile::gotoBeginEnv()
{
	m_edit->gotoEnvironment( currentView(),true );
}

void Kile::gotoEndEnv()
{
	m_edit->gotoEnvironment( currentView(),false );
}

void Kile::matchEnv()
{
	m_edit->matchEnvironment( currentView() );
}

void Kile::closeEnv()
{
	m_edit->closeEnvironment( currentView() );
}

//////////////////// texgroup commands (dani) ////////////////////

void Kile::selectTexgroupInside()
{
	m_edit->selectTexgroup( currentView(),true );
}

void Kile::selectTexgroupOutside()
{
	m_edit->selectTexgroup( currentView(),false );
}

void Kile::deleteTexgroupInside()
{
	m_edit->deleteTexgroup( currentView(),true );
}

void Kile::deleteTexgroupOutside()
{
	m_edit->deleteTexgroup( currentView(),false );
}

void Kile::gotoBeginTexgroup()
{
	m_edit->gotoTexgroup( currentView(),true );
}

void Kile::gotoEndTexgroup()
{
	m_edit->gotoTexgroup( currentView(),false );
}

void Kile::matchTexgroup()
{
	m_edit->matchTexgroup( currentView() );
}

void Kile::closeTexgroup()
{
	m_edit->closeTexgroup( currentView() );
}

//////////////////// select/delete commands (dani) ////////////////////

void Kile::selectParagraph()
{
	m_edit->selectParagraph( currentView() );
}

void Kile::selectLine()
{
	m_edit->selectLine( currentView() );
}

void Kile::selectWord()
{
	m_edit->selectWord( currentView(),KileEdit::smTex );
}

void Kile::deleteParagraph()
{
	m_edit->deleteParagraph( currentView() );
}

void Kile::deleteWord()
{
	m_edit->deleteWord( currentView(),KileEdit::smTex );
}

//////////////////// help commands (dani) ////////////////////


void Kile::helpTetexGuide()
{
	m_help->helpTetex(KileHelp::HelpTetexGuide);
}

void Kile::helpTetexDoc()
{
	m_help->helpTetex(KileHelp::HelpTetexDoc);
}

void Kile::helpLatexIndex()
{
	m_help->helpLatex(KileHelp::HelpLatexIndex);
}

void Kile::helpLatexCommand()
{
	m_help->helpLatex(KileHelp::HelpLatexCommand);
}

void Kile::helpLatexSubject()
{
	kdDebug() << "HELP LATEX SUBJECT" << endl;
	m_help->helpLatex(KileHelp::HelpLatexSubject);
}

void Kile::helpLatexEnvironment()
{
	m_help->helpLatex(KileHelp::HelpLatexEnvironment);
}

void Kile::helpKeyword()
{
	m_help->helpKeyword(currentView());
}

#include "kile.moc"
