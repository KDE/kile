project(kile)

cmake_minimum_required(VERSION 3.0.0)

find_package(ECM 5.19.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)

kde_enable_exceptions()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(Qt5 5.3 CONFIG REQUIRED
	Core
	DBus
	Widgets
	Script
	Test
)

find_package(KF5 5.31 REQUIRED COMPONENTS
	Config
	CoreAddons
	Crash
	DBusAddons
	DocTools
	GuiAddons
	I18n
	IconThemes
	Init
	KHtml
	KIO
	Parts
	TextEditor
	WindowSystem
	XmlGui
)

find_package(Okular5 REQUIRED)
set_package_properties(Okular5 PROPERTIES
	DESCRIPTION "Okular core library"
	URL "http://okular.kde.org")

find_package(Poppler COMPONENTS Qt5)
set_package_properties("Poppler" PROPERTIES
	TYPE RECOMMENDED
	PURPOSE "Support for PDF file operations.")

if(Poppler_Qt5_FOUND)
	set(HAVE_POPPLER TRUE)
	include_directories(${Poppler_INCLUDE_DIRS})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

# find_package(SharedMimeInfo REQUIRED)

add_definitions(
#   -DQT_USE_QSTRINGBUILDER
#   -DQT_NO_CAST_FROM_ASCII
#   -DQT_NO_CAST_FROM_BYTEARRAY
#   -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_NO_CAST_TO_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

set(KILE_VERSION CACHE STRING "Kile's version; can be used to install special documentation files in share/doc/kile-$KILE_VERSION/")
set(kile_PO_FILE "kile.po")
set(kile_DOCBOOK "index.docbook")

macro(KILE_ADD_DOCBOOK_DIRECTORY _sources _relative_dir _docbook _language)
	# only depend on images
	file(GLOB_RECURSE _dependencies ${CMAKE_CURRENT_SOURCE_DIR}/${_relative_dir}/*.[pP][nN][gG])
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir})

	set(_source_docbook ${CMAKE_CURRENT_SOURCE_DIR}/${_relative_dir}/${_docbook})
	set(_compiled_docbook ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}/index.cache.bz2)

	add_custom_command(OUTPUT ${_compiled_docbook}
			COMMAND ${KDE4_MEINPROC_EXECUTABLE} --check --cache index.cache.bz2 ${_source_docbook}
			DEPENDS ${_dependencies} ${_source_docbook}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}
			COMMENT "Compiling ${_language} docbook in ${_relative_dir}/${_docbook}")

	install(FILES ${_compiled_docbook} ${_source_docbook} ${_dependencies} DESTINATION ${HTML_INSTALL_DIR}/${_language}/kile)
	set(${_sources} ${${_sources}} ${_compiled_docbook})
endmacro(KILE_ADD_DOCBOOK_DIRECTORY _sources _docbook _language)

macro(KILE_ADD_TRANSLATION_DIRECTORY _sources _relative_dir _file _language)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir})

	get_filename_component(_base_name ${_file} NAME_WE)
	set(_source_file ${CMAKE_CURRENT_SOURCE_DIR}/${_relative_dir}/${_file})
	set(_base_compiled_file ${_base_name}.mo)
	set(_compiled_file ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}/${_base_compiled_file})

	add_custom_command(OUTPUT ${_compiled_file}
			COMMAND ${KILE_MSGFMT} -o ${_compiled_file} ${_source_file}
			DEPENDS ${_source_file}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}
			COMMENT "Compiling ${_language} translation file")

	install(FILES ${_compiled_file} DESTINATION ${LOCALE_INSTALL_DIR}/${_language}/LC_MESSAGES)
	set(${_sources} ${${_sources}} ${_compiled_file})
endmacro(KILE_ADD_TRANSLATION_DIRECTORY _sources _relative_dir _file _language)

macro(KILE_ADD_TRANSLATION_DIRECTORIES _documentation_sources _translation_sources _dir)
	file(GLOB _subdirectories ${_dir}/*)

	foreach(_entry ${_subdirectories})
		if(IS_DIRECTORY ${_entry})
			get_filename_component(_language ${_entry} NAME)
			if(KILE_MSGFMT AND EXISTS ${_entry}/messages)
				KILE_ADD_TRANSLATION_DIRECTORY(${_translation_sources} translations/${_language}/messages ${kile_PO_FILE} ${_language})
				set(_translations_built ${_translations_built} ${_language})
			endif(KILE_MSGFMT AND EXISTS ${_entry}/messages)
			if(KDE4_MEINPROC_EXECUTABLE AND EXISTS ${_entry}/doc)
				KILE_ADD_DOCBOOK_DIRECTORY(${_documentation_sources} translations/${_language}/doc ${kile_DOCBOOK} ${_language})
				set(_docbooks_built ${_docbooks_built} ${_language})
			endif(KDE4_MEINPROC_EXECUTABLE AND EXISTS ${_entry}/doc)
		endif(IS_DIRECTORY ${_entry})
	endforeach(_entry ${_subdirectories})

	if(_translations_built)
		message(STATUS "Building translations for ${_translations_built}")
	else(_translations_built)
		message(STATUS "Not building translations")
	endif(_translations_built)

	if(_docbooks_built)
		message(STATUS "Building documentation for ${_docbooks_built}")
	else(_docbooks_built)
		message(STATUS "Not building additional documentation")
	endif(_docbooks_built)
endmacro(KILE_ADD_TRANSLATION_DIRECTORIES _documentation_sources _translation_sources _dir)

macro(KILE_FIND_DOC_TOOLS)
	if(KDE4_MEINPROC_EXECUTABLE)
		message(STATUS "Found meinproc4: ${KDE4_MEINPROC_EXECUTABLE}")
	else(KDE4_MEINPROC_EXECUTABLE)
		message(STATUS "Could not find meinproc4")
	endif(KDE4_MEINPROC_EXECUTABLE)

	find_program(KILE_MSGFMT "msgfmt" DOC "path of the msgfmt executable")
	if(KILE_MSGFMT)
		message(STATUS "Found msgfmt: ${KILE_MSGFMT}")
	else(KILE_MSGFMT)
		 message(STATUS "Could not find msgfmt")
	endif(KILE_MSGFMT)

	if(KDE4_MEINPROC_EXECUTABLE AND KILE_MSGFMT)
		set(KILE_DOC_TOOLS_FOUND true)
	endif(KDE4_MEINPROC_EXECUTABLE AND KILE_MSGFMT)
endmacro(KILE_FIND_DOC_TOOLS)

if(EXISTS ${kile_SOURCE_DIR}/doc OR EXISTS ${kile_SOURCE_DIR}/translations)
	KILE_FIND_DOC_TOOLS()
endif(EXISTS ${kile_SOURCE_DIR}/doc OR EXISTS ${kile_SOURCE_DIR}/translations)

if(EXISTS ${kile_SOURCE_DIR}/doc)
	if(KDE4_MEINPROC_EXECUTABLE)
		message(STATUS "Building standard documentation")
		KILE_ADD_DOCBOOK_DIRECTORY(kile_DOCUMENTATION_SOURCES doc index.docbook en)
	endif(KDE4_MEINPROC_EXECUTABLE)
else(EXISTS ${kile_SOURCE_DIR}/doc)
	message(STATUS "Not building standard documentation")
endif(EXISTS ${kile_SOURCE_DIR}/doc)

if(EXISTS ${kile_SOURCE_DIR}/translations)
	if(KDE4_MEINPROC_EXECUTABLE OR KILE_MSGFMT)
		KILE_ADD_TRANSLATION_DIRECTORIES(kile_DOCUMENTATION_SOURCES kile_TRANSLATION_SOURCES ${kile_SOURCE_DIR}/translations)
	endif(KDE4_MEINPROC_EXECUTABLE OR KILE_MSGFMT)
else(EXISTS ${kile_SOURCE_DIR}/translations)
	message(STATUS "Not building translations")
endif(EXISTS ${kile_SOURCE_DIR}/translations)

if(kile_DOCUMENTATION_SOURCES)
	add_custom_target(docbooks ALL DEPENDS ${kile_DOCUMENTATION_SOURCES})
endif(kile_DOCUMENTATION_SOURCES)

if(kile_TRANSLATION_SOURCES)
	add_custom_target(translations ALL DEPENDS ${kile_TRANSLATION_SOURCES})
endif(kile_TRANSLATION_SOURCES)

add_subdirectory(doc)
add_subdirectory(src)

########### install files ###############

install(
	FILES kile.xml
	DESTINATION ${XDG_MIME_INSTALL_DIR})
# update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

install(
	FILES
		kile.upd
		kile1.6_upd.pl
		kile1.7_upd.pl
		kile1.9_upd.pl
		kile1.9-1_upd.pl
		kile2.0-1_upd.pl
		kile2.0-2_upd.pl
		kile2.0-3_upd.pl
		kile2.0-4_upd.pl
		kile2.0-5_upd.pl
	DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

if(KILE_VERSION)
	set(_KILE_SPECIAL_DOC_DIR "share/doc/kile-${KILE_VERSION}")
else(KILE_VERSION)
	set(_KILE_SPECIAL_DOC_DIR "share/doc/kile")
endif(KILE_VERSION)

install(
	FILES
		AUTHORS
		ChangeLog
		README
		README.cwl
		kile-remote-control.txt
		README.MacOSX
	DESTINATION ${_KILE_SPECIAL_DOC_DIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
