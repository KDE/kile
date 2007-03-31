####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kile

## INCLUDES were found outside kdevelop specific part


kile_SOURCES = latexcmd.cpp latexcmddialog.cpp texdocdialog.cpp userhelp.cpp \
	userhelpdialog.cpp floatdialog.cpp mathenvdialog.cpp tabulardialog.cpp \
	postscriptdialog.cpp previewconfigwidget.cpp quickpreview.cpp includegraphicsdialog.cpp \
	newfilewizard.cpp managetemplatesdialog.cpp templates.cpp \
	usermenudialog.cpp kileconfigdialog.cpp tabbingdialog.cpp symbolview.cpp \
	quickdocumentdialog.cpp kilefileselect.cpp kileapplication.cpp docpart.cpp kile.cpp main.cpp \
	kileactions.cpp kilestdactions.cpp kileinfo.cpp kiledocumentinfo.cpp kileproject.cpp \
	kileprojectdlgs.cpp kileprojectview.cpp kilelistselector.cpp kilelyxserver.cpp \
	latexoutputfilter.cpp outputinfo.cpp latexoutputinfo.cpp outputfilter.cpp kilegrepdialog.cpp \
	kiletool.cpp kilelauncher.cpp kiletoolmanager.cpp kilestdtools.cpp kilelogwidget.cpp \
	kileoutputwidget.cpp kilekonsolewidget.cpp kilewizard.cpp kilestructurewidget.cpp \
	codecompletion.cpp configcodecompletion.cpp kiletoolconfigwidget.cpp kileedit.cpp \
	cleandialog.cpp kilehelp.cpp configstructure.cpp kileappIface.skel convert.cpp \
	helpconfigwidget.ui kiledocmanager.cpp kileviewmanager.cpp kileeventfilter.cpp \
	latexconfigwidget.ui kileconfig.kcfgc generalconfigwidget.ui kilespell.cpp \
	kileerrorhandler.cpp configcheckerdlg.cpp configcheckerwidget.ui configtester.cpp \
	kilesidebar.cpp toolconfigwidget.ui librarytoolconfigwidget.ui \
	processtoolconfigwidget.ui quicktoolconfigwidget.ui plaintolatexconverter.cpp \
	latextoolconfigwidget.ui newtoolwizard.ui kileuntitled.cpp \
	kilestatsdlg.cpp kilestatswidget.cpp envconfigwidget.ui structureconfigwidget.ui \
	previewwidget.cpp kilejscript.h kilejscript.cpp scriptsmanagementwidget.h scriptsmanagementwidget.cpp \
	editorkeysequencemanager.h editorkeysequencemanager.cpp scriptingconfigwidget.ui \
	kileabbrevview.cpp kileextensions.cpp kileversion.cpp graphicsconfigwidget.ui

kile_LDADD   = -lkjs -lkdeprint -lktexteditor -lkatepartinterfaces $(LIB_KSPELL) $(LIB_KHTML) 

SUBDIRS = icons pics symbols templates syntax complete help encodings test

EXTRA_DIST = main.cpp kile.cpp kile.h \
	docpart.cpp docpart.h kileappIface.h kileapplication.cpp kileapplication.h kile.desktop kilefileselect.cpp 	kilefileselect.h kileui.rc docpartui.rc kmultiverttabbar.cpp kmultiverttabbar.h letterdialog.cpp letterdialog.h quickdocumentdialog.cpp quickdocumentdialog.h symbolview.cpp symbolview.h tabbingdialog.cpp tabbingdialog.h \
	kileconfigdialog.cpp kileconfigdialog.h usermenudialog.cpp \
	usermenudialog.h kileabbrevview.cpp kileabbrevview.h \
	newfilewizard.cpp newfilewizard.h templates.cpp templates.h managetemplatesdialog.cpp \
	managetemplatesdialog.h kileinfo.h kileinfo.cpp kilestdactions.cpp \
	kilestdactions.h kileproject.h kileproject.cpp kileprojectdlgs.cpp \
	kileprojectdlgs.h kileprojectview.cpp kileprojectview.h \
	kilelistselector.cpp kilelistselector.h kilelyxserver.cpp kilelyxserver.h \
	latexoutputfilter.cpp outputinfo.cpp latexoutputinfo.cpp \
	outputfilter.cpp latexoutputfilter.h outputinfo.h latexoutputinfo.h \
	includegraphicsdialog.h includegraphicsdialog.cpp kiletoolconfigwidget.cpp kiletoolconfigwidget.h outputfilter.h kilegrepdialog.cpp kilegrepdialog.h codecompletion.h codecompletion.cpp configcodecompletion.cpp configcodecompletion.h kileedit.cpp cleandialog.cpp kileedit.h cleandialog.h  kilehelp.cpp kilehelp.h configstructure.cpp configstructure.h configenvironment.h convert.h convert.cpp \
	kileconstants.h kilejscript.h kilejscript.cpp scriptsmanagementwidget.h scriptsmanagementwidget.cpp editorkeysequencemanager.h editorkeysequencemanager.cpp

xdg_apps_DATA = kile.desktop

kde_kcfg_DATA = kile.kcfg

KDE_ICON = AUTO

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
rc_DATA = kileui.rc docpartui.rc kilestdtools.rc tips

messages: rc.cpp
	$(PREPARETIPS) > tips.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kile.pot; \
	fi
	rm -f tips.cpp

syntaxdir = $(kde_appsdir)/katepart/syntax
noinst_HEADERS = kiletool.h kilelauncher.h kiletool_enums.h kiletoolmanager.h \
		kilestdtools.h kilelogwidget.h kileoutputwidget.h kilekonsolewidget.h kilewizard.h \
		kilestructurewidget.h convert.h kiledocmanager.h kileviewmanager.h kileeventfilter.h \
		latexconfigwidget.h kilespell.h kileerrorhandler.h configcheckerdlg.h configtester.h \
		kilesidebar.h plaintolatexconverter.h kileuntitled.h 	kilestatsdlg.h \
	kilestatswidget.h envconfigwidget.h structureconfigwidget.h kileversion.h kileconstants.h

CREATE_HASH_TABLE = $(top_srcdir)/kile/kile/create_hash_table

kilejscript.lut.h : kilejscript.cpp $(CREATE_HASH_TABLE)
	$(PERL) $(CREATE_HASH_TABLE) $(top_srcdir)/kile/kile/kilejscript.cpp > $@
kilejscript.lo: kilejscript.lut.h

kilejscript.o: kilejscript.lut.h

LUT_FILES = kilejscript.lut.h

CLEANFILES = $(LUT_FILES)
