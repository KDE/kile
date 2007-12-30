ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( test )

INCLUDE( FindPerl )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} )

ADD_DEFINITIONS(-DQT3_SUPPORT)

########### next target ###############


# taken from KDE4's KJS CMakeLists.txt file

SET(CREATE_HASH_TABLE ${CMAKE_CURRENT_SOURCE_DIR}/create_hash_table )

MACRO(CREATE_LUT _srcs_LIST _in_FILE _out_FILE _dep_FILE)
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
		COMMAND ${PERL_EXECUTABLE} ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} -n KileJScript > ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} )
	SET( ${_srcs_LIST}  ${${_srcs_LIST}} ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE})
ENDMACRO(CREATE_LUT)

CREATE_LUT(kile_SRCS kilejscript.cpp kilejscript.lut.h kilejscript.cpp)

SET(kile_SRCS
	${kile_SRCS}
	latexcmd.cpp
	latexcmddialog.cpp
	texdocdialog.cpp
	userhelp.cpp
	userhelpdialog.cpp
	floatdialog.cpp
	mathenvdialog.cpp
	tabulardialog.cpp
	postscriptdialog.cpp
	widgets/previewconfigwidget.cpp
	quickpreview.cpp
	includegraphicsdialog.cpp
	newfilewizard.cpp
	managetemplatesdialog.cpp
	templates.cpp
	usermenudialog.cpp
	dialogs/configurationdialog.cpp
	tabbingdialog.cpp
	symbolview.cpp
	quickdocumentdialog.cpp
	widgets/filebrowserwidget.cpp
	docpart.cpp
	kile.cpp
	main.cpp
	kileactions.cpp
	kilestdactions.cpp
	kileinfo.cpp
	kiledocumentinfo.cpp
	kileproject.cpp
	dialogs/projectdialogs.cpp
	kileprojectview.cpp
	kilelistselector.cpp
	kilelyxserver.cpp
	latexoutputfilter.cpp
	outputinfo.cpp
	latexoutputinfo.cpp
	outputfilter.cpp
	kilegrepdialog.cpp
	kiletool.cpp
	kilelauncher.cpp
	kiletoolmanager.cpp
	kilestdtools.cpp
	kilelogwidget.cpp
	kileoutputwidget.cpp
	widgets/konsolewidget.cpp
	kilewizard.cpp
	kilestructurewidget.cpp
	codecompletion.cpp
	widgets/codecompletionconfigwidget.cpp
	widgets/toolconfigwidget.cpp
	kileedit.cpp
	cleandialog.cpp
	kilehelp.cpp
	configstructure.cpp
	convert.cpp
	kiledocmanager.cpp
	kileviewmanager.cpp
	kileeventfilter.cpp
	kileerrorhandler.cpp
	dialogs/configcheckerdialog.cpp
	configtester.cpp
	kilesidebar.cpp
	plaintolatexconverter.cpp
	kileuntitled.cpp
	dialogs/statisticsdialog.cpp
	widgets/statisticswidget.cpp
	previewwidget.cpp
	kilejscript.h
	kilejscript.cpp
	widgets/scriptsmanagementwidget.h
	widgets/scriptsmanagementwidget.cpp
	editorkeysequencemanager.h
	editorkeysequencemanager.cpp
	kileversion.cpp
	kileabbrevview.cpp
	kileextensions.cpp
	widgets/categorycombobox.cpp
	widgets/extendedscrollarea.cpp
)

SET( kile_UI
	widgets/helpconfigwidget.ui
	widgets/latexconfigwidget.ui
	widgets/generalconfigwidget.ui
	widgets/configcheckerwidget.ui
	widgets/maintoolconfigwidget.ui
	librarytoolconfigwidget.ui
	processtoolconfigwidget.ui
	widgets/quicktoolconfigwidget.ui
	latextoolconfigwidget.ui
	newtoolwizard_class_page.ui
	newtoolwizard_toolname_page.ui
	widgets/environmentconfigwidget.ui
	widgets/structureviewconfigwidget.ui
	widgets/symbolviewconfigwidget.ui
	widgets/scriptingconfigwidget.ui
	widgets/graphicsconfigwidget.ui
	newdocumentwidget.ui
	tabbingdialog_base.ui
	floatdialog_base.ui
	postscriptdialog_base.ui
	includegraphicsdialog_base.ui
)

SET( kile_UI_SRCS
	widgets/configcheckerwidget.cpp
	widgets/environmentconfigwidget.cpp
	widgets/graphicsconfigwidget.cpp
	widgets/generalconfigwidget.cpp
	widgets/helpconfigwidget.cpp
	widgets/latexconfigwidget.cpp
	latextoolconfigwidget.cpp
	librarytoolconfigwidget.cpp
	newdocumentwidget.cpp
	newtoolwizard.cpp
	processtoolconfigwidget.cpp
	widgets/quicktoolconfigwidget.cpp
	widgets/scriptingconfigwidget.cpp
	widgets/structureviewconfigwidget.cpp
	widgets/symbolviewconfigwidget.cpp
	widgets/maintoolconfigwidget.cpp
)

SET(kile_SRCS ${kile_SRCS} ${kile_UI_SRCS})

KDE4_ADD_UI_FILES(kile_SRCS ${kile_UI})

SET( kile_DCOP_SKEL_SRCS
	kileappIface.h
)

# FIXME: disabling this for now
# KDE3_ADD_DCOP_SKELS(kile_SRCS ${kile_DCOP_SKEL_SRCS})

SET( kile_KCFG_SRCS
	kileconfig.kcfgc
)

KDE4_ADD_KCFG_FILES(kile_SRCS ${kile_KCFG_SRCS})

KDE4_ADD_EXECUTABLE(kile ${kile_SRCS})

#need qt3support library for now
TARGET_LINK_LIBRARIES(kile ${QT_AND_KDECORE_LIBS} kjs ktexteditor khtml ${QT_QT3SUPPORT_LIBRARY} kde3support)

INSTALL( TARGETS kile DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES kile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL( FILES kile.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
INSTALL( FILES kileui.rc docpartui.rc kilestdtools.rc tips DESTINATION
${DATA_INSTALL_DIR}/kile )

# FIXME: disabling this for now
kde4_install_icons( ${ICON_INSTALL_DIR}  )
