/*
 * Copyright (C) 2003 Roland Schulz <mail@r2s2.de>
 */

#include "plugin_kile.h"

#include <kaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kconfig.h>
#include <qimage.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qfile.h>
#include <kate/application.h>
#include "symbolview.h"
#include "structview.h"
#include "newfilewizard.h"
#include "managetemplatesdialog.h"
#include "compile.h"

class PluginView : public KXMLGUIClient
{
  friend class KatePluginKile;

  public:
    Kate::MainWindow *win;
};

extern "C"
{
  void* init_libkileplugin()
  {
    KGlobal::locale()->insertCatalogue("katekile");
    return new KatePluginFactory;
  }
}

KatePluginFactory::KatePluginFactory()
{
  s_instance = new KInstance( "kate" );
}

KatePluginFactory::~KatePluginFactory()
{
  delete s_instance;
}

QObject* KatePluginFactory::createObject( QObject* parent, const char* name, const char*, const QStringList & )
{
  return new KatePluginKile( parent, name );
}

KInstance* KatePluginFactory::s_instance = 0L;

KatePluginKile::KatePluginKile( QObject* parent, const char* name )
    : Kate::Plugin ( (Kate::Application*)parent, name )
{

}

KatePluginKile::~KatePluginKile()
{
}

void KatePluginKile::addView(Kate::MainWindow *win)
{
   //create a head pixmap for the tab
   QPixmap pixmap;

   readSettings();

   //create the browser and put it into a dockwidget using kate's tool view manager
   Kate::ToolViewManager *tool_view_manager = win->toolViewManager();
   for (int i=0; i<5; i++) {
   	pixmap.load(locate("data","kile/pics/math"+QString::number(i+1)+".png"));
		SymbolView* symbolview = new SymbolView(i+1, NULL, "symbolview"+QString::number(i+1));
      symbolview->setFocusPolicy(QWidget::NoFocus);
      tool_view_manager->addToolViewWidget(KDockWidget::DockLeft, symbolview, pixmap, i18n("Kile Symbol View"));
		connect(symbolview, SIGNAL(SymbolSelected(QString)), SLOT(InsertSymbol(QString)));
		m_widgets.append(symbolview);
	}

   /*Structview*/
   outstruct = new StructView(win->viewManager(),NULL, "Structview" );
	pixmap.load(locate("data","kile/pics/structure.png"));
	tool_view_manager->addToolViewWidget(KDockWidget::DockLeft, outstruct, pixmap, i18n("Kile Symbol View"));
   outstruct->setFocusPolicy(QWidget::NoFocus);
	m_widgets.append(outstruct);

	Compile* compile = new Compile(win);
	m_widgets.append(compile->LogWidget);
	m_widgets.append(compile->OutputWidget);
   m_compiles.append(compile);

   PluginView *view = new PluginView ();

    (void) new KAction ( i18n("Update Structure"), 0, outstruct,
                      SLOT( UpdateStructure() ), view->actionCollection(),
                      "update_structure" );

    (void) new KAction ( i18n("New As..."), 0, this,
                      SLOT( fileNewAs() ), view->actionCollection(),
                      "file_new_as" );

	 (void) new KAction( i18n("Remove Template..."),0,this,
                      SLOT( removeTemplate() ), view->actionCollection(),
                      "removetemplate");

    (void) new KAction( i18n("Create Template From Document..."),0,this,
                      SLOT(createTemplate() ), view->actionCollection(),
                      "CreateTemplate");

    view->setInstance (new KInstance("kate"));
    view->setXMLFile("plugins/kile/plugin_kile.rc");
    win->guiFactory()->addClient (view);
    view->win = win;

   m_views.append (view);
}
void KatePluginKile::removeView(Kate::MainWindow *win)
{

  QPtrListIterator<PluginView> itv( m_views );
  PluginView *view;
  while ( (view = itv.current()) != NULL ) {
      ++itv;
      if (view->win == win) {
         m_views.remove (view);
         win->guiFactory()->removeClient (view);
         delete view;
      }
  }

  QPtrListIterator<QWidget> itw( m_widgets );
  QWidget *w;
  while ( (w = itw.current()) != NULL ) {
      ++itw;
      if (win->toolViewManager()->removeToolViewWidget(w)) {   //toolViewManager should only delete it incase it belongs to him
		  	m_widgets.remove(w);
      }
   }

  QPtrListIterator<Compile> itc( m_compiles );
  Compile *c;
  while ( (c = itc.current()) != NULL ) {
      ++itc;
      if (c->m_win == win) {
         m_compiles.remove (c);
         win->guiFactory()->removeClient (c);
         delete c;
      }
  }
}

void KatePluginKile::InsertSymbol(QString symbolcode){
  Kate::View *kv = application()->activeMainWindow()->viewManager()->activeView();

  if (kv)
    kv->insertText (symbolcode);
  for (uint i=0; i<symbolcode.length();i++)
  	 kv->cursorRight();
}

void KatePluginKile::fileNewAs()
{
	 Kate::View* kv = application()->activeMainWindow()->viewManager()->activeView();
	 NewFileWizard nfw(kv);
    if (nfw.exec()) {
      application()->activeMainWindow()->viewManager()->openURL(KURL());
      kv = application()->activeMainWindow()->viewManager()->activeView();
      kv->getDoc()->setDocName("unnamed"); //TODO
      QString sel = nfw.getSelection();
      if (sel != DEFAULT_EMPTY_CAPTION) {
         QString name = "templates/template_"+QString(sel)+".tex";
         QFile f(KGlobal::dirs()->findResource("data", "kile/"+name));
         if (f.open(IO_ReadOnly) ) {
            QString line;
            Kate::Document* doc = kv->getDoc();
            while (f.readLine(line,80)>0) {
               replaceTemplateVariables(line);
               line = line.left(line.length()-1); //strip newline
               doc->insertLine(doc->numLines()-1,line);
            }
            f.close();
         } else { KMessageBox::error(kv, i18n("Couldn't find template: %1").arg(name),i18n("File Not Found!")); }
      }
	}
}

void KatePluginKile::createTemplate() {
	Kate::View* kv = application()->activeMainWindow()->viewManager()->activeView();
   if (kv->getDoc()->isModified() ) {
      KMessageBox::information(kv,i18n("Please save the file first!"));
      return;
   }

   QFileInfo fi(kv->getDoc()->docName());
   ManageTemplatesDialog mtd(fi,i18n("Create Template From Document"));
   mtd.exec();
}

void KatePluginKile::removeTemplate() {
	ManageTemplatesDialog mtd(i18n("Remove a template."));
	mtd.exec();
}

void KatePluginKile::replaceTemplateVariables(QString &line)
{
	line=line.replace("$$AUTHOR$$",templAuthor);
	line=line.replace("$$DOCUMENTCLASSOPTIONS$$",templDocClassOpt);
	if (templEncoding != "") { line=line.replace("$$INPUTENCODING$$", "\\input["+templEncoding+"]{inputenc}");}
	else { line = line.replace("$$INPUTENCODING$$","");}
}

void KatePluginKile::readSettings()
{
KConfig* config = KGlobal::config();
config->setGroup( "User" );
templAuthor=config->readEntry("Author","");
templDocClassOpt=config->readEntry("DocumentClassOptions","a4paper,10pt");
templEncoding=config->readEntry("Template Encoding","");
}


QPixmap MyUserIcon(const QString& name){
QPixmap pixmap;
pixmap.load(locate("data","kile/pics/"+name+".png"));
return pixmap;
}
#include "plugin_kile.moc"

