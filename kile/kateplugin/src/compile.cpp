/***************************************************************************
 *   Copyright (C) 2003 by Roland Schulz                                   *
 *   mail@r2s2.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <krun.h>
#include <kdebug.h>
#include <kate/application.h>
#include <kate/view.h>
#include <kate/viewmanager.h>
#include <kate/document.h>
#include <kate/toolviewmanager.h>
#include "compile.h"
#include "messagewidget.h"
#include "plugin_kile.h"

Compile::Compile(Kate::MainWindow *win, QObject *parent, const char *name)
 : QObject(parent, name)
{
   KConfig* config = KGlobal::config();
   config->setGroup( "Tools" );
   latex_command=config->readEntry("Latex","latex -interaction=nonstopmode %S.tex");
   logpresent=false;
   singlemode=true;
   MasterName="";
   m_win = win;
   addView();
   StopAction = new KAction(i18n("&Stop"),"stop",Key_Escape,this,SIGNAL(stopProcess()),actionCollection(),"Stop");
   StopAction->setEnabled(false);
   (void) new KAction("LaTeX","latex", Key_F2, this, SLOT(Latex()), actionCollection(),"Latex" );
   setInstance (new KInstance("kate"));
   setXMLFile("plugins/kile/plugin_kile_compile.rc");
   win->guiFactory()->addClient (this);
   //errorlist=new QStrList();
}


Compile::~Compile()
{
}

void Compile::addView()
{
	Kate::ToolViewManager *tool_view_manager = m_win->toolViewManager();
   LogWidget = new MessageWidget( NULL ,"LogWidget" );
   LogWidget->setFocusPolicy(QWidget::ClickFocus);
   LogWidget->setMinimumHeight(40);
   LogWidget->setReadOnly(true);
   tool_view_manager->addToolViewWidget(KDockWidget::DockBottom, LogWidget, MyUserIcon("viewlog"), i18n("Log/Messages"));

   OutputWidget = new MessageWidget( NULL , "OutputWidget" );
   OutputWidget->setFocusPolicy(QWidget::ClickFocus);
   OutputWidget->setMinimumHeight(40);
   OutputWidget->setReadOnly(true);
   tool_view_manager->addToolViewWidget(KDockWidget::DockBottom, OutputWidget, MyUserIcon("output_win"), i18n("Output"));

   connect(OutputWidget, SIGNAL(clicked(int,int)),this,SLOT(ClickedOnOutput(int,int)));
}

void Compile::Latex()
{
  Kate::View* kv = Kate::application()->activeMainWindow()->viewManager()->activeView();
  QString finame;
  if ( (finame=prepareForCompile("LaTeX")) == QString::null)  return;

  QFileInfo fic(finame);
  QStringList command;
  command << latex_command;
  CommandProcess *proc=execCommand(command,fic,true);
  connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotProcessExited(KProcess*) ));

  if ( ! proc->start(KProcess::NotifyOnExit, KProcess::AllOutput) )
  {
     KMessageBox::error( kv,i18n("Could not start LaTeX, make sure you have installed the LaTeX package on your system."));
  }
  else
  {
     OutputWidget->clear();
     LogWidget->clear();
     logpresent=false;
     LogWidget->insertLine(i18n("Launched: %1").arg(proc->command()));
  }

  //UpdateLineColStatus();
}

//command : a list representing the command to be started
//          i.e. latex %.tex -interactionmode=nonstop
//          is represented by the list (latex,%S.tex,-interactionmode=nonstop)
//file    : the file to be passed as an argument to the command, %S is substituted
//          by the basename of this file
//enablestop : whether or not this process can be stopped by pressing the STOP button
CommandProcess* Compile::execCommand(const QStringList &command, const QFileInfo &file, bool enablestop) {
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
   (*i).replace(QRegExp("%S"),name);
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
QString Compile::prepareForCompile(const QString & command) {
  Kate::View* kv = Kate::application()->activeMainWindow()->viewManager()->activeView();
  QString finame;
  /*
  finame = kv->getDoc()->url();
  if (finame == "untitled") {
     if (KMessageBox::warningYesNo(kv,i18n("You need to save an untitled document before you run %1 on it.\n"
                                             "Do you want to save it? Click Yes to save and No to abort.").arg(command),
                                   i18n("File Needs to be Saved!"))
         == KMessageBox::No) return QString::null;
  }*/

  //save the file before doing anything
  //attempting to save an untitled document will result in a file-save dialog pop-up
  if (kv->save()!=Kate::View::SAVE_OK) return "";

  //determine the name of the file to be compiled
  if (singlemode) {finame=kv->getDoc()->url().path();}
  else {
     finame=MasterName; //FIXME: MasterFile does not get saved if it is modified
  }

  QFileInfo fic(finame);

  if (!fic.exists() )
  {
     KMessageBox::error(kv,i18n("The file %1 does not exist. Are you working with a master document which is accidently deleted?")
                        .arg(finame));
     return QString::null;
  }

  if (!fic.isReadable() )
  {
     KMessageBox::error(kv, i18n("You do not have read permission for the file: %1").arg(finame));
     return QString::null;
  }

  return finame;
}

void Compile::slotProcessExited(KProcess* proc)
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
//UpdateLineColStatus();
currentProcess=0;
}

void Compile::slotDisableStop() {
   StopAction->setEnabled(false);
}

void Compile::slotProcessOutput(KProcess* /*proc*/,char* buffer,int buflen)
{
int row = (OutputWidget->paragraphs() == 0)? 0 : OutputWidget->paragraphs()-1;
int col = OutputWidget->paragraphLength(row);
QString s=QCString(buffer,buflen+1);
OutputWidget->setCursorPosition(row,col);
OutputWidget->insertAt(s, row, col);
}

void Compile::ClickedOnOutput(int parag, int /*index*/)
{

//if ( !currentEditorView() ) return;

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

Kate::View* kv = Kate::application()->activeMainWindow()->viewManager()->activeView();
int l=line.toInt(&ok,10)-1;
kdDebug() << "gotoline " << l << endl;
if (ok && l<=kv->getDoc()->numLines())
 {
 //currentEditorView()->editor->viewport()->setFocus();
 kv->setCursorPosition(l,0);
  //UpdateLineColStatus();
 }
}
