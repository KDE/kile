/*
 * Copyright (C) 2003 Roland Schulz <mail@r2s2.de>
 */

#include "plugin_kile.h"

#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <qimage.h>
#include "symbolview.h"
//#include "pybrowse.xpm"

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
   for (int i=0; i<5; i++) {
	symbolview[i] = NULL;
	my_dock[i] = NULL;
   }
}

KatePluginKile::~KatePluginKile()
{
}

void KatePluginKile::addView(Kate::MainWindow *win)
{
    // TODO: doesn't this have to be deleted?
    PluginView *view = new PluginView ();

     (void) new KAction ( i18n("Insert Hello World"), 0, this,
                      SLOT( slotInsertHello() ), view->actionCollection(),
                      "edit_insert_kile" );

    view->setInstance (new KInstance("kate"));
    view->setXMLFile("plugins/kile/plugin_kile.rc");
    win->guiFactory()->addClient (view);
    view->win = win;

   //create a head pixmap for the tab
   QPixmap pixmap;

   //create the browser and put it into a dockwidget using kate's tool view manager
   Kate::ToolViewManager *tool_view_manager = win->toolViewManager();
   for (int i=0; i<5; i++) {
   	pixmap.load(locate("data","kile/pics/math"+QString::number(i+1)+".png"));
	my_dock[i] = tool_view_manager->addToolView(KDockWidget::DockLeft, "symbolview"+i, pixmap, i18n("Kile Symbol View"));
	symbolview[i] = new SymbolView(i+1, my_dock[i], "symbolview"+i);
	my_dock[i]->setWidget(symbolview[i]);
	my_dock[i]->setToolTipString(i18n("Kile Symbol View"));
	symbolview[i]->show();
	connect(symbolview[i], SIGNAL(SymbolSelected(QString)), SLOT(InsertSymbol(QString)));
    }


   m_views.append (view);
}
void KatePluginKile::removeView(Kate::MainWindow *win)
{

  for (uint z=0; z < m_views.count(); z++)
    if (m_views.at(z)->win == win)
    {
      PluginView *view = m_views.at(z);
      m_views.remove (view);
      win->guiFactory()->removeClient (view);
      delete view;
    }
    for (int i=0; i<5; i++) {
	if (my_dock[i] != NULL)
	{
		main_win->toolViewManager()->removeToolView(my_dock[i]);
		//delete my_dock[i];
		//delete symbolview[i];
		my_dock[i] = NULL;
		symbolview[i] = NULL;
	}
    }

}

void KatePluginKile::slotInsertHello()
{
  Kate::View *kv = application()->activeMainWindow()->viewManager()->activeView();

  if (kv)
    kv->insertText ("Hello World");
}
void KatePluginKile::InsertSymbol(QString symbolcode){
  Kate::View *kv = application()->activeMainWindow()->viewManager()->activeView();

  if (kv)
    kv->insertText (symbolcode);
}
#include "plugin_kile.moc"
