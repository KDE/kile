/***************************************************************************
                          kile.cpp  -  description
                             -------------------
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 by Pascal Brachet, 2003 by Jeroen Wijnhout
    email                : 
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

#include <qiconset.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qfontdatabase.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfileinfo.h>
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

#include "templates.h"
#include "newfilewizard.h"
#include "managetemplatesdialog.h"

Kile::Kile( QWidget *parent, const char *name ): KParts::MainWindow( name, WDestructiveClose),DCOPObject( "Kile" )
{
config = KGlobal::config();
ReadSettings();
setXMLFile( "kileui.rc" );
htmlpresent=false;
pspresent=false;
dvipresent=false;
watchfile=false;
partManager = new KParts::PartManager( this );
connect( partManager, SIGNAL( activePartChanged( KParts::Part * ) ), this, SLOT(ActivePartGUI ( KParts::Part * ) ) );
kspell = 0;
setupActions();
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
ButtonBar->insertTab(UserIcon("math1"),2,"Relation Symbols");
connect(ButtonBar->getTab(2),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
ButtonBar->insertTab(UserIcon("math2"),3,"Arrow Symbols");
connect(ButtonBar->getTab(3),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
ButtonBar->insertTab(UserIcon("math3"),4,"Miscellaneous Symbols");
connect(ButtonBar->getTab(4),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
ButtonBar->insertTab(UserIcon("math4"),5,"Delimiters");
connect(ButtonBar->getTab(5),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
ButtonBar->insertTab(UserIcon("math5"),6,"Greek Letters");
connect(ButtonBar->getTab(6),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));
ButtonBar->insertTab(UserIcon("metapost"),7,"MetaPost Commands");
connect(ButtonBar->getTab(7),SIGNAL(clicked(int)),this,SLOT(showVertPage(int)));

splitter2=new QSplitter(QSplitter::Vertical,splitter1 , "splitter2" );
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
Outputview->addTab(LogWidget,UserIcon("viewlog"), i18n("Log"));

OutputWidget = new MessageWidget( Outputview );
OutputWidget->setFocusPolicy(QWidget::ClickFocus);
OutputWidget->setMinimumHeight(40);
OutputWidget->setReadOnly(true);
Outputview->addTab(OutputWidget,UserIcon("output_win"), i18n("Messages"));


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


showmaintoolbar=!showmaintoolbar;ToggleShowMainToolbar();
showtoolstoolbar=!showtoolstoolbar;ToggleShowToolsToolbar();
showedittoolbar=!showedittoolbar;ToggleShowEditToolbar();
showmathtoolbar=!showmathtoolbar;ToggleShowMathToolbar();

KileApplication::closeSplash();
show();
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
  (void) new KAction(i18n("New"),"filenew", CTRL+Key_N, this, SLOT(fileNew()), actionCollection(),"New" );
  (void) new KAction(i18n("Open..."),"fileopen", CTRL+Key_O, this, SLOT(fileOpen()), actionCollection(),"Open" );
  RecentAction=new KSelectAction(i18n("Open Recent"), 0, actionCollection(), "Recent");
  RecentAction->setItems(recentFilesList);
  connect(RecentAction, SIGNAL(activated(const QString &)), SLOT(fileOpenRecent(const QString &)));
  (void) new KAction(i18n("Save"),"filesave", CTRL+Key_S, this, SLOT(fileSave()), actionCollection(),"Save" );
  (void) new KAction(i18n("Save As..."),0, this, SLOT(fileSaveAs()), actionCollection(),"SaveAs" );
  (void) new KAction(i18n("Save All"),0, this, SLOT(fileSaveAll()), actionCollection(),"SaveAll" );
  (void) new KAction(i18n("Create Template From Document..."),0,this,SLOT(createTemplate()), actionCollection(),"CreateTemplate");
  (void) new KAction(i18n("Print Source..."),"fileprint",CTRL+Key_P, this, SLOT(filePrint()), actionCollection(),"PrintSource");
  (void) new KAction(i18n("Close"),"fileclose", CTRL+Key_W, this, SLOT(fileClose()), actionCollection(),"Close" );
  (void) new KAction(i18n("Close All"),0, this, SLOT(fileCloseAll()), actionCollection(),"CloseAll" );
  (void) new KAction(i18n("Exit"),"exit", CTRL+Key_Q, this, SLOT(fileExit()), actionCollection(),"Exit" );

  (void) new KAction(i18n("Undo"),"undo", CTRL+Key_Z, this, SLOT(editUndo()), actionCollection(),"Undo" );
  (void) new KAction(i18n("Redo"),"redo", CTRL+Key_Y, this, SLOT(editRedo()), actionCollection(),"Redo" );
  (void) new KAction(i18n("Copy"),"editcopy", CTRL+Key_C, this, SLOT(editCopy()), actionCollection(),"Copy" );
  (void) new KAction(i18n("Cut"),"editcut", CTRL+Key_X, this, SLOT(editCut()), actionCollection(),"Cut" );
  (void) new KAction(i18n("Paste"),"editpaste", CTRL+Key_V, this, SLOT(editPaste()), actionCollection(),"Paste" );
  (void) new KAction(i18n("Select All"), CTRL+Key_A, this, SLOT(editSelectAll()), actionCollection(),"selectAll" );
  (void) new KAction(i18n("Spelling"),"spellcheck", 0, this, SLOT(spellcheck()), actionCollection(),"Spell" );
  (void) new KAction(i18n("Comment Selection"),0, this, SLOT(editComment()), actionCollection(),"Comment" );
  (void) new KAction(i18n("Uncomment Selection"),0, this, SLOT(editUncomment()), actionCollection(),"Uncomment" );
  (void) new KAction(i18n("Indent Selection"),0, this, SLOT(editIndent()), actionCollection(),"Indent" );
  (void) new KAction(i18n("Find..."),"find",CTRL+Key_F , this, SLOT(editFind()), actionCollection(),"find" );
  (void) new KAction(i18n("Find Next"),"next",CTRL+Key_M , this, SLOT(editFindNext()), actionCollection(),"findnext" );
  (void) new KAction(i18n("Replace..."),CTRL+Key_R , this, SLOT(editReplace()), actionCollection(),"Replace" );
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
  (void) new KAction("Kdvi Forward Search","dvisearch",0, this, SLOT(KdviForwardSearch()), actionCollection(),"KdviForwardSearch" );
  (void) new KAction(i18n("Clean"),0 , this, SLOT(CleanAll()), actionCollection(),"CleanAll" );
  (void) new KAction(i18n("Mpost"),0 , this, SLOT(MetaPost()), actionCollection(),"MetaPost" );

  (void) new KAction(i18n("Editor View"),"edit",CTRL+Key_E , this, SLOT(ShowEditorWidget()), actionCollection(),"EditorView" );
  (void) new KAction(i18n("Next Document"),"down",ALT+Key_PageDown, this, SLOT(gotoNextDocument()), actionCollection(), "gotoNextDocument" );
  (void) new KAction(i18n("Previous Document"),"up",ALT+Key_PageUp, this, SLOT(gotoPrevDocument()), actionCollection(), "gotoPrevDocument" );

  BackAction=new KAction(i18n("Back"),"back",0 , this, SLOT(BrowserBack()), actionCollection(),"Back" );
  ForwardAction=new KAction(i18n("Forward"),"forward",0 , this, SLOT(BrowserForward()), actionCollection(),"Forward" );
  HomeAction=new KAction(i18n("Home"),"gohome",0 , this, SLOT(BrowserHome()), actionCollection(),"Home" );

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
  (void) new KAction("\\item","item",ALT+Key_H, this, SLOT(Insert20()), actionCollection(),"28" );

  (void) new KAction("\\textit - Italics","text_italic",ALT+Key_I, this, SLOT(Insert21()), actionCollection(),"29" );
  (void) new KAction("\\textsl - Slanted",ALT+Key_A, this, SLOT(Insert22()), actionCollection(),"30" );
  (void) new KAction("\\textbf - Boldface","text_bold",ALT+Key_B, this, SLOT(Insert23()), actionCollection(),"31" );
  (void) new KAction("\\texttt - Typewriter",ALT+Key_T, this, SLOT(Insert24()), actionCollection(),"32" );
  (void) new KAction("\\textsc - Small caps",ALT+Key_C, this, SLOT(Insert25()), actionCollection(),"33" );

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


  (void) new KAction("$...$","mathmode",ALT+Key_M, this, SLOT(InsertMath1()), actionCollection(),"52" );
  (void) new KAction("$$...$$",ALT+Key_E, this, SLOT(InsertMath2()), actionCollection(),"53" );
  (void) new KAction("\\begin{equation}",0, this, SLOT(InsertMath74()), actionCollection(),"54" );
  (void) new KAction("\\begin{eqnarray}",0, this, SLOT(InsertMath75()), actionCollection(),"55" );
  (void) new KAction("subscript  _{}","indice",ALT+Key_D, this, SLOT(InsertMath3()), actionCollection(),"56" );
  (void) new KAction("superscript  ^{}","puissance",ALT+Key_U, this, SLOT(InsertMath4()), actionCollection(),"57" );
  (void) new KAction("\\frac{}{}","smallfrac",ALT+Key_F, this, SLOT(InsertMath5()), actionCollection(),"58" );
  (void) new KAction("\\dfrac{}{}","dfrac",ALT+Key_Q, this, SLOT(InsertMath6()), actionCollection(),"59" );
  (void) new KAction("\\sqrt{}","racine",ALT+Key_S, this, SLOT(InsertMath7()), actionCollection(),"60" );
  (void) new KAction("\\left",ALT+Key_L, this, SLOT(InsertMath8()), actionCollection(),"61" );
  (void) new KAction("\\right",ALT+Key_R, this, SLOT(InsertMath9()), actionCollection(),"62" );
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

  (void) new KAction("Article in Journal",0 , this, SLOT(InsertBib1()), actionCollection(),"131" );
  (void) new KAction("Article in Conference Proceedings",0 , this, SLOT(InsertBib2()), actionCollection(),"132" );
  (void) new KAction("Article in Collection",0 , this, SLOT(InsertBib3()), actionCollection(),"133" );
  (void) new KAction("Chapter or Pages in Book",0 , this, SLOT(InsertBib4()), actionCollection(),"134" );
  (void) new KAction("Conference Proceedings",0 , this, SLOT(InsertBib5()), actionCollection(),"135" );
  (void) new KAction("Book",0 , this, SLOT(InsertBib6()), actionCollection(),"136" );
  (void) new KAction("Booklet",0 , this, SLOT(InsertBib7()), actionCollection(),"137" );
  (void) new KAction("PhD. Thesis",0 , this, SLOT(InsertBib8()), actionCollection(),"138" );
  (void) new KAction("Master's Thesis",0 , this, SLOT(InsertBib9()), actionCollection(),"139" );
  (void) new KAction("Technical Report",0 , this, SLOT(InsertBib10()), actionCollection(),"140" );
  (void) new KAction("Technical Manual",0 , this, SLOT(InsertBib11()), actionCollection(),"141" );
  (void) new KAction("Unpublished",0 , this, SLOT(InsertBib12()), actionCollection(),"142" );
  (void) new KAction("Miscellaneous",0 , this, SLOT(InsertBib13()), actionCollection(),"143" );
  (void) new KAction(i18n("Clean"),0 , this, SLOT(CleanBib()), actionCollection(),"CleanBib" );

  UserAction1=new KAction("1: "+UserMenuName[0],SHIFT+Key_F1 , this, SLOT(InsertUserTag1()), actionCollection(),"user1" );
  UserAction2=new KAction("2: "+UserMenuName[1],SHIFT+Key_F2 , this, SLOT(InsertUserTag2()), actionCollection(),"user2" );
  UserAction3=new KAction("3: "+UserMenuName[2],SHIFT+Key_F3 , this, SLOT(InsertUserTag3()), actionCollection(),"user3" );
  UserAction4=new KAction("4: "+UserMenuName[3],SHIFT+Key_F4 , this, SLOT(InsertUserTag4()), actionCollection(),"user4" );
  UserAction5=new KAction("5: "+UserMenuName[4],SHIFT+Key_F5 , this, SLOT(InsertUserTag5()), actionCollection(),"user5" );
  UserAction6=new KAction("6: "+UserMenuName[5],SHIFT+Key_F6 , this, SLOT(InsertUserTag6()), actionCollection(),"user6" );
  UserAction7=new KAction("7: "+UserMenuName[6],SHIFT+Key_F7 , this, SLOT(InsertUserTag7()), actionCollection(),"user7" );
  UserAction8=new KAction("8: "+UserMenuName[7],SHIFT+Key_F8 , this, SLOT(InsertUserTag8()), actionCollection(),"user8" );
  UserAction9=new KAction("9: "+UserMenuName[8],SHIFT+Key_F9 , this, SLOT(InsertUserTag9()), actionCollection(),"user9" );
  UserAction10=new KAction("10: "+UserMenuName[9],SHIFT+Key_F10 , this, SLOT(InsertUserTag10()), actionCollection(),"user10" );
  (void) new KAction(i18n("Edit User Tags"),0 , this, SLOT(EditUserMenu()), actionCollection(),"EditUserMenu" );

  UserToolAction1=new KAction("1: "+UserToolName[0],SHIFT+ALT+Key_F1 , this, SLOT(UserTool1()), actionCollection(),"usertool1" );
  UserToolAction2=new KAction("2: "+UserToolName[1],SHIFT+ALT+Key_F2 , this, SLOT(UserTool2()), actionCollection(),"usertool2" );
  UserToolAction3=new KAction("3: "+UserToolName[2],SHIFT+ALT+Key_F3 , this, SLOT(UserTool3()), actionCollection(),"usertool3" );
  UserToolAction4=new KAction("4: "+UserToolName[3],SHIFT+ALT+Key_F4 , this, SLOT(UserTool4()), actionCollection(),"usertool4" );
  UserToolAction5=new KAction("5: "+UserToolName[4],SHIFT+ALT+Key_F5 , this, SLOT(UserTool5()), actionCollection(),"usertool5" );
  (void) new KAction(i18n("Edit User Commands"),0 , this, SLOT(EditUserTool()), actionCollection(),"EditUserTool" );


  (void) new KAction("Xfig","xfig",0 , this, SLOT(RunXfig()), actionCollection(),"144" );
  (void) new KAction(i18n("Gnuplot Front End"),"xgfe",0 , this, SLOT(RunGfe()), actionCollection(),"145" );

  ModeAction=new KToggleAction(i18n("Define Current Document as 'Master Document'"),"master",0 , this, SLOT(ToggleMode()), actionCollection(),"Mode" );
  (void) new KAction(i18n("Configure Kile..."),"configure",0 , this, SLOT(GeneralOptions()), actionCollection(),"146" );
  (void) new KAction(i18n("Configure Shortcuts..."),"configure_shortcuts",0 , this, SLOT(ConfigureKeys()), actionCollection(),"147" );
  (void) new KAction(i18n("Configure Toolbars..."),"configure_toolbars",0 , this, SLOT(ConfigureToolbars()), actionCollection(),"148" );
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
  ListAction5 = new KSelectAction("Other", 0, actionCollection(), "other_list");
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
  ListAction3 = new KSelectAction("Left Delimiter", 0, actionCollection(), "left_list");
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
  ListAction4 = new KSelectAction("Right Delimiter", 0, actionCollection(), "right_list");
  ListAction4->setItems(list);
  connect(ListAction4,SIGNAL(activated(const QString&)),this,SLOT(RightDelimiter(const QString&)));

  (void) new KAction(i18n("New Line"),"newline",SHIFT+Key_Return , this, SLOT(NewLine()), actionCollection(),"151" );

  const KAboutData *aboutData = KGlobal::instance()->aboutData();
  help_menu = new KHelpMenu( this, aboutData);
  (void) new KAction(i18n("LaTeX Reference"),"help",0 , this, SLOT(LatexHelp()), actionCollection(),"help1" );
  (void) new KAction(i18n("User Manual"),"help",0 , this, SLOT(UserManualHelp()), actionCollection(),"help2" );
  (void) new KAction(i18n("About Kile"),QIconSet(kapp->miniIcon()),0 , help_menu, SLOT(aboutApplication()), actionCollection(),"help4" );
  (void) new KAction(i18n("About KDE"),"about_kde",0 , help_menu, SLOT(aboutKDE()), actionCollection(),"help5" );

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
    QTextCodec* codec = QTextCodec::codecForName(input_encoding);
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
    AddRecentFile(f);
    ShowStructure();
}

LatexEditorView *Kile::currentEditorView() const
{
    if ( tabWidget->currentPage() &&
	 tabWidget->currentPage()->inherits( "LatexEditorView" ) )
	return (LatexEditorView*)tabWidget->currentPage();
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
    edit->editor->setModified(false);
    doConnections( edit->editor );

    QString sel = nfw->getSelection();
    if (sel != DEFAULT_EMPTY_CAPTION) {
       QString name = "templates/template_"+QString(sel)+".tex";
       QFile f(KGlobal::dirs()->findResource("appdata", name));
       if (f.open(IO_ReadOnly) ) {
       QString line;
       while (f.readLine(line,80)>0) {
          edit->editor->append(line);
       }
       f.close();
       } else { KMessageBox::error(this, i18n("Couldn't find template: %1").arg(name),i18n("File Not Found!")); }
    }

    UpdateCaption();
    UpdateLineColStatus();
    }
}

void Kile::fileOpen()
{
QString currentDir=QDir::currentDirPath();
if (!lastDocument.isEmpty())
  {
  QFileInfo fi(lastDocument);
  if (fi.exists() && fi.isReadable()) currentDir=fi.dirPath();
  }
QString fn = KFileDialog::getOpenFileName( currentDir, "*.tex *.bib *.sty *.cls *.mp|TeX files\n*|All files", this,i18n("Open File") );
if ( !fn.isEmpty() ) load( fn );
}

void Kile::fileOpenRecent(const QString &fn)
{
if ( !fn.isEmpty() ) load( fn );
RecentAction->clear();
RecentAction->setItems(recentFilesList);
}


void Kile::AddRecentFile(const QString &f)
{
lastDocument=f;
QStringList::ConstIterator it1 = recentFilesList.begin();
for ( ; it1 != recentFilesList.end(); ++it1 )
    {
    if (*it1==f) return;
    }
if( recentFilesList.count() ==5 )
{
   recentFilesList.remove( recentFilesList.last() );
}
recentFilesList.prepend( f );

RecentAction->clear();
RecentAction->setItems(recentFilesList);
}

bool Kile::FileAlreadyOpen(QString f)
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

void Kile::fileSave()
{
if ( !currentEditorView() )	return;
QString fn;
if ( getName()=="untitled" ) {fileSaveAs();}
else
  {
	QFile file( *filenames.find( currentEditorView() ) );
  if ( !file.open( IO_WriteOnly ) )
      {
       KMessageBox::sorry(this,i18n("The file could not be saved. Please check if you have write permission."));
       return;
      }
	QTextStream ts( &file );
	ts.setEncoding(QTextStream::Locale);
	QTextCodec* codec = QTextCodec::codecForName(currentEditorView()->editor->getEncoding());
  ts.setCodec(codec ? codec : QTextCodec::codecForLocale());
	ts << currentEditorView()->editor->text();
  currentEditorView()->editor->setModified(false);
  fn=getName();
  AddRecentFile(fn);
  }
UpdateCaption();
UpdateLineColStatus();
}

void Kile::fileSaveAs()
{
int query=KMessageBox::Yes;
if ( !currentEditorView() ) 	return;
QString fn = KFileDialog::getSaveFileName( QString::null,"*.tex *.bib *.sty *.cls *.mp|TeX Files\n*|All Files", this,i18n("Save As") );
if ( !fn.isEmpty() )
    {
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

void Kile::fileSaveAll()
{
LatexEditorView *temp = new LatexEditorView( tabWidget,"",EditorFont,parenmatch,showline,editor_color);
temp=currentEditorView();
FilesMap::Iterator it;
for( it = filenames.begin(); it != filenames.end(); ++it )
   {
   tabWidget->showPage( it.key() );
   fileSave();
   }
tabWidget->showPage(temp);
UpdateCaption();
UpdateLineColStatus();
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

   QFileInfo fi(getName());
   ManageTemplatesDialog mtd(fi,i18n("Create Template From Document"));
   mtd.exec();
}

void Kile::filePrint()
{
KPrinter printer;
QString finame;
if (!htmlpresent )
 {
    if ( !currentEditorView() )	return;
    finame=getName();
    if (finame=="untitled" || finame=="")
      {
      KMessageBox::error( this,i18n("Could not start the command."));
      return;
      }
    fileSave();
    QFileInfo fic(finame);
    if (fic.exists() && fic.isReadable() )
       {
         if ( printer.setup(this) )
         {
         QSize margins = printer.margins();
         int marginHeight = margins.height();
         QPainter p;
         if( !p.begin( &printer ) )  return;
         int yPos        = 0;
         p.setFont( EditorFont );
         QFontMetrics fm = p.fontMetrics();
         QPaintDeviceMetrics metrics( &printer );
         QFile f( finame );
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
  linenumber.sprintf(i18n("Line: %d Col: %d"), para + 1, index + 1);
  statusBar()->changeItem(linenumber.data(), ID_LINE_COLUMN);
  }
}

void Kile::NewDocumentStatus(bool m)
{
if ( !currentEditorView() )	return;
if (m) tabWidget->changeTab( currentEditorView(),UserIcon("modified"), QFileInfo( getName() ).fileName() );
else tabWidget->changeTab( currentEditorView(),UserIcon("empty"), QFileInfo( getName() ).fileName() );
}

QString Kile::getName()
{
QString title;
if ( !currentEditorView() )	{title="";}
else {title=filenames[currentEditorView()];}
return title;
}

void Kile::UpdateCaption()
{
QString title;
if   ( !currentEditorView() )	{title=i18n("No Document");}
else
   {
   title=i18n("Document :")+" "+getName();
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
      KMessageBox::error( this,i18n("Could not start ") + compile_command + i18n(". Make sure this package is installed on your system."));
   }
   else
   {
      OutputWidget->clear();
      LogWidget->clear();
      Outputview->showPage(LogWidget);
      logpresent=false;
      LogWidget->insertLine(i18n("Quick build..."));
      LogWidget->insertLine(i18n("Launched: ") + proc->command());
      LogWidget->insertLine(i18n("Compilation..."));
   }

   UpdateLineColStatus();
}

void Kile::EndQuickCompile()
{
switch (quickmode)
 {
  case 1:
    {
    ViewLog();
    QuickLatexError();
    if (errorlist->isEmpty()) {QuickDviToPS();}
    else {NextError();}
    }break;
  case 2:
    {
    ViewLog();
    QuickLatexError();
    if (errorlist->isEmpty() && !watchfile) {ViewDvi();}
    else {NextError();}
    }break;
 case 3:
    {
    ViewLog();
    QuickLatexError();
    if (errorlist->isEmpty()) {KdviForwardSearch();}
    else {NextError();}
    }break;
 case 4:
    {
    ViewLog();
    QuickLatexError();
    if (errorlist->isEmpty() && !watchfile) {ViewPDF();}
    else {NextError();}
    }break;
 case 5:
    {
    ViewLog();
    QuickLatexError();
    if (errorlist->isEmpty()) {QuickDviPDF();}
    else {NextError();}
    }break;
 case 6:
    {
    ViewLog();
    QuickLatexError();
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
      KMessageBox::error( this,i18n("Could not start ")+ dvips_command + i18n(". Make sure this package is installed on your system."));
  }
  else
  {
      logpresent=false;
      LogWidget->insertLine("DviPs...");
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
     KMessageBox::error( this,i18n("Could not start ")+dvipdf_command+i18n(". Make sure this package is installed on your system."));
  }
  else
  {
     logpresent=false;
     LogWidget->insertLine("DviPdf ...");
  }

  UpdateLineColStatus();
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
     KMessageBox::error( this,i18n("Could not start ") + ps2pdf_command + i18n(". Make sure this packages is installed on your system."));
  }
  else
  {
     logpresent=false;
     LogWidget->insertLine(i18n("Ps2Pdf..."));
  }
  
  UpdateLineColStatus();
}


/////////////////// TOOLS /////////////////////////

//command : a list representing the command to be started
//          i.e. latex %.tex -interactionmode=nonstop
//          is represented by the list (latex,%S.tex,-interactionmode=nonstop)
//file    : the file to be passed as an argument to the command, %S is substituted
//          by the basename of this file
//enablestop : whether or not this process can be stopped by pressing the STOP button
CommandProcess* Kile::execCommand(const QStringList &command, const QFileInfo &file, bool enablestop) {
 //substitute %S for the basename of the file
 QStringList cmmnd = command;
 QString dir = file.dirPath();
 QString name = file.baseName();
                               
 CommandProcess* proc = new CommandProcess();
 currentProcess=proc;
 proc->clearArguments();
 
 KRun::shellQuote(const_cast<QString&>(dir));
 (*proc) << "cd " << dir << "&&";

 for ( QValueListIterator<QString> i = cmmnd.begin(); i != cmmnd.end(); i++) {
   (*i).replace("%S",name);
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
  QString finame = getName();
  if (finame == "untitled") {
     if (KMessageBox::warningYesNo(this,i18n("You need to save an untitled document before you run ")+command+i18n(" on it. Do you want to save it? Click Yes to save and No to abort."),"File Needs to be Saved!")
         == KMessageBox::No) return QString::null;
  }

  //save the file before doing anything
  //attempting to save an untitled document will result in a file-save dialog pop-up
  fileSave();

  //determine the name of the file to be compiled
  if (singlemode) {finame=getName();}
  else {
     finame=MasterName; //FIXME: MasterFile does not get saved if it is modified
  }

  //we need to check for finame=="untitled" etc. because the user could have
  //escaped the file save dialog
  if ((singlemode && !currentEditorView()) || finame=="untitled" || finame=="")
  {
     KMessageBox::error( this,i18n("Could not start the ") +command+i18n(" command, because there is no file to run "+command+" on. Make sure you have the file you want to compile open and saved."));
     return QString::null;
  }

  QFileInfo fic(finame);
  
  if (!fic.exists() )
  {
     KMessageBox::error(this,i18n("The file ")+finame+i18n(" does not exist. Are you working with a master document which is accidently deleted?"));
     return QString::null;
  }

  if (!fic.isReadable() )
  {
     KMessageBox::error(this, i18n("You do not have read permission for the file: ") + finame);
     return QString::null;
  }
  
  return finame;
}

QStringList Kile::prepareForConversion(const QString &command, const QString &from, const QString &to)
{
   QStringList list,empty;
   QString finame, fromName, toName;
   if (singlemode) {finame=getName();}
   else {
     finame=MasterName; 
   }

   if (finame == "untitled") {
      KMessageBox::error(this,i18n("You need to save an untitled document and make a ") + from.upper()
                                + i18n(" file out of it. After you have done this, you can turn it into a ")+to.upper()
                                + i18n(" file."),"File needs to be saved and compiled!");
      return empty;   
   }

   if ((singlemode && !currentEditorView()) || finame=="")
   {
     KMessageBox::error( this,i18n("Could not start the ") +command+i18n(" command, because there is no file to run "+command+" on. Make sure you have the source file of the file you want to convert open and saved."));
     return empty;
   }

   QFileInfo fic(finame);
   fromName = fic.dirPath() + "/" +fic.baseName() + "." + from;
   toName = fic.dirPath() + "/" +fic.baseName() + "." + to;

   fic.setFile(fromName);
   if (!(fic.exists() && fic.isReadable()))
   {
      KMessageBox::error(this, i18n("The ")+ from.upper() + i18n(" file does not exists or you do not have read permission. Did you forget to compile to source file to turn it into a ") + from.upper() + i18n(" file?"));
   }

   list.append(fromName);
   list.append(toName);
   
   return list;
}

QString Kile::prepareForViewing(const QString & command, const QString &ext)
{
   QString finame;
   if (singlemode) {finame=getName();}
   else {
     finame=MasterName;
   }

   if (finame == "untitled") {
      KMessageBox::error(this,i18n("You need to save an untitled document and make a ") + ext.upper()
                                + i18n(" file out of it. After you have done this, you can view the ")+ext.upper()
                                + i18n(" file."),"File needs to be saved and compiled!");
      return QString::null;
   }

   if ((singlemode && !currentEditorView()) || finame=="")
   {
     KMessageBox::error( this,i18n("Unable to determine which ") +ext.upper()+i18n(" file to show. Please open the source file of the ") + ext.upper() + i18n(" file to want to view."));
     return QString::null;
   }

   QFileInfo fic(finame);
   finame = fic.dirPath() + "/" + fic.baseName() + "." + ext;
   fic.setFile(finame);

   if ( ! (fic.exists() && fic.isReadable() ) )
   {
      KMessageBox::error(this,i18n("The ") + ext.upper() + i18n(" file does not exists or you do not have read permission. Maybe you forgot to create the ") + ext.upper() + i18n(" file?"));
      return QString::null;
   }
   
   return finame;
}

void Kile::Latex()
{
  QString finame;
  if ( (finame=prepareForCompile("LaTeX")) == QString::null)  return;
  
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
     Outputview->showPage(LogWidget);
     logpresent=false;
     LogWidget->insertLine(i18n("Launched: ") + proc->command());
  }

  UpdateLineColStatus();
}


void Kile::ViewDvi()
{
  QString finame;
  if ( (finame=prepareForViewing("ViewDvi","dvi")) == QString::null) return;

  QFileInfo fic(finame);

  if (viewdvi_command=="Embedded viewer")
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
       KMessageBox::error( this,i18n("Could not start ")+ viewdvi_command + i18n(". make sure you have this package installed.") );
    }
    else
    {
         OutputWidget->clear();
         LogWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ") + proc->command());
     }
  }
 
  UpdateLineColStatus();
}

void Kile::KdviForwardSearch()
{
  QString finame;
  if ( (finame = prepareForViewing("KDVIForwardSearch","dvi")) == QString::null) return;
  
  LogWidget->clear();
  Outputview->showPage(LogWidget);
  logpresent=false;
  LogWidget->insertLine("You must be in 'Normal mode' to use this command.");
  LogWidget->insertLine("If you do not have a TeX-binary which includes inverse search information natively :");
  LogWidget->insertLine("- copy the files srcltx.sty and srctex.sty to the directory where your TeX-file resides.");
  LogWidget->insertLine("- add the line \\usepackage[active]{srcltx} to the preamble of your TeX-file.");
  LogWidget->insertLine("(see the kdvi handbook for more details)");

  QFileInfo fic(finame);
  QString dviname=finame;
  QString texname=fic.baseName()+".tex";
  int para=0;
  int index=0;
  currentEditorView()->editor->viewport()->setFocus();
  currentEditorView()->editor->getCursorPosition( &para, &index);
  if (viewdvi_command=="Embedded viewer")
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
       LogWidget->insertLine(i18n("Launched: ") + "kdvi");
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
      KMessageBox::error( this,i18n("Could not start ") + dvips_command + i18n(". Make sure this package is installed on your system."));
  }
  else
  {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ") + proc->command());

  }

  UpdateLineColStatus();
}

void Kile::ViewPS()
{
  QString finame;
  if ( (finame=prepareForViewing("ViewPS","ps")) == QString::null) return;
  
  QFileInfo fic(finame);

   if (viewps_command=="Embedded viewer")
   {
   ResetPart();
   KLibFactory *psfactory;
   psfactory = KLibLoader::self()->factory("libkghostviewpart");
   if (!psfactory)
      {
      KMessageBox::error(this, i18n("Couldn't find the embedded postscript viewer! Install kviewshell."));
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
       KMessageBox::error( this,i18n("Could not start ") + viewps_command + i18n(". Make sure this package is installed on your system"));
    }
    else
        {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched:")+proc->command());
         }
    }

    UpdateLineColStatus();
}

void Kile::PDFLatex()
{
  QString finame;
  if ( (finame= prepareForCompile("PDFLaTeX")) == QString::null) return;

  QFileInfo fic(finame);

  QStringList command; command << pdflatex_command;
  CommandProcess *proc=execCommand(command,fic,true);
  connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
     KMessageBox::error( this,i18n("Could not start PDFLaTeX, make sure you have this package installed on your system."));
  }
  else
  {
     OutputWidget->clear();
     Outputview->showPage(LogWidget);
     logpresent=false;
     LogWidget->insertLine(i18n("Launched: ")+ proc->command());
  }

  UpdateLineColStatus();
}

void Kile::ViewPDF()
{
  QString finame;
  if ( (finame = prepareForViewing("ViewPDF","pdf")) == QString::null ) return;

  QFileInfo fic(finame);
   if (viewpdf_command=="Embedded viewer")
   {
   ResetPart();
   KLibFactory *psfactory;
   psfactory = KLibLoader::self()->factory("libkghostviewpart");
   if (!psfactory)
      {
      KMessageBox::error(this, i18n("Couldn't find the embedded pdf viewer! Install kviewshell."));
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
       KMessageBox::error( this,i18n("Could not start ") + viewpdf_command + i18n(". Make sure this package is installed on your system."));
    }
    else
        {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ") + proc->command());
         }
    }


 UpdateLineColStatus();
}

void Kile::MakeBib()
{
  QString finame = getName();
  if (finame == "untitled") {
     KMessageBox::error(this,i18n("You need to save this file first. Then run LaTeX to create an AUX file which is required to run ")+bibtex_command,i18n("File needs to be saved!"));
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
     KMessageBox::error( this,i18n("Unable to determine on which file to run ") +bibtex_command+i18n(" . Make sure you have the source file of the file you want to run ")+bibtex_command+i18n(" on  open and saved."));
     return;
  }

  QFileInfo fic(finame);
  finame = fic.dirPath()+"/"+fic.baseName()+".aux";
  fic.setFile(finame);

  if (!(fic.exists() && fic.isReadable()) )
  {
     KMessageBox::error(this,i18n("The file ")+finame+i18n(" does not exist or you do not have read permission. You need to run LaTeX to create this file."));
     return;
  }

  //QString name=fic.dirPath()+"/"+fic.baseName();
  //fic.setFile(name);
	
    QStringList command; command << bibtex_command;
    CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
    {
       KMessageBox::error( this,i18n("Could not start ") + bibtex_command + i18n(". Make sure this package is installed on your system."));
    }
    else
        {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ")+proc->command());
         }


 UpdateLineColStatus();
}

void Kile::MakeIndex()
{
  //TODO: figure out how makeindex works ;-))
  //I'm just guessing here
  
  QString finame = getName();
  if (finame == "untitled") {
     KMessageBox::error(this,i18n("You need to save this file first. Then run LaTeX to create an idx file which is required to run ")+makeindex_command,i18n("File needs to be saved!"));
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
     KMessageBox::error( this,i18n("Unable to determine on which file to run ") +makeindex_command+i18n(" . Make sure you have the source file of the file you want to run ")+makeindex_command+i18n(" on  open and saved."));
     return;
  }

  QFileInfo fic(finame);
  finame = fic.dirPath()+"/"+fic.baseName()+".idx";
  fic.setFile(finame);

  if (!(fic.exists() && fic.isReadable()) )
  {
     KMessageBox::error(this,i18n("The file ")+finame+i18n(" does not exist or you do not have read permission. You need to run LaTeX to create this file."));
     return;
  }

  
    QStringList command; command << makeindex_command;
    CommandProcess *proc=execCommand(command,fic,true);
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

    if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )  { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ")+ proc->command());
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
       KMessageBox::error( this,i18n("Could not start ") + ps2pdf_command + i18n(". Make sure this packages is installed on your system."));
    }
    else
        {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ")+proc->command());
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
       KMessageBox::error( this,i18n("Could not start ")+dvipdf_command+i18n(". Make sure this package is installed on your system."));
    }
    else
        {
         OutputWidget->clear();
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ")+ proc->command());
         }

  UpdateLineColStatus();
}

void Kile::MetaPost()
{
  //TODO: what the h*ll is MetaPost, how should we deal with the
  //error messages?
  
  QString finame;

  finame=getName();
  if (!currentEditorView() ||finame=="untitled" || finame=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();
  QFileInfo fi(finame);
  QString name=fi.dirPath()+"/"+fi.baseName()+".mp";
  QString mpname=fi.baseName()+".mp";
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
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ") + "mpost");
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
  QString finame;

  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
  
  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="")
  {
     KMessageBox::error( this,i18n("Unable to determine what to clean-up. Make sure you have the file opened and saved, then choose Clean All."));
     return;
  }
  
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
      prettyList.append(fic.baseName()+extlist[i]);
      command << "rm -f" << fic.baseName()+extlist[i];
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
         Outputview->showPage(LogWidget);
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
    if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="") {return;}
    QFileInfo fi(finame);
    QString texname=fi.dirPath()+"/"+fi.baseName()+".tex";
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

    l2hDlg = new l2hdialog(this,i18n("LaTex2Html Options"));
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
         Outputview->showPage(LogWidget);
         logpresent=false;
         LogWidget->insertLine(i18n("Launched: ")+ "latex2html");
        }
    }
    delete (l2hDlg);
  
   UpdateLineColStatus();
}

void Kile::slotProcessOutput(KProcess* proc,char* buffer,int buflen)
{
int row = (OutputWidget->paragraphs() == 0)? 0 : OutputWidget->paragraphs()-1;
int col = OutputWidget->paragraphLength(row);
QString s=QCString(buffer,buflen+1);
OutputWidget->setCursorPosition(row,col);
OutputWidget->insertAt(s, row, col);
}

void Kile::slotProcessExited(KProcess* proc)
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
LogWidget->clear();
Outputview->showPage(LogWidget);
logpresent=false;
QString finame;

//TODO: add sanity tests here (make error messages more specific)
if (singlemode) {finame=getName();}
else {finame=MasterName;}
if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="" )
 {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
 }
fileSave();
QFileInfo fi(finame);
QString name=fi.baseName();
QString htmlname=fi.dirPath()+"/"+name+"/index.html";
QFileInfo fih(htmlname);
if (fih.exists() && fih.isReadable() )
  {
    ResetPart();
    htmlpart = new docpart(topWidgetStack,"help");
    connect(htmlpart,    SIGNAL(updateStatus(bool, bool)), SLOT(updateNavAction( bool, bool)));
    htmlpresent=true;
    htmlpart->openURL(htmlname);
    htmlpart->addToHistory(htmlname);
    topWidgetStack->addWidget(htmlpart->widget() , 1 );
    topWidgetStack->raiseWidget(1);
    partManager->addPart(htmlpart, true);
    partManager->setActivePart( htmlpart);
  }
else
  {
   KMessageBox::error(this, i18n("HTML file not found!"));
  }
}

void Kile::UserTool1()
{
  QString finame;
  QString commandline=UserToolCommand[0];
  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="" || commandline=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();
  QFileInfo fi(finame);
	if (fi.exists() && fi.isReadable() )
  {
    KShellProcess* proc = new KShellProcess("/bin/sh");
    proc->clearArguments();
    QString docdir=fi.dirPath();
    KRun::shellQuote(docdir);
    commandline.replace(QRegExp("%"),fi.baseName());
    (*proc) << "cd " << docdir << "&&";
    (*proc) << commandline ;
    connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         Outputview->showPage(OutputWidget);
         logpresent=false;
         OutputWidget->insertLine(i18n("Process launched"));
         }
  }
 else
 {
  KMessageBox::error(this, i18n("File not found!"));
 }
UpdateLineColStatus();
}

void Kile::UserTool2()
{
  QString finame;
  QString commandline=UserToolCommand[1];
  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="" || commandline=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();
  QFileInfo fi(finame);
	if (fi.exists() && fi.isReadable() )
  {
    KShellProcess* proc = new KShellProcess("/bin/sh");
    proc->clearArguments();
    QString docdir=fi.dirPath();
    KRun::shellQuote(docdir);
    commandline.replace(QRegExp("%"),fi.baseName());
    (*proc) << "cd " << docdir << "&&";
    (*proc) << commandline ;
    connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         Outputview->showPage(OutputWidget);
         logpresent=false;
         OutputWidget->insertLine(i18n("Process launched"));
         }
  }
 else
 {
  KMessageBox::error(this, i18n("File not found!"));
 }
UpdateLineColStatus();
}

void Kile::UserTool3()
{
  QString finame;
  QString commandline=UserToolCommand[2];
  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="" || commandline=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();
  QFileInfo fi(finame);
	if (fi.exists() && fi.isReadable() )
  {
    KShellProcess* proc = new KShellProcess("/bin/sh");
    proc->clearArguments();
    QString docdir=fi.dirPath();
    KRun::shellQuote(docdir);
    commandline.replace(QRegExp("%"),fi.baseName());
    (*proc) << "cd " << docdir << "&&";
    (*proc) << commandline ;
    connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         Outputview->showPage(OutputWidget);
         logpresent=false;
         OutputWidget->insertLine(i18n("Process launched"));
         }
  }
 else
 {
  KMessageBox::error(this, i18n("File not found!"));
 }
UpdateLineColStatus();
}

void Kile::UserTool4()
{
  QString finame;
  QString commandline=UserToolCommand[3];
  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="" || commandline=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();
  QFileInfo fi(finame);
	if (fi.exists() && fi.isReadable() )
  {
    KShellProcess* proc = new KShellProcess("/bin/sh");
    proc->clearArguments();
    QString docdir=fi.dirPath();
    KRun::shellQuote(docdir);
    commandline.replace(QRegExp("%"),fi.baseName());
    (*proc) << "cd " << docdir << "&&";
    (*proc) << commandline ;
    connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         Outputview->showPage(OutputWidget);
         logpresent=false;
         OutputWidget->insertLine(i18n("Process launched"));
         }
  }
 else
 {
  KMessageBox::error(this, i18n("File not found!"));
 }
UpdateLineColStatus();
}

void Kile::UserTool5()
{
  QString finame;
  QString commandline=UserToolCommand[4];
  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
  if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="" || commandline=="")
  {
  KMessageBox::error( this,i18n("Could not start the command."));
  return;
  }
  fileSave();
  QFileInfo fi(finame);
	if (fi.exists() && fi.isReadable() )
  {
    KShellProcess* proc = new KShellProcess("/bin/sh");
    proc->clearArguments();
    QString docdir=fi.dirPath();
    KRun::shellQuote(docdir);
    commandline.replace(QRegExp("%"),fi.baseName());
    (*proc) << "cd " << docdir << "&&";
    (*proc) << commandline ;
    connect(proc, SIGNAL( receivedStdout(KProcess*, char*, int) ), this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL( receivedStderr(KProcess*, char*, int) ),this, SLOT(slotProcessOutput(KProcess*, char*, int ) ) );
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*)));
    if ( !proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) { KMessageBox::error( this,i18n("Could not start the command."));}
    else
        {
         OutputWidget->clear();
         Outputview->showPage(OutputWidget);
         logpresent=false;
         OutputWidget->insertLine(i18n("Process launched"));
         }
  }
 else
 {
  KMessageBox::error(this, i18n("File not found!"));
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
QString shortName = getName();
if ((shortName.right(4)!=".tex") && (shortName!="untitled"))  return;
int pos;
while ( (pos = (int)shortName.find('/')) != -1 )
shortName.remove(0,pos+1);
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
    s=s.mid(tagStart+7,qstrlen(s));
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
    s=s.mid(tagStart+8,qstrlen(s));
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
    s=s.mid(tagStart+6,qstrlen(s));
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
    s=s.mid(tagStart+qstrlen(struct_level1),qstrlen(s));
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
    s=s.mid(tagStart+qstrlen(struct_level2),qstrlen(s));
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
    s=s.mid(tagStart+qstrlen(struct_level3),qstrlen(s));
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
    s=s.mid(tagStart+qstrlen(struct_level4),qstrlen(s));
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
    s=s.mid(tagStart+qstrlen(struct_level5),qstrlen(s));
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

void Kile::ClickedOnStructure(QListViewItem *)
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
bool ok;
QString s=*it2;
if (s!="include" && s!="input")
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
    if (fname.right(5)==".tex}") fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,qstrlen(fname)-2);
    else fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,qstrlen(fname)-2)+".tex";
    QFileInfo fi(fname);
    if (fi.exists() && fi.isReadable())
      {
      load(fname);
      }
    }
else if (s=="input")
    {
    QString fname=*it1;
    if (fname.right(5)==".tex}") fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,qstrlen(fname)-2);
    else fname=QFileInfo(getName()).dirPath()+"/"+fname.mid(1,qstrlen(fname)-2)+".tex";
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
LogWidget->clear();
logpresent=false;
QString finame;
  if (singlemode) {finame=getName();}
  else {finame=MasterName;}
if ((singlemode && !currentEditorView()) ||finame=="untitled" || finame=="")
   {
   KMessageBox::error( this,i18n("Could not start the command."));
   return;
   }
QFileInfo fi(finame);
QString name=fi.dirPath()+"/"+fi.baseName()+".log";
QString logname=fi.baseName()+".log";
QFileInfo fic(name);
if (fic.exists() && fic.isReadable() )
  {
  LogWidget->insertLine("************** LOG FILE *************** :");
  QFile f(name);
		if ( f.open(IO_ReadOnly) )
      {
			QTextStream t( &f );
			QString s;
			while ( !t.eof() )
       {
				s = t.readLine();
        int row = (LogWidget->paragraphs() == 0)? 0 : LogWidget->paragraphs()-1;
        int col = qstrlen(LogWidget->text(row));
        if (s.left(1) == "\n" && col == 0)  s = QString(" ")+s;
        LogWidget->insertLine(s);
        }
		  }
  	f.close();
    logpresent=true;
    LogWidget->highlight();
    LatexError();
  }
else {KMessageBox::error( this,i18n("Log file not found!"));}
UpdateLineColStatus();
}

void Kile::ClickedOnOutput(int parag, int index)
{

if ( !currentEditorView() ) return;
 int Start, End;
 bool ok;
 QString s;
 QString line="";
 //// l. ///
 s = LogWidget->text(parag);
 Start=End=0;
 Start=s.find(QRegExp("l.[0-9]"), End);
 if (Start!=-1)
  {
  Start=Start+2;
  s=s.mid(Start,qstrlen(s));
  End=s.find(QRegExp("[ a-zA-Z.\\-]"),0);
  if (End!=-1)
    line=s.mid(0,End);
  else
    line=s.mid(0,qstrlen(s));
  };
 //// line ///
 s = LogWidget->text(parag);
 Start=End=0;
 Start=s.find(QRegExp("line [0-9]"), End);
 if (Start!=-1)
  {
  Start=Start+5;
  s=s.mid(Start,qstrlen(s));
  End=s.find(QRegExp("[ a-zA-Z.\\-]"),0);
  if (End!=-1)
    line=s.mid(0,End);
  else
    line=s.mid(0,qstrlen(s));
  };
 //// lines ///
 s = LogWidget->text(parag);
 Start=End=0;
 Start=s.find(QRegExp("lines [0-9]"), End);
 if (Start!=-1)
  {
  Start=Start+6;
  s=s.mid(Start,qstrlen(s));
  End=s.find(QRegExp("[ a-zA-Z.\\-]"),0);
  if (End!=-1)
    line=s.mid(0,End);
  else
    line=s.mid(0,qstrlen(s));
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
void Kile::LatexError()
{
errorlist->clear();
QString s;
for(int i = 0; i < LogWidget->paragraphs(); i++)
 {
 s = LogWidget->text(i);
 int tagStart, tagEnd;
 //// ! ////
 tagStart=tagEnd=0;
 tagStart=s.find("!", tagEnd);
 if (tagStart==0)
  {
    errorlist->append(QString::number(i));
  };
 //// latex warning ////
 tagStart=tagEnd=0;
 tagStart=s.find("LaTeX Warning", tagEnd);
 if (tagStart!=-1)
  {
    errorlist->append(QString::number(i));
  };
 }
}

void Kile::QuickLatexError()
{
errorlist->clear();
QString s;
for(int i = 0; i < LogWidget->paragraphs(); i++)
 {
 s = LogWidget->text(i);
 int tagStart, tagEnd;
 //// ! ////
 tagStart=tagEnd=0;
 tagStart=s.find("!", tagEnd);
 if (tagStart==0)
  {
    errorlist->append(QString::number(i));
  };
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
  int id=errorlist->findRef(errorlist->next());
  if (id>=0)
  {
  line=errorlist->at(id);
  }
  else
  {
  line=errorlist->at(0);
  }
  int l=line.toInt(&ok,10);
  if (ok && l<=LogWidget->paragraphs())
    {
    LogWidget->setCursorPosition(0 , 0);
    LogWidget->setCursorPosition(l+3 , 0);
    }
  }
if (logpresent && errorlist->isEmpty())
  {
LogWidget->insertLine(i18n("No LaTeX errors detected!"));
  }
}

void Kile::PreviousError()
{
QString line="";
bool ok;
if (!logpresent) {ViewLog();}
if (logpresent && !errorlist->isEmpty())
  {
  Outputview->showPage(LogWidget);
  int id=errorlist->findRef(errorlist->prev());
  if (id>=0)
  {
  line=errorlist->at(id);
  }
  else
  {
  line=errorlist->at(errorlist->count()-1);
  }
  int l=line.toInt(&ok,10);
  if (ok && l<=LogWidget->paragraphs())
    {
    LogWidget->setCursorPosition(0 , 0 );
    LogWidget->setCursorPosition(l+3 , 0);
    }
  }

if (logpresent && errorlist->isEmpty())
  {
LogWidget->insertLine(i18n("No LaTeX errors detected!"));
  }
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
  OutputWidget->clear();
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
	startDlg = new quickdocumentdialog(this,i18n("Quick Start"));
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
	quickDlg = new tabdialog(this,i18n("Tabular"));
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
  tabDlg = new tabbingdialog(this,"Tabbing");
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
	arrayDlg = new arraydialog(this,i18n("Array"));
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
	ltDlg = new letterdialog(this,i18n("Letter"));
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
if (finame!="untitled") currentDir=fi.dirPath();
sfDlg = new FileChooser(this,i18n("Select Image File"));
sfDlg->setFilter("*.eps *.pdf *.png|Graphic Files\n*|All Files");
sfDlg->setDir(currentDir);
if (sfDlg->exec() )
  {
   QString fn=sfDlg->fileName();
   QFileInfo fi(fn);
   InsertTag("\\includegraphics[scale=1]{"+fi.baseName()+"."+fi.extension()+"} ",26,0);
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
if (finame!="untitled") currentDir=fi.dirPath();
sfDlg = new FileChooser(this,i18n("Select File"));
sfDlg->setFilter("*.tex|TeX Files\n*|All Files");
sfDlg->setDir(currentDir);
if (sfDlg->exec() )
  {
QString fn=sfDlg->fileName();
QFileInfo fi(fn);
InsertTag("\\include{"+fi.baseName()+"}",9,0);
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
if (finame!="untitled") currentDir=fi.dirPath();
sfDlg = new FileChooser(this,i18n("Select File"));
sfDlg->setFilter("*.tex|TeX Files\n*|All Files");
sfDlg->setDir(currentDir);
if (sfDlg->exec() )
  {
QString fn=sfDlg->fileName();
QFileInfo fi(fn);
InsertTag("\\input{"+fi.baseName()+"}",7,0);
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
tag +=fi.baseName();
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
  InsertTag(tag,qstrlen(tag),0);
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
  InsertTag(tag,qstrlen(tag),0);
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
InsertTag(code_symbol,qstrlen(code_symbol),0);
}

void Kile::InsertMetaPost(QListBoxItem *)
{
QString mpcode=mpview->currentText();
if (mpcode!="----------") InsertTag(mpcode,qstrlen(mpcode),0);
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
void Kile::InsertUserTag1()
{
if (UserMenuTag[0].left(1)=="%")
 {
 QString t=UserMenuTag[0];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[0],0,0);
 }
}

void Kile::InsertUserTag2()
{
if (UserMenuTag[1].left(1)=="%")
 {
 QString t=UserMenuTag[1];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[1],0,0);
 }
}

void Kile::InsertUserTag3()
{
if (UserMenuTag[2].left(1)=="%")
 {
 QString t=UserMenuTag[2];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[2],0,0);
 }
}

void Kile::InsertUserTag4()
{
if (UserMenuTag[3].left(1)=="%")
 {
 QString t=UserMenuTag[3];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[3],0,0);
 }
}

void Kile::InsertUserTag5()
{
if (UserMenuTag[4].left(1)=="%")
 {
 QString t=UserMenuTag[4];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[4],0,0);
 }
}

void Kile::InsertUserTag6()
{
if (UserMenuTag[5].left(1)=="%")
 {
 QString t=UserMenuTag[5];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[5],0,0);
 }
}

void Kile::InsertUserTag7()
{
if (UserMenuTag[6].left(1)=="%")
 {
 QString t=UserMenuTag[6];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[6],0,0);
 }
}

void Kile::InsertUserTag8()
{
if (UserMenuTag[7].left(1)=="%")
 {
 QString t=UserMenuTag[7];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[7],0,0);
 }
}

void Kile::InsertUserTag9()
{
if (UserMenuTag[8].left(1)=="%")
 {
 QString t=UserMenuTag[8];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[8],0,0);
 }
}

void Kile::InsertUserTag10()
{
if (UserMenuTag[9].left(1)=="%")
 {
 QString t=UserMenuTag[9];
 t=t.remove(0,1);
 QString s="\\begin{"+t+"}\n\n\\end{"+t+"}\n";
 InsertTag(s,0,1);
 }
else
 {
 InsertTag(UserMenuTag[9],0,0);
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

void Kile::UserManualHelp()
{
QFileInfo fic(locate("appdata","doc/usermanual.html"));
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
      htmlpart->openURL(locate("appdata","doc/usermanual.html"));
      htmlpart->addToHistory(locate("appdata","doc/usermanual.html"));
      }
    else { KMessageBox::error( this,i18n("File not found"));}
}

///////////////////// USER ///////////////
void Kile::EditUserMenu()
{
umDlg = new usermenudialog(this,i18n("Edit User Tags"));
for ( int i = 0; i <= 9; i++ )
    {
    umDlg->Name[i]=UserMenuName[i];
    umDlg->Tag[i]=UserMenuTag[i];
    umDlg->init();
    }
if ( umDlg->exec() )
 {
for ( int i = 0; i <= 9; i++ )
    {
    UserMenuName[i]=umDlg->Name[i];
    UserMenuTag[i]=umDlg->Tag[i];
    }

  UserAction1->setText("1: "+UserMenuName[0]);
  UserAction2->setText("2: "+UserMenuName[1]);
  UserAction3->setText("3: "+UserMenuName[2]);
  UserAction4->setText("4: "+UserMenuName[3]);
  UserAction5->setText("5: "+UserMenuName[4]);
  UserAction6->setText("6: "+UserMenuName[5]);
  UserAction7->setText("7: "+UserMenuName[6]);
  UserAction8->setText("8: "+UserMenuName[7]);
  UserAction9->setText("9: "+UserMenuName[8]);
  UserAction10->setText("10: "+UserMenuName[9]);
 }
delete umDlg;
}

void Kile::EditUserTool()
{
utDlg = new usertooldialog(this,i18n("Edit User Commands"));
for ( int i = 0; i <= 4; i++ )
    {
    utDlg->Name[i]=UserToolName[i];
    utDlg->Tool[i]=UserToolCommand[i];
    utDlg->init();
    }
if ( utDlg->exec() )
 {
for ( int i = 0; i <= 4; i++ )
    {
    UserToolName[i]=utDlg->Name[i];
    UserToolCommand[i]=utDlg->Tool[i];
    }

  UserToolAction1->setText("1: "+UserToolName[0]);
  UserToolAction2->setText("2: "+UserToolName[1]);
  UserToolAction3->setText("3: "+UserToolName[2]);
  UserToolAction4->setText("4: "+UserToolName[3]);
  UserToolAction5->setText("5: "+UserToolName[4]);
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
  LogWidget->insertLine(i18n("Launched: ")+ "xfig");
  }
}

void Kile::RunGfe()
{
  LogWidget->clear();
  Outputview->showPage(LogWidget);
  logpresent=false;
  UpdateLineColStatus();
  if (!gfe_widget) gfe_widget=new Qplotmaker(0,i18n("Gnuplot Front End"));
  gfe_widget->setIcon(kapp->miniIcon());
  gfe_widget->raise();
  gfe_widget->show();
}
/////////////// CONFIG ////////////////////
void Kile::ReadSettings()
{
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

config->setGroup( "Tools" );
quickmode=config->readNumEntry( "Quick Mode",1);
latex_command=config->readEntry("Latex","latex -interaction=nonstopmode %S.tex");
viewdvi_command=config->readEntry("Dvi","Embedded viewer");
dvips_command=config->readEntry("Dvips","dvips -o %S.ps %S.dvi");
viewps_command=config->readEntry("Ps","Embedded viewer");
ps2pdf_command=config->readEntry("Ps2pdf","ps2pdf %S.ps %S.pdf");
makeindex_command=config->readEntry("Makeindex","makeindex %S.idx");
bibtex_command=config->readEntry("Bibtex","bibtex %S.aux");
pdflatex_command=config->readEntry("Pdflatex","pdflatex %S.tex");
viewpdf_command=config->readEntry("Pdf","Embedded viewer");
dvipdf_command=config->readEntry("Dvipdf","dvipdfm %S.dvi");
l2h_options=config->readEntry("L2h Options","");
userClassList=config->readListEntry("User Class", ':');
userPaperList=config->readListEntry("User Paper", ':');
userEncodingList=config->readListEntry("User Encoding", ':');
userOptionsList=config->readListEntry("User Options", ':');

config->setGroup( "Files" );
lastDocument=config->readEntry("Last Document","");
recentFilesList=config->readListEntry("Recent Files", ':');
input_encoding=config->readEntry("Input Encoding", QString::fromLatin1(QTextCodec::codecForLocale()->name()));

config->setGroup( "User" );
UserMenuName[0]=config->readEntry("Menu1","");
UserMenuTag[0]=config->readEntry("Tag1","");
UserMenuName[1]=config->readEntry("Menu2","");
UserMenuTag[1]=config->readEntry("Tag2","");
UserMenuName[2]=config->readEntry("Menu3","");
UserMenuTag[2]=config->readEntry("Tag3","");
UserMenuName[3]=config->readEntry("Menu4","");
UserMenuTag[3]=config->readEntry("Tag4","");
UserMenuName[4]=config->readEntry("Menu5","");
UserMenuTag[4]=config->readEntry("Tag5","");
UserMenuName[5]=config->readEntry("Menu6","");
UserMenuTag[5]=config->readEntry("Tag6","");
UserMenuName[6]=config->readEntry("Menu7","");
UserMenuTag[6]=config->readEntry("Tag7","");
UserMenuName[7]=config->readEntry("Menu8","");
UserMenuTag[7]=config->readEntry("Tag8","");
UserMenuName[8]=config->readEntry("Menu9","");
UserMenuTag[8]=config->readEntry("Tag9","");
UserMenuName[9]=config->readEntry("Menu10","");
UserMenuTag[9]=config->readEntry("Tag10","");
UserToolName[0]=config->readEntry("ToolName1","");
UserToolCommand[0]=config->readEntry("Tool1","");
UserToolName[1]=config->readEntry("ToolName2","");
UserToolCommand[1]=config->readEntry("Tool2","");
UserToolName[2]=config->readEntry("ToolName3","");
UserToolCommand[2]=config->readEntry("Tool3","");
UserToolName[3]=config->readEntry("ToolName4","");
UserToolCommand[3]=config->readEntry("Tool4","");
UserToolName[4]=config->readEntry("ToolName5","");
UserToolCommand[4]=config->readEntry("Tool5","");

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

void Kile::SaveSettings()
{
ShowEditorWidget();
QValueList<int> sizes;
QValueList<int>::Iterator it;

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
config->writeEntry("Last Document",lastDocument);
config->writeEntry("Recent Files",recentFilesList, ':');
config->writeEntry("Input Encoding", input_encoding);

config->setGroup( "User" );
config->writeEntry("Menu1",UserMenuName[0]);
config->writeEntry("Tag1",UserMenuTag[0]);
config->writeEntry("Menu2",UserMenuName[1]);
config->writeEntry("Tag2",UserMenuTag[1]);
config->writeEntry("Menu3",UserMenuName[2]);
config->writeEntry("Tag3",UserMenuTag[2]);
config->writeEntry("Menu4",UserMenuName[3]);
config->writeEntry("Tag4",UserMenuTag[3]);
config->writeEntry("Menu5",UserMenuName[4]);
config->writeEntry("Tag5",UserMenuTag[4]);
config->writeEntry("Menu6",UserMenuName[5]);
config->writeEntry("Tag6",UserMenuTag[5]);
config->writeEntry("Menu7",UserMenuName[6]);
config->writeEntry("Tag7",UserMenuTag[6]);
config->writeEntry("Menu8",UserMenuName[7]);
config->writeEntry("Tag8",UserMenuTag[7]);
config->writeEntry("Menu9",UserMenuName[8]);
config->writeEntry("Tag9",UserMenuTag[8]);
config->writeEntry("Menu10",UserMenuName[9]);
config->writeEntry("Tag10",UserMenuTag[9]);
config->writeEntry("ToolName1",UserToolName[0]);
config->writeEntry("Tool1",UserToolCommand[0]);
config->writeEntry("ToolName2",UserToolName[1]);
config->writeEntry("Tool2",UserToolCommand[1]);
config->writeEntry("ToolName3",UserToolName[2]);
config->writeEntry("Tool3",UserToolCommand[2]);
config->writeEntry("ToolName4",UserToolName[3]);
config->writeEntry("Tool4",UserToolCommand[3]);
config->writeEntry("ToolName5",UserToolName[4]);
config->writeEntry("Tool5",UserToolCommand[4]);

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
      ModeAction->setText(i18n("Normal mode (current master document:")+shortName+")");
      ModeAction->setChecked(true);
      statusBar()->changeItem(i18n("Master document: ")+shortName, ID_HINTTEXT);
      singlemode=false;
      return;
      }
ModeAction->setChecked(false);
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
toDlg = new toolsoptionsdialog(this,i18n("Configure Kile"));
for ( int i = 0; i <= 7; i++ )
    {
    toDlg->colors[i]=editor_color[i];
    }
toDlg->init();    
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
if (wordwrap) {toDlg->checkWordWrap->setChecked(true);}
else {toDlg->checkWordWrap->setChecked(false);}
if (parenmatch) {toDlg->checkParen->setChecked(true);}
else {toDlg->checkParen->setChecked(false);}
if (showline) {toDlg->checkLine->setChecked(true);}
else {toDlg->checkLine->setChecked(false);}
if (toDlg->exec())
  {
   toDlg->ksc->writeGlobalSettings ();
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
int index_start=0;
int index_end=0;
spell_text="";
if ( !currentEditorView() ) return;
par_start=0;
par_end=currentEditorView()->editor->paragraphs();
if (currentEditorView()->editor->hasSelectedText())
   {
    currentEditorView()->editor->getSelection( &par_start, &index_start,&par_end, &index_end, 0 );
    currentEditorView()->editor->removeSelection(0);
   }
for (int i = par_start; i <= par_end; i++)
{
spell_text+=currentEditorView()->editor->text(i);
}

if (kspell) return;

kspell = new KSpell(this, i18n("Spellcheck"), this,SLOT( spell_started(KSpell *)));
connect (kspell, SIGNAL ( death()),this, SLOT ( spell_finished( )));
connect (kspell, SIGNAL (progress (unsigned int)),this, SLOT (spell_progress (unsigned int)));
connect (kspell, SIGNAL (misspelling (const QString & , const QStringList & , unsigned int )),this, SLOT (misspelling (const QString & , const QStringList & , unsigned int )));
connect (kspell, SIGNAL (corrected (const QString & , const QString & , unsigned int )),this, SLOT (corrected (const QString & , const QString & , unsigned int )));
connect (kspell, SIGNAL (done(const QString&)), this, SLOT (spell_done(const QString&)));
}

void Kile::spell_started( KSpell *)
{
   kspell->setProgressResolution(2);
   kspell->check(spell_text);
}

void Kile::spell_progress (unsigned int percent)
{
  QString s;
  s = QString(i18n("Spellcheck:  %1% complete")).arg(percent);
}

void Kile::spell_done(const QString& newtext)
{
currentEditorView()->editor->removeSelection(0);
//if (kspell->dlgResult() == 0)
//  {
//     //currentEditorView()->editor->setText( newtext);
//  }
kspell->cleanUp();
UpdateLineColStatus();
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

void Kile::misspelling (const QString & originalword, const QStringList & suggestions,unsigned int pos)
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
   QTextCodec* codec1 = QTextCodec::codecForName(currentEditorView()->editor->getEncoding());
   if(!codec1) codec1 = QTextCodec::codecForLocale();
   QString tmp =currentEditorView()->editor->text();
   QString unicodetmp=codec1->toUnicode(tmp);
   QTextCodec* codec2 = QTextCodec::codecForName(input_encoding);
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
       line=line.right(qstrlen(line)-15);
       /// l ///
       tagStart=line.find(" ",0);
       if (tagStart!=-1)
           {
           l=line.left(tagStart);
           line=line.right(qstrlen(line)-tagStart-1);
           }
       /// t ///
       tagStart=line.find(" ",0);
       if (tagStart!=-1)
           {
           t=line.left(tagStart);
           line=line.right(qstrlen(line)-tagStart-1);
           }
       /// r ///
       tagStart=line.find(" ",0);
       if (tagStart!=-1)
           {
           r=line.left(tagStart);
           line=line.right(qstrlen(line)-tagStart-1);
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
