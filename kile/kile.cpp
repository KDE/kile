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

Kile::Kile( QWidget *, const char *name ): DCOPObject( "Kile" ), KParts::MainWindow( name, WDestructiveClose)
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
setupActions();

// Read Settings should be after setupActions() because fileOpenRecentAction needs to be
// initialized before calling ReadSettnigs().
ReadSettings();

statusBar()->insertFixedItem( i18n("Line:000000 Col: 000"), ID_LINE_COLUMN );
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
KileFS->setDir(KURL::fromPathOrURL(currentDir));
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
connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT(UpdateCaption()) );

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
UpdateCaption();
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
}

void Kile::setupActions()
{
  KSelectAction *ListAction1, *ListAction2, *ListAction3, *ListAction4, *ListAction5;
  QStringList list;

  PrintAction=KStdAction::print( 0, 0, actionCollection(), "print" );
  (void) KStdAction::openNew(this, SLOT(fileNew()), actionCollection(), "New" );
  (void) KStdAction::open(this, SLOT(fileOpen()), actionCollection(),"Open" );
  fileOpenRecentAction = KStdAction::openRecent(this, SLOT(fileOpen(const KURL&)), actionCollection(), "Recent");
  (void) KStdAction::save(this, SLOT(fileSave()), actionCollection(),"Save" );
  (void) KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection(),"SaveAs" );
  (void) new KAction(i18n("Save All"),0, this, SLOT(fileSaveAll()), actionCollection(),"SaveAll" );
  (void) new KAction(i18n("Create Template From Document..."),0,this,SLOT(createTemplate()), actionCollection(),"CreateTemplate");
  (void) new KAction(i18n("Print Source..."),"fileprint",CTRL+Key_P, this, SLOT(filePrint()), actionCollection(),"PrintSource");
  (void) KStdAction::close(this, SLOT(fileClose()), actionCollection(),"Close" );
  (void) new KAction(i18n("Close All"),0, this, SLOT(fileCloseAll()), actionCollection(),"CloseAll" );
  (void) KStdAction::quit(this, SLOT(fileExit()), actionCollection(),"Exit" );

  (void) KStdAction::undo(this, SLOT(editUndo()), actionCollection(),"Undo" );
  (void) KStdAction::redo(this, SLOT(editRedo()), actionCollection(),"Redo" );
  (void) KStdAction::copy(this, SLOT(editCopy()), actionCollection(),"Copy" );
  (void) KStdAction::cut(this, SLOT(editCut()), actionCollection(),"Cut" );
  (void) KStdAction::paste(this, SLOT(editPaste()), actionCollection(),"Paste" );
  (void) KStdAction::selectAll(this, SLOT(editSelectAll()), actionCollection(),"selectAll" );
  (void) KStdAction::spelling(this, SLOT(spellcheck()), actionCollection(),"Spell" );
  (void) new KAction(i18n("Comment Selection"),0, this, SLOT(editComment()), actionCollection(),"Comment" );
  (void) new KAction(i18n("Uncomment Selection"),0, this, SLOT(editUncomment()), actionCollection(),"Uncomment" );
  (void) new KAction(i18n("Indent Selection"),0, this, SLOT(editIndent()), actionCollection(),"Indent" );
  (void) KStdAction::find(this, SLOT(editFind()), actionCollection(),"find" );
  (void) KStdAction::findNext(this, SLOT(editFindNext()), actionCollection(),"findnext" );
  (void) KStdAction::replace(this, SLOT(editReplace()), actionCollection(),"Replace" );
  (void) new KAction(i18n("Goto Line..."),"goto",CTRL+Key_G , this, SLOT(editGotoLine()), actionCollection(),"GotoLine" );
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

  (void) new KAction("\\documentclass",0, this, SLOT(Insert1()), actionCollection(),"1" );
  (void) new KAction("\\usepackage{}",0, this, SLOT(Insert1bis()), actionCollection(),"2" );
  (void) new KAction(i18n("AMS Packages"),0, this, SLOT(Insert1ter()), actionCollection(),"3" );
  (void) new KAction("\\begin{document}",0, this, SLOT(Insert2()), actionCollection(),"4" );
  (void) new KAction("\\author{}",0, this, SLOT(Insert45()), actionCollection(),"5" );
  (void) new KAction("\\title{}",0, this, SLOT(Insert46()), actionCollection(),"6" );
  (void) new KAction("\\maketitle",0, this, SLOT(Insert51()), actionCollection(),"158" );
  (void) new KAction("\\tableofcontents",0, this, SLOT(Insert52()), actionCollection(),"159" );

  (void) new KAction("\\part","part",0 , this, SLOT(Insert3()), actionCollection(),"7" );
  (void) new KAction("\\chapter","chapter",0 , this, SLOT(Insert4()), actionCollection(),"8" );
  (void) new KAction("\\section","section",0 , this, SLOT(Insert5()), actionCollection(),"9" );
  (void) new KAction("\\subsection","subsection",0 , this, SLOT(Insert6()), actionCollection(),"10" );
  (void) new KAction("\\subsubsection","subsubsection",0 , this, SLOT(Insert6bis()), actionCollection(),"11" );
  (void) new KAction("\\paragraph",0, this, SLOT(Insert7()), actionCollection(),"12" );
  (void) new KAction("\\subparagraph",0, this, SLOT(Insert8()), actionCollection(),"13" );


  (void) new KAction("\\begin{center}","text_center",0, this, SLOT(Insert9()), actionCollection(),"14" );
  (void) new KAction("\\begin{flushleft}","text_left",0, this, SLOT(Insert10()), actionCollection(),"15" );
  (void) new KAction("\\begin{flushright}","text_right",0, this, SLOT(Insert11()), actionCollection(),"16" );
  (void) new KAction("\\begin{quote}",0, this, SLOT(Insert12()), actionCollection(),"17" );
  (void) new KAction("\\begin{quotation}",0, this, SLOT(Insert13()), actionCollection(),"18" );
  (void) new KAction("\\begin{verse}",0, this, SLOT(Insert14()), actionCollection(),"19" );
  (void) new KAction("\\begin{verbatim}",0, this, SLOT(Insert15()), actionCollection(),"20" );
  (void) new KAction("\\begin{table}",0, this, SLOT(Insert42()), actionCollection(),"21" );
  (void) new KAction("\\begin{figure}",0, this, SLOT(Insert43()), actionCollection(),"22" );
  (void) new KAction("\\begin{titlepage}",0, this, SLOT(Insert44()), actionCollection(),"23" );

  (void) new KAction("\\begin{itemize}","itemize",0, this, SLOT(Insert16()), actionCollection(),"24" );
  (void) new KAction("\\begin{enumerate}","enumerate",0, this, SLOT(Insert17()), actionCollection(),"25" );
  (void) new KAction("\\begin{description}",0, this, SLOT(Insert18()), actionCollection(),"26" );
  (void) new KAction("\\begin{list}",0, this, SLOT(Insert19()), actionCollection(),"27" );
  altH_action = new KAction("\\item","item",ALT+Key_H, this, SLOT(Insert20()), actionCollection(),"28" );

  altI_action = new KAction(i18n("\\textit - Italics"),"text_italic",ALT+Key_I, this, SLOT(Insert21()), actionCollection(),"29" );
  altA_action = new KAction(i18n("\\textsl - Slanted"),ALT+Key_A, this, SLOT(Insert22()), actionCollection(),"30" );
  altB_action = new KAction(i18n("\\textbf - Boldface"),"text_bold",ALT+Key_B, this, SLOT(Insert23()), actionCollection(),"31" );
  altT_action = new KAction(i18n("\\texttt - Typewriter"),ALT+Key_T, this, SLOT(Insert24()), actionCollection(),"32" );
  altC_action = new KAction(i18n("\\textsc - Small caps"),ALT+Key_C, this, SLOT(Insert25()), actionCollection(),"33" );

  (void) new KAction("\\begin{tabbing}",0, this, SLOT(Insert26()), actionCollection(),"34" );
  (void) new KAction("\\begin{tabular}",0, this, SLOT(Insert27()), actionCollection(),"35" );
  (void) new KAction("\\multicolumn",0, this, SLOT(Insert28()), actionCollection(),"36" );
  (void) new KAction("\\hline",0, this, SLOT(Insert29()), actionCollection(),"37" );
  (void) new KAction("\\vline",0, this, SLOT(Insert30()), actionCollection(),"38" );
  (void) new KAction("\\cline",0, this, SLOT(Insert31()), actionCollection(),"39" );

  (void) new KAction("\\newpage",0, this, SLOT(Insert32()), actionCollection(),"40" );
  (void) new KAction("\\linebreak",0, this, SLOT(Insert33()), actionCollection(),"41" );
  (void) new KAction("\\pagebreak",0, this, SLOT(Insert34()), actionCollection(),"42" );
  (void) new KAction("\\bigskip",0, this, SLOT(Insert35()), actionCollection(),"43" );
  (void) new KAction("\\medskip",0, this, SLOT(Insert36()), actionCollection(),"44" );

  (void) new KAction("\\includegraphics{file.eps}",0, this, SLOT(Insert37()), actionCollection(),"45" );
  (void) new KAction("\\include{file}","include",0 , this, SLOT(Insert37bis()), actionCollection(),"46" );
  (void) new KAction("\\input{file}","include",0 , this, SLOT(Insert37ter()), actionCollection(),"47" );

  (void) new KAction("\\bibliographystyle{}",0, this, SLOT(Insert39()), actionCollection(),"50" );
  (void) new KAction("\\bibliography{}",0, this, SLOT(Insert40()), actionCollection(),"51" );


  altM_action = new KAction("$...$","mathmode",ALT+Key_M, this, SLOT(InsertMath1()), actionCollection(),"52" );
  altE_action = new KAction("$$...$$",ALT+Key_E, this, SLOT(InsertMath2()), actionCollection(),"53" );
  (void) new KAction("\\begin{equation}",0, this, SLOT(InsertMath74()), actionCollection(),"54" );
  (void) new KAction("\\begin{eqnarray}",0, this, SLOT(InsertMath75()), actionCollection(),"55" );
  altD_action = new KAction("subscript  _{}","indice",ALT+Key_D, this, SLOT(InsertMath3()), actionCollection(),"56" );
  altU_action = new KAction("superscript  ^{}","puissance",ALT+Key_U, this, SLOT(InsertMath4()), actionCollection(),"57" );
  altF_action = new KAction("\\frac{}{}","smallfrac",ALT+Key_F, this, SLOT(InsertMath5()), actionCollection(),"58" );
  altQ_action = new KAction("\\dfrac{}{}","dfrac",ALT+Key_Q, this, SLOT(InsertMath6()), actionCollection(),"59" );
  altS_action = new KAction("\\sqrt{}","racine",ALT+Key_S, this, SLOT(InsertMath7()), actionCollection(),"60" );
  altL_action = new KAction("\\left",ALT+Key_L, this, SLOT(InsertMath8()), actionCollection(),"61" );
  altR_action = new KAction("\\right",ALT+Key_R, this, SLOT(InsertMath9()), actionCollection(),"62" );
  (void) new KAction("\\begin{array}",0, this, SLOT(InsertMath10()), actionCollection(),"63" );

  (void) new KAction("\\mathrm{}",0, this, SLOT(InsertMath66()), actionCollection(),"64" );
  (void) new KAction("\\mathit{}",0, this, SLOT(InsertMath67()), actionCollection(),"65" );
  (void) new KAction("\\mathbf{}",0, this, SLOT(InsertMath68()), actionCollection(),"66" );
  (void) new KAction("\\mathsf{}",0, this, SLOT(InsertMath69()), actionCollection(),"67" );
  (void) new KAction("\\mathtt{}",0, this, SLOT(InsertMath70()), actionCollection(),"68" );
  (void) new KAction("\\mathcal{}",0, this, SLOT(InsertMath71()), actionCollection(),"69" );
  (void) new KAction("\\mathbb{}",0, this, SLOT(InsertMath72()), actionCollection(),"70" );
  (void) new KAction("\\mathfrak{}",0, this, SLOT(InsertMath73()), actionCollection(),"71" );

  (void) new KAction("\\acute{}","acute",0 , this, SLOT(InsertMath76()), actionCollection(),"72" );
  (void) new KAction("\\grave{}","grave",0 , this, SLOT(InsertMath77()), actionCollection(),"73" );
  (void) new KAction("\\tilde{}","tilde",0 , this, SLOT(InsertMath78()), actionCollection(),"74" );
  (void) new KAction("\\bar{}","bar",0 , this, SLOT(InsertMath79()), actionCollection(),"75" );
  (void) new KAction("\\vec{}","vec",0 , this, SLOT(InsertMath80()), actionCollection(),"76" );
  (void) new KAction("\\hat{}","hat",0 , this, SLOT(InsertMath81()), actionCollection(),"77" );
  (void) new KAction("\\check{}","check",0 , this, SLOT(InsertMath82()), actionCollection(),"78" );
  (void) new KAction("\\breve{}","breve",0 , this, SLOT(InsertMath83()), actionCollection(),"79" );
  (void) new KAction("\\dot{}","dot",0 , this, SLOT(InsertMath84()), actionCollection(),"80" );
  (void) new KAction("\\ddot{}","ddot",0 , this, SLOT(InsertMath85()), actionCollection(),"81" );

  (void) new KAction("small",0, this, SLOT(InsertMath86()), actionCollection(),"82" );
  (void) new KAction("medium",0, this, SLOT(InsertMath87()), actionCollection(),"83" );
  (void) new KAction("large",0, this, SLOT(InsertMath88()), actionCollection(),"84" );
  (void) new KAction("\\quad",0, this, SLOT(InsertMath89()), actionCollection(),"85" );
  (void) new KAction("\\qquad",0, this, SLOT(InsertMath90()), actionCollection(),"86" );

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

   menuUserTags = new KActionMenu(i18n("User Tags"),actionCollection(),"menuUserTags");
   mapUserTagSignals = new QSignalMapper(this,"mapUserTagSignals");
   KAction *menuItem;

   KShortcut tagaccels[12] = {SHIFT+Key_F1, SHIFT+Key_F2,SHIFT+Key_F3,SHIFT+Key_F4,SHIFT+Key_F5,SHIFT+Key_F6,SHIFT+Key_F7,
   	SHIFT+Key_F8,SHIFT+Key_F9,SHIFT+Key_F10,SHIFT+Key_F11,SHIFT+Key_F12};

   KShortcut toolaccels[12] = {SHIFT+ALT+Key_F1, SHIFT+ALT+Key_F2,SHIFT+ALT+Key_F3,SHIFT+ALT+Key_F4,SHIFT+ALT+Key_F5,SHIFT+ALT+Key_F6,SHIFT+ALT+Key_F7,
   	SHIFT+ALT+Key_F8,SHIFT+ALT+Key_F9,SHIFT+ALT+Key_F10,SHIFT+ALT+Key_F11,SHIFT+ALT+Key_F12};

   KShortcut sc=0;
   QString name;

   menuUserTags->insert(new KAction(i18n("Edit User Tags"),0 , this, SLOT(EditUserMenu()), menuUserTags,"EditUserMenu" ));
   for (uint i=0; i<listUserTags.size(); i++)
   {
   	if (i<12) sc = tagaccels[i];
	else sc=0;
	name=QString::number(i+1)+": "+listUserTags[i].name;
	menuItem = new KAction(name,sc,mapUserTagSignals,SLOT(map()), menuUserTags, name.ascii());
	listUserTagsActions.append(menuItem);
	menuUserTags->insert(menuItem);
	mapUserTagSignals->setMapping(menuItem,i);
   }
   connect(mapUserTagSignals,SIGNAL(mapped(int)),this,SLOT(insertUserTag(int)));

   menuUserTools = new KActionMenu(i18n("User Tools"), actionCollection(), "menuUserTools");
   mapUserToolsSignals = new QSignalMapper(this,"mapUserToolsSignals");


   menuUserTools->insert(new KAction(i18n("Edit User Tools"),0 , this, SLOT(EditUserTool()), actionCollection(),"EditUserTool" ));
   for (uint i=0; i<listUserTools.size(); i++)
   {
   	if (i<12) sc = toolaccels[i];
	else sc=0;
	name=QString::number(i+1)+": "+listUserTools[i].name;
	menuItem = new KAction(name,sc,mapUserToolsSignals,SLOT(map()), menuUserTools, name.ascii());
	listUserToolsActions.append(menuItem);
	menuUserTools->insert(menuItem);
	mapUserToolsSignals->setMapping(menuItem,i);
   }
   connect(mapUserToolsSignals,SIGNAL(mapped(int)), this, SLOT(execUserTool(int)));


  (void) new KAction("Xfig","xfig",0 , this, SLOT(RunXfig()), actionCollection(),"144" );
  (void) new KAction(i18n("Gnuplot Front End"),"xgfe",0 , this, SLOT(RunGfe()), actionCollection(),"145" );

  ModeAction=new KToggleAction(i18n("Define Current Document as 'Master Document'"),"master",0 , this, SLOT(ToggleMode()), actionCollection(),"Mode" );

  MenuAccelsAction = new KToggleAction(i18n("Standard Menu Shortcuts"), 0, this,SLOT(ToggleAccels()),actionCollection(),"MenuAccels" );
  MenuAccelsAction->setChecked(menuaccels);

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

  list.clear();
  list.append("part");
  list.append("chapter");
  list.append("section");
  list.append("subsection");
  list.append("subsubsection");
  list.append("paragraph");
  list.append("subparagraph");
  ListAction1 = new KSelectAction(i18n("Sectionning"), 0, actionCollection(), "structure_list");
  ListAction1->setItems(list);
  connect(ListAction1, SIGNAL(activated(const QString&)),this,SLOT(SectionCommand(const QString&)));

  list.clear();
  list.append("tiny");
  list.append("scriptsize");
  list.append("footnotesize");
  list.append("small");
  list.append("normalsize");
  list.append("large");
  list.append("Large");
  list.append("LARGE");
  list.append("huge");
  list.append("Huge");
  ListAction2 = new KSelectAction("Size", 0, actionCollection(), "size_list");
  ListAction2->setItems(list);
  connect(ListAction2, SIGNAL(activated(const QString&)),this,SLOT(SizeCommand(const QString&)));

  (void) new KAction("\\label{}",0, this, SLOT(Insert41()), actionCollection(),"152" );
  (void) new KAction("\\ref{}",0, this, SLOT(Insert48()), actionCollection(),"153" );
  (void) new KAction("\\pageref{}",0, this, SLOT(Insert49()), actionCollection(),"154" );
  (void) new KAction("\\index{}",0, this, SLOT(Insert47()), actionCollection(),"155" );
  (void) new KAction("\\cite{}",0, this, SLOT(Insert38()), actionCollection(),"156" );
  (void) new KAction("\\footnote{}",0, this, SLOT(Insert50()), actionCollection(),"157" );
  list.clear();
  list.append("label");
  list.append("ref");
  list.append("pageref");
  list.append("index");
  list.append("cite");
  list.append("footnote");
  ListAction5 = new KSelectAction(i18n("Other"), 0, actionCollection(), "other_list");
  ListAction5->setItems(list);
  connect(ListAction5, SIGNAL(activated(const QString&)),this,SLOT(OtherCommand(const QString&)));


  (void) new KAction(i18n("Underline"),"text_under",0 , this, SLOT(InsertMath16()), actionCollection(),"150" );

  list.clear();
  list.append("left (");
  list.append("left [");
  list.append("left {");
  list.append("left <");
  list.append("left )");
  list.append("left ]");
  list.append("left }");
  list.append("left >");
  list.append("left.");
  ListAction3 = new KSelectAction(i18n("Left Delimiter"), 0, actionCollection(), "left_list");
  ListAction3->setItems(list);
  connect(ListAction3,SIGNAL(activated(const QString&)),this,SLOT(LeftDelimiter(const QString&)));


  list.clear();
  list.append("right )");
  list.append("right ]");
  list.append("right }");
  list.append("right >");
  list.append("right (");
  list.append("right [");
  list.append("right {");
  list.append("right <");
  list.append("right.");
  ListAction4 = new KSelectAction(i18n("Right Delimiter"), 0, actionCollection(), "right_list");
  ListAction4->setItems(list);
  connect(ListAction4,SIGNAL(activated(const QString&)),this,SLOT(RightDelimiter(const QString&)));

  (void) new KAction(i18n("New Line"),"newline",SHIFT+Key_Return , this, SLOT(NewLine()), actionCollection(),"151" );

  const KAboutData *aboutData = KGlobal::instance()->aboutData();
  help_menu = new KHelpMenu( this, aboutData);
  (void) new KAction(i18n("LaTeX Reference"),"help",0 , this, SLOT(LatexHelp()), actionCollection(),"help1" );
  (void) new KAction(i18n("Kile Handbook"),"contents",0 , this, SLOT(invokeHelp()), actionCollection(),"help2" );
  (void) KStdAction::aboutApp(help_menu, SLOT(aboutApplication()), actionCollection(),"help4" );
  (void) KStdAction::aboutKDE(help_menu, SLOT(aboutKDE()), actionCollection(),"help5" );

  actionCollection()->readShortcutSettings();

  setHelpMenuEnabled(false);
}

////////////////////////////// FILE /////////////////////////////
void Kile::load( const QString &f )
{
    raise();
    KWin::setActiveWindow(this->winId());
    ShowEditorWidget();
    if (FileAlreadyOpen(f) || !QFile::exists( f )) return;
    LatexEditorView *edit = new LatexEditorView( tabWidget,"",EditorFont,parenmatch,showline,editor_color);
    edit->editor->setReadOnly(false);
    edit->editor->setEncoding(input_encoding);
    edit->editor->setFile(f);

    if (wordwrap) {edit->editor->setWordWrap(LatexEditor::WidgetWidth);}
    else {edit->editor->setWordWrap(LatexEditor::NoWrap);}
    tabWidget->addTab( edit, QFileInfo( f ).fileName() );
    QFile file( f );
    if ( !file.open( IO_ReadOnly ) )
       {
       KMessageBox::sorry(this, i18n("You do not have read permission to this file."));
       return;
       }
    QTextStream ts( &file );
    QTextCodec* codec = QTextCodec::codecForName(input_encoding.latin1());
    if(!codec) codec = QTextCodec::codecForLocale();
    ts.setEncoding(QTextStream::Locale);
    ts.setCodec(codec);
    edit->editor->setText( ts.read() );
    tabWidget->showPage( edit );
    edit->editor->viewport()->setFocusPolicy(StrongFocus);
    edit->editor->viewport()->setFocus();
    filenames.replace( edit, f );
    edit->editor->setModified(false);
    doConnections( edit->editor );
    UpdateCaption();
    UpdateLineColStatus();
    fileOpenRecentAction->addURL(KURL::fromPathOrURL(f));
    ShowStructure();
}

LatexEditorView *Kile::currentEditorView() const
{
    if ( tabWidget->currentPage() &&
	 tabWidget->currentPage()->inherits( "LatexEditorView" ) )
	return (LatexEditorView*)tabWidget->currentPage();
    return 0;
}

LatexEditor *Kile::currentEditor() const
{
    if ( tabWidget->currentPage() &&
	 tabWidget->currentPage()->inherits( "LatexEditorView" ) )
	return ((LatexEditorView*)tabWidget->currentPage())->editor;
    return 0;
}

QFileInfo *Kile::currentFileInfo() const
{
    if ( tabWidget->currentPage() &&
	 tabWidget->currentPage()->inherits( "LatexEditorView" ) )
	return ((LatexEditorView*)tabWidget->currentPage())->editor->fileInfo();
    return 0;
}

void Kile::setLine( const QString &line )
{
bool ok;
int l=line.toInt(&ok,10);
if (currentEditorView() && ok)
  {
    if ( !gotoLineDialog ) gotoLineDialog = new GotoLineDialog(this, 0,TRUE );
    gotoLineDialog->SetEditor(currentEditorView()->editor);
    gotoLineDialog->show();
    gotoLineDialog->raise();
    gotoLineDialog->spinLine->setFocus();
    gotoLineDialog->spinLine->setMinValue( 1 );
    gotoLineDialog->spinLine->setMaxValue( currentEditorView()->editor->paragraphs() );
    gotoLineDialog->spinLine->setValue(l );
    gotoLineDialog->spinLine->selectAll();
  }
}

void Kile::doConnections( LatexEditor *e )
{
connect(e, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(UpdateLineColStatus()));
connect(e, SIGNAL(modificationChanged(bool)), this, SLOT(NewDocumentStatus(bool)));
}

void Kile::fileNew()
{
    NewFileWizard *nfw = new NewFileWizard(this);

    if (nfw->exec()) {
    LatexEditorView *edit = new LatexEditorView( tabWidget,"",EditorFont,parenmatch,showline,editor_color);
    edit->editor->setReadOnly(false);
    edit->editor->setEncoding(input_encoding);
    doConnections( edit->editor );
    tabWidget->addTab( edit, "untitled" );
    tabWidget->showPage( edit );
    edit->editor->viewport()->setFocusPolicy(StrongFocus);
    edit->editor->viewport()->setFocus();
    if (wordwrap) {edit->editor->setWordWrap(LatexEditor::WidgetWidth);}
    else {edit->editor->setWordWrap(LatexEditor::NoWrap);}
    filenames.replace( edit, "untitled" );
    edit->editor->setFile("untitled");
    edit->editor->setModified(false);
    doConnections( edit->editor );

    TemplateItem *sel = nfw->getSelection();
    kdDebug() << "filenew start read" << endl;
    if (sel->name() != DEFAULT_EMPTY_CAPTION) {
       QFile f(sel->path());
       if (f.open(IO_ReadOnly) ) {
       QString line;
       while (f.readLine(line,80)>0) {
          replaceTemplateVariables(line);
          edit->editor->append(line);
       }
       f.close();
       } else { KMessageBox::error(this, i18n("Couldn't find template: %1").arg(sel->name()),i18n("File Not Found!")); }
    }
    kdDebug() << "filenew end read" << endl;

    UpdateCaption();
    UpdateLineColStatus();
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
	QString currentDir=KileFS->dirOperator()->url().path();
	QFileInfo *fi = currentFileInfo();
	if (fi != 0)
	{
		if (fi->exists() && fi->isReadable()) currentDir=fi->dirPath();
	}
	QString fn = KFileDialog::getOpenFileName( currentDir, i18n("*.ltx *.tex *.dtx *.bib *.sty *.cls *.mp|TeX files\n*|All files"), this,i18n("Open File") );
	if ( !fn.isEmpty() ) load( fn );
}

void Kile::fileOpen(const KURL& url)
{
  // Convert from URL to QString
  // TODO : QString everywhere should be replaced by KURL eventually!
  QString fn;
  fn = url.path();
  kdDebug() << "DEBUG: Got Path: " << fn << endl;
  load( fn );
}


bool Kile::FileAlreadyOpen(const QString &f)
{
bool rep=false;
FilesMap::Iterator it;
for( it = filenames.begin(); it != filenames.end(); ++it )
 {
 if (filenames[it.key()]==f)
    {
     tabWidget->showPage( it.key() );
     rep=true;
    }
 }
return rep;
}

void Kile::fileSave(bool amAutoSaving )
{
	if ( !currentEditorView() )	return;
	QString fn;
	if ( getShortName()=="untitled" )
	{
		if (!amAutoSaving) fileSaveAs();
	}
	else
	{
		QFile file( *filenames.find( currentEditorView() ) );
		if (amAutoSaving)
		{
			file.setName(file.name()+i18n(".backup"));
			statusBar()->changeItem(i18n("Autosaving %1 ...").arg(file.name()),ID_HINTTEXT);
		}

		if ( !file.open( IO_WriteOnly ) )
		{
      if(!amAutoSaving) {
        // do not annoy user by giving error messages if auto save is not possible
  			QString cap = i18n("Sorry");
	  		KMessageBox::sorry(this,i18n("The file could not be saved. Please check if you have write permission."),cap);
      }
			return;
		}

		QTextStream ts( &file );
		ts.setEncoding(QTextStream::Locale);
		QTextCodec* codec = QTextCodec::codecForName(currentEditorView()->editor->getEncoding().latin1());
		ts.setCodec(codec ? codec : QTextCodec::codecForLocale());
		ts << currentEditorView()->editor->text();

		if (!amAutoSaving)
		{
			currentEditorView()->editor->setModified(false);
			fn=getName();
			fileOpenRecentAction->addURL(KURL::fromPathOrURL(fn));
		}
	}
	UpdateCaption();
	UpdateLineColStatus();
}

void Kile::fileSaveAs()
{
	int query=KMessageBox::Yes;
	if ( !currentEditorView() ) 	return;
	QString fn = KFileDialog::getSaveFileName( QString::null,i18n("*.dtx *.ltx *.tex *.bib *.sty *.cls *.mp|TeX Files\n*|All Files"), this,i18n("Save As") );
	if ( !fn.isEmpty() )
	{
		currentEditor()->setFile(fn);
		QFileInfo fic(fn);
		if( fic.exists() ) query = KMessageBox::warningYesNoCancel( this,i18n( "A document with this name already exists.\nDo you want to overwrite it?" ) );
		if (query==KMessageBox::Yes)
		{
			filenames.replace( currentEditorView(), fn );
			fileSave();
			tabWidget->setTabLabel( currentEditorView(), fic.fileName() );
		}
	}
	UpdateCaption();
	UpdateLineColStatus();
}

void Kile::fileSaveAll(bool amAutoSaving)
{
LatexEditorView *temp = new LatexEditorView( tabWidget,"",EditorFont,parenmatch,showline,editor_color);
temp=currentEditorView();
FilesMap::Iterator it;
for( it = filenames.begin(); it != filenames.end(); ++it )
   {
   tabWidget->showPage( it.key() );
   fileSave(amAutoSaving);
   }
tabWidget->showPage(temp);
UpdateCaption();
UpdateLineColStatus();
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
   if (currentEditorView()){
      if (currentEditorView()->editor->isModified() ) {
      KMessageBox::information(this,i18n("Please save the file first!"));
      return;
      }
   } else {
      KMessageBox::information(this,i18n("Open/create a document first!"));
      return;
   }

   QFileInfo *fi=currentFileInfo();
   ManageTemplatesDialog mtd(fi,i18n("Create Template From Document"));
   mtd.exec();
}

void Kile::removeTemplate() {
	ManageTemplatesDialog mtd(i18n("Remove a template."));
	mtd.exec();
}

void Kile::filePrint()
{
KPrinter printer;
QString finame;
if (!htmlpresent )
 {
    finame=getName();
    if ( (!currentEditorView()) ||  getShortName() =="" )
    {
    	KMessageBox::error(this,i18n("Please open or create the document you want to print first!"));
	return;
    }

    if (getShortName()=="untitled")
    {
      if (KMessageBox::warningYesNo( this,
      i18n("You will have to save the document before you can print it. Save it now?"),
      "Kile",i18n("Save"),i18n("Don't save"))
      == KMessageBox::No) return;
    }
    fileSave();
    //finame=getName();
    //QFileInfo fic(finame);
    QFileInfo *fic = currentFileInfo();
    if (fic->exists() && fic->isReadable() )
       {
         if ( !printer.setup(this, i18n("Print %1").arg(getShortName())) )
	 {
		return;
	 }
	 else
         {
         QSize margins = printer.margins();
         int marginHeight = margins.height();
         QPainter p;
         if( !p.begin( &printer ) )
	 {
		KMessageBox::error(this,i18n("Could not connect to the printer!"));
	 	return;
	 }
         int yPos        = 0;
         p.setFont( EditorFont );
         QFontMetrics fm = p.fontMetrics();
         QPaintDeviceMetrics metrics( &printer );
         QFile f( fic->absFilePath() );
         if ( !f.open( IO_ReadOnly ) ) return;
         QTextStream t(&f);
         while ( !t.eof() )
           {
             QString s = t.readLine();
             if ( marginHeight + yPos > metrics.height() - marginHeight )
             {
               printer.newPage();
               yPos = 0;
             }
             p.drawText( marginHeight, marginHeight + yPos,metrics.width(), fm.lineSpacing(),ExpandTabs,s );
             yPos = yPos + fm.lineSpacing();
           }
           f.close();
         p.end();
         }
         UpdateLineColStatus();
       }
   }
}

void Kile::fileClose()
{
if ( !currentEditorView() )	return;
if (currentEditorView()->editor->isModified())
{
   switch(  KMessageBox::warningYesNoCancel(this,
				     i18n("The current document (%1) has been modified.\nDo you want to save it before closing?").arg(getName()),"Kile",
				     i18n("Save"), i18n("Don't Save") ) )
   {
       case (KMessageBox::Yes):
	   fileSave();
     filenames.remove(currentEditorView());
	   delete currentEditorView();
	   break;
       case (KMessageBox::No):
     filenames.remove(currentEditorView());
	   delete currentEditorView();
	   break;
       case (KMessageBox::Cancel):
       default:
	   return;
	   break;
   }

}
else
{
filenames.remove(currentEditorView());
delete currentEditorView();
}

if ( currentEditorView() )	currentEditorView()->editor->viewport()->setFocus();
UpdateCaption();
UpdateLineColStatus();
}

void Kile::fileCloseAll()
{
bool go=true;
while (currentEditorView() && go)
 {
	if (currentEditorView()->editor->isModified())
      {
   switch(  KMessageBox::warningYesNoCancel(this,
				     i18n("The current document has been modified.\nDo you want to save it before closing?"),"Kile",
				     i18n("Save"), i18n("Don't Save") ) )
          {
          case (KMessageBox::Yes):
       	    fileSave();
            filenames.remove(currentEditorView());
       	    delete currentEditorView();
            break;
          case (KMessageBox::No):
            filenames.remove(currentEditorView());
       	    delete currentEditorView();
            break;
          case (KMessageBox::Cancel):
          default:
            go=false;
       	    return;
       	    break;
          }
			}
	else
			{
      filenames.remove(currentEditorView());
      delete currentEditorView();
      }

 }
UpdateCaption();
UpdateLineColStatus();
}

void Kile::fileExit()
{
SaveSettings();
bool accept=true;
    while (currentEditorView() && accept)
    {
      if (currentEditorView()->editor->isModified())
      {
   switch(  KMessageBox::warningYesNoCancel(this,
				     i18n("The current document has been modified.\nDo you want to save it before closing?"),"Kile",
				     i18n("Save"), i18n("Don't Save") ) )
      {
       case (KMessageBox::Yes):
	     fileSave();
       filenames.remove(currentEditorView());
	     delete currentEditorView();
	     break;
       case (KMessageBox::No):
       filenames.remove(currentEditorView());
	     delete currentEditorView();
	     break;
       case (KMessageBox::Cancel):
       default:
	     accept=false;
	     break;
     }
     }
     else
        {
        filenames.remove(currentEditorView());
        delete currentEditorView();
        }
    }
    if (accept) qApp->quit();
}

void Kile::closeEvent(QCloseEvent *e)
{
SaveSettings();
bool accept=true;
    while (currentEditorView() && accept)
    {
      if (currentEditorView()->editor->isModified())
      {
   switch(  KMessageBox::warningYesNoCancel(this,
				     i18n("The current document has been modified.\nDo you want to save it before closing?"),"Kile",
				     i18n("Save"), i18n("Don't Save") ) )
      {
       case (KMessageBox::Yes):
	     fileSave();
       filenames.remove(currentEditorView());
	     delete currentEditorView();
	     break;
       case (KMessageBox::No):
       filenames.remove(currentEditorView());
	     delete currentEditorView();
	     break;
       case (KMessageBox::Cancel):
       default:
	     accept=false;
	     break;
     }
     }
     else
        {
         filenames.remove(currentEditorView());
         delete currentEditorView();
         }
    }
    if (accept) e->accept();
}

void Kile::fileSelected(const KFileItem *file)
{
    QString sa =file->url().prettyURL() ;
    if ( sa.left(5) == "file:" ) sa = sa.remove(0, 5);
    input_encoding =KileFS->comboEncoding->lineEdit()->text();
    load(sa );
}
//////////////////////////// EDIT ///////////////////////
void Kile::editUndo()
{
    if ( !currentEditorView() )
	return;
    currentEditorView()->editor->undo();
}

void Kile::editRedo()
{
    if ( !currentEditorView() )
	return;
    currentEditorView()->editor->redo();
}

void Kile::editCut()
{
    if ( !currentEditorView() )
	return;
    currentEditorView()->editor->cut();
}

void Kile::editCopy()
{
    if ( !currentEditorView() )
	return;
    currentEditorView()->editor->copy();
}

void Kile::editPaste()
{
    if ( !currentEditorView() )
	return;
    currentEditorView()->editor->paste();
}

void Kile::editSelectAll()
{
    if ( !currentEditorView() )
	return;
    currentEditorView()->editor->selectAll(true);
}

void Kile::editFind()
{
    if ( !currentEditorView() )	return;
    if (!findDialog) findDialog = new FindDialog(this, 0, TRUE );
    findDialog->SetEditor(currentEditorView()->editor);
    findDialog->show();
    findDialog->raise();
    findDialog->comboFind->setFocus();
    findDialog->comboFind->lineEdit()->selectAll();
}

void Kile::editFindNext()
{
    if ( !currentEditorView() )	return;
    if (!findDialog)
       {
       findDialog = new FindDialog(this, 0, true );
       findDialog->SetEditor(currentEditorView()->editor);
       findDialog->show();
       findDialog->raise();
       findDialog->comboFind->setFocus();
       findDialog->comboFind->lineEdit()->selectAll();
       }
    else
       {
       findDialog->SetEditor(currentEditorView()->editor);
       findDialog->doFind();
       }
}


void Kile::editReplace()
{
    if ( !currentEditorView() )	return;
    if ( !replaceDialog )  replaceDialog = new ReplaceDialog(this, 0, TRUE );
    replaceDialog->SetEditor(currentEditorView()->editor);
    replaceDialog->show();
    replaceDialog->raise();
    replaceDialog->comboFind->setFocus();
    replaceDialog->comboFind->lineEdit()->selectAll();
}

void Kile::editGotoLine()
{
    if ( !currentEditorView() )	return;
    if ( !gotoLineDialog ) gotoLineDialog = new GotoLineDialog(this, 0,TRUE );
    gotoLineDialog->SetEditor(currentEditorView()->editor);
    gotoLineDialog->show();
    gotoLineDialog->raise();
    gotoLineDialog->spinLine->setFocus();
    gotoLineDialog->spinLine->setMinValue( 1 );
    gotoLineDialog->spinLine->setMaxValue( currentEditorView()->editor->paragraphs() );
    gotoLineDialog->spinLine->selectAll();
}

void Kile::editComment()
{
    if ( !currentEditorView() )	return;
    currentEditorView()->editor->commentSelection();
    UpdateLineColStatus();
}

void Kile::editUncomment()
{
    if ( !currentEditorView() )	return;
    currentEditorView()->editor->uncommentSelection();
    UpdateLineColStatus();
}

void Kile::editIndent()
{
    if ( !currentEditorView() )	return;
    currentEditorView()->editor->indentSelection();
    UpdateLineColStatus();
}
////////////////// GENERAL SLOTS //////////////
void Kile::UpdateLineColStatus()
{
 if ( !currentEditorView() )
  {
  statusBar()->changeItem( i18n("Line: 1 Col: 1"), ID_LINE_COLUMN );
  }
else
  {
  QString linenumber;
	int para=0;
  int index=0;
  currentEditorView()->editor->viewport()->setFocus();
  currentEditorView()->editor->getCursorPosition( &para, &index);
  linenumber = i18n("Line: %1 Col: %2").arg(para + 1).arg(index + 1);
  statusBar()->changeItem(linenumber, ID_LINE_COLUMN);
  }
}

void Kile::NewDocumentStatus(bool m)
{
if ( !currentEditorView() )	return;
if (m) tabWidget->changeTab( currentEditorView(),UserIcon("modified"), QFileInfo( getName() ).fileName() );
else tabWidget->changeTab( currentEditorView(),UserIcon("empty"), QFileInfo( getName() ).fileName() );
}

QString Kile::getName() const
{
QString title;
//if ( !currentEditorView() )	{title="";}
//else {title=filenames[currentEditorView()];}
if ( !currentEditorView() )	{title="";}
else
{
	if (currentEditor()->fileInfo() != 0)
	{
		title=currentEditor()->fileInfo()->absFilePath();
	}
	else
	{
		title="";
	}
}
return title;
}

QString Kile::getShortName() const
{
QString title;
//if ( !currentEditorView() )	{title="";}
//else {title=filenames[currentEditorView()];}
if ( !currentEditorView() )	{title="";}
else
{
	if (currentEditor()->fileInfo() != 0)
	{
		title=currentEditor()->fileInfo()->fileName();
	}
	else
	{
		title="";
	}
}
return title;
}

void Kile::UpdateCaption()
{
QString title;
if   ( !currentEditorView() )	{title=i18n("No Document");}
else
   {
   title=i18n("Document: %1").arg(getName());
   input_encoding=currentEditorView()->editor->getEncoding();
   KileFS->comboEncoding->lineEdit()->setText(input_encoding);
   }
setCaption(title);
UpdateStructure();
if (Outputview->currentPage()->inherits("TexKonsoleWidget")) syncTerminal();
/*if (singlemode)
 {
 OutputWidget->clear();
 LogWidget->clear();
 logpresent=false;
 }
 */
UpdateLineColStatus();
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
UpdateLineColStatus();
UpdateCaption();
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
    toolBar("ToolBar1")->hide();
    toolBar("ToolBar2")->hide();
    toolBar("Extra")->show();
    toolBar("ToolBar4")->hide();
    toolBar("ToolBar5")->hide();
    }
    else if (pspresent && pspart)
    {
    stateChanged( "State2" );
    toolBar("ToolBar1")->hide();
    toolBar("ToolBar2")->hide();
    toolBar("Extra")->show();
    toolBar("ToolBar4")->hide();
    toolBar("ToolBar5")->hide();
    }
    else if (dvipresent && dvipart)
    {
    stateChanged( "State3" );
    toolBar("ToolBar1")->hide();
    toolBar("ToolBar2")->hide();
    toolBar("Extra")->show();
    toolBar("ToolBar4")->hide();
    toolBar("ToolBar5")->hide();
    }
    else
    {
    stateChanged( "State4" );
    topWidgetStack->raiseWidget(0);
    if (showmaintoolbar) {toolBar("ToolBar1")->show();}
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

   UpdateLineColStatus();
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

  UpdateLineColStatus();
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
  QString finame = getShortName();
  if (finame == "untitled") {
     if (KMessageBox::warningYesNo(this,i18n("You need to save an untitled document before you run %1 on it.\n"
                                             "Do you want to save it? Click Yes to save and No to abort.").arg(command),
                                   i18n("File Needs to be Saved!"))
         == KMessageBox::No) return QString::null;
  }

  //save the file before doing anything
  //attempting to save an untitled document will result in a file-save dialog pop-up
  fileSave();

  //determine the name of the file to be compiled
  if (singlemode)
  {
	finame=getName();
	if (!currentEditor()->isLaTeXRoot())
	{
		if (KMessageBox::warningYesNo(this,i18n("This document doesn't contain a LaTeX header.\nIt should probably be used with a master document.\nContinue anyway?"))
			== KMessageBox::No)
			return QString::null;
	}
  }
  else {
     finame=MasterName; //FIXME: MasterFile does not get saved if it is modified
  }

  //we need to check for finame=="untitled" etc. because the user could have
  //escaped the file save dialog
  if ((singlemode && !currentEditorView()) || finame=="untitled" || finame=="")
  {
     KMessageBox::error( this,i18n("Could not start the %1 command, because there is no file to run %1 on.\n"
                                   "Make sure you have the file you want to compile open and saved.")
                         .arg(command).arg(command));
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
   QStringList list,empty;
   QString finame, fromName, toName;
   if (singlemode) {finame=getName();}
   else {
     finame=MasterName;
   }

   if (getShortName() == "untitled") {
      KMessageBox::error(this,i18n("You need to save an untitled document and make a %1 "
                                   "file out of it. After you have done this, you can turn it into a %2 file.")
                                   .arg(from.upper()).arg(to.upper()),
                         i18n("File needs to be saved and compiled!"));
      return empty;
   }

   if ((singlemode && !currentEditorView()) || finame=="")
   {
     KMessageBox::error( this,i18n("Could not start the %1 command, because there is no file to run %1 on. "
                                   "Make sure you have the source file of the file you want to convert open and saved.")
                         .arg(command).arg(command));
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
   QString finame;
   if (singlemode) {finame=getName();}
   else {
     finame=MasterName;
   }

   if (getShortName() == "untitled") {
      KMessageBox::error(this,i18n("You need to save an untitled document and make a %1 "
                                   "file out of it. After you have done this, you can view the %1 file.")
                                   .arg(ext.upper()).arg(ext.upper()),
                         i18n("File needs to be saved and compiled!"));
      return QString::null;
   }

   if ((singlemode && !currentEditorView()) || finame=="")
   {
     KMessageBox::error( this, i18n("Unable to determine which %1 file to show. Please open the source file of the %1 file to want to view.")
                         .arg(ext.upper()).arg(ext.upper()));
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

   if ( ! (fic.exists() && fic.isReadable() ) )
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

  UpdateLineColStatus();
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

  UpdateLineColStatus();
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
  QString texname=fic.baseName(TRUE)+".tex";
  int para=0;
  int index=0;
  currentEditorView()->editor->viewport()->setFocus();
  currentEditorView()->editor->getCursorPosition( &para, &index);
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


   UpdateLineColStatus();
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

  UpdateLineColStatus();
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

    UpdateLineColStatus();
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

  UpdateLineColStatus();
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


 UpdateLineColStatus();
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
  if ((singlemode && !currentEditorView()) || finame=="")
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


 UpdateLineColStatus();
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
  if ((singlemode && !currentEditorView()) || finame=="")
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

  UpdateLineColStatus();
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
    UpdateLineColStatus();
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

  UpdateLineColStatus();
}

void Kile::MetaPost()
{
  //TODO: what the h*ll is MetaPost, how should we deal with the
  //error messages?

  QString finame;

  finame=getShortName();
  if (!currentEditorView() ||finame=="untitled" || finame=="")
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
UpdateLineColStatus();
}

void Kile::CleanAll()
{
  QString finame = getShortName();

  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="")
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

   UpdateLineColStatus();
}

void Kile::syncTerminal()
{
    QString finame;
    if (singlemode) {finame=getName();}
    else {finame=MasterName;}
    if ((singlemode && !currentEditorView()) ||getShortName()=="untitled" || getShortName()=="") {return;}

    QFileInfo fi(finame);
    QString texname=fi.dirPath()+"/"+fi.baseName(TRUE)+".tex";
    QFileInfo fic(texname);
  	if (fic.exists() && fic.isReadable() )
    {
    texkonsole->SetDirectory(fic.dirPath());
    texkonsole->activate();
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

   UpdateLineColStatus();
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
UpdateLineColStatus();
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
UpdateLineColStatus();
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
	QString finame;
	QString commandline=listUserTools[i].tag;
	QFileInfo fi;

	bool documentpresent=true;

	if (singlemode) {finame=getName();}
	else {finame=MasterName;}
	if ((singlemode && !currentEditorView()) ||getShortName()=="untitled" || getShortName()=="")
	{
		documentpresent=false;
	}

	QStringList command;
	if (documentpresent)
	{
		fi.setFile(finame);
		fileSave();
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

	UpdateLineColStatus();
}

////////////////// STRUCTURE ///////////////////
void Kile::ShowStructure()
{
showVertPage(1);
}
void Kile::UpdateStructure()
{
outstruct->clear();
if ( !currentEditorView() ) return;
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
for(int i = 0; i < currentEditorView()->editor->paragraphs(); i++)
 {
  int tagStart, tagEnd;
 //// label ////
 tagStart=tagEnd=0;
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
 s=currentEditorView()->editor->text(i);
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
if (currentEditorView() ){currentEditorView()->editor->viewport()->setFocus();}
}

void Kile::ClickedOnStructure(QListViewItem * item)
{
//return if user didn't click on an item
if (item==0) return;

bool ok = false;
if ( !currentEditorView() ) return;
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
 int l=s.toInt(&ok,10);
 if (ok && l<=currentEditorView()->editor->paragraphs())
  {
  currentEditorView()->editor->viewport()->setFocus();
  currentEditorView()->editor->setCursorPosition(l, 0);
  UpdateLineColStatus();
  }
 }
 }
}

void Kile::DoubleClickedOnStructure(QListViewItem *)
{
if ( !currentEditorView() ) return;
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
		kdDebug() << "about to show log" << endl;
		LogWidget->setText(tempLog);
		LogWidget->highlight();
		LogWidget->scrollToBottom();
		UpdateLineColStatus();
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

if ( !currentEditorView() ) return;

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


int l=line.toInt(&ok,10)-1;

if (ok && l<=currentEditorView()->editor->paragraphs())
 {
 currentEditorView()->editor->viewport()->setFocus();
 currentEditorView()->editor->setCursorPosition(l, 0);
 UpdateLineColStatus();
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
	kdDebug() << "starting to read log" << endl;
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
	kdDebug() << "log read" << endl;
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
		kdDebug() << "current item " << errorlist->findRef(errorlist->current()) << endl;
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
		kdDebug() << "current item " << errorlist->findRef(errorlist->current()) << endl;
	}

	if (logpresent && errorlist->isEmpty())
	{
		LogWidget->insertLine(i18n("No LaTeX errors detected!"));
	}

	m_bNewErrorlist=false;
}
/////////////////////// LATEX TAGS ///////////////////
void Kile::InsertTag(QString Entity, int dx, int dy)
{
 if ( !currentEditorView() )	return;
	int para=0;
  int index=0;
  currentEditorView()->editor->viewport()->setFocus();
  currentEditorView()->editor->getCursorPosition( &para, &index);
  currentEditorView()->editor->insertAt(Entity,para,index);
  currentEditorView()->editor->setCursorPosition(para+dy,index+dx);
  currentEditorView()->editor->viewport()->setFocus();
  LogWidget->clear();
  Outputview->showPage(LogWidget);
  logpresent=false;
  UpdateLineColStatus();
}

void Kile::QuickDocument()
{
QString opt="";
int li=3;
  if ( !currentEditorView() )	return;
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
  if ( !currentEditorView() )	return;
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
  if ( !currentEditorView() )	return;
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
  if ( !currentEditorView() )	return;
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
  if ( !currentEditorView() )	return;
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

void Kile::Insert1()
{
InsertTag("\\documentclass[10pt]{}",21,0);
LogWidget->insertLine( "\\documentclass[options]{class}" );
LogWidget->insertLine( "class : article,report,book,letter" );
LogWidget->insertLine( "size options : 10pt, 11pt, 12pt" );
LogWidget->insertLine( "paper size options: a4paper, a5paper, b5paper, letterpaper, legalpaper, executivepaper" );
LogWidget->insertLine("other options: ");
LogWidget->insertLine("landscape -- selects landscape format. Default is portrait. ");
LogWidget->insertLine("titlepage, notitlepage -- selects if there should be a separate title page.");
LogWidget->insertLine("leqno -- equation number on left side of equations. Default is right side.");
LogWidget->insertLine("fleqn -- displayed formulas flush left. Default is centred.");
LogWidget->insertLine("onecolumn, twocolumn -- one or two columns. Defaults to one column");
LogWidget->insertLine("oneside, twoside -- selects one- or twosided layout.");
}
void Kile::Insert1bis()
{
InsertTag("\\usepackage{} ",12,0);
LogWidget->insertLine("\\usepackage[options]{pkg}");
LogWidget->insertLine("Any options given in the \\documentclass command that are unknown by the selected document class");
LogWidget->insertLine("are passed on to the packages loaded with \\usepackage.");
}
void Kile::Insert1ter()
{
InsertTag("\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n",0,3);
LogWidget->insertLine("The principal American Mathematical Society packages");
}
void Kile::Insert2()
{
InsertTag("\\begin{document}\n\n\\end{document} ",0,1);
LogWidget->insertLine("Text is allowed only between \\begin{document} and \\end{document}.");
LogWidget->insertLine("The 'preamble' (before \\begin{document}} ) may contain declarations only.");
}

void Kile::Insert3()
{
if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\part");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\part{");}
  else
  {tag=QString("\\part*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\part{title}");
LogWidget->insertLine( "\\part*{title} : do not include a number and do not make an entry in the table of contents");
delete( stDlg);
}

void Kile::Insert4()
{
 if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\chapter");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\chapter{");}
  else
  {tag=QString("\\chapter*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\chapter{title}");
LogWidget->insertLine( "\\chapter*{title} : do not include a number and do not make an entry in the table of contents");
LogWidget->insertLine( "Only for 'report' and 'book' class document.");
delete( stDlg);
}

void Kile::Insert5()
{
 if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\section");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\section{");}
  else
  {tag=QString("\\section*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\section{title}");
LogWidget->insertLine( "\\section*{title} : do not include a number and do not make an entry in the table of contents");
delete( stDlg);
}

void Kile::Insert6()
{
 if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\subsection");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\subsection{");}
  else
  {tag=QString("\\subsection*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\subsection{title}");
LogWidget->insertLine( "\\subsection*{title} : do not include a number and do not make an entry in the table of contents");
delete( stDlg);
}

void Kile::Insert6bis()
{
 if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\subsubsection");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\subsubsection{");}
  else
  {tag=QString("\\subsubsection*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\subsubsection{title}");
LogWidget->insertLine( "\\subsubsection*{title} : do not include a number and do not make an entry in the table of contents");
delete( stDlg);
}

void Kile::Insert7()
{
 if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\paragraph");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\paragraph{");}
  else
  {tag=QString("\\paragraph*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\paragraph{title}");
LogWidget->insertLine( "\\paragraph*{title} : do not include a number and do not make an entry in the table of contents");
delete( stDlg);
}

void Kile::Insert8()
{
 if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
stDlg = new structdialog(this," \\subparagraph");
if ( stDlg->exec() )
  {
  if (stDlg->checkbox->isChecked())
  {tag=QString("\\subparagraph{");}
  else
  {tag=QString("\\subparagraph*{");}
  tag +=stDlg->title_edit->text();
  tag +=QString("}\n");
  InsertTag(tag,0,1);
  UpdateStructure();
  }
LogWidget->insertLine( "\\subparagraph{title}");
LogWidget->insertLine( "\\subparagraph*{title} : do not include a number and do not make an entry in the table of contents");
delete( stDlg);
}

void Kile::Insert9()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
   InsertTag("\\begin{center}\n\n\\end{center} ",0,1);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\begin{center}\n",0,1);
   currentEditorView()->editor->paste();
   InsertTag("\n\\end{center}",0,0);
   }
LogWidget->insertLine( "Each line must be terminated with the string \\\\.");
}

void Kile::Insert10()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
   InsertTag("\\begin{flushleft}\n\n\\end{flushleft} ",0,1);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\begin{flushleft}\n",0,1);
   currentEditorView()->editor->paste();
   InsertTag("\n\\end{flushleft}",0,0);
   }
LogWidget->insertLine( "Each line must be terminated with the string \\\\.");
}

void Kile::Insert11()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
   InsertTag("\\begin{flushright}\n\n\\end{flushright} ",0,1);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\begin{flushright}\n",0,1);
   currentEditorView()->editor->paste();
   InsertTag("\n\\end{flushright}",0,0);
   }
LogWidget->insertLine( "Each line must be terminated with the string \\\\.");
}

void Kile::Insert12()
{
InsertTag("\\begin{quote}\n\n\\end{quote} ",0,1);
LogWidget->insertLine("The text is justified at both margins.");
LogWidget->insertLine(" Leaving a blank line between text produces a new paragraph.");
}

void Kile::Insert13()
{
InsertTag("\\begin{quotation}\n\n\\end{quotation} ",0,1);
LogWidget->insertLine("The text is justified at both margins and there is paragraph indentation.");
LogWidget->insertLine(" Leaving a blank line between text produces a new paragraph.");
}

void Kile::Insert14()
{
InsertTag("\\begin{verse}\n\n\\end{verse} ",0,1);
LogWidget->insertLine("The verse environment is designed for poetry.");
LogWidget->insertLine("Separate the lines of each stanza with \\\\, and use one or more blank lines to separate the stanzas.");
}

void Kile::Insert15()
{
InsertTag("\\begin{verbatim}\n\n\\end{verbatim} ",0,1);
LogWidget->insertLine("Environment that gets LaTeX to print exactly what you type in.");
}

void Kile::Insert16()
{
InsertTag("\\begin{itemize}\n\\item \n\\end{itemize} ",6,1);
LogWidget->insertLine("The itemize environment produces a 'bulleted' list.");
LogWidget->insertLine("Each item of an itemized list begins with an \\item command.");
}

void Kile::Insert17()
{
InsertTag("\\begin{enumerate}\n\\item \n\\end{enumerate} ",6,1);
LogWidget->insertLine("The enumerate environment produces a numbered list.");
LogWidget->insertLine("Each item of an enumerated list begins with an \\item command.");
}

void Kile::Insert18()
{
InsertTag("\\begin{description}\n\\item[]\n\\end{description} ",6,1);
LogWidget->insertLine("The description environment is used to make labelled lists.");
LogWidget->insertLine("Each item of the list begins with an \\item[label] command.");
LogWidget->insertLine("The 'label' is bold face and flushed right.");
}

void Kile::Insert19()
{
InsertTag("\\begin{list}{}{}\n\\item \n\\end{list} ",13,0);
LogWidget->insertLine("\\begin{list}{label}{spacing}");
LogWidget->insertLine("The {label} argument is a piece of text that is inserted in a box to form the label. ");
LogWidget->insertLine("The {spacing} argument contains commands to change the spacing parameters for the list.");
LogWidget->insertLine("Each item of the list begins with an \\item command.");
}

void Kile::Insert20()
{
InsertTag("\\item ",6,0);
LogWidget->insertLine("\\item[label] Hello!");
}

void Kile::Insert21()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\textit{} ",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\textit{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",0,0);
   }
LogWidget->insertLine("\\textit{italic text}");
}

void Kile::Insert22()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\textsl{} ",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\textsl{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",0,0);
   }
LogWidget->insertLine("\\textsl{slanted text}");
}

void Kile::Insert23()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\textbf{} ",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\textbf{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",0,0);
   }
LogWidget->insertLine("\\textbf{boldface text}");
}

void Kile::Insert24()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\texttt{} ",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\texttt{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",0,0);
   }
LogWidget->insertLine("\\texttt{typewriter text}");
}

void Kile::Insert25()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\textsc{} ",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\textsc{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",0,0);
   }
LogWidget->insertLine("\\textsc{small caps text}");
}

void Kile::Insert26()
{
InsertTag("\\begin{tabbing}\n\n\\end{tabbing} ",0,1);
LogWidget->insertLine("The tabbing environment provides a way to align text in columns.");
LogWidget->insertLine("\\begin{tabbing}");
LogWidget->insertLine("text \\= more text \\= still more text \\= last text \\\\");
LogWidget->insertLine("second row \\>  \\> more \\\\");
LogWidget->insertLine("\\end{tabbing}");
LogWidget->insertLine("Commands :");
LogWidget->insertLine("\\=  Sets a tab stop at the current position.");
LogWidget->insertLine("\\>  Advances to the next tab stop.");
LogWidget->insertLine("\\<  Allows you to put something to the left of the local margin without changing the margin. Can only be used at the start of the line.");
LogWidget->insertLine("\\+  Moves the left margin of the next and all the following commands one tab stop to the right");
LogWidget->insertLine("\\-  Moves the left margin of the next and all the following commands one tab stop to the left");
LogWidget->insertLine("\\'  Moves everything that you have typed so far in the current column to the right of the previous column, flush against the current column's tab stop. ");
LogWidget->insertLine("\\`  Allows you to put text flush right against any tab stop, including tab stop 0");
LogWidget->insertLine("\\kill  Sets tab stops without producing text.");
LogWidget->insertLine("\\a  In a tabbing environment, the commands \\=, \\' and \\` do not produce accents as normal. Instead, the commands \\a=, \\a' and \\a` are used.");
}

void Kile::Insert27()
{
InsertTag("\\begin{tabular}{}\n\n\\end{tabular} ",16,0);
LogWidget->insertLine("\\begin{tabular}[pos]{cols}");
LogWidget->insertLine("column 1 entry & column 2 entry ... & column n entry \\\\");
LogWidget->insertLine("...");
LogWidget->insertLine("\\end{tabular}");
LogWidget->insertLine("pos : Specifies the vertical position; default is alignment on the centre of the environment.");
LogWidget->insertLine("     t - align on top row");
LogWidget->insertLine("     b - align on bottom row");
LogWidget->insertLine("cols : Specifies the column formatting.");
LogWidget->insertLine("     l - A column of left-aligned items.");
LogWidget->insertLine("     r - A column of right-aligned items.");
LogWidget->insertLine("     c - A column of centred items.");
LogWidget->insertLine("     | - A vertical line the full height and depth of the environment.");
LogWidget->insertLine("     @{text} - This inserts text in every row.");
LogWidget->insertLine("The \\hline command draws a horizontal line the width of the table.");
LogWidget->insertLine("The \\cline{i-j} command draws horizontal lines across the columns specified, beginning in column i and ending in column j,");
LogWidget->insertLine("The \\vline command draws a vertical line extending the full height and depth of its row.");

}

void Kile::Insert28()
{
InsertTag("\\multicolumn{}{}{} ",13,0);
LogWidget->insertLine("\\multicolumn{cols}{pos}{text}");
LogWidget->insertLine("col, specifies the number of columns to span.");
LogWidget->insertLine("pos specifies the formatting of the entry: c for centred, l for flushleft, r for flushright.");
LogWidget->insertLine("text specifies what text is to make up the entry.");
}

void Kile::Insert29()
{
InsertTag("\\hline ",7,0);
LogWidget->insertLine("The \\hline command draws a horizontal line the width of the table.");
}

void Kile::Insert30()
{
InsertTag("\\vline ",7,0);
LogWidget->insertLine("The \\vline command draws a vertical line extending the full height and depth of its row.");
}

void Kile::Insert31()
{
InsertTag("\\cline{-} ",7,0);
LogWidget->insertLine("The \\cline{i-j} command draws horizontal lines across the columns specified, beginning in column i and ending in column j,");
}

void Kile::Insert32()
{
InsertTag("\\newpage ",9,0);
LogWidget->insertLine("The \\newpage command ends the current page");
}

void Kile::Insert33()
{
InsertTag("\\linebreak ",11,0);
LogWidget->insertLine("The \\linebreak command tells LaTeX to break the current line at the point of the command.");
}

void Kile::Insert34()
{
InsertTag("\\pagebreak ",11,0);
LogWidget->insertLine("The \\pagebreak command tells LaTeX to break the current page at the point of the command.");
}

void Kile::Insert35()
{
InsertTag("\\bigskip ",9,0);
LogWidget->insertLine("The \\bigskip command adds a 'big' vertical space.");
}

void Kile::Insert36()
{
InsertTag("\\medskip ",9,0);
LogWidget->insertLine("The \\medskip command adds a 'medium' vertical space.");
}

void Kile::Insert37()
{
if ( !currentEditorView() )	return;
QString currentDir=QDir::currentDirPath();
QString finame;
if (singlemode) {finame=getName();}
else {finame=MasterName;}
QFileInfo fi(finame);
if (getShortName() !="untitled") currentDir=fi.dirPath();
sfDlg = new FileChooser(this,"Select Image File",i18n("Select Image File"));
sfDlg->setFilter(i18n("*.eps *.pdf *.png|Graphic Files\n*|All Files"));
sfDlg->setDir(currentDir);
if (sfDlg->exec() )
  {
   QString fn=sfDlg->fileName();
   QFileInfo fi(fn);
   InsertTag("\\includegraphics[scale=1]{"+fi.baseName(TRUE)+"."+fi.extension()+"} ",26,0);
   LogWidget->insertLine("This command is used to import image files (\\usepackage{graphicx} is required)");
   LogWidget->insertLine("Examples :");
   LogWidget->insertLine("\\includegraphics{file} ; \\includegraphics[width=10cm]{file} ; \\includegraphics*[scale=0.75]{file}");
   if (fi.extension()=="eps")
     {
     if (fi.dirPath()==".") fn=currentDir+"/"+fn;
     LogWidget->insertLine("*************  ABOUT THIS IMAGE  *************");
     LogWidget->insertLine(DetectEpsSize(fn));
     }
  }
delete sfDlg;
}

void Kile::Insert37bis()
{
if ( !currentEditorView() )	return;
QString currentDir=QDir::currentDirPath();
QString finame;
if (singlemode) {finame=getName();}
else {finame=MasterName;}
QFileInfo fi(finame);
if (getShortName()!="untitled") currentDir=fi.dirPath();
sfDlg = new FileChooser(this,"Select File",i18n("Select File"));
sfDlg->setFilter(i18n("*.tex|TeX Files\n*|All Files"));
sfDlg->setDir(currentDir);
if (sfDlg->exec() )
  {
QString fn=sfDlg->fileName();
QFileInfo fi(fn);
InsertTag("\\include{"+fi.baseName(TRUE)+"}",9,0);
  }
delete sfDlg;
UpdateStructure();
LogWidget->insertLine("\\include{file}");
LogWidget->insertLine("The \\include command is used in conjunction with the \\includeonly command for selective inclusion of files.");
}

void Kile::Insert37ter()
{
if ( !currentEditorView() )	return;
QString currentDir=QDir::currentDirPath();
QString finame;
if (singlemode) {finame=getName();}
else {finame=MasterName;}
QFileInfo fi(finame);
if (getShortName()!="untitled") currentDir=fi.dirPath();
sfDlg = new FileChooser(this,"Select File",i18n("Select File"));
sfDlg->setFilter(i18n("*.tex|TeX Files\n*|All Files"));
sfDlg->setDir(currentDir);
if (sfDlg->exec() )
  {
QString fn=sfDlg->fileName();
QFileInfo fi(fn);
InsertTag("\\input{"+fi.baseName(TRUE)+"}",7,0);
  }
delete sfDlg;
UpdateStructure();
OutputWidget->insertLine("\\input{file}");
OutputWidget->insertLine("The \\input command causes the indicated file to be read and processed, exactly as if its contents had been inserted in the current file at that point.");
}

void Kile::Insert38()
{
InsertTag("\\cite{} ",6,0);
LogWidget->insertLine("\\cite{ref} :");
LogWidget->insertLine("This command generates an in-text citation to the reference associated with the ref entry in the bib file");
LogWidget->insertLine("You can open the bib file with Kile to see all the available references");
}

void Kile::Insert39()
{
InsertTag("\\bibliographystyle{} ",19,0);
LogWidget->insertLine("The argument to \\bibliographystyle refers to a file style.bst, which defines how your citations will look");
LogWidget->insertLine("The standard styles distributed with BibTeX are:");
LogWidget->insertLine("alpha : sorted alphabetically. Labels are formed from name of author and year of publication.");
LogWidget->insertLine("plain  : sorted alphabetically. Labels are numeric.");
LogWidget->insertLine("unsrt : like plain, but entries are in order of citation.");
LogWidget->insertLine("abbrv  : like plain, but more compact labels.");
}
void Kile::Insert40()
{
if ( !currentEditorView() )	return;
currentEditorView()->editor->viewport()->setFocus();
QString tag;
QFileInfo fi(getName());
tag=QString("\\bibliography{");
tag +=fi.baseName(TRUE);
tag +=QString("}\n");
InsertTag(tag,0,1);
LogWidget->insertLine("The argument to \\bibliography refers to the bib file (without extension)");
LogWidget->insertLine("which should contain your database in BibTeX format.");
LogWidget->insertLine("Kile inserts automatically the base name of the TeX file");
}
void Kile::Insert41()
{
InsertTag("\\label{} ",7,0);
LogWidget->insertLine("\\label{key}");
}
void Kile::Insert42()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
   InsertTag("\\begin{table}\n\n\\caption{}\n\\end{table} ",0,1);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\begin{table}\n",0,1);
   currentEditorView()->editor->paste();
   InsertTag("\n\\caption{}\n\\end{table}",9,1);
   }
LogWidget->insertLine( "\\begin{table}[placement]");
LogWidget->insertLine( "body of the table");
LogWidget->insertLine( "\\caption{table title}");
LogWidget->insertLine( "\\end{table}");
LogWidget->insertLine( "Tables are objects that are not part of the normal text, and are usually floated to a convenient place");
LogWidget->insertLine( "The optional argument [placement] determines where LaTeX will try to place your table");
LogWidget->insertLine( "h : Here - at the position in the text where the table environment appear");
LogWidget->insertLine( "t : Top - at the top of a text page");
LogWidget->insertLine( "b : Bottom - at the bottom of a text page");
LogWidget->insertLine( "p : Page of floats - on a separate float page, which is a page containing no text, only floats");
LogWidget->insertLine( "The body of the table is made up of whatever text, LaTeX commands, etc., you wish.");
LogWidget->insertLine( "The \\caption command allows you to title your table.");

}
void Kile::Insert43()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
   InsertTag("\\begin{figure}\n\n\\caption{}\n\\end{figure} ",0,1);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\begin{figure}\n",0,1);
   currentEditorView()->editor->paste();
   InsertTag("\n\\caption{}\n\\end{figure}",9,1);
   }
LogWidget->insertLine( "\\begin{figure}[placement]");
LogWidget->insertLine( "body of the figure");
LogWidget->insertLine( "\\caption{figure title}");
LogWidget->insertLine( "\\end{figure}");
LogWidget->insertLine( "Figures are objects that are not part of the normal text, and are usually floated to a convenient place");
LogWidget->insertLine( "The optional argument [placement] determines where LaTeX will try to place your figure");
LogWidget->insertLine( "h : Here - at the position in the text where the figure environment appear");
LogWidget->insertLine( "t : Top - at the top of a text page");
LogWidget->insertLine( "b : Bottom - at the bottom of a text page");
LogWidget->insertLine( "p : Page of floats - on a separate float page, which is a page containing no text, only floats");
LogWidget->insertLine( "The body of the figure is made up of whatever text, LaTeX commands, etc., you wish.");
LogWidget->insertLine( "The \\caption command allows you to title your figure.");
}
void Kile::Insert44()
{
InsertTag("\\begin{titlepage}\n\n\\end{titlepage} ",0,1);
LogWidget->insertLine( "\\begin{titlepage}");
LogWidget->insertLine( "text");
LogWidget->insertLine( "\\end{titlepage}");
LogWidget->insertLine( "The titlepage environment creates a title page, i.e. a page with no printed page number or heading.");
}
void Kile::Insert45()
{
InsertTag("\\author{}",8,0);
LogWidget->insertLine( "\\author{names}");
LogWidget->insertLine( "The \\author command declares the author(s), where names is a list of authors separated by \\and commands.");
}
void Kile::Insert46()
{
InsertTag("\\title{}",7,0);
LogWidget->insertLine( "\\title{text}");
LogWidget->insertLine( "The \\title command declares text to be the title.");
LogWidget->insertLine( "Use \\\\ to tell LaTeX where to start a new line in a long title.");
}
void Kile::Insert47()
{
InsertTag("\\index{}",7,0);
LogWidget->insertLine( "\\index{word}");
}

void Kile::Insert48()
{
UpdateStructure();
QString tag="";
refDlg = new refdialog(this,"Labels");
refDlg->combo1->insertStringList(labelitem);
if (!labelitem.isEmpty() && refDlg->exec() )
  {
  tag="\\ref{"+refDlg->combo1->currentText()+"}";
  InsertTag(tag,tag.length(),0);
  }
else InsertTag("\\ref{}",5,0);
delete refDlg;
LogWidget->insertLine( "\\ref{key}");
}

void Kile::Insert49()
{
UpdateStructure();
QString tag="";
refDlg = new refdialog(this,"Labels");
refDlg->combo1->insertStringList(labelitem);
if (!labelitem.isEmpty() && refDlg->exec() )
  {
  tag="\\pageref{"+refDlg->combo1->currentText()+"}";
  InsertTag(tag,tag.length(),0);
  }
else InsertTag("\\pageref{}",9,0);
delete refDlg;
LogWidget->insertLine( "\\pageref{key}");
}

void Kile::Insert50()
{
InsertTag("\\footnote{}",10,0);
LogWidget->insertLine( "\\footnote{text}");
}

void Kile::Insert51()
{
InsertTag("\\maketitle",10,0);
LogWidget->insertLine( "This command generates a title on a separate title page\n- except in the article class, where the title normally goes at the top of the first page.");
}

void Kile::Insert52()
{
InsertTag("\\tableofcontents",16,0);
LogWidget->insertLine( "Put this command where you want the table of contents to go");
}

void Kile::SizeCommand(const QString& text)
{
if ( !currentEditorView() )	return;
if (text=="tiny")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{tiny}\\end{tiny}",12,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{tiny}",12,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{tiny}",0,0);
       }
    }

if (text=="scriptsize")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{scriptsize}\\end{scriptsize}",18,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{scriptsize}",18,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{scriptsize}",0,0);
       }
    }

if (text=="footnotesize")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{footnotesize}\\end{footnotesize}",20,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{footnotesize}",20,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{footnotesize}",0,0);
       }
    }

if (text=="small")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{small}\\end{small}",13,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{small}",13,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{small}",0,0);
       }
    }

if (text=="normalsize")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{normalsize}\\end{normalsize}",18,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{normalsize}",18,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{normalsize}",0,0);
       }
    }

if (text=="large")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{large}\\end{large}",13,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{large}",13,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{large}",0,0);
       }
    }

if (text=="Large")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{Large}\\end{Large}",13,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{Large}",13,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{Large}",0,0);
       }
    }

if (text=="LARGE")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{LARGE}\\end{LARGE}",13,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{LARGE}",13,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{LARGE}",0,0);
       }
    }

if (text=="huge")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{huge}\\end{huge}",12,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{huge}",12,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{huge}",0,0);
       }
    }

if (text=="Huge")
    {
    if (!currentEditorView()->editor->hasSelectedText())
       {
       InsertTag("\\begin{Huge}\\end{Huge}",12,0);
       }
    else
       {
       currentEditorView()->editor->cut();
       InsertTag("\\begin{Huge}",12,0);
       currentEditorView()->editor->paste();
       InsertTag("\\end{Huge}",0,0);
       }
    }
}

void Kile::SectionCommand(const QString& text)
{
if ( !currentEditorView() )	return;
if (text=="part") Insert3();
if (text=="chapter") Insert4();
if (text=="section") Insert5();
if (text=="subsection") Insert6();
if (text=="subsubsection") Insert6bis();
if (text=="paragraph") Insert7();
if (text=="subparagraph") Insert8();
}

void Kile::OtherCommand(const QString& text)
{
if ( !currentEditorView() )	return;
if (text=="label") Insert41();
if (text=="ref") Insert48();
if (text=="pageref") Insert49();
if (text=="index") Insert47();
if (text=="cite") Insert38();
if (text=="footnote") Insert50();
}

void Kile::NewLine()
{
InsertTag("\\\\\n",0,1);
}
//////////////////////////// MATHS TAGS/////////////////////////////////////
void Kile::InsertMath1()
{
InsertTag("$  $",2,0);
}

void Kile::InsertMath2()
{
InsertTag("$$  $$",3,0);
}

void Kile::InsertMath3()
{
InsertTag("_{}",2,0);
}

void Kile::InsertMath4()
{
InsertTag("^{}",2,0);
}

void Kile::InsertMath5()
{
InsertTag("\\frac{}{}",6,0);
}

void Kile::InsertMath6()
{
InsertTag("\\dfrac{}{}",7,0);
}

void Kile::InsertMath7()
{
InsertTag("\\sqrt{}",6,0);
}
void Kile::InsertMath8()
{
InsertTag("\\left ",6,0);
}

void Kile::InsertMath9()
{
InsertTag("\\right ",7,0);
}

void Kile::InsertMath10()
{
InsertTag("\\begin{array}{}\n\n\\end{array}",14,0);
LogWidget->insertLine("\\begin{array}{col1col2...coln}");
LogWidget->insertLine("column 1 entry & column 2 entry ... & column n entry \\\\");
LogWidget->insertLine("...");
LogWidget->insertLine("\\end{array}");
LogWidget->insertLine("Each column, coln, is specified by a single letter that tells how items in that row should be formatted.");
LogWidget->insertLine("     c -- for centered ");
LogWidget->insertLine("     l -- for flush left ");
LogWidget->insertLine("     r -- for flush right");
}
void Kile::InsertMath16()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\underline{}",11,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\underline{",11,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}

void Kile::InsertMath66()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathrm{}",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathrm{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath67()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathit{}",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathit{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath68()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathbf{}",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathbf{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath69()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathsf{}",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathsf{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath70()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathtt{}",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathtt{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath71()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathcal{}",9,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathcal{",9,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath72()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathbb{}",8,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathbb{",8,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath73()
{
if ( !currentEditorView() )	return;
if (!currentEditorView()->editor->hasSelectedText())
   {
    InsertTag("\\mathfrak{} ",10,0);
   }
else
   {
   currentEditorView()->editor->cut();
   InsertTag("\\mathfrak{",10,0);
   currentEditorView()->editor->paste();
   InsertTag("}",1,0);
   }
}
void Kile::InsertMath74()
{
InsertTag("\\begin{equation}\n\n\\end{equation} ",0,1);
}
void Kile::InsertMath75()
{
InsertTag("\\begin{eqnarray}\n\n\\end{eqnarray} ",0,1);
}
void Kile::InsertMath76()
{
InsertTag("\\acute{}",7,0);
}
void Kile::InsertMath77()
{
InsertTag("\\grave{}",7,0);
}
void Kile::InsertMath78()
{
InsertTag("\\tilde{}",7,0);
}
void Kile::InsertMath79()
{
InsertTag("\\bar{}",5,0);
}
void Kile::InsertMath80()
{
InsertTag("\\vec{}",5,0);
}
void Kile::InsertMath81()
{
InsertTag("\\hat{}",5,0);
}
void Kile::InsertMath82()
{
InsertTag("\\check{}",7,0);
}
void Kile::InsertMath83()
{
InsertTag("\\breve{}",7,0);
}
void Kile::InsertMath84()
{
InsertTag("\\dot{}",5,0);
}
void Kile::InsertMath85()
{
InsertTag("\\ddot{}",6,0);
}
void Kile::InsertMath86()
{
InsertTag("\\,",2,0);
}
void Kile::InsertMath87()
{
InsertTag("\\:",2,0);
}
void Kile::InsertMath88()
{
InsertTag("\\;",2,0);
}
void Kile::InsertMath89()
{
InsertTag("\\quad",5,0);
}
void Kile::InsertMath90()
{
InsertTag("\\qquad",6,0);
}

void Kile::LeftDelimiter(const QString& text)
{
if (text=="left (") InsertTag("\\left( ",7,0);
if (text=="left [") InsertTag("\\left[ ",7,0);
if (text=="left {") InsertTag("\\left\\lbrace ",13,0);
if (text=="left <") InsertTag("\\left\\langle ",13,0);
if (text=="left )") InsertTag("\\left) ",7,0);
if (text=="left ]") InsertTag("\\left] ",7,0);
if (text=="left }") InsertTag("\\left\\rbrace ",13,0);
if (text=="left >") InsertTag("\\left\\rangle ",13,0);
if (text=="left.") InsertTag("\\left. ",7,0);
}
void Kile::RightDelimiter(const QString& text)
{
if (text=="right (") InsertTag("\\right( ",8,0);
if (text=="right [") InsertTag("\\right[ ",8,0);
if (text=="right {") InsertTag("\\right\\lbrace ",14,0);
if (text=="right <") InsertTag("\\right\\langle ",14,0);
if (text=="right )") InsertTag("\\right) ",8,0);
if (text=="right ]") InsertTag("\\right] ",8,0);
if (text=="right }") InsertTag("\\right\\rbrace ",14,0);
if (text=="right >") InsertTag("\\right\\rangle ",14,0);
if (text=="right.") InsertTag("\\right. ",8,0);
}

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
	if (listUserTags[i].tag.left(1)=="%")
	{
		QString t=listUserTags[i].tag;
		t=t.remove(0,1);
		QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
		InsertTag(s,0,1);
	}
	else
	{
		InsertTag(listUserTags[i].tag,0,0);
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
	umDlg = new usermenudialog(listUserTags,this,"Edit User Tags", i18n("Edit User Tags"));
	for ( uint i = 0; i < listUserTags.size(); i++ )
	{
		umDlg->Name.append(listUserTags[i].name);
		umDlg->Tag.append(listUserTags[i].tag);
	}
	umDlg->init();
	if ( umDlg->exec() )
	{
		userItem item;
		KAction *menuItem;
		QString name;
		KShortcut sc;
		int i=0;
		KShortcut tagaccels[12] = {SHIFT+Key_F1, SHIFT+Key_F2,SHIFT+Key_F3,SHIFT+Key_F4,SHIFT+Key_F5,SHIFT+Key_F6,SHIFT+Key_F7,
   		SHIFT+Key_F8,SHIFT+Key_F9,SHIFT+Key_F10,SHIFT+Key_F11,SHIFT+Key_F12};
		switch (umDlg->result())
		{
			case usermenudialog::Add :
				item.name=umDlg->Name[umDlg->index()];
				item.tag=umDlg->Tag[umDlg->index()];
				listUserTags.append(item);
				if (listUserTags.size() <13) sc = tagaccels[listUserTags.size()-1];
				else sc=0;
				name=QString::number(listUserTags.size())+": "+listUserTags[listUserTags.size()-1].name;
				menuItem = new KAction(name,sc,mapUserTagSignals,SLOT(map()), menuUserTags, name.ascii());
				listUserTagsActions.append(menuItem);
				menuUserTags->insert(menuItem);
				mapUserTagSignals->setMapping(menuItem,listUserTags.size()-1);
			break;
			case usermenudialog::Remove :
				i=listUserTagsActions.count();
				kdDebug() << "index " << umDlg->index() << endl;
				//remove all actions below and including the removed one (they need to be recreated)
				for (int j=umDlg->index(); j< i; j++)
				{
					menuItem = listUserTagsActions.getLast();
					mapUserTagSignals->removeMappings(menuItem);
					menuUserTags->remove(menuItem);
					listUserTagsActions.removeLast();
					delete menuItem;
				}
				//remove the tag from the list
				listUserTags.erase(listUserTags.at(umDlg->index()));
				//recreate the action below the removed one
				for (int j=umDlg->index(); j < (i-1); j++)
				{
					if (j <12) sc = tagaccels[j];
					else sc=0;
					name=QString::number(j+1)+": "+listUserTags[j].name;
					kdDebug() << "adding " << name << " at " << j<< endl;
					menuItem = new KAction(name,sc,mapUserTagSignals,SLOT(map()), menuUserTags, name.ascii());
					listUserTagsActions.append(menuItem);
					menuUserTags->insert(menuItem);
					mapUserTagSignals->setMapping(menuItem,j);
				}
			break;
			case usermenudialog::Edit :
			for ( uint i = 0; i < listUserTags.size(); i++ )
			{
				listUserTags[i].name=umDlg->Name[i];
				listUserTags[i].tag=umDlg->Tag[i];
				listUserTagsActions.at(i)->setText(QString::number(i+1)+": "+umDlg->Name[i]);
			}
			break;
			default : break;
		}

	}

	delete umDlg;
}

void Kile::EditUserTool()
{
	utDlg = new usertooldialog(listUserTools,this,"Edit User Tools", i18n("Edit User Tools"));
	for ( uint i = 0; i < listUserTools.size(); i++ )
	{
		utDlg->Name.append(listUserTools[i].name);
		utDlg->Tool.append(listUserTools[i].tag);
	}
	utDlg->init();
	if ( utDlg->exec() )
	{
		userItem item;
		KAction *menuItem;
		QString name;
		int i=0;
		KShortcut sc;
		KShortcut toolaccels[12] = {SHIFT+ALT+Key_F1, SHIFT+ALT+Key_F2,SHIFT+ALT+Key_F3,SHIFT+ALT+Key_F4,SHIFT+ALT+Key_F5,SHIFT+ALT+Key_F6,SHIFT+ALT+Key_F7,
		   	SHIFT+ALT+Key_F8,SHIFT+ALT+Key_F9,SHIFT+ALT+Key_F10,SHIFT+ALT+Key_F11,SHIFT+ALT+Key_F12};


		switch (utDlg->result())
		{
			case usertooldialog::Add :
				item.name=utDlg->Name[utDlg->index()];
				item.tag=utDlg->Tool[utDlg->index()];
				listUserTools.append(item);
				if (listUserTools.size() <13) sc = toolaccels[listUserTools.size()-1];
				else sc=0;
				name=QString::number(listUserTools.size())+": "+listUserTools[listUserTools.size()-1].name;
				menuItem = new KAction(name,sc,mapUserToolsSignals,SLOT(map()), menuUserTools, name.ascii());
				listUserToolsActions.append(menuItem);
				menuUserTools->insert(menuItem);
				mapUserToolsSignals->setMapping(menuItem,listUserTools.size()-1);
			break;
			case usertooldialog::Remove :
				i=listUserToolsActions.count();
				kdDebug() << "index " << utDlg->index() << endl;
				//remove all actions below and including the removed one (they need to be recreated)
				for (int j=utDlg->index(); j< i; j++)
				{
					menuItem = listUserToolsActions.getLast();
					mapUserToolsSignals->removeMappings(menuItem);
					menuUserTools->remove(menuItem);
					listUserToolsActions.removeLast();
					delete menuItem;
				}
				//remove the tag from the list
				listUserTools.erase(listUserTools.at(utDlg->index()));
				//recreate the action below the removed one
				for (int j=utDlg->index(); j < (i-1); j++)
				{
					if (j <12) sc = toolaccels[j];
					else sc=0;
					name=QString::number(j+1)+": "+listUserTools[j].name;
					kdDebug() << "adding " << name << " at " << j<< endl;
					menuItem = new KAction(name,sc,mapUserToolsSignals,SLOT(map()), menuUserTools, name.ascii());
					listUserToolsActions.append(menuItem);
					menuUserTools->insert(menuItem);
					mapUserToolsSignals->setMapping(menuItem,j);
				}
			break;
			case usertooldialog::Edit :
			for ( uint i = 0; i < listUserTools.size(); i++ )
			{
				listUserTools[i].name=utDlg->Name[i];
				listUserTools[i].tag=utDlg->Tool[i];
				listUserToolsActions.at(i)->setText(QString::number(i+1)+": "+utDlg->Name[i]);
			}
			break;
			default : break;
		}
	}
	delete utDlg;
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
  UpdateLineColStatus();
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

config->setGroup( "Editor" );
QString fam=config->readEntry("Font Family",KGlobalSettings::fixedFont().family());
int si=config->readNumEntry( "Font Size",KGlobalSettings::fixedFont().pointSize());
QFont F(fam,si);
EditorFont=F;
wordwrap=config->readBoolEntry( "WordWrap",true);
parenmatch=config->readBoolEntry( "Parentheses Matching",true);
showline=config->readBoolEntry( "Line Numbers",true);
editor_color[0]=QColor(0xFF, 0xFF, 0xFF);
editor_color[1]=QColor(0x00, 0x00, 0x00);
editor_color[2]=QColor(0x83, 0x81, 0x83);
editor_color[3]=QColor(0x00,0x80, 0x00);
editor_color[4]=QColor(0x80, 0x00, 0x00);
editor_color[5]=QColor(0x00, 0x00, 0xCC);
editor_color[6]=QColor(0xCC, 0x00, 0x00);
editor_color[7]=QColor(0xCC, 0xE8, 0xC3);
editor_color[0] = config->readColorEntry("Color Background",&editor_color[0]);
editor_color[1] = config->readColorEntry("Color Foreground",&editor_color[1]);
editor_color[2] = config->readColorEntry("Color Comment",&editor_color[2]);
editor_color[3] = config->readColorEntry("Color Math",&editor_color[3]);
editor_color[4] = config->readColorEntry("Color Command",&editor_color[4]);
editor_color[5] = config->readColorEntry("Color Structure",&editor_color[5]);
editor_color[6] = config->readColorEntry("Color Environment",&editor_color[6]);
editor_color[7] = config->readColorEntry("racket Highlight",&editor_color[7]);

config->setGroup( "Show" );
showoutputview=config->readBoolEntry("Outputview",true);
showstructview=config->readBoolEntry( "Structureview",true);
showmaintoolbar=config->readBoolEntry("ShowMainToolbar",true);
showtoolstoolbar=config->readBoolEntry("ShowToolsToolbar",true);
showedittoolbar=config->readBoolEntry("ShowEditToolbar",true);
showmathtoolbar=config->readBoolEntry("ShowMathToolbar",true);
menuaccels=config->readBoolEntry("MenuAccels", false);

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

config->setGroup( "Files" );
lastDocument=config->readPathEntry("Last Document");
//recentFilesList=config->readPathEntry("Recent Files", ':');
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
  if(config->hasKey("Recent Files")) {
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

config->setGroup( "User" );
templAuthor=config->readEntry("Author");
templDocClassOpt=config->readEntry("DocumentClassOptions","a4paper,10pt");
templEncoding=config->readEntry("Template Encoding");

userItem tempItem;
int len = config->readNumEntry("nUserTags",0);
for (int i = 0; i < len; i++)
{
	tempItem.name=config->readEntry("userTagName"+QString::number(i),i18n("no name"));
	tempItem.tag =config->readEntry("userTag"+QString::number(i));
	listUserTags.append(tempItem);
}

len= config->readNumEntry("nUserTools",0);
for (int i=0; i< len; i++)
{
	tempItem.name=config->readEntry("userToolName"+QString::number(i),i18n("no name"));
	tempItem.tag =config->readEntry("userTool"+QString::number(i));
	listUserTools.append(tempItem);
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
author=config->readEntry("Author");
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

config->setGroup( "Editor" );
config->writeEntry("Font Family",EditorFont.family());
config->writeEntry( "Font Size",EditorFont.pointSize());
config->writeEntry( "WordWrap",wordwrap);
config->writeEntry( "Parentheses Matching",parenmatch);
config->writeEntry( "Line Numbers",showline);
config->writeEntry("Color Background", editor_color[0]);
config->writeEntry("Color Foreground", editor_color[1]);
config->writeEntry("Color Comment", editor_color[2]);
config->writeEntry("Color Math", editor_color[3]);
config->writeEntry("Color Command", editor_color[4]);
config->writeEntry("Color Structure", editor_color[5]);
config->writeEntry("Color Environment", editor_color[6]);
config->writeEntry("Bracket Highlight", editor_color[7]);

config->setGroup( "Show" );
config->writeEntry("Outputview",showoutputview);
config->writeEntry( "Structureview",showstructview);
config->writeEntry("ShowMainToolbar",showmaintoolbar);
config->writeEntry("ShowToolsToolbar",showtoolstoolbar);
config->writeEntry("ShowEditToolbar",showedittoolbar);
config->writeEntry("ShowMathToolbar",showmathtoolbar);
config->writeEntry("MenuAccels", menuaccels);

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
config->writePathEntry("Last Document",lastDocument);
//config->writePathEntry("Recent Files", recentFilesList, ":");
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
config->writeEntry("nUserTags",static_cast<int>(listUserTags.size()));
for (uint i=0; i<listUserTags.size(); i++)
{
	tempItem = listUserTags[i];
	config->writeEntry("userTagName"+QString::number(i),tempItem.name);
	config->writeEntry("userTag"+QString::number(i),tempItem.tag);
}

config->writeEntry("nUserTools",static_cast<int>(listUserTools.size()));
for (uint i=0; i<listUserTools.size(); i++)
{
	tempItem = listUserTools[i];
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
if (singlemode && currentEditorView())  {
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
  menuaccels = MenuAccelsAction->isChecked();
  ToggleMenuShortcut(bar, menuaccels, i18n("&File"),         i18n("File"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Edit"),         i18n("Edit"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Tools"),        i18n("Tools"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&LaTeX"),        i18n("LaTeX"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Math"),         i18n("Math"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Wizard"),       i18n("Wizard"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Bibliography"), i18n("Bibliography"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&User"),         i18n("User"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Graph"),        i18n("Graph"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&View"),         i18n("View"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Settings"),     i18n("Settings"));
  ToggleMenuShortcut(bar, menuaccels, i18n("&Help"),         i18n("Help"));

  ToggleKeyShortcut(altH_action, menuaccels);
  ToggleKeyShortcut(altI_action, menuaccels);
  ToggleKeyShortcut(altA_action, menuaccels);
  ToggleKeyShortcut(altB_action, menuaccels);
  ToggleKeyShortcut(altT_action, menuaccels);
  ToggleKeyShortcut(altC_action, menuaccels);
  ToggleKeyShortcut(altM_action, menuaccels);
  ToggleKeyShortcut(altE_action, menuaccels);
  ToggleKeyShortcut(altD_action, menuaccels);
  ToggleKeyShortcut(altU_action, menuaccels);
  ToggleKeyShortcut(altF_action, menuaccels);
  ToggleKeyShortcut(altQ_action, menuaccels);
  ToggleKeyShortcut(altS_action, menuaccels);
  ToggleKeyShortcut(altL_action, menuaccels);
  ToggleKeyShortcut(altR_action, menuaccels);
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
      toolBar("ToolBar1")->show();
  } else {
    toolBar("ToolBar1")->hide();
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
for ( int i = 0; i <= 7; i++ )
    {
    toDlg->colors[i]=editor_color[i];
    }
toDlg->init();
toDlg->asIntervalInput->setText(QString::number(autosaveinterval/60000));
toDlg->templAuthor->setText(templAuthor);
toDlg->templDocClassOpt->setText(templDocClassOpt);
toDlg->templEncoding->setText(templEncoding);
toDlg->checkAutosave->setChecked(autosave);
toDlg->LineEdit6->setText(latex_command);
toDlg->LineEdit7->setText(pdflatex_command);
toDlg->comboFamily->lineEdit()->setText(EditorFont.family() );
toDlg->comboDvi->lineEdit()->setText(viewdvi_command );
toDlg->comboPdf->lineEdit()->setText(viewpdf_command );
toDlg->comboPs->lineEdit()->setText(viewps_command );
toDlg->spinSize->setValue(EditorFont.pointSize() );
if (quickmode==1) {toDlg->checkLatex->setChecked(true);}
if (quickmode==2) {toDlg->checkDvi->setChecked(true);}
if (quickmode==3) {toDlg->checkDviSearch->setChecked(true);}
if (quickmode==4)  {toDlg->checkPdflatex->setChecked(true);}
if (quickmode==5)  {toDlg->checkDviPdf->setChecked(true);}
if (quickmode==6)  {toDlg->checkPsPdf->setChecked(true);}
toDlg->checkWordWrap->setChecked(wordwrap);
toDlg->checkParen->setChecked(parenmatch);
toDlg->checkLine->setChecked(showline);
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
   QString fam=toDlg->comboFamily->lineEdit()->text();
   int si=toDlg->spinSize->value();
   QFont F(fam,si);
   EditorFont=F;
   wordwrap=toDlg->checkWordWrap->isChecked();
   parenmatch=toDlg->checkParen->isChecked();
   showline=toDlg->checkLine->isChecked();
   for ( int i = 0; i <= 7; i++ )
    {
    editor_color[i]=toDlg->colors[i];
    }
   if (currentEditorView())
  {
   LatexEditorView *temp = new LatexEditorView( tabWidget,"",EditorFont,parenmatch,showline,editor_color );
   temp=currentEditorView();
   FilesMap::Iterator it;
   for( it = filenames.begin(); it != filenames.end(); ++it )
      {
        tabWidget->showPage( it.key() );
        bool  MODIFIED =currentEditorView()->editor->isModified();
        QString tmp =currentEditorView()->editor->text();
        if (wordwrap) {currentEditorView()->editor->setWordWrap(LatexEditor::WidgetWidth);}
        else {currentEditorView()->editor->setWordWrap(LatexEditor::NoWrap);}
        currentEditorView()->changeSettings(EditorFont,parenmatch, showline, editor_color);
        currentEditorView()->editor->clear();
        currentEditorView()->editor->setText( tmp );
        if( MODIFIED ) currentEditorView()->editor->setModified(TRUE );
        else currentEditorView()->editor->setModified( FALSE );
      }
   tabWidget->showPage(temp);
   UpdateCaption();
   UpdateLineColStatus();
   }
  }
delete toDlg;
}
////////////// SPELL ///////////////
void Kile::spellcheck()
{
	if ( !currentEditorView() ) return;

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
	if ( currentEditorView()->editor->hasSelectedText() )
	{
		kspell->check(currentEditorView()->editor->selectedText());
		currentEditorView()->editor->getSelection(&par_start,&index_start,&par_end,&index_end,0);
	}
	else
	{
		kspell->check(currentEditorView()->editor->text());
		par_start=0;
		par_end=currentEditorView()->editor->paragraphs()-1;
	}
}

void Kile::spell_progress (unsigned int /*percent*/)
{
}

void Kile::spell_done(const QString& /*newtext*/)
{
  currentEditorView()->editor->removeSelection(0);
  kspell->cleanUp();
	KMessageBox::information(this,i18n("Corrected %1 words.").arg(ks_corrected),i18n("Spell checking done"));
}

void Kile::spell_finished( )
{
	UpdateLineColStatus();
	KSpell::spellStatus status = kspell->status();

	delete kspell;
	kspell = 0;
	if (status == KSpell::Error)
  {
     KMessageBox::sorry(this, i18n("I(A)Spell could not be started."));
  }
	else if (status == KSpell::Crashed)
  {
     currentEditorView()->editor->removeSelection(0);
     KMessageBox::sorry(this, i18n("I(A)Spell seems to have crashed."));
  }
}

void Kile::misspelling (const QString & originalword, const QStringList & /*suggestions*/,unsigned int pos)
{
  int l=par_start;
  int cnt=0;
  int col=0;
  int p=pos;

  while ((cnt+currentEditorView()->editor->paragraphLength(l)<=p) && (l < par_end))
  {
  cnt+=currentEditorView()->editor->paragraphLength(l)+1;
  l++;
  }
  col=p-cnt;
  currentEditorView()->editor->setCursorPosition(l,col);
  currentEditorView()->editor->setSelection( l,col,l,col+originalword.length(),0);
}


void Kile::corrected (const QString & originalword, const QString & newword, unsigned int pos)
{
  int l=par_start;
  int cnt=0;
  int col=0;
  int p=pos;
  if( newword != originalword )
  {
    while ((cnt+currentEditorView()->editor->paragraphLength(l)<=p) && (l < par_end))
    {
    cnt+=currentEditorView()->editor->paragraphLength(l)+1;
    l++;
    }
    col=p-cnt;
    currentEditorView()->editor->setCursorPosition(l,col);
    currentEditorView()->editor->setSelection( l,col,l,col+originalword.length(),0);
    currentEditorView()->editor->removeSelectedText();
    currentEditorView()->editor->insert( newword, FALSE, FALSE );
    currentEditorView()->editor->setModified( TRUE );
  }
  currentEditorView()->editor->removeSelection(0);

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
input_encoding=KileFS->comboEncoding->lineEdit()->text();
if (currentEditorView())
  {
   QTextCodec* codec1 = QTextCodec::codecForName(currentEditorView()->editor->getEncoding().latin1());
   if(!codec1) codec1 = QTextCodec::codecForLocale();
   QString tmp =currentEditorView()->editor->text();
   QString unicodetmp=codec1->toUnicode(tmp.latin1());
   QTextCodec* codec2 = QTextCodec::codecForName(input_encoding.latin1());
   if(!codec2) codec2 = QTextCodec::codecForLocale();
   QString newtmp= codec2->fromUnicode( unicodetmp );
   currentEditorView()->editor->clear();
   currentEditorView()->editor->setText( newtmp );
   currentEditorView()->editor->setModified(TRUE );
   currentEditorView()->editor->setEncoding(input_encoding);
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
if ( !currentEditorView() )	return;
int i=0;
while(i < currentEditorView()->editor->paragraphs())
   {
    s = currentEditorView()->editor->text(i);
    s=s.left(3);
    if (s=="OPT" || s=="ALT")
        {
        currentEditorView()->editor->removeParagraph (i );
        currentEditorView()->editor->setModified(true);
        }
    else i++;
   }
}

#include "kile.moc"
