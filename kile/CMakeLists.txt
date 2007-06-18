ADD_SUBDIRECTORY( icons )
ADD_SUBDIRECTORY( pics )
ADD_SUBDIRECTORY( symbols )
ADD_SUBDIRECTORY( templates )
ADD_SUBDIRECTORY( syntax )
ADD_SUBDIRECTORY( complete )
ADD_SUBDIRECTORY( help )
ADD_SUBDIRECTORY( encodings )
ADD_SUBDIRECTORY( test )

INCLUDE( KDE3Macros )
INCLUDE( FindPerl )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

########### next target ###############


# taken from KDE4's KJS CMakeLists.txt file

SET(CREATE_HASH_TABLE ${CMAKE_CURRENT_SOURCE_DIR}/create_hash_table )

MACRO(CREATE_LUT _srcs_LIST _in_FILE _out_FILE _dep_FILE)
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
		COMMAND ${PERL_EXECUTABLE} ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} > ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
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
	previewconfigwidget.cpp
	quickpreview.cpp
	includegraphicsdialog.cpp
	newfilewizard.cpp
	managetemplatesdialog.cpp
	templates.cpp
	usermenudialog.cpp
	kileconfigdialog.cpp
	tabbingdialog.cpp
	symbolview.cpp
	quickdocumentdialog.cpp
	kilefileselect.cpp
	kileapplication.cpp
	docpart.cpp
	kile.cpp
	main.cpp
	kileactions.cpp
	kilestdactions.cpp
	kileinfo.cpp
	kiledocumentinfo.cpp
	kileproject.cpp
	kileprojectdlgs.cpp
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
	kilekonsolewidget.cpp
	kilewizard.cpp
	kilestructurewidget.cpp
	codecompletion.cpp
	configcodecompletion.cpp
	kiletoolconfigwidget.cpp
	kileedit.cpp
	cleandialog.cpp
	kilehelp.cpp
	configstructure.cpp
	convert.cpp
	kiledocmanager.cpp
	kileviewmanager.cpp
	kileeventfilter.cpp
	kileerrorhandler.cpp
	configcheckerdlg.cpp
	configtester.cpp
	kilesidebar.cpp
	plaintolatexconverter.cpp
	kileuntitled.cpp
	kilestatsdlg.cpp
	kilestatswidget.cpp
	previewwidget.cpp
	kilejscript.h
	kilejscript.cpp
	scriptsmanagementwidget.h
	scriptsmanagementwidget.cpp
	editorkeysequencemanager.h
	editorkeysequencemanager.cpp
	kileversion.cpp
	kileabbrevview.cpp
	kileextensions.cpp
	kilemultitabbar.cpp
)

KDE3_AUTOMOC(${kile_SRCS})

SET( kile_UI
	helpconfigwidget.ui
	latexconfigwidget.ui
	generalconfigwidget.ui
	configcheckerwidget.ui
	toolconfigwidget.ui
	librarytoolconfigwidget.ui
	processtoolconfigwidget.ui
	quicktoolconfigwidget.ui
	latextoolconfigwidget.ui
	newtoolwizard.ui
	envconfigwidget.ui
	structureconfigwidget.ui
	symbolviewconfig.ui
	scriptingconfigwidget.ui
	graphicsconfigwidget.ui
	newdocumentwidget.ui
)

KDE3_ADD_UI_FILES(kile_SRCS ${kile_UI} )

SET( kile_DCOP_SKEL_SRCS
	kileappIface.h
)

KDE3_ADD_DCOP_SKELS(kile_SRCS ${kile_DCOP_SKEL_SRCS})

SET( kile_KCFG_SRCS
	kileconfig.kcfgc
)

KDE3_ADD_KCFG_FILES(kile_SRCS ${kile_KCFG_SRCS})

KDE3_ADD_EXECUTABLE(kile ${kile_SRCS})

TARGET_LINK_LIBRARIES(kile ${QT_AND_KDECORE_LIBS} kjs kdeprint ktexteditor katepartinterfaces khtml )

INSTALL( TARGETS kile DESTINATION bin )


########### install files ###############

INSTALL( FILES kile.desktop DESTINATION share/applications/kde )
INSTALL( FILES kile.kcfg DESTINATION share/config.kcfg )
INSTALL( FILES kileui.rc docpartui.rc kilestdtools.rc tips DESTINATION share/apps/kile )

KDE3_INSTALL_ICONS( hicolor )
