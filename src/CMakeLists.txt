add_subdirectory(data)
add_subdirectory(test)

include(FindPerl)

include_directories(
	${CMAKE_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

get_directory_property(CMAKE_REQUIRED_INCLUDES INCLUDE_DIRECTORIES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

set(kile_SRCS
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
	widgets/konsolewidget.cpp
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
	livepreview.cpp
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
	codecompletion.cpp
)

set(kile_UI
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

set(kile_UI_SRCS
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

set(kile_SRCS ${kile_SRCS} ${kile_UI_SRCS})

ki18n_wrap_ui(kile_SRCS ${kile_UI})

set(kile_SRCS ${kile_SRCS} net.sourceforge.kile.main.xml)
QT5_ADD_DBUS_ADAPTOR(kile_SRCS net.sourceforge.kile.main.xml kile.h Kile)

set(kile_KCFG_SRCS kileconfig.kcfgc)

kconfig_add_kcfg_files(kile_SRCS ${kile_KCFG_SRCS})

# add icon to executable on windows and mac
ecm_add_app_icon(kile_SRCS
	ICONS
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/16-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/22-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/32-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/48-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/64-apps-kile.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/icons/128-apps-kile.png
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # for Mac OS X
  set(PROGNAME Kile)
else(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(PROGNAME kile)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

kf5_add_kdeinit_executable(kile ${kile_SRCS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # for Mac OS X
	set_target_properties(${PROGNAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Kile")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(kdeinit_kile
PUBLIC
    Qt5::Script
    KF5::CoreAddons
    KF5::Crash
    KF5::DBusAddons
    KF5::GuiAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::TextEditor
    KF5::WindowSystem
)
# optional dependencies
if(POPPLER_FOUND)
	target_link_libraries(kdeinit_kile PUBLIC ${POPPLER_LIBRARY})
endif()

if(Okular_FOUND)
  # We don't need to link to okular since it gets loaded dynamically at runtime.
  # All we need are the include directories which we can get with this generator expression:
  target_include_directories(kdeinit_kile PRIVATE $<TARGET_PROPERTY:Okular::okularcore,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

install(TARGETS kile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kdeinit_kile ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES net.sourceforge.kile.main.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(PROGRAMS org.kde.kile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kile.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES data/bibtexentries.rc data/biblatexentries.rc tips DESTINATION ${DATA_INSTALL_DIR}/kile)
install(FILES kileui.rc docpartui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kile)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	install(FILES data/kilestdtools-win.rc
           DESTINATION ${DATA_INSTALL_DIR}/kile
           RENAME kilestdtools.rc)
else(CMAKE_SYSTEM_NAME MATCHES "Windows")
	install (FILES data/kilestdtools.rc
           DESTINATION ${DATA_INSTALL_DIR}/kile)
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")
