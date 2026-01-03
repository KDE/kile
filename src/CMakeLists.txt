add_subdirectory(data)
add_subdirectory(test)

include(FindPerl)

include_directories(
	${CMAKE_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

get_directory_property(CMAKE_REQUIRED_INCLUDES INCLUDE_DIRECTORIES)

########### next target ###############

set(kile_SRCS
	abbreviationmanager.cpp
	codecompletion.cpp
	configtester.cpp
	configurationmanager.cpp
	convert.cpp
	dialogs/abbreviationinputdialog.cpp
	dialogs/cleandialog.cpp
	dialogs/configcheckerdialog.cpp
	dialogs/configurationdialog.cpp
	dialogs/findfilesdialog.cpp
	dialogs/floatdialog.cpp
	dialogs/includegraphicsdialog.cpp
	dialogs/latexcommanddialog.cpp
	dialogs/listselector.cpp
	dialogs/managetemplatesdialog.cpp
	dialogs/mathenvironmentdialog.cpp
	dialogs/newfilewizard.cpp
	dialogs/pdf-wizard/pdfdialog.cpp
	dialogs/postscriptdialog.cpp
	dialogs/projectdialogs.cpp
	dialogs/quickdocumentdialog.cpp
	dialogs/scriptshortcutdialog.cpp
	dialogs/statisticsdialog.cpp
	dialogs/tabbingdialog.cpp
	dialogs/tabular/multicolumnborderhelper.cpp
	dialogs/tabular/newtabulardialog.cpp
	dialogs/tabular/selectcoloraction.cpp
	dialogs/tabular/selectframeaction.cpp
	dialogs/tabular/tabularcell.cpp
	dialogs/tabular/tabularcelldelegate.cpp
	dialogs/tabular/tabularheaderitem.cpp
	dialogs/tabular/tabularproperties.cpp
	dialogs/tabular/tabulartable.cpp
	dialogs/texdocumentationdialog.cpp
	dialogs/userhelpdialog.cpp
	dialogs/usermenu/usermenudialog.cpp
	dialogs/usermenu/usermenuitem.cpp
	dialogs/usermenu/usermenutree.cpp
	dialogs/validatorinputdialog.cpp
	documentinfo.cpp
	editorcommands.cpp
	editorextension.cpp
	editorkeysequencemanager.cpp
	errorhandler.cpp
	eventfilter.cpp
	kileactions.cpp
	kile.cpp
	kiledocmanager.cpp
	kileextensions.cpp
	kilehelp.cpp
	kileinfo.cpp
	kilelauncher.cpp
	kilelyxserver.cpp
	kileproject.cpp
	kilestdactions.cpp
	kilestdtools.cpp
	kiletool.cpp
	kiletoolmanager.cpp
	kileversion.cpp
	kileviewmanager.cpp
	kilewizard.cpp
	latexcmd.cpp
	livepreview.cpp
	livepreview_utils.cpp
	main.cpp
	outputinfo.cpp
	parser/bibtexparser.cpp
	parser/latexoutputparser.cpp
	parser/latexparser.cpp
	parser/parser.cpp
	parser/parsermanager.cpp
	parser/parserthread.cpp
	plaintolatexconverter.cpp
	quickpreview.cpp
	scripting/kilescriptdocument.cpp
	scripting/kilescriptobject.cpp
	scripting/kilescriptview.cpp
	scripting/script.cpp
	scriptmanager.cpp
	symbolviewclasses.h
	templates.cpp
	tool_utils.cpp
	userhelp.cpp
	usermenu/usermenu.cpp
	usermenu/usermenudata.cpp
	utilities.cpp
	widgets/abbreviationview.cpp
	widgets/categorycombobox.cpp
	widgets/codecompletionconfigwidget.cpp
	widgets/commandview.cpp
	widgets/filebrowserwidget.cpp
	widgets/konsolewidget.cpp
	widgets/logwidget.cpp
	widgets/outputview.cpp
	widgets/previewconfigwidget.cpp
	widgets/previewwidget.cpp
	widgets/progressdialog.cpp
	widgets/projectview.cpp
	widgets/scriptsmanagementwidget.cpp
	widgets/sidebar.cpp
	widgets/statisticswidget.cpp
	widgets/statusbar.cpp
	widgets/structurewidget.cpp
	widgets/symbolview.cpp
	widgets/toolconfigwidget.cpp
	widgets/scrollwidget.cpp
)

set(kile_UI
	dialogs/floatdialog_base.ui
	dialogs/includegraphicsdialog_base.ui
	dialogs/latexcommanddialog_base.ui
	dialogs/newtoolwizard_class_page.ui
	dialogs/newtoolwizard_toolname_page.ui
	dialogs/pdf-wizard/pdfdialog_base.ui
	dialogs/postscriptdialog_base.ui
	dialogs/scriptshortcutdialog_base.ui
	dialogs/tabbingdialog_base.ui
	dialogs/usermenu/usermenudialog_base.ui
	widgets/appearanceconfigwidget.ui
	widgets/codecompletionconfigwidget.ui
	widgets/configcheckerwidget.ui
	widgets/environmentconfigwidget.ui
	widgets/generalconfigwidget.ui
	widgets/graphicsconfigwidget.ui
	widgets/helpconfigwidget.ui
	widgets/latexconfigwidget.ui
	widgets/latextoolconfigwidget.ui
	widgets/livepreviewconfigwidget.ui
	widgets/maintoolconfigwidget.ui
	widgets/newdocumentwidget.ui
	widgets/processtoolconfigwidget.ui
	widgets/quicktoolconfigwidget.ui
	widgets/scriptingconfigwidget.ui
	widgets/structureviewconfigwidget.ui
	widgets/symbolviewconfigwidget.ui
	widgets/usermenuconfigwidget.ui
)

set(kile_UI_SRCS
	dialogs/newtoolwizard.cpp
	widgets/appearanceconfigwidget.cpp
	widgets/configcheckerwidget.cpp
	widgets/environmentconfigwidget.cpp
	widgets/generalconfigwidget.cpp
	widgets/graphicsconfigwidget.cpp
	widgets/helpconfigwidget.cpp
	widgets/latexconfigwidget.cpp
	widgets/latextoolconfigwidget.cpp
	widgets/livepreviewconfigwidget.cpp
	widgets/maintoolconfigwidget.cpp
	widgets/newdocumentwidget.cpp
	widgets/processtoolconfigwidget.cpp
	widgets/quicktoolconfigwidget.cpp
	widgets/scriptingconfigwidget.cpp
	widgets/structureviewconfigwidget.cpp
	widgets/symbolviewconfigwidget.cpp
	widgets/usermenuconfigwidget.cpp
)

set(kile_SRCS ${kile_SRCS} ${kile_UI_SRCS})

ki18n_wrap_ui(kile_SRCS ${kile_UI})

set(kile_SRCS ${kile_SRCS} org.kde.kile.main.xml)
qt_add_dbus_adaptor(kile_SRCS org.kde.kile.main.xml kile.h Kile)

set(kile_KCFG_SRCS kileconfig.kcfgc)

kconfig_add_kcfg_files(kile_SRCS ${kile_KCFG_SRCS})

set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/kileconfig.h" PROPERTIES SKIP_AUTOMOC ON)   # otherwise we get a warning about
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/kileconfig.cpp" PROPERTIES SKIP_AUTOMOC ON) # the CMP0071 policy (with CMake 3.11)

set(ICON_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/sc-apps-kile.svg
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/16-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/22-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/32-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/48-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/64-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/128-apps-kile.png
)
ecm_add_app_icon(ICONS_SOURCES ICONS ${ICON_FILES})

# add the resource file containing the ui files
qt_add_resources(kile_SRCS kile.qrc)

ecm_qt_declare_logging_category(kile_SRCS HEADER kile_main_debug.h IDENTIFIER LOG_KILE_MAIN CATEGORY_NAME org.kde.kile.main EXPORT KILE DESCRIPTION "Kile" DEFAULT_SEVERITY Warning)
ecm_qt_declare_logging_category(kile_SRCS HEADER kile_parser_debug.h IDENTIFIER LOG_KILE_PARSER CATEGORY_NAME org.kde.kile.parser EXPORT KILE DESCRIPTION "Kile (Parser)" DEFAULT_SEVERITY Warning)
ecm_qt_declare_logging_category(kile_SRCS HEADER kile_codecompletion_debug.h IDENTIFIER LOG_KILE_CODECOMPLETION CATEGORY_NAME org.kde.kile.codecompletion EXPORT KILE DESCRIPTION "Kile (Code completion)" DEFAULT_SEVERITY Warning)

add_executable(kile ${kile_SRCS})
target_sources(kile PRIVATE ${ICONS_SOURCES})

qt_add_resources(kile "icons"
    BASE ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/
    PREFIX "/icons"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/sc-apps-kile.svg)

if(APPLE)
    # extract the last copyright year from "kileversion.h"
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/kileversion.h" LAST_COPYRIGHT_YEAR_LINE REGEX "KILE_LAST_COPYRIGHT_YEAR (.*)")
    string(REGEX MATCH "KILE_LAST_COPYRIGHT_YEAR \"([0-9]+)\"" TMP_MATCH ${LAST_COPYRIGHT_YEAR_LINE})
    set(LAST_COPYRIGHT_YEAR ${CMAKE_MATCH_1})

    set_target_properties(kile PROPERTIES
        OUTPUT_NAME                     "Kile"
        MACOSX_BUNDLE_BUNDLE_NAME       "Kile"
        MACOSX_BUNDLE_GUI_IDENTIFIER    "org.kde.kile"
        MACOSX_BUNDLE_DISPLAY_NAME      "Kile"
        MACOSX_BUNDLE_INFO_STRING       "Kile, an KDE Integrated LaTeX Environment"
        MACOSX_BUNDLE_COPYRIGHT         "the Kile Team (2003 - ${LAST_COPYRIGHT_YEAR})")
    set_target_properties(kile PROPERTIES
        MACOSX_BUNDLE_LONG_VERSION_STRING "Kile ${KILE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${KILE_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${KILE_VERSION}")
endif()

target_link_libraries(kile
PUBLIC
    KF6::Codecs
    KF6::CoreAddons
    KF6::Crash
    KF6::DBusAddons
    KF6::GuiAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::TextEditor
    KF6::TextWidgets
    KF6::WindowSystem
    KF6::ColorScheme
)

target_link_libraries(kile PUBLIC Qt6::Qml Qt6::Core5Compat KF6::IconWidgets)
# optional dependencies
if(TARGET Poppler::Qt6)
	target_link_libraries(kile PUBLIC Poppler::Qt6)
endif()

if(TARGET Okular::Core)
  # We don't need to link to okular since it gets loaded dynamically at runtime.
  # All we need are the include directories which we can get with this generator expression:
  target_include_directories(kile PRIVATE $<TARGET_PROPERTY:Okular::Core,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

install(TARGETS kile ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES org.kde.kile.main.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES org.kde.kile.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kile.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES data/bibtexentries.rc data/biblatexentries.rc DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile)
install(FILES data/org.kde.kile.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	install(FILES data/kilestdtools-win.rc
           DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile
           RENAME kilestdtools.rc)
else(CMAKE_SYSTEM_NAME MATCHES "Windows")
	install (FILES data/kilestdtools.rc
           DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile)
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

ecm_qt_install_logging_categories(
    EXPORT KILE
    FILE kile.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
