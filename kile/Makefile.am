####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kile

## INCLUDES were found outside kdevelop specific part

kile_SOURCES = newfilewizard.cpp managetemplatesdialog.cpp templates.cpp commandprocess.cpp metapostview.cpp refdialog.cpp addoptiondialog.cpp usermenudialog.cpp toolsoptionsdialog.cpp texkonsolewidget.cpp tabdialog.cpp tabbingdialog.cpp symbolview.cpp structdialog.cpp quickdocumentdialog.cpp messagewidget.cpp letterdialog.cpp l2hdialog.cpp kmultiverttabbar.cpp kileiconview.cpp kilefileselect.cpp kileapplication.cpp kileappIface_skel.cpp filechooser.cpp docpart.cpp arraydialog.cpp kile.cpp main.cpp kileactions.cpp kilestdactions.h kilestdactions.cpp
kile_LDADD   = ./gfe/libgfe.a -lkdeprint -lktexteditor $(LIB_KSPELL) $(LIB_KHTML)

SUBDIRS = pics symbols gfe templates syntax

EXTRA_DIST = main.cpp kile.cpp kile.h arraydialog.cpp arraydialog.h docpart.cpp docpart.h filechooser.cpp filechooser.h kile16x16.png kile32x32.png kile48x48.png kileappIface.h kileappIface_skel.cpp kileapplication.cpp kileapplication.h kile.desktop kilefileselect.cpp kilefileselect.h kileiconview.cpp kileiconview.h kileui.rc kmultiverttabbar.cpp kmultiverttabbar.h l2hdialog.cpp l2hdialog.h letterdialog.cpp letterdialog.h messagewidget.cpp messagewidget.h quickdocumentdialog.cpp quickdocumentdialog.h structdialog.cpp structdialog.h symbolview.cpp symbolview.h tabbingdialog.cpp tabbingdialog.h tabdialog.cpp tabdialog.h texkonsolewidget.cpp texkonsolewidget.h toolsoptionsdialog.cpp toolsoptionsdialog.h usermenudialog.cpp usermenudialog.h addoptiondialog.cpp addoptiondialog.h refdialog.cpp refdialog.h kile22x22.png metapostview.cpp metapostview.h commandprocess.cpp commandprocess.h newfilewizard.cpp newfilewizard.h templates.cpp templates.h managetemplatesdialog.cpp managetemplatesdialog.h

apps_DATA = kile.desktop
appsdir = $(kde_appsdir)/Applications

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/kile16x16.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/kile.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/kile32x32.png $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/kile.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/kile48x48.png $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/kile.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/
	$(INSTALL_DATA) $(srcdir)/kile22x22.png $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/kile.png

uninstall-local:
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/kile.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/kile.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/kile.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/kile.png

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
INCLUDES = $(all_includes)

METASOURCES = AUTO

# the library search path. 
kile_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kile
rc_DATA = kileui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kile.pot; \
	fi

syntaxdir = $(kde_appsdir)/katepart/syntax
