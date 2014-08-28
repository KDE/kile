ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( test )

INCLUDE( FindPerl )

INCLUDE_DIRECTORIES( ${CMAKE_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} )

GET_DIRECTORY_PROPERTY(CMAKE_REQUIRED_INCLUDES INCLUDE_DIRECTORIES)

# we now look for Okular with "viewerinterface.h"
# MACRO_OPTIONAL_FIND_PACKAGE(Okular)
IF( OKULAR_FOUND )
	FIND_FILE(HAVE_VIEWERINTERFACE_H okular/interfaces/viewerinterface.h PATHS ${OKULAR_INCLUDE_DIR} NO_DEFAULT_PATH)
	IF( NOT HAVE_VIEWERINTERFACE_H )
		SET(OKULAR_FOUND 0)
	ENDIF( NOT HAVE_VIEWERINTERFACE_H )
ENDIF( OKULAR_FOUND )
IF( OKULAR_FOUND )
	SET(HAVE_VIEWERINTERFACE_H 1)
	SET(LIVEPREVIEW_POSSIBLE 1)
	INCLUDE_DIRECTORIES(${OKULAR_INCLUDE_DIR})
	MESSAGE( STATUS "Building with live preview" )
ELSE( OKULAR_FOUND )
	SET(HAVE_VIEWERINTERFACE_H 0)
	SET(LIVEPREVIEW_POSSIBLE 0)
	MESSAGE( STATUS "Not building with live preview" )
ENDIF( OKULAR_FOUND )
# MACRO_LOG_FEATURE(OKULAR_FOUND "Okular" "okularcore library, with viewerinterface.h" "http://okular.kde.org" FALSE "0.14 (KDE 4.8)" "Support for live preview.")
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# MACRO_OPTIONAL_FIND_PACKAGE(PopplerQt4)
IF( POPPLER_QT4_FOUND )
	SET(HAVE_POPPLER_QT4 TRUE)
	INCLUDE_DIRECTORIES(${POPPLER_QT4_INCLUDE_DIR})
ELSE( POPPLER_QT4_FOUND )
	SET(LIBPOPPLER_QT4_AVAILABLE 0)
ENDIF( POPPLER_QT4_FOUND )
# MACRO_LOG_FEATURE(POPPLER_QT4_FOUND "Poppler-Qt4" "A PDF rendering library" "http://poppler.freedesktop.org" FALSE "" ".")
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

SET(kile_SRCS
	latexcmd.cpp
	dialogs/latexcommanddialog.cpp
	dialogs/texdocumentationdialog.cpp
	userhelp.cpp
	dialogs/userhelpdialog.cpp
	dialogs/floatdialog.cpp
	dialogs/mathenvironmentdialog.cpp
	dialogs/postscriptdialog.cpp
	dialogs/pdf-wizard/pdfdialog.cpp
	widgets/previewconfigwidget.cpp
	quickpreview.cpp
	dialogs/includegraphicsdialog.cpp
	dialogs/newfilewizard.cpp
	dialogs/managetemplatesdialog.cpp
	templates.cpp
	dialogs/configurationdialog.cpp
	dialogs/tabbingdialog.cpp
	widgets/symbolview.cpp
	widgets/commandview.cpp
	dialogs/quickdocumentdialog.cpp
	widgets/filebrowserwidget.cpp
# 	docpart.cpp
	symbolviewclasses.h
	kile.cpp
	main.cpp
	kileactions.cpp
	kilestdactions.cpp
	kileinfo.cpp
	documentinfo.cpp
	kileproject.cpp
	dialogs/projectdialogs.cpp
	widgets/projectview.cpp
	dialogs/listselector.cpp
	kilelyxserver.cpp
	outputinfo.cpp
	dialogs/findfilesdialog.cpp
	kiletool.cpp
	tool_utils.cpp
	kilelauncher.cpp
	kiletoolmanager.cpp
	kilestdtools.cpp
	widgets/logwidget.cpp
	widgets/outputview.cpp
# 	widgets/konsolewidget.cpp
	kilewizard.cpp
	widgets/structurewidget.cpp
	widgets/codecompletionconfigwidget.cpp
	widgets/toolconfigwidget.cpp
	editorextension.cpp
	dialogs/cleandialog.cpp
	kilehelp.cpp
	convert.cpp
	kiledocmanager.cpp
	kileviewmanager.cpp
	eventfilter.cpp
	errorhandler.cpp
	dialogs/configcheckerdialog.cpp
	configtester.cpp
	widgets/sidebar.cpp
	plaintolatexconverter.cpp
	dialogs/statisticsdialog.cpp
	widgets/statisticswidget.cpp
	widgets/previewwidget.cpp
	widgets/scriptsmanagementwidget.h
	widgets/scriptsmanagementwidget.cpp
	editorkeysequencemanager.h
	editorkeysequencemanager.cpp
	kileversion.cpp
	widgets/abbreviationview.cpp
	dialogs/abbreviationinputdialog.cpp
	kileextensions.cpp
	widgets/categorycombobox.cpp
	widgets/progressdialog.h
	widgets/progressdialog.cpp
	dialogs/scriptshortcutdialog.cpp
	scripting/script.cpp
	scripting/kilescriptview.cpp
	scripting/kilescriptdocument.cpp
	scripting/kilescriptobject.cpp
	scriptmanager.cpp
# 	dialogs/tabular/multicolumnborderhelper.cpp
# 	dialogs/tabular/newtabulardialog.cpp
# 	dialogs/tabular/selectcoloraction.cpp
# 	dialogs/tabular/selectframeaction.cpp
# 	dialogs/tabular/tabularcell.cpp
# 	dialogs/tabular/tabularcelldelegate.cpp
# 	dialogs/tabular/tabularheaderitem.cpp
# 	dialogs/tabular/tabularproperties.cpp
# 	dialogs/tabular/tabulartable.cpp
	configurationmanager.cpp
	utilities.cpp
	abbreviationmanager.cpp
	editorcommands.cpp
# 	dialogs/usermenu/usermenudialog.cpp
	dialogs/usermenu/usermenuitem.cpp
# 	dialogs/usermenu/usermenutree.cpp
	usermenu/usermenudata.cpp
	usermenu/usermenu.cpp
	parser/parsermanager.cpp
	parser/parserthread.cpp
	parser/parser.cpp
	parser/latexparser.cpp
	parser/bibtexparser.cpp
	parser/latexoutputparser.cpp
	livepreview_utils.cpp
)

SET(kile_SRCS ${kile_SRCS} livepreview.cpp)

SET( kile_UI
	widgets/codecompletionconfigwidget.ui
	widgets/helpconfigwidget.ui
	widgets/latexconfigwidget.ui
	widgets/generalconfigwidget.ui
	widgets/configcheckerwidget.ui
	widgets/maintoolconfigwidget.ui
	widgets/librarytoolconfigwidget.ui
	widgets/processtoolconfigwidget.ui
	widgets/quicktoolconfigwidget.ui
	widgets/latextoolconfigwidget.ui
	dialogs/newtoolwizard_class_page.ui
	dialogs/newtoolwizard_toolname_page.ui
	widgets/environmentconfigwidget.ui
	widgets/structureviewconfigwidget.ui
	widgets/symbolviewconfigwidget.ui
	widgets/scriptingconfigwidget.ui
	widgets/graphicsconfigwidget.ui
	widgets/newdocumentwidget.ui
	widgets/usermenuconfigwidget.ui
	dialogs/tabbingdialog_base.ui
	dialogs/floatdialog_base.ui
	dialogs/postscriptdialog_base.ui
	dialogs/pdf-wizard/pdfdialog_base.ui
	dialogs/includegraphicsdialog_base.ui
	dialogs/latexcommanddialog_base.ui
	dialogs/usermenu/usermenudialog_base.ui
	widgets/livepreviewconfigwidget.ui
	widgets/appearanceconfigwidget.ui
	dialogs/scriptshortcutdialog_base.ui
)

SET( kile_UI_SRCS
	widgets/configcheckerwidget.cpp
	widgets/environmentconfigwidget.cpp
	widgets/graphicsconfigwidget.cpp
	widgets/generalconfigwidget.cpp
	widgets/helpconfigwidget.cpp
	widgets/latexconfigwidget.cpp
	widgets/latextoolconfigwidget.cpp
	widgets/librarytoolconfigwidget.cpp
	widgets/newdocumentwidget.cpp
	dialogs/newtoolwizard.cpp
	widgets/processtoolconfigwidget.cpp
	widgets/quicktoolconfigwidget.cpp
	widgets/scriptingconfigwidget.cpp
	widgets/structureviewconfigwidget.cpp
	widgets/symbolviewconfigwidget.cpp
	widgets/maintoolconfigwidget.cpp
	widgets/usermenuconfigwidget.cpp
	widgets/livepreviewconfigwidget.cpp
	widgets/appearanceconfigwidget.cpp
)

# Special handling of code completion due to the removal of 'KTextEditor::SmartRange' from
# KDE SC 4.5 onwards

# SET(kile_SRCS ${kile_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/_codecompletion.h
#                            ${CMAKE_CURRENT_SOURCE_DIR}/_codecompletion.cpp)

# if( ${KDE_VERSION} VERSION_EQUAL "4.5" OR  ${KDE_VERSION} VERSION_GREATER "4.5" )
#   MESSAGE( STATUS "Configuring code completion for KDE SC >= 4.5" )

  SET(CODECOMPLETION_MODELCONTROLLERINTERFACE_CLASSNAME CodeCompletionModelControllerInterface)
  SET(CODECOMPLETION_RANGE_CLASSNAME "Range")
  SET(CODECOMPLETION_RANGE_EXTRA "const")
  SET(CODECOMPLETION_RANGE_RETURN "KTextEditor::Range")
# else( ${KDE_VERSION} VERSION_EQUAL "4.5" OR  ${KDE_VERSION} VERSION_GREATER "4.5" )
#   MESSAGE( STATUS "Configuring code completion for KDE SC < 4.5")

#   SET(CODECOMPLETION_MODELCONTROLLERINTERFACE_CLASSNAME CodeCompletionModelControllerInterface)
#   SET(CODECOMPLETION_RANGE_CLASSNAME "SmartRange")
#   SET(CODECOMPLETION_RANGE_EXTRA "")
#   SET(CODECOMPLETION_RANGE_RETURN "void")
# ENDIF( ${KDE_VERSION} VERSION_EQUAL "4.5" OR  ${KDE_VERSION} VERSION_GREATER "4.5" )

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/_codecompletion.h
               ${CMAKE_CURRENT_BINARY_DIR}/codecompletion.h)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/_codecompletion.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/codecompletion.cpp)

SET(kile_SRCS
    ${kile_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/codecompletion.h
    ${CMAKE_CURRENT_BINARY_DIR}/codecompletion.cpp)




SET(kile_SRCS ${kile_SRCS} ${kile_UI_SRCS})

ki18n_wrap_ui(kile_SRCS ${kile_UI})

SET(kile_SRCS ${kile_SRCS} net.sourceforge.kile.main.xml)
QT5_ADD_DBUS_ADAPTOR(kile_SRCS net.sourceforge.kile.main.xml kile.h Kile)

SET( kile_KCFG_SRCS kileconfig.kcfgc)

kconfig_add_kcfg_files (kile_SRCS ${kile_KCFG_SRCS})

#TODO KF5
# KDE4_ADD_APP_ICON(kile_SRCS "data/icons/hi*-app-kile.png")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # for Mac OS X
  SET(PROGNAME Kile)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(PROGNAME kile)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# KDE4_ADD_EXECUTABLE(${PROGNAME} ${kile_SRCS})
kf5_add_kdeinit_executable(kile ${kile_SRCS})

IF( POPPLER_QT4_FOUND )
TARGET_LINK_LIBRARIES(${PROGNAME} ${POPPLER_QT4_LIBRARIES})
ENDIF( POPPLER_QT4_FOUND )

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # for Mac OS X
  SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Kile")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# TARGET_LINK_LIBRARIES(${PROGNAME} ${KDE4_KHTML_LIBS} ${KDE4_KROSSCORE_LIBS}
#                       ${KDE4_KFILE_LIBS} ${KDE4_KTEXTEDITOR_LIBS})

target_link_libraries(kdeinit_kile
PUBLIC
    Qt5::Script
    KF5::TextEditor
    KF5::I18n
    KF5::KIOFileWidgets
    KF5::IconThemes
    KF5::WindowSystem
    KF5::GuiAddons
    KF5::DBusAddons
    KF5::KDELibs4Support)

INSTALL( TARGETS ${PROGNAME} ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

INSTALL( FILES net.sourceforge.kile.main.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
INSTALL( PROGRAMS kile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL( FILES kile.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
INSTALL( FILES data/bibtexentries.rc data/biblatexentries.rc
kileui.rc docpartui.rc tips DESTINATION ${DATA_INSTALL_DIR}/kile )

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
  INSTALL (FILES data/kilestdtools-win.rc
           DESTINATION ${DATA_INSTALL_DIR}/kile
           RENAME kilestdtools.rc)

ELSE(CMAKE_SYSTEM_NAME MATCHES "Windows")
  INSTALL (FILES data/kilestdtools.rc
           DESTINATION ${DATA_INSTALL_DIR}/kile)

ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
