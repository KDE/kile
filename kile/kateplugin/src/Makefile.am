# $Id$

INCLUDES    = $(all_includes)
METASOURCES = AUTO

KDE_ICON = kile

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libkileplugin.la

libkileplugin_la_SOURCES = plugin_kile.cpp symbolview.cpp structview.cpp newfilewizard.cpp templates.cpp managetemplatesdialog.cpp compile.cpp commandprocess.cpp messagewidget.cpp
libkileplugin_la_LIBADD = -lkateinterfaces
libkileplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/kate/plugins/kile
plugins_DATA = plugin_kile.rc plugin_kile_compile.rc

kde_services_DATA = kile.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kateplugin_kile.pot

