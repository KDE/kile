INCLUDES    = $(all_includes)
METASOURCES = AUTO

KDE_ICON = kile

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libkileplugin.la

libkileplugin_la_SOURCES = plugin_kile.cpp symbolview.cpp symbolview.h structview.h structview.cpp newfilewizard.cpp newfilewizard.h templates.cpp templates.h managetemplatesdialog.cpp managetemplatesdialog.h compile.cpp compile.h commandprocess.cpp commandprocess.h messagewidget.cpp messagewidget.h
libkileplugin_la_LIBADD = -lkateinterfaces
libkileplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/kate/plugins/kile
plugins_DATA = plugin_kile.rc plugin_kile_compile.rc

kde_services_DATA = kile.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kile.pot

