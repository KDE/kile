/***************************************************************************
                          kile.h  -  description
                             -------------------
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2002 by Pascal Brachet, 2003 Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILE_H
#define KILE_H


#include <kmainwindow.h>
#include <dcopobject.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <khelpmenu.h>
#include <kpopupmenu.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <kspell.h>
#include <kprocess.h>
#include <kaction.h>
#include <kfileitem.h>

#include <qmap.h>
#include <qsplitter.h>
#include <qwidget.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qguardedptr.h>
#include <qlayout.h>
#include <qframe.h>
#include <qstring.h>
#include <qcolor.h>


#include "kileappIface.h"
#include "latexeditorview.h"
#include "latexeditor.h"
#include "messagewidget.h"
#include "structdialog.h"
#include "quickdocumentdialog.h"
#include "letterdialog.h"
#include "tabdialog.h"
#include "arraydialog.h"
#include "tabbingdialog.h"
#include "usermenudialog.h"
#include "usertooldialog.h"
#include "gotolinedialog.h"
#include "replacedialog.h"
#include "finddialog.h"
#include "toolsoptionsdialog.h"
#include "l2hdialog.h"
#include "filechooser.h"
#include "gfe/qplotmaker.h"
#include "gfe/qplotdialog.h"
#include "docpart.h"
#include "symbolview.h"
#include "texkonsolewidget.h"
#include "kmultiverttabbar.h"
#include "kilefileselect.h"
#include "refdialog.h"
#include "metapostview.h"

#include "commandprocess.h"

#define ID_HINTTEXT 301
#define ID_LINE_COLUMN 302

typedef  QMap<LatexEditorView*, QString> FilesMap;
typedef  QString Userlist[10];
typedef  QString UserCd[5];
typedef  QColor ListColors[8];

class Kile : public KParts::MainWindow, public KileAppDCOPIface
{
    Q_OBJECT

public:
    Kile( QWidget *parent = 0, const char *name = 0 );
    ~Kile();
    QString getName();
    QFont EditorFont;

public slots:
    void load( const QString &f );
    void setLine( const QString &line );
private:
    void setupActions();
    void closeEvent(QCloseEvent *e);
    LatexEditorView *currentEditorView() const;
    void doConnections( LatexEditor *e );
    bool FileAlreadyOpen(QString f);

    KStatusBar *StatusBar;
    KConfig* config;
    KSpell *kspell;

    KileFileSelect *KileFS;
    KMultiVertTabBar *ButtonBar;
    structdialog *stDlg;
    quickdocumentdialog *startDlg;
    refdialog *refDlg;
    letterdialog *ltDlg;
    tabdialog *quickDlg;
    arraydialog *arrayDlg;
    tabbingdialog *tabDlg;
    usermenudialog *umDlg;
    usertooldialog *utDlg;
    l2hdialog *l2hDlg;
    QGuardedPtr<FindDialog> findDialog;
    QGuardedPtr<ReplaceDialog> replaceDialog;
    QGuardedPtr<GotoLineDialog> gotoLineDialog;
    toolsoptionsdialog *toDlg;
    FileChooser *sfDlg;
    QGuardedPtr<Qplotmaker> gfe_widget;
    SymbolView *symbol_view;
    docpart *htmlpart;
    KParts::PartManager *partManager;
    KParts::ReadOnlyPart *pspart, *dvipart;
    QString document_class, typeface_size, paper_size, document_encoding, author;
    bool ams_packages, makeidx_package;
    QString latex_command, viewdvi_command, dvips_command, dvipdf_command;
    QString viewps_command, ps2pdf_command, makeindex_command, bibtex_command, pdflatex_command, viewpdf_command, l2h_options;
    QString lastDocument,MasterName, input_encoding;
    QString struct_level1, struct_level2, struct_level3, struct_level4, struct_level5;
    QStringList recentFilesList;
    Userlist UserMenuName, UserMenuTag;
    UserCd UserToolName, UserToolCommand;
    ListColors editor_color;
    bool logpresent, singlemode, showstructview,showoutputview, wordwrap, parenmatch,showline,
      showmaintoolbar,showtoolstoolbar, showedittoolbar, showmathtoolbar;
    bool htmlpresent,pspresent, dvipresent, symbol_present, watchfile, color_mode;
    int split1_right, split1_left, split2_top, split2_bottom, quickmode, lastvtab;
    QTabWidget *tabWidget, *Outputview;
    QFrame *Structview;
    QHBoxLayout *Structview_layout;
    QWidgetStack *topWidgetStack;
    QSplitter *splitter1, *splitter2 ;
    QListView* outstruct;
    QListViewItem *parent_level[5],*lastChild, *Child;
    metapostview *mpview;
    MessageWidget *OutputWidget, *LogWidget;
    TexKonsoleWidget* texkonsole;
    QStrList *errorlist;
    QStringList structlist, labelitem, structitem, userClassList, userPaperList, userEncodingList, userOptionsList;
    FilesMap filenames;
    KPopupMenu*help;
    KHelpMenu* help_menu;
    KAction *PrintAction, *BackAction, *ForwardAction, *HomeAction, *StopAction;
    KAction *UserAction1, *UserAction2, *UserAction3, *UserAction4, *UserAction5, *UserAction6, *UserAction7, *UserAction8, *UserAction9, *UserAction10;
    KAction *UserToolAction1, *UserToolAction2, *UserToolAction3, *UserToolAction4, *UserToolAction5;
    KToggleAction *ModeAction, *StructureAction, *MessageAction, *WatchFileAction,
      *ShowMainToolbarAction, *ShowToolsToolbarAction, *ShowEditToolbarAction, *ShowMathToolbarAction;

    KSelectAction *RecentAction;
    int par_start, par_end;
    QString spell_text;

    KShellProcess *currentProcess;

signals:
   void stopProcess();
   
private slots:
    void fileNew();
    void fileOpen();
    void fileOpenRecent(const QString &fn);
    void fileSave();
    void fileSaveAll();
    void fileSaveAs();
    void createTemplate();
    void filePrint();
    void fileClose();
    void fileCloseAll();
    void fileExit();
    void AddRecentFile(const QString &f);
    void fileSelected(const KFileItem *file);

    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();
    void editFind();
    void editFindNext();
    void editReplace();
    void editGotoLine();
    void editComment();
    void editUncomment();
    void editIndent();

    void InsertTag(QString Entity, int dx, int dy);
    void UpdateLineColStatus();
    void UpdateCaption();
    void NewDocumentStatus(bool m);
    void gotoNextDocument();
    void gotoPrevDocument();

    void ReadSettings();
    void SaveSettings();
    void GeneralOptions();

    void QuickBuild();
    void EndQuickCompile();
    void QuickDviToPS();
    void QuickDviPDF();
    void QuickPS2PDF();

    CommandProcess* execCommand(const QStringList & command, const QFileInfo &file, bool enablestop);
    QString prepareForCompile(const QString & command);
    QStringList prepareForConversion(const QString &command, const QString &from, const QString &to);
    QString prepareForViewing(const QString & command, const QString &ext);
    void Latex();
    void ViewDvi();
    void KdviForwardSearch();
    void DviToPS();
    void ViewPS();
    void PDFLatex();
    void ViewPDF();
    void CleanAll();
    void MakeBib();
    void MakeIndex();
    void PStoPDF();
    void DVItoPDF();
    void syncTerminal();
    void RunTerminal(QWidget *w);
    void LatexToHtml();
    void MetaPost();
    void slotProcessOutput(KProcess* proc,char* buffer,int buflen);
    void slotProcessExited(KProcess* proc);
    void slotDisableStop();
    void slotl2hExited(KProcess* proc);
    void HtmlPreview();
    void UserTool1();
    void UserTool2();
    void UserTool3();
    void UserTool4();
    void UserTool5();

   void UpdateStructure();
   void ShowStructure();
   void ClickedOnStructure(QListViewItem *);
   void DoubleClickedOnStructure(QListViewItem *);

   void ViewLog();
   void ClickedOnOutput(int parag, int index);
   void QuickLatexError();
   void LatexError();
   void NextError();
   void PreviousError();

   void QuickTabular();
   void QuickArray();
   void QuickTabbing();
   void QuickLetter();
   void QuickDocument();
   void Insert1();
   void Insert1bis();
   void Insert1ter();
   void Insert2();
   void Insert3();
   void Insert4();
   void Insert5();
   void Insert6();
   void Insert6bis();
   void Insert7();
   void Insert8();
   void Insert9();
   void Insert10();
   void Insert11();
   void Insert12();
   void Insert13();
   void Insert14();
   void Insert15();
   void Insert16();
   void Insert17();
   void Insert18();
   void Insert19();
   void Insert20();
   void Insert21();
   void Insert22();
   void Insert23();
   void Insert24();
   void Insert25();
   void Insert26();
   void Insert27();
   void Insert28();
   void Insert29();
   void Insert30();
   void Insert31();
   void Insert32();
   void Insert33();
   void Insert34();
   void Insert35();
   void Insert36();
   void Insert37();
   void Insert37bis();
   void Insert37ter();
   void Insert38();
   void Insert39();
   void Insert40();
   void Insert41();
   void Insert42();
   void Insert43();
   void Insert44();
   void Insert45();
   void Insert46();
   void Insert47();
   void Insert48();
   void Insert49();
   void Insert50();
   void Insert51();
   void Insert52();
   void SizeCommand(const QString& text);
   void SectionCommand(const QString& text);
   void OtherCommand(const QString& text);
   void NewLine();

   void InsertMath1();
   void InsertMath2();
   void InsertMath3();
   void InsertMath4();
   void InsertMath5();
   void InsertMath6();
   void InsertMath7();
   void InsertMath8();
   void InsertMath9();
   void InsertMath10();
   void InsertMath16();
   void InsertMath66();
   void InsertMath67();
   void InsertMath68();
   void InsertMath69();
   void InsertMath70();
   void InsertMath71();
   void InsertMath72();
   void InsertMath73();
   void InsertMath74();
   void InsertMath75();
   void InsertMath76();
   void InsertMath77();
   void InsertMath78();
   void InsertMath79();
   void InsertMath80();
   void InsertMath81();
   void InsertMath82();
   void InsertMath83();
   void InsertMath84();
   void InsertMath85();
   void InsertMath86();
   void InsertMath87();
   void InsertMath88();
   void InsertMath89();
   void InsertMath90();
   void LeftDelimiter(const QString& text);
   void RightDelimiter(const QString& text);
   void InsertSymbol();
   void InsertMetaPost(QListBoxItem *);


   void InsertBib1();
   void InsertBib2();
   void InsertBib3();
   void InsertBib4();
   void InsertBib5();
   void InsertBib6();
   void InsertBib7();
   void InsertBib8();
   void InsertBib9();
   void InsertBib10();
   void InsertBib11();
   void InsertBib12();
   void InsertBib13();

   void InsertUserTag1();
   void InsertUserTag2();
   void InsertUserTag3();
   void InsertUserTag4();
   void InsertUserTag5();
   void InsertUserTag6();
   void InsertUserTag7();
   void InsertUserTag8();
   void InsertUserTag9();
   void InsertUserTag10();
   void EditUserMenu();
   void EditUserTool();

   void RunXfig();
   void RunGfe();


   void ToggleMode();
   void ToggleStructView();
   void ToggleOutputView();
   void ToggleWatchFile();
   void ToggleShowMainToolbar();
   void ToggleShowToolsToolbar();
   void ToggleShowEditToolbar();
   void ToggleShowMathToolbar();
   void ShowStructView(bool change);
   void ShowOutputView(bool change);

   void ResetPart();
   void ActivePartGUI(KParts::Part * the_part);
   void ShowEditorWidget();
   void BrowserBack();
   void BrowserForward();
   void BrowserHome();
   void LatexHelp();
   void UserManualHelp();

   void spellcheck();
   void spell_started ( KSpell *);
   void spell_progress (unsigned int percent);
   void spell_done(const QString&);
   void spell_finished();
   void corrected (const QString & originalword, const QString & newword, unsigned int pos);
   void misspelling (const QString & originalword, const QStringList & suggestions,unsigned int pos);
   void ConfigureKeys();
   void ConfigureToolbars();
   void updateNavAction( bool, bool);

   void showVertPage(int page);
   void changeInputEncoding();

   QString DetectEpsSize(const QString &epsfile);
   void CleanBib();

};


#endif

