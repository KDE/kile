####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kile

## INCLUDES were found outside kdevelop specific part

kile_SOURCES = managetemplatesdialog.cpp templates.cpp newfilewizard.cpp projectwizard.cpp commandprocess.cpp linenumberwidget.cpp latexeditorview.cpp usertooldialog.cpp metapostview.cpp refdialog.cpp addoptiondialog.cpp usermenudialog.cpp toolsoptionsdialog.cpp texkonsolewidget.cpp tabdialog.cpp tabbingdialog.cpp syntaxlatex.cpp symbolview.cpp structdialog.cpp replacedialog.cpp quickdocumentdialog.cpp parenmatcher.cpp messagewidget.cpp letterdialog.cpp latexeditor.cpp l2hdialog.cpp kmultiverttabbar.cpp kileiconview.cpp kilefileselect.cpp kileapplication.cpp kileappIface_skel.cpp gotolinedialog.cpp finddialog.cpp filechooser.cpp docpart.cpp arraydialog.cpp kile.cpp main.cpp 
kile_LDADD   = ./gfe/libgfe.a -lkdeprint $(LIB_KSPELL) $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = pics symbols gfe templates 

EXTRA_DIST = main.cpp kile.cpp kile.h arraydialog.cpp arraydialog.h docpart.cpp docpart.h filechooser.cpp filechooser.h finddialog.cpp finddialog.h gotolinedialog.cpp gotolinedialog.h kile16x16.png kile32x32.png kile48x48.png kileappIface.h kileappIface_skel.cpp kileapplication.cpp kileapplication.h kile.desktop kilefileselect.cpp kilefileselect.h kileiconview.cpp kileiconview.h kileui.rc kmultiverttabbar.cpp kmultiverttabbar.h l2hdialog.cpp l2hdialog.h latexeditor.cpp latexeditor.h letterdialog.cpp letterdialog.h messagewidget.cpp messagewidget.h paragdata.h parenmatcher.cpp parenmatcher.h quickdocumentdialog.cpp quickdocumentdialog.h replacedialog.cpp replacedialog.h structdialog.cpp structdialog.h symbolview.cpp symbolview.h syntaxlatex.cpp syntaxlatex.h tabbingdialog.cpp tabbingdialog.h tabdialog.cpp tabdialog.h texkonsolewidget.cpp texkonsolewidget.h toolsoptionsdialog.cpp toolsoptionsdialog.h usermenudialog.cpp usermenudialog.h addoptiondialog.cpp addoptiondialog.h refdialog.cpp refdialog.h kile22x22.png metapostview.cpp metapostview.h usertooldialog.cpp usertooldialog.h latexeditorview.cpp latexeditorview.h linenumberwidget.cpp linenumberwidget.h commandprocess.cpp commandprocess.h projectwizard.cpp projectwizard.h newfilewizard.cpp newfilewizard.h templates.cpp templates.h managetemplatesdialog.cpp managetemplatesdialog.h 

install-data-local:
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/kile16x16.png $(kde_icondir)/hicolor/16x16/apps/kile.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/kile32x32.png $(kde_icondir)/hicolor/32x32/apps/kile.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/kile48x48.png $(kde_icondir)/hicolor/48x48/apps/kile.png
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kile.desktop $(kde_appsdir)/Applications/kile.desktop
	$(mkinstalldirs) $(kde_datadir)/kile/
	$(INSTALL_DATA) $(srcdir)/kileui.rc $(kde_datadir)/kile/kileui.rc
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/apps/
	$(INSTALL_DATA) $(srcdir)/kile22x22.png $(kde_icondir)/hicolor/22x22/apps/kile.png

uninstall-local:
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kile.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kile.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kile.png
	-rm -f $(kde_appsdir)/Applications/kile.desktop
	-rm -f $(kde_datadir)/kile/kileui.rc
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kile.png

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kile_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE?s XML GUI builing
#rcdir = $(kde_datadir)/kile
#rc_DATA = kileui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: rc.cpp
	$(EXTRACTRC) *.rc >> ./rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kile.pot; \
	fi

