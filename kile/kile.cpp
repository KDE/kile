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

#include <qfileinfo.h>
#include <qregexp.h>
#include <qiconset.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qfontdatabase.h>
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

Kile::Kile( QWidget *, const char *name ): DCOPObject( "Kile" ), KParts::MainWindow( name, WDestructiveClose), m_activeView(NULL)
{
config = KGlobal::config();
m_AutosaveTimer= new QTimer();
connect(m_AutosaveTimer,SIGNAL(timeout()),this,SLOT(autoSaveAll()));

//workaround for kdvi crash when started with Tooltips
config->setGroup("TipOfDay");
config->writeEntry( "RunOnStart",false);
setXMLFile( "kileui.rc" );
htmlpresent=false;
pspresent=false;
dvipresent=false;
watchfile=false;
partManager = new KParts::PartManager( this );
connect( partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(ActivePartGUI ( KParts::Part * ) ) );
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


Structview=new QFrame(splitter1);
Structview->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
Structview->setLineWidth( 2 );
Structview_layout=0;
ButtonBar=new KMultiVertTabBar(Structview);

ButtonBar->insertTab(SmallIcon("fileopen"),0,i18n("Open File"));
connect(ButtonBar->getTab(0),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
KileFS= new KileFileSelect(Structview,"File Selector");
connect(KileFS->dirOperator(),SIGNAL(fileSelected(const KFileItem*)),this,SLOT(fileSelected(const KFileItem*)));
connect(KileFS->comboEncoding, SIGNAL(activated(int)),this,SLOT(changeInputEncoding()));
QString currentDir=QDir::currentDirPath();
if (!lastDocument.isEmpty())
  {
  QFileInfo fi(lastDocument);
  if (fi.exists() && fi.isReadable()) currentDir=fi.dirPath();
  }
KileFS->setDir(KURL(currentDir));
KileFS->comboEncoding->lineEdit()->setText(input_encoding);

ButtonBar->insertTab(UserIcon("structure"),1,i18n("Structure"));
connect(ButtonBar->getTab(1),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
outstruct = new QListView( Structview );
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
ButtonBar->insertTab(UserIcon("metapost"),7,i18n("MetaPost Commands"));
connect(ButtonBar->getTab(7),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));

splitter2=new QSplitter(QSplitter::Vertical, splitter1, "splitter2");
tabWidget=new QTabWidget(splitter2);
tabWidget->setFocusPolicy(QWidget::ClickFocus);
tabWidget->setFocus();
connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(newCaption()) );
connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(activateView( QWidget * )) );

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
singlemode=true;
MasterName=getName();
applyMainWindowSettings(config, "KileMainWindow" );
partManager->setActivePart( 0L );
htmlpart=0L;
pspart=0L;
dvipart=0L;
m_bNewErrorlist=true;
m_bCheckForLaTeXErrors=false;

showmaintoolbar=!showmaintoolbar;ToggleShowMainToolbar();
showtoolstoolbar=!showtoolstoolbar;ToggleShowToolsToolbar();
showedittoolbar=!showedittoolbar;ToggleShowEditToolbar();
showmathtoolbar=!showmathtoolbar;ToggleShowMathToolbar();

KileApplication::closeSplash();
show();
ToggleAccels();
connect(Outputview, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(RunTerminal(QWidget * )) );

}

Kile::~Kile()
{
	kdDebug() << "cleaning up..." << endl;
	if (errorlist !=0 ) delete errorlist;
	if (m_AutosaveTimer != 0) delete m_AutosaveTimer;
}

void Kile::setupActions()
{
  (void) KStdAction::openNew(this, SLOT(fileNew()), actionCollection(), "New" );
  (void) KStdAction::open(this, SLOT(fileOpen()), actionCollection(),"Open" );
  fileOpenRecentAction = KStdAction::openRecent(this, SLOT(fileOpen(const KURL&)), actionCollection(), "Recent");
  (void) new KAction(i18n("Save All"),0, this, SLOT(fileSaveAll()), actionCollection(),"SaveAll" );
  (void) new KAction(i18n("Create Template From Document..."),0,this,SLOT(createTemplate()), actionCollection(),"CreateTemplate");
  (void) KStdAction::close(this, SLOT(fileClose()), actionCollection(),"Close" );
  (void) new KAction(i18n("Close All"),0, this, SLOT(fileCloseAll()), actionCollection(),"CloseAll" );
  (void) KStdAction::quit(this, SLOT(close()), actionCollection(),"Exit" );

  (void) KStdAction::spelling(this, SLOT(spellcheck()), actionCollection(),"Spell" );
  (void) new KAction(i18n("Refresh Structure"),"structure",0 , this, SLOT(ShowStructure()), actionCollection(),"RefreshStructure" );

  (void) new KAction(i18n("Quick Build"),"quick", Key_F1, this, SLOT(QuickBuild()), actionCollection(),"QuickBuild" );
  (void) new KAction(i18n("View Log File"),"viewlog", Key_F10, this, SLOT(ViewLog()), actionCollection(),"ViewLog" );
  (void) new KAction(i18n("Previous LaTeX Error"),"errorprev", 0, this, SLOT(PreviousError()), actionCollection(),"PreviousError" );
  (void) new KAction(i18n("Next LaTeX Error"),"errornext", 0, this, SLOT(NextError()), actionCollection(),"NextError" );
  StopAction = new KAction(i18n("&Stop"),"stop",Key_Escape,this,SIGNAL(stopProcess()),actionCollection(),"Stop");
  StopAction->setEnabled(false);
  (void) new KAction("LaTeX","latex", Key_F2, this, SLOT(Latex()), actionCollection(),"Latex" );
  (void) new KAction(i18n("View Dvi"),"viewdvi", Key_F3, this, SLOT(ViewDvi()), actionCollection(),"ViewDvi" );
  (void) new KAction(i18n("Dvi to PS"),"dvips", Key_F4, this, SLOT(DviToPS()), actionCollection(),"DvitoPS" );
  (void) new KAction(i18n("View PS"),"viewps", Key_F5, this, SLOT(ViewPS()), actionCollection(),"ViewPS" );
  (void) new KAction(i18n("PDFLaTeX"),"latex", Key_F6, this, SLOT(PDFLatex()), actionCollection(),"PDFLatex" );
  (void) new KAction(i18n("View PDF"),"viewpdf", Key_F7, this, SLOT(ViewPDF()), actionCollection(),"ViewPDF" );
  (void) new KAction(i18n("PS to PDF"),"ps2pdf", Key_F8, this, SLOT(PStoPDF()), actionCollection(),"PStoPDF" );
  (void) new KAction(i18n("DVI to PDF"),"dvipdf",Key_F9, this, SLOT(DVItoPDF()), actionCollection(),"DVItoPDF" );
  (void) new KAction(i18n("BibTeX"),Key_F11, this, SLOT(MakeBib()), actionCollection(),"MakeBib" );
  (void) new KAction(i18n("Make Index"),Key_F12, this, SLOT(MakeIndex()), actionCollection(),"MakeIndex" );
  (void) new KAction(i18n("LaTeX to HTML"),"l2h",0, this, SLOT(LatexToHtml()), actionCollection(),"LaTeXtoHtml" );
  (void) new KAction(i18n("View HTML"),"viewhtml", 0, this, SLOT(HtmlPreview()), actionCollection(),"HtmlPreview" );
  (void) new KAction(i18n("Kdvi Forward Search"),"dvisearch",0, this, SLOT(KdviForwardSearch()), actionCollection(),"KdviForwardSearch" );
  (void) new KAction(i18n("Clean"),0 , this, SLOT(CleanAll()), actionCollection(),"CleanAll" );
  (void) new KAction(i18n("Mpost"),0 , this, SLOT(MetaPost()), actionCollection(),"MetaPost" );

  (void) new KAction(i18n("Editor View"),"edit",CTRL+Key_E , this, SLOT(ShowEditorWidget()), actionCollection(),"EditorView" );
  (void) new KAction(i18n("Next Document"),"down",ALT+Key_PageDown, this, SLOT(gotoNextDocument()), actionCollection(), "gotoNextDocument" );
  (void) new KAction(i18n("Previous Document"),"up",ALT+Key_PageUp, this, SLOT(gotoPrevDocument()), actionCollection(), "gotoPrevDocument" );

  BackAction = KStdAction::back(this, SLOT(BrowserBack()), actionCollection(),"Back" );
  ForwardAction = KStdAction::forward(this, SLOT(BrowserForward()), actionCollection(),"Forward" );
  HomeAction = KStdAction::home(this, SLOT(BrowserHome()), actionCollection(),"Home" );

	QPtrList<KAction> alt_list;
	KileStdActions::setupStdTags(this, &alt_list, &labelitem);
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

  (void) new KAction(i18n("Quick Start"),"wizard",0 , this, SLOT(QuickDocument()), actionCollection(),"127" );
  (void) new KAction(i18n("Letter"),"wizard",0 , this, SLOT(QuickLetter()), actionCollection(),"128" );
  (void) new KAction(i18n("Tabular"),"wizard",0 , this, SLOT(QuickTabular()), actionCollection(),"129" );
  (void) new KAction(i18n("Tabbing"),"wizard",0 , this, SLOT(QuickTabbing()), actionCollection(),"149" );
  (void) new KAction(i18n("Array"),"wizard",0 , this, SLOT(QuickArray()), actionCollection(),"130" );

  (void) new KAction(i18n("Article in Journal"),0 , this, SLOT(InsertBib1()), actionCollection(),"131" );
  (void) new KAction(i18n("Article in Conference Proceedings"),0 , this, SLOT(InsertBib2()), actionCollection(),"132" );
  (void) new KAction(i18n("Article in Collection"),0 , this, SLOT(InsertBib3()), actionCollection(),"133" );
  (void) new KAction(i18n("Chapter or Pages in Book"),0 , this, SLOT(InsertBib4()), actionCollection(),"134" );
  (void) new KAction(i18n("Conference Proceedings"),0 , this, SLOT(InsertBib5()), actionCollection(),"135" );
  (void) new KAction(i18n("Book"),0 , this, SLOT(InsertBib6()), actionCollection(),"136" );
  (void) new KAction(i18n("Booklet"),0 , this, SLOT(InsertBib7()), actionCollection(),"137" );
  (void) new KAction(i18n("PhD. Thesis"),0 , this, SLOT(InsertBib8()), actionCollection(),"138" );
  (void) new KAction(i18n("Master's Thesis"),0 , this, SLOT(InsertBib9()), actionCollection(),"139" );
  (void) new KAction(i18n("Technical Report"),0 , this, SLOT(InsertBib10()), actionCollection(),"140" );
  (void) new KAction(i18n("Technical Manual"),0 , this, SLOT(InsertBib11()), actionCollection(),"141" );
  (void) new KAction(i18n("Unpublished"),0 , this, SLOT(InsertBib12()), actionCollection(),"142" );
  (void) new KAction(i18n("Miscellaneous"),0 , this, SLOT(InsertBib13()), actionCollection(),"143" );
  (void) new KAction(i18n("Clean"),0 , this, SLOT(CleanBib()), actionCollection(),"CleanBib" );

  (void) new KAction("Xfig","xfig",0 , this, SLOT(RunXfig()), actionCollection(),"144" );
  (void) new KAction(i18n("Gnuplot Front End"),"xgfe",0 , this, SLOT(RunGfe()), actionCollection(),"145" );

  ModeAction=new KToggleAction(i18n("Define Current Document as 'Master Document'"),"master",0 , this, SLOT(ToggleMode()), actionCollection(),"Mode" );

  MenuAccelsAction = new KToggleAction(i18n("Standard Menu Shortcuts"), 0, this,SLOT(ToggleAccels()),actionCollection(),"MenuAccels" );
  MenuAccelsAction->setChecked(m_menuaccels);

  (void) KStdAction::preferences(this, SLOT(GeneralOptions()), actionCollection(),"146" );
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

  if (singlemode) {ModeAction->setChecked(false);}
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

  (void) KStdAction::aboutApp(help_menu, SLOT(aboutApplication()), actionCollection(),"help4" );
  (void) KStdAction::aboutKDE(help_menu, SLOT(aboutKDE()), actionCollection(),"help5" );


	m_menuUserTags = new KActionMenu(i18n("User Tags"),actionCollection(),"menuUserTags");
	m_mapUserTagSignals = new QSignalMapper(this,"mapUserTagSignals");
	setupUserTagActions();
	connect(m_mapUserTagSignals,SIGNAL(mapped(int)),this,SLOT(insertUserTag(int)));

	m_menuUserTools = new KActionMenu(i18n("User Tools"), actionCollection(), "menuUserTools");
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

	KShortcut tagaccels[12] = {CTRL+SHIFT+Key_F1, SHIFT+CTRL+Key_F2,SHIFT+CTRL+Key_F3,SHIFT+CTRL+Key_F4,SHIFT+CTRL+Key_F5,SHIFT+CTRL+Key_F6,SHIFT+CTRL+Key_F7,
		SHIFT+CTRL+Key_F8,SHIFT+CTRL+Key_F9,SHIFT+CTRL+Key_F10,SHIFT+CTRL+Key_F11,SHIFT+CTRL+Key_F12};

	m_actionEditTag = new KAction(i18n("Edit User Tags"),0 , this, SLOT(EditUserMenu()), m_menuUserTags,"EditUserMenu" );
	m_menuUserTags->insert(m_actionEditTag);
	for (uint i=0; i<m_listUserTags.size(); i++)
	{
		if (i<12) sc = tagaccels[i];
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

	KShortcut toolaccels[12] = {SHIFT+ALT+Key_F1, SHIFT+ALT+Key_F2,SHIFT+ALT+Key_F3,SHIFT+ALT+Key_F4,SHIFT+ALT+Key_F5,SHIFT+ALT+Key_F6,SHIFT+ALT+Key_F7,
		SHIFT+ALT+Key_F8,SHIFT+ALT+Key_F9,SHIFT+ALT+Key_F10,SHIFT+ALT+Key_F11,SHIFT+ALT+Key_F12};

	m_actionEditTool = new KAction(i18n("Edit User Tools"),0 , this, SLOT(EditUserTool()), actionCollection(),"EditUserTool" );
	m_menuUserTools->insert(m_actionEditTool);
	for (uint i=0; i<m_listUserTools.size(); i++)
	{
		if (i<12) sc = toolaccels[i];
		else sc=0;
		name=QString::number(i+1)+": "+m_listUserTools[i].name;
		menuItem = new KAction(name,sc,m_mapUserToolsSignals,SLOT(map()), m_menuUserTools, name.ascii());
		m_listUserToolsActions.append(menuItem);
		m_menuUserTools->insert(menuItem);
		m_mapUserToolsSignals->setMapping(menuItem,i);
	}
}

////////////////////////////// FILE /////////////////////////////
Kate::View* Kile::load( const KURL &url , const QString & encoding)
{
	//create a new document and a view
	Kate::View *view;
	Kate::Document *doc = (Kate::Document*) KTextEditor::createDocument ("libkatepart", this, "Kate::Document");

	view = (Kate::View*) doc->createView (tabWidget, 0L);

	//set the default encoding
	QString enc = encoding.isNull() ? QString::fromLatin1(QTextCodec::codecForLocale()->name()) : encoding;
	KileFS->comboEncoding->lineEdit()->setText(enc);
	doc->setEncoding(enc);

	//load the contents into the doc and set the docname (we can't always use doc::url() since this returns "" for untitled documents)
	doc->openURL(url);
	if ( !url.isEmpty() )
	{
		doc->setDocName(url.path());
		fileOpenRecentAction->addURL(url);
	}
	else
	{
		//doc->openURL(KURL("file:untitled"));
		doc->setDocName("untitled");
	}

 	setHighlightMode(doc);

	//insert the view in the tab widget
	tabWidget->addTab( view, getShortName(doc) );
	tabWidget->showPage( view );
	m_viewList.append(view);

	//handle changes of the document
	connect(view, SIGNAL(viewStatusMsg(const QString&)), this, SLOT(newStatus(const QString&)));
	connect(view, SIGNAL(newStatus()), this, SLOT(newCaption()));
	connect(doc, SIGNAL(nameChanged(Kate::Document *)), this, SLOT(slotNameChanged(Kate::Document *)));
	connect(doc, SIGNAL(nameChanged(Kate::Document *)), this, SLOT(newCaption()));
	connect(doc, SIGNAL(modStateChanged(Kate::Document*)), this, SLOT(newDocumentStatus(Kate::Document*)));

	//activate the newly created view
	activateView(view);
	KParts::GUIActivateEvent ev( true );
	QApplication::sendEvent( view, &ev );

    	newStatus();
	newCaption();

	view->setFocusPolicy(QWidget::StrongFocus);
	view->setFocus();

    	ShowStructure();

	return view;
}

void Kile::slotNameChanged(Kate::Document * doc)
{
	//set the doc name so we can use the docname to set the caption
	//(we want the caption to be untitled for an new document not ""
	//doc->setDocName(doc->url().path());
	QPtrList<KTextEditor::View> list = doc->views();
	for (uint i=0; i < list.count(); i++)
	{
		tabWidget->setTabLabel((Kate::View*) list.at(i), getShortName(doc));
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
	int l=line.toInt(&ok,10);
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

void Kile::setHighlightMode(Kate::Document * doc)
{
	int c = doc->hlModeCount();
	bool found = false;
	int i;
	for (i = 0; i < c; i++)
	{
		if (doc->hlModeName(i) == "LaTeX-Kile") { found = true; break; }
	}

	if (found)
	{
		doc->setHlMode(i);
	}
	else
	{
		doc->setHlMode(0);
		kdWarning() << "could not find the LaTeX2 highlighting definitions" << endl;
	}
}

void Kile::fileNew()
{

    NewFileWizard *nfw = new NewFileWizard(this);

    if (nfw->exec()) {
	Kate::View *view = load(KURL());

	TemplateItem *sel = nfw->getSelection();

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
    }
}

void Kile::activateView(QWidget* w)  //Needs to be QWidget because of QTabWidget::currentChanged
{
	Kate::View* view = (Kate::View*)w;
	if (!view) return;
	if (view != m_activeView ) //Pointer comp. OK?!?
	{
		if (m_activeView)
		{
			guiFactory()->removeClient( m_activeView );
		}
		m_activeView=view;
		guiFactory()->addClient( view );
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
	KURL::List urls = KFileDialog::getOpenURLs( currentDir, i18n("*.ltx *.tex *.dtx *.bib *.sty *.cls *.mp|TeX files\n*|All files"), this,i18n("Open File") );

	//open them
	for (uint i=0; i < urls.count(); i++)
	{
		fileOpen(urls[i]);
	}
}


void Kile::fileOpen(const KURL& url)
{
	if ( url.path() == "untitled" || !isOpen(url)) load(url);
}


bool Kile::isOpen(const KURL & url)
{
	for ( uint i = 0; i < m_viewList.count();  i++ )
	{
		if ( m_viewList.at(i)->getDoc()->url()  == url )
		{
			tabWidget->showPage(m_viewList.at(i));
			return true;
		}
	}

	return false;
}

void Kile::fileSaveAll(bool amAutoSaving)
{
	Kate::View *view;
	QFileInfo fi;

	for (uint i = 0; i < m_viewList.count(); i++)
	{
		view = m_viewList.at(i);

		if (view && view->getDoc()->isModified())
		{
			//don't save unwritable and untitled documents when autosaving
			if ( ! (amAutoSaving && ((view->getDoc()->docName() == "untitled") || !fi.isWritable() )))
			{
				view->save();
			}
		}
	}
}

void Kile::autoSaveAll()
{
	fileSaveAll(true);
	if (singlemode)
	{
		statusBar()->changeItem(i18n("Normal mode"), ID_HINTTEXT);
	}
	else
	{
		QString shortName = MasterName;
      		int pos = shortName.findRev('/');
      		shortName.remove(0,pos+1);
		statusBar()->changeItem(i18n("Master document: %1").arg(shortName), ID_HINTTEXT);
	}


}

void Kile::enableAutosave(bool as)
{
	autosave=as;
	if (autosave) m_AutosaveTimer->start(autosaveinterval);
	else m_AutosaveTimer->stop();
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


void Kile::fileClose()
{
	Kate::View *view = currentView();
	if (view)
	{
		if (view->getDoc()->closeURL() )
		{
			guiFactory()->removeClient( view );
			m_viewList.remove(view);
			m_activeView=0;
			delete view;
		}
	}
}

bool Kile::fileCloseAll()
{
	m_activeView=0;
	Kate::View * view;
	while( ! m_viewList.isEmpty() )
    	{
		view = m_viewList.first();

		if (view->getDoc()->closeURL())
		{
			guiFactory()->removeClient( view );
			m_viewList.removeFirst();
			delete view;
		}
		else //user chose CANCEL
		{
			return false;
		}
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
	return fileCloseAll();
}

void Kile::fileSelected(const KFileItem *file)
{
    	QString encoding =KileFS->comboEncoding->lineEdit()->text();

	if (!isOpen(file->url()) )
		load(file->url(), encoding);
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
		QPtrList<KTextEditor::View> list = doc->views();
		QString icon = doc->isModified() ? "modified" : "empty";

		for (uint i=0; i < list.count(); i++)
		{
			tabWidget->changeTab( list.at(i),UserIcon(icon), getShortName(doc) );
		}
	}
}

QString Kile::getName(Kate::Document *doc)
{
	QString title;
	if (doc)
	{
		title=doc->url().path();
		if (title == "") title = doc->docName();
	}

	Kate::View *view = currentView();

	if ( view )
	{
		title=view->getDoc()->url().path();
		if (title == "") title = view->getDoc()->docName();
	}
	else
		title="";

	return title;
}

QString Kile::getShortName(Kate::Document *doc)
{
	QString title;
	if (doc)
	{
		title=doc->url().fileName();
		if (title == "") title = doc->docName();
		return title;
	}

	Kate::View *view = currentView();
	if ( view )
	{
		title=view->getDoc()->url().fileName();
		if (title == "") title = view->getDoc()->docName();
	}
	else
		title="";

	return title;
}

void Kile::newCaption()
{
	Kate::View *view = currentView();
	if (view)
	{
		setCaption(i18n("Document: %1").arg(getName(view->getDoc())));
		//UpdateStructure(); FIXME: is this necessary?
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

bool Kile::isLaTeXRoot(Kate::Document *doc)
{
	if (	!doc->text().contains("\\documentclass", true) &&
		!doc->text().contains("\\documentstyle", true))
	{
		return false;
	}

	return true;
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
  if (singlemode && !view)
  {
     KMessageBox::error( this,i18n("Could not start the %1 command, because there is no file to run %1 on.\n"
                                   "Make sure you have the file you want to compile open and saved.")
                         .arg(command).arg(command));
     return QString::null;
  }

  QString finame = getShortName();
  if (finame == "untitled" || finame == "") {
     if (KMessageBox::warningYesNo(this,i18n("You need to save an untitled document before you run %1 on it.\n"
                                             "Do you want to save it? Click Yes to save and No to abort.").arg(command),
                                   i18n("File Needs to be Saved!"))
         == KMessageBox::No) return QString::null;
  }

  //save the file before doing anything
  //attempting to save an untitled document will result in a file-save dialog pop-up
  if (view) view->save();

  //determine the name of the file to be compiled
  if (singlemode)
  {
	finame=getName();
	if (view && !isLaTeXRoot(view->getDoc()))
	{
		if (KMessageBox::warningYesNo(this,i18n("This document doesn't contain a LaTeX header.\nIt should probably be used with a master document.\nContinue anyway?"))
			== KMessageBox::No)
			return QString::null;
	}
  }
  else {
     finame=MasterName; //FIXME: MasterFile does not get saved if it is modified
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
   if (singlemode && !view)
   {
     KMessageBox::error( this,i18n("Could not start the %1 command, because there is no file to run %1 on. "
                                   "Make sure you have the source file of the file you want to convert open and saved.")
                         .arg(command).arg(command));
     return empty;
   }

   if (singlemode) {finame=getName();}
   else {
     finame=MasterName;
   }

   if (getShortName() == "untitled" || finame == "") {
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

QString Kile::prepareForViewing(const QString & /*command*/, const QString &ext, const QString &target = QString::null)
{
   Kate::View *view = currentView();
   QString finame;
   if (singlemode) {finame=getName();}
   else {
     finame=MasterName;
   }

   //warn if there is no active view
   if (singlemode && !view)
   {
     KMessageBox::error( this, i18n("Unable to determine which %1 file to show. Please open the source file of the %1 file to want to view.")
                         .arg(ext.upper()).arg(ext.upper()));
     return QString::null;
   }

   if (getShortName() == "untitled" || finame == "") {
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
		finame += target;
		finame = finame.replace("%S",fic.baseName(TRUE));
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

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
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
  LogWidget->insertLine(i18n("You must be in 'Normal mode' to use this command."));
  LogWidget->insertLine(i18n("If you do not have a TeX-binary which includes inverse search information natively :"));
  LogWidget->insertLine(i18n("- copy the files srcltx.sty and srctex.sty to the directory where your TeX-file resides."));
  LogWidget->insertLine(i18n("- add the line \\usepackage[active]{srcltx} to the preamble of your TeX-file."));
  LogWidget->insertLine(i18n("(see the kdvi handbook for more details)"));

  QFileInfo fic(finame);
  QString dviname=finame;
	QString texname;
	QString finame_cur = getName();
	QFileInfo fic_cur(finame_cur);

	if (singlemode)
		texname=fic.baseName(TRUE)+".tex";
	else
		texname=fic_cur.fileName();

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

   dvipart->openURL("file:"+finame+"#src:"+QString::number(para+1)+"./"+texname);
   }
   else
   {
    QStringList command; command << "kdvi" <<"--unique" <<"file:./%S.dvi#src:"+QString::number(para + 1)+"./%S.tex";
    CommandProcess *proc=execCommand(command,fic,false);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
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

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
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

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
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
  QString finame = getShortName();
  if (finame == "untitled") {
     KMessageBox::error(this,i18n("You need to save this file first. Then run LaTeX to create an AUX file which is required to run %1").arg(bibtex_command),
                        i18n("File needs to be saved!"));
     return;
  }

  if (singlemode) {finame=getName();}
  else {
     finame=MasterName; //FIXME: MasterFile does not get saved if it is modified
  }

  //we need to check for finame=="untitled" etc. because the user could have
  //escaped the file save dialog
  if ((singlemode && !currentView()) || finame=="")
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

  QString finame = getShortName();
  if (finame == "untitled") {
     KMessageBox::error(this,i18n("You need to save this file first. Then run LaTeX to create an idx file "
                                  "which is required to run %1.").arg(makeindex_command),
                        i18n("File needs to be saved!"));
     return;
  }

  if (singlemode) {finame=getName();}
  else {
     finame=MasterName; //FIXME: MasterFile does not get saved if it is modified
  }

  //we need to check for finame=="untitled" etc. because the user could have
  //escaped the file save dialog
  if ((singlemode && !currentView()) || finame=="")
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
  if (!currentView() ||finame=="untitled" || finame=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();

  if (singlemode) { finame= getName();}
  else { finame = MasterName;}

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

  if ((singlemode && !currentView()) ||finame=="untitled" || finame=="")
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
		if (singlemode) {finame=view->getDoc()->url().path();}
    		else {finame=MasterName;}

		if (finame == "" || finame == "untitled" ) return;

    		QFileInfo fic(finame);
  		if ( fic.isReadable() )
    		{
    			texkonsole->SetDirectory(fic.dirPath());
    			texkonsole->activate();
		}
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
//int row = (LogWidget->paragraphs() == 0)? 0 : LogWidget->paragraphs()-1;
//int col = LogWidget->paragraphLength(row);
//LogWidget->setCursorPosition(row,col);
//LogWidget->insertAt(result, row, col);
LogWidget->append(result);
//newStatus();
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

void Kile::execUserTool(int i)
{
	Kate::View *view = currentView();
	QString finame;
	QString commandline=m_listUserTools[i].tag;
	QFileInfo fi;

	bool documentpresent=true;

	if (singlemode) {finame=getName();}
	else {finame=MasterName;}
	if ((singlemode && !view) ||getShortName()=="untitled" || getShortName()=="")
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
void Kile::UpdateStructure()
{
outstruct->clear();
if ( !currentView() ) return;
QString shortName = getShortName();
if ((shortName.right(4)!=".tex") && (shortName!="untitled"))  return;

QListViewItem *top=  new QListViewItem( outstruct, shortName,"0",0 );
top->setOpen(TRUE);
top->setPixmap(0,UserIcon("doc"));
Child=lastChild=parent_level[0]=parent_level[1]=parent_level[2]=parent_level[3]=parent_level[4]=top;
structlist.clear();
structitem.clear();
labelitem.clear();
structlist.append(QString::number(0));
structitem.append(shortName);
QListViewItem *toplabel=  new QListViewItem(top,"LABELS","0",0 );
structlist.append(QString::number(0));
structitem.append("LABELS");
QString s;
for(uint i = 0; i < currentView()->getDoc()->numLines(); i++)
 {
  int tagStart, tagEnd;
 //// label ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find("\\label{", tagEnd);
 if (tagStart!=-1)
  {
    s=s.mid(tagStart+7,s.length());
    tagStart=s.find("}", tagEnd);
    if (tagStart!=-1)
    {
    s=s.mid(0,tagStart);
    labelitem.append(s);
    structlist.append(QString::number(i));
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = toplabel->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    Child=new QListViewItem( toplabel,lastChild,s );
    }
  };

 //// include ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find("\\include{", tagEnd);
 if (tagStart!=-1)
  {
    s=s.mid(tagStart+8,s.length());
    tagStart=s.find("}", tagEnd);
    if (tagStart!=-1)
    {
    s=s.mid(0,tagStart+1);
    structlist.append("include");
    structitem.append(s);
    Child = top->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    Child=new QListViewItem( top,lastChild,s );
    Child->setPixmap(0,UserIcon("include"));
    }
  };
 //// input ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find("\\input{", tagEnd);
 if (tagStart!=-1)
  {
    s=s.mid(tagStart+6,s.length());
    tagStart=s.find("}", tagEnd);
    if (tagStart!=-1)
    {
    s=s.mid(0,tagStart+1);
    structlist.append("input");
    structitem.append(s);
    Child = top->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    Child=new QListViewItem( top,lastChild,s );
    Child->setPixmap(0,UserIcon("include"));
    }
  };
 //// part ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find(QRegExp("\\\\"+struct_level1+"\\*?[\\{\\[]"), tagEnd);
 if (tagStart!=-1)
  {
    structlist.append(QString::number(i));
    tagStart=s.find(struct_level1, tagEnd);
    s=s.mid(tagStart+struct_level1.length(),s.length());
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = top->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    parent_level[0]=new QListViewItem( top,lastChild,s );
    parent_level[0]->setPixmap(0,UserIcon("part"));
    parent_level[1]=parent_level[2]=parent_level[3]=parent_level[4]=parent_level[0];
  };
 //// chapter ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find(QRegExp("\\\\"+struct_level2+"\\*?[\\{\\[]"), tagEnd);
 if (tagStart!=-1)
  {
    structlist.append(QString::number(i));
    tagStart=s.find(struct_level2, tagEnd);
    s=s.mid(tagStart+struct_level2.length(),s.length());
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = parent_level[0]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    parent_level[1]=new QListViewItem(parent_level[0],lastChild , s );
    parent_level[1]->setPixmap(0,UserIcon("chapter"));
    parent_level[2]=parent_level[3]=parent_level[4]=parent_level[1];
  };
 //// section ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find(QRegExp("\\\\"+struct_level3+"\\*?[\\{\\[]"), tagEnd);
 if (tagStart!=-1)
  {
    structlist.append(QString::number(i));
    tagStart=s.find(struct_level3, tagEnd);
    s=s.mid(tagStart+struct_level3.length(),s.length());
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = parent_level[1]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    parent_level[2]=new QListViewItem( parent_level[1],lastChild, s);
    parent_level[2]->setPixmap(0,UserIcon("section"));
    parent_level[3]=parent_level[4]=parent_level[2];
  };
 //// subsection ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find(QRegExp("\\\\"+struct_level4+"\\*?[\\{\\[]"), tagEnd);
 if (tagStart!=-1)
  {
    structlist.append(QString::number(i));
    tagStart=s.find(struct_level4, tagEnd);
    s=s.mid(tagStart+struct_level4.length(),s.length());
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = parent_level[2]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    parent_level[3]=new QListViewItem( parent_level[2],lastChild, s);
    parent_level[3]->setPixmap(0,UserIcon("subsection"));
    parent_level[4]=parent_level[3];
  };
 //// subsubsection ////
 tagStart=tagEnd=0;
 s=currentView()->getDoc()->textLine(i);
 tagStart=s.find(QRegExp("\\\\"+struct_level5+"\\*?[\\{\\[]"), tagEnd);
 if (tagStart!=-1)
  {
    structlist.append(QString::number(i));
    tagStart=s.find(struct_level5, tagEnd);
    s=s.mid(tagStart+struct_level5.length(),s.length());
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = parent_level[3]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    parent_level[4]=new QListViewItem( parent_level[3],lastChild, s);
    parent_level[4]->setPixmap(0,UserIcon("subsubsection"));
  };
 }
if (currentView() ){currentView()->setFocus();}
}

void Kile::ClickedOnStructure(QListViewItem * item)
{
//return if user didn't click on an item
if (item==0) return;

bool ok = false;
if ( !currentView() ) return;
QString it;
if ((item) && (!structlist.isEmpty()))
 {
 QStringList::ConstIterator it1 = structitem.begin();
 QStringList::ConstIterator it2 = structlist.begin();
 for ( ; it1 !=structitem.end(); ++it1 )
    {
    if (*it1==item->text(0)) {ok=true; break;}
    ++it2;
    }

if (!ok) return;

QString s=*it2;
if (s!="include" && s!="input" && s!="LABELS")
 {
 uint l=s.toInt(&ok,10);
 if (ok && l<=currentView()->getDoc()->numLines())
  {
  currentView()->setFocus();
  currentView()->setCursorPosition(l, 0);
  //newStatus();
  }
 }
 }
}

void Kile::DoubleClickedOnStructure(QListViewItem *)
{
if ( !currentView() ) return;
QListViewItem *item = outstruct->currentItem();
QString it;
if ((item) && (!structlist.isEmpty()))
 {
 QStringList::ConstIterator it1 = structitem.begin();
 QStringList::ConstIterator it2 = structlist.begin();
 for ( ; it1 !=structitem.end(); ++it1 )
    {
    if (*it1==item->text(0)) break;
    ++it2;
    }
QString s=*it2;
if (s=="include")
    {
    QString fname=*it1;
    if (fname.right(5)==".tex}") fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,fname.length()-2);
    else fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,fname.length()-2)+".tex";
    QFileInfo fi(fname);
    if (fi.exists() && fi.isReadable())
      {
      load(fname);
      }
    }
else if (s=="input")
    {
    QString fname=*it1;
    if (fname.right(5)==".tex}") fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,fname.length()-2);
    else fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,fname.length()-2)+".tex";
    QFileInfo fi(fname);
    if (fi.exists() && fi.isReadable())
      {
      load(fname);
      }
    }
}
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
	m_nErrors=m_nWarnings=0;
	m_bNewErrorlist=true;
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
					errorlist->append(num.ascii());
				}
			}
			i++;
		}
		f.close();
	}
}

void Kile::NextError()
{
	QString line="";
	bool ok;
	if (!logpresent) {ViewLog();}
	if (logpresent && !errorlist->isEmpty())
	{
		Outputview->showPage(LogWidget);
		uint id=errorlist->findRef(errorlist->current());
		if (m_bNewErrorlist)
		{
			line=errorlist->at(0);
		}
		else
		if (id < (errorlist->count()-1))
		{
			line=errorlist->at(id+1);
		}
		else
		{
			line=errorlist->at(id);
		}
		int l=line.toInt(&ok,10);
		if (ok && l<=LogWidget->paragraphs())
		{
			//LogWidget->setCursorPosition(0 , 0);
			LogWidget->setCursorPosition(l+3 , 0);
			LogWidget->setSelection(l,0,l,LogWidget->paragraphLength(l));
		}
	}

	if (logpresent && errorlist->isEmpty())
	{
		LogWidget->insertLine(i18n("No LaTeX errors detected!"));
	}

	m_bNewErrorlist=false;
}

void Kile::PreviousError()
{
	QString line="";
	bool ok;
	if (!logpresent) {ViewLog();}

	if (logpresent && !errorlist->isEmpty())
	{
		Outputview->showPage(LogWidget);
		uint id=errorlist->findRef(errorlist->current());
		if (m_bNewErrorlist)
		{
			line=errorlist->at(0);
		}
		else
		if (id>0)
		{
			line=errorlist->at(id-1);
		}
		else
		{
			line=errorlist->at(0);
		}
		int l=line.toInt(&ok,10);
		if (ok && l<=LogWidget->paragraphs())
		{
			//LogWidget->setCursorPosition(0 , 0 );
			LogWidget->setCursorPosition(l+3 , 0);
			LogWidget->setSelection(l,0,l,LogWidget->paragraphLength(l));
		}
	}

	if (logpresent && errorlist->isEmpty())
	{
		LogWidget->insertLine(i18n("No LaTeX errors detected!"));
	}

	m_bNewErrorlist=false;
}
/////////////////////// LATEX TAGS ///////////////////
void Kile::insertTag(const KileAction::TagData& data)
{
	Kate::View *view = currentView();
	int para,index;

	if ( !view ) return;

	view->setFocus();

	//whether or not to wrap tag around selection
	bool wrap = (data.tagEnd != QString::null && view->getDoc()->hasSelection());

	//save current cursor position
	para=view->cursorLine();
	index=view->cursorColumn();

	//if there is a selection act as if cursor is at the beginning of selection
	if (wrap)
	{
		index = view->getDoc()->selStartCol();
		para  = view->getDoc()->selStartLine();
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
	kdDebug() << QString("insertTag: inserting %1 at (%2,%3)").arg(ins).arg(para).arg(index) << endl;
	view->getDoc()->insertText(para,index,ins);

	//move cursor to the new position
	view->setCursorPosition(para+data.dy,index+data.dx);

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

void Kile::InsertTag(QString Entity, int dx, int dy)
{
	Kate::View *view = currentView();
	if ( !view ) return;
	int para=0;
	int index=0;
	view->setFocus();
	para=view->cursorLine();
	index=view->cursorColumn();
	view->getDoc()->insertText(para,index,Entity);
	view->setCursorPosition(para+dy,index+dx);
	LogWidget->clear();
	Outputview->showPage(LogWidget);
	logpresent=false;
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
  tag+=QString("\\begin{document}\n\n\\end{document}");
  InsertTag(tag,0,li);
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
  InsertTag(tag,0,0);
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
  tag += QString("\n\\end{tabbing} ");
  InsertTag(tag,0,2);
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
  InsertTag(tag,0,0);
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
  if (ltDlg->checkbox1->isChecked()) {InsertTag(tag,9,5);}
  else {InsertTag(tag,9,2);}
  }
  delete( ltDlg);
}

//////////////////////////// MATHS TAGS/////////////////////////////////////
void Kile::InsertSymbol()
{
QString code_symbol=symbol_view->getSymbolCode();
InsertTag(code_symbol,code_symbol.length(),0);
}

void Kile::InsertMetaPost(QListBoxItem *)
{
QString mpcode=mpview->currentText();
if (mpcode!="----------") InsertTag(mpcode,mpcode.length(),0);
}

////////////////////////// BIBLIOGRAPHY //////////////////////////
void Kile::InsertBib1()
{
QString tag = QString("@Article{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="journal = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTvolume = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTpages = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,9,0);
LogWidget->insertLine("Bib fields - Article in Journal");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib2()
{
QString tag = QString("@InProceedings{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="booktitle = {},\n";
tag+="OPTcrossref = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTpages = {},\n";
tag+="OPTyear = {},\n";
tag+="OPTeditor = {},\n";
tag+="OPTvolume = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTseries = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTorganization = {},\n";
tag+="OPTpublisher = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,15,0);
LogWidget->insertLine("Bib fields - Article in Conference Proceedings");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib3()
{
QString tag = QString("@InCollection{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="booktitle = {},\n";
tag+="OPTcrossref = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTpages = {},\n";
tag+="OPTpublisher = {},\n";
tag+="OPTyear = {},\n";
tag+="OPTeditor = {},\n";
tag+="OPTvolume = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTseries = {},\n";
tag+="OPTtype = {},\n";
tag+="OPTchapter = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTedition = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,14,0);
LogWidget->insertLine("Bib fields - Article in a Collection");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib4()
{
QString tag = QString("@InBook{,\n");
tag+="ALTauthor = {},\n";
tag+="ALTeditor = {},\n";
tag+="title = {},\n";
tag+="chapter = {},\n";
tag+="publisher = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTvolume = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTseries = {},\n";
tag+="OPTtype = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTedition = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTpages = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,8,0);
LogWidget->insertLine("Bib fields - Chapter or Pages in a Book");
LogWidget->insertLine( "ALT.... : you have the choice between these two fields");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib5()
{
QString tag = QString("@Proceedings{,\n");
tag+="title = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTeditor = {},\n";
tag+="OPTvolume = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTseries = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTorganization = {},\n";
tag+="OPTpublisher = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,13,0);
LogWidget->insertLine("Bib fields - Conference Proceedings");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib6()
{
QString tag = QString("@Book{,\n");
tag+="ALTauthor = {},\n";
tag+="ALTeditor = {},\n";
tag+="title = {},\n";
tag+="publisher = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTvolume = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTseries = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTedition = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,6,0);
LogWidget->insertLine("Bib fields - Book");
LogWidget->insertLine( "ALT.... : you have the choice between these two fields");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib7()
{
QString tag = QString("@Booklet{,\n");
tag+="title = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTauthor = {},\n";
tag+="OPThowpublished = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTyear = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,9,0);
LogWidget->insertLine("Bib fields - Booklet");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib8()
{
QString tag = QString("@PhdThesis{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="school = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTtype = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,11,0);
LogWidget->insertLine("Bib fields - PhD. Thesis");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib9()
{
QString tag = QString("@MastersThesis{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="school = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTtype = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,15,0);
LogWidget->insertLine("Bib fields - Master's Thesis");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib10()
{
QString tag = QString("@TechReport{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="institution = {},\n";
tag+="year = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTtype = {},\n";
tag+="OPTnumber = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,12,0);
LogWidget->insertLine("Bib fields - Technical Report");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib11()
{
QString tag = QString("@Manual{,\n");
tag+="title = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTauthor = {},\n";
tag+="OPTorganization = {},\n";
tag+="OPTaddress = {},\n";
tag+="OPTedition = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTyear = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,8,0);
LogWidget->insertLine("Bib fields - Technical Manual");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib12()
{
QString tag = QString("@Unpublished{,\n");
tag+="author = {},\n";
tag+="title = {},\n";
tag+="note = {},\n";
tag+="OPTkey = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTyear = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,13,0);
LogWidget->insertLine("Bib fields - Unpublished");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
void Kile::InsertBib13()
{
QString tag = QString("@Misc{,\n");
tag+="OPTkey = {},\n";
tag+="OPTauthor = {},\n";
tag+="OPTtitle = {},\n";
tag+="OPThowpublished = {},\n";
tag+="OPTmonth = {},\n";
tag+="OPTyear = {},\n";
tag+="OPTnote = {},\n";
tag+="OPTannote = {}\n";
tag+="}\n";
InsertTag(tag,6,0);
LogWidget->insertLine("Bib fields - Miscellaneous");
LogWidget->insertLine( "OPT.... : optionnal fields (use the 'Clean' command to remove them)");
}
//////////////// USER //////////////////
void Kile::insertUserTag(int i)
{
	if (m_listUserTags[i].tag.left(1)=="%")
	{
		QString t=m_listUserTags[i].tag;
		t=t.remove(0,1);
		QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
		InsertTag(s,0,1);
	}
	else
	{
		InsertTag(m_listUserTags[i].tag,0,0);
	}
}

//////////////// HELP /////////////////
void Kile::LatexHelp()
{
QFileInfo fic(locate("appdata","doc/latexhelp.html"));
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
      htmlpart->openURL(locate("appdata","doc/latexhelp.html"));
      htmlpart->addToHistory(locate("appdata","doc/latexhelp.html"));
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

singlemode=true;
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
m_menuaccels=config->readBoolEntry("MenuAccels", false);

config->setGroup( "Tools" );
quickmode=config->readNumEntry( "Quick Mode",1);
if (old)
{
	latex_command="latex -interaction=nonstopmode %S.tex";
	viewdvi_command="Embedded Viewer";
	dvips_command="dvips -o %S.ps %S.dvi";
	viewps_command="Embedded Viewer";
	ps2pdf_command="ps2pdf %S.ps %S.pdf";
	makeindex_command="makeindex %S.idx";
	bibtex_command="bibtex %S";
	pdflatex_command="pdflatex %S.tex";
	viewpdf_command="Embedded Viewer";
	dvipdf_command="dvipdfm %S.dvi";
	l2h_options="";
}
else
{
	latex_command=config->readEntry("Latex","latex -interaction=nonstopmode %S.tex");
	viewdvi_command=config->readEntry("Dvi","Embedded Viewer");
	dvips_command=config->readEntry("Dvips","dvips -o %S.ps %S.dvi");
	viewps_command=config->readEntry("Ps","Embedded Viewer");
	ps2pdf_command=config->readEntry("Ps2pdf","ps2pdf %S.ps %S.pdf");
	makeindex_command=config->readEntry("Makeindex","makeindex %S.idx");
	bibtex_command=config->readEntry("Bibtex","bibtex %S");
	pdflatex_command=config->readEntry("Pdflatex","pdflatex %S.tex");
	viewpdf_command=config->readEntry("Pdf","Embedded Viewer");
	dvipdf_command=config->readEntry("Dvipdf","dvipdfm %S.dvi");
	l2h_options=config->readEntry("L2h Options","");
	userClassList=config->readListEntry("User Class", ':');
	userPaperList=config->readListEntry("User Paper", ':');
	userEncodingList=config->readListEntry("User Encoding", ':');
	userOptionsList=config->readListEntry("User Options", ':');
}

config->setGroup( "User" );
templAuthor=config->readEntry("Author","");
templDocClassOpt=config->readEntry("DocumentClassOptions","a4paper,10pt");
templEncoding=config->readEntry("Template Encoding","");

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
}

void Kile::ReadRecentFileSettings()
{
	config->setGroup( "Files" );
	lastDocument=config->readEntry("Last Document","");
	input_encoding=config->readEntry("Input Encoding", QString::fromLatin1(QTextCodec::codecForLocale()->name()));
	autosave=config->readBoolEntry("Autosave",true);
	autosaveinterval=config->readLongNumEntry("AutosaveInterval",600000);
	enableAutosave(autosave);
	setAutosaveInterval(autosaveinterval);
	// Loading of Recent Files

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

config->setGroup("Tools");
config->writeEntry( "Quick Mode",quickmode);
config->writeEntry("Latex",latex_command);
config->writeEntry("Dvi",viewdvi_command);
config->writeEntry("Dvips",dvips_command);
config->writeEntry("Ps",viewps_command);
config->writeEntry("Ps2pdf",ps2pdf_command);
config->writeEntry("Makeindex",makeindex_command);
config->writeEntry("Bibtex",bibtex_command);
config->writeEntry("Pdflatex",pdflatex_command);
config->writeEntry("Pdf",viewpdf_command);
config->writeEntry("Dvipdf",dvipdf_command);
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
config->writeEntry("Autosave",autosave);
config->writeEntry("AutosaveInterval",autosaveinterval);

  // Store recent files
  fileOpenRecentAction->saveEntries(config,"Recent Files");

config->setGroup( "User" );
config->writeEntry("Author",templAuthor);
config->writeEntry("DocumentClassOptions",templDocClassOpt);
config->writeEntry("Template Encoding",templEncoding);

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

actionCollection()->writeShortcutSettings();
saveMainWindowSettings(config, "KileMainWindow" );
config->sync();
}

/////////////////  OPTIONS ////////////////////
void Kile::ToggleMode()
{
if (!singlemode)
     {
     ModeAction->setText(i18n("Define Current Document as 'Master Document'"));
     ModeAction->setChecked(false);
     statusBar()->changeItem(i18n("Normal mode"), ID_HINTTEXT);
     OutputWidget->clear();
     Outputview->showPage(OutputWidget);
     logpresent=false;
     singlemode=true;
     return;
     }
if (singlemode && currentView())  {
      MasterName=getName();
      if (MasterName=="untitled" || MasterName=="")
       {
       ModeAction->setChecked(false);
       KMessageBox::error( this,i18n("Could not start the command."));
       return;
       }
      QString shortName = MasterName;
      int pos;
      while ( (pos = (int)shortName.find('/')) != -1 )
      shortName.remove(0,pos+1);
      ModeAction->setText(i18n("Normal mode (current master document: %1)").arg(shortName));
      ModeAction->setChecked(true);
      statusBar()->changeItem(i18n("Master document: %1").arg(shortName), ID_HINTTEXT);
      singlemode=false;
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
	toDlg = new toolsoptionsdialog(this,"Configure Kile");

	//initialize dialog with current settings
	toDlg->asIntervalInput->setText(QString::number(autosaveinterval/60000));
	toDlg->templAuthor->setText(templAuthor);
	toDlg->templDocClassOpt->setText(templDocClassOpt);
	toDlg->templEncoding->setText(templEncoding);
	toDlg->checkAutosave->setChecked(autosave);
	toDlg->LineEdit6->setText(latex_command);
	toDlg->LineEdit7->setText(pdflatex_command);
	toDlg->comboDvi->lineEdit()->setText(viewdvi_command );
	toDlg->comboPdf->lineEdit()->setText(viewpdf_command );
	toDlg->comboPs->lineEdit()->setText(viewps_command );

	if (quickmode==1) {toDlg->checkLatex->setChecked(true);}
	if (quickmode==2) {toDlg->checkDvi->setChecked(true);}
	if (quickmode==3) {toDlg->checkDviSearch->setChecked(true);}
	if (quickmode==4)  {toDlg->checkPdflatex->setChecked(true);}
	if (quickmode==5)  {toDlg->checkDviPdf->setChecked(true);}
	if (quickmode==6)  {toDlg->checkPsPdf->setChecked(true);}

	//execute the dialog
	if (toDlg->exec())
	{
		toDlg->ksc->writeGlobalSettings ();
		autosaveinterval=60000*(toDlg->asIntervalInput->text().toLong());
		setAutosaveInterval(autosaveinterval);
		autosave=toDlg->checkAutosave->isChecked();
		enableAutosave(autosave);
		templAuthor=toDlg->templAuthor->text();
		templDocClassOpt=toDlg->templDocClassOpt->text();
		templEncoding=toDlg->templEncoding->text().stripWhiteSpace();
		if (toDlg->checkLatex->isChecked()) quickmode=1;
		if (toDlg->checkDvi->isChecked()) quickmode=2;
		if (toDlg->checkDviSearch->isChecked()) quickmode=3;
		if (toDlg->checkPdflatex->isChecked()) quickmode=4;
		if (toDlg->checkDviPdf->isChecked()) quickmode=5;
		if (toDlg->checkPsPdf->isChecked()) quickmode=6;
		viewdvi_command=toDlg->comboDvi->lineEdit()->text();
		viewps_command=toDlg->comboPs->lineEdit()->text();
		viewpdf_command=toDlg->comboPdf->lineEdit()->text();
		latex_command=toDlg->LineEdit6->text();
		pdflatex_command=toDlg->LineEdit7->text();
	}
	delete toDlg;
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

	kspell = new KSpell(this, i18n("Spellcheck"), this,SLOT( spell_started(KSpell *)));
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
	if ( currentView()->getDoc()->hasSelection() )
	{
		kspell->check(currentView()->getDoc()->selection());
	}
	else
	{
		kspell->check(currentView()->getDoc()->text());
		par_start=0;
		par_end=currentView()->getDoc()->numLines()-1;
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
  int p=pos;

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
  int p=pos;
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
  KKeyDialog::configure(actionCollection(), this, true);
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
   UpdateStructure();
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
else if (page==7)
   {
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
else
   {
      KileFS->hide();
      outstruct->hide();
      mpview->hide();
      if (symbol_view && symbol_present) delete symbol_view;
      if (Structview_layout) delete Structview_layout;
      Structview_layout=new QHBoxLayout(Structview);
      symbol_view = new SymbolView(page-1,Structview,"Symbols");
      connect(symbol_view, SIGNAL(SymbolSelected()), SLOT(InsertSymbol()));
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

#include "kile.moc"
