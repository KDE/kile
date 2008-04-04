ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( test )

INCLUDE( FindPerl )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} )

ADD_DEFINITIONS(-DQT3_SUPPORT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

SET(kile_SRCS
	${kile_SRCS}
	latexcmd.cpp
	dialogs/latexcommanddialog.cpp
	dialogs/texdocumentationdialog.cpp
	userhelp.cpp
	dialogs/userhelpdialog.cpp
	dialogs/floatdialog.cpp
	dialogs/mathenvironmentdialog.cpp
	tabulardialog.cpp
	dialogs/postscriptdialog.cpp
	widgets/previewconfigwidget.cpp
	quickpreview.cpp
	dialogs/includegraphicsdialog.cpp
	dialogs/newfilewizard.cpp
	dialogs/managetemplatesdialog.cpp
	templates.cpp
	dialogs/usertagsdialog.cpp
	dialogs/configurationdialog.cpp
	dialogs/tabbingdialog.cpp
	widgets/symbolview.cpp
	dialogs/quickdocumentdialog.cpp
	widgets/filebrowserwidget.cpp
	docpart.cpp
	kile.cpp
	main.cpp
	kileactions.cpp
	kilestdactions.cpp
	kileinfo.cpp
	documentinfo.cpp
	kileproject.cpp
	dialogs/projectdialogs.cpp
	widgets/projectview.cpp
	kilelistselector.cpp
	kilelyxserver.cpp
	latexoutputfilter.cpp
	outputinfo.cpp
	latexoutputinfo.cpp
	outputfilter.cpp
	dialogs/findfilesdialog.cpp
	kiletool.cpp
	kilelauncher.cpp
	kiletoolmanager.cpp
	kilestdtools.cpp
	widgets/logwidget.cpp
	widgets/outputview.cpp
	widgets/konsolewidget.cpp
	kilewizard.cpp
	widgets/structurewidget.cpp
	codecompletion.cpp
	widgets/codecompletionconfigwidget.cpp
	widgets/toolconfigwidget.cpp
	editorextension.cpp
	dialogs/cleandialog.cpp
	kilehelp.cpp
	convert.cpp
	kiledocmanager.cpp
	kileviewmanager.cpp
	eventfilter.cpp
	kileerrorhandler.cpp
	dialogs/configcheckerdialog.cpp
	configtester.cpp
	widgets/sidebar.cpp
	plaintolatexconverter.cpp
	kileuntitled.cpp
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
	widgets/extendedscrollarea.cpp
	scripting/bindings.cpp
	scriptmanager.cpp
	dialogs/newtabulardialog.cpp
	configurationmanager.cpp
	spellcheck.cpp
)

SET( kile_UI
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
	dialogs/tabbingdialog_base.ui
	dialogs/floatdialog_base.ui
	dialogs/postscriptdialog_base.ui
	dialogs/includegraphicsdialog_base.ui
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
)

SET(kile_SRCS ${kile_SRCS} ${kile_UI_SRCS})

KDE4_ADD_UI_FILES(kile_SRCS ${kile_UI})

# this doesn't seem to work right now:
# QT4_GENERATE_DBUS_INTERFACE(kile.h org.kde.kile.xml)
# use qdbuscpp2xml -a kileappIface.h -o net.sourceforge.kile.main.xml instead
SET(kile_SRCS ${kile_SRCS} net.sourceforge.kile.main.xml)

QT4_ADD_DBUS_ADAPTOR(kile_SRCS net.sourceforge.kile.main.xml kile.h Kile)

SET( kile_KCFG_SRCS
	kileconfig.kcfgc
)

KDE4_ADD_KCFG_FILES(kile_SRCS ${kile_KCFG_SRCS})

KDE4_ADD_EXECUTABLE(kile ${kile_SRCS})

#need qt3support library for now
TARGET_LINK_LIBRARIES(kile ${QT_AND_KDECORE_LIBS} ${KDE4_KHTML_LIBS}
${KDE4_KHTML_LIBS} ${KDE4_KROSSCORE_LIBS} ${QT_QT3SUPPORT_LIBRARY}
${KDE4_KDE3SUPPORT_LIBS})

INSTALL( TARGETS kile DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES net.sourceforge.kile.main.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
INSTALL( FILES kile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL( FILES kile.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
INSTALL( FILES kileui.rc docpartui.rc kilestdtools.rc tips DESTINATION
${DATA_INSTALL_DIR}/kile )

# FIXME: disabling this for now
kde4_install_icons( ${ICON_INSTALL_DIR}  )
