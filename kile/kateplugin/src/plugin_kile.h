/*
 * Copyright (C) 2003 Roland Schulz <mail@r2s2.de>
 */

#ifndef _PLUGIN_KILE_H_
#define _PLUGIN_KILE_H_

#include <kate/application.h>
#include <kate/documentmanager.h>
#include <kate/document.h>
#include <kate/mainwindow.h>
#include <kate/plugin.h>
#include <kate/view.h>
#include <kate/viewmanager.h>
#include <kate/toolviewmanager.h>

#include <kdockwidget.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstddirs.h>
#include "symbolview.h"
#include "structview.h"

class KatePluginFactory : public KLibFactory
{
  Q_OBJECT

  public:
    KatePluginFactory();
    virtual ~KatePluginFactory();

    virtual QObject* createObject( QObject* parent = 0, const char* pname = 0, const char* name = "QObject", const QStringList &args = QStringList() );

  private:
    static KInstance* s_instance;
};

class KatePluginKile : public Kate::Plugin, Kate::PluginViewInterface
{
  Q_OBJECT

  public:
    KatePluginKile( QObject* parent = 0, const char* name = 0 );
    virtual ~KatePluginKile();

    void addView (Kate::MainWindow *win);
    void removeView (Kate::MainWindow *win);
     Kate::MainWindow *main_win;

  private slots:
    void InsertSymbol(QString);
    void fileNewAs();
    void createTemplate();
    void removeTemplate();

  private:
    void replaceTemplateVariables(QString &line);
    void readSettings();

  private:
    QPtrList<class PluginView> m_views;
    QPtrList<class QWidget> m_widgets;
    QPtrList<class Compile> m_compiles;
    StructView* outstruct;
    QString templAuthor, templDocClassOpt, templEncoding;
};

QPixmap MyUserIcon(const QString& name);

#endif // _PLUGIN_KILE_H_
