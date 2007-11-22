PROJECT( kile )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

CMAKE_MINIMUM_REQUIRED( VERSION 2.4.4 FATAL_ERROR )


SET( KILE_VERSION CACHE STRING "Kile's version; can be used to install special documentation files in share/doc/kile-$KILE_VERSION/" )

SET( kile_PO_FILE "kile.po" )
SET( kile_DOCBOOK "index.docbook" )

MACRO( KILE_ADD_DOCBOOK_DIRECTORY _sources _relative_dir _docbook _language )
	# only depend on images
	FILE( GLOB_RECURSE _dependencies ${CMAKE_CURRENT_SOURCE_DIR}/${_relative_dir}/*.[pP][nN][gG] )
	FILE( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir} )

	SET( _source_docbook ${CMAKE_CURRENT_SOURCE_DIR}/${_relative_dir}/${_docbook} )
	SET( _compiled_docbook ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}/index.cache.bz2 )

	ADD_CUSTOM_COMMAND(OUTPUT ${_compiled_docbook}
			COMMAND ${KILE_MEINPROC} --check --cache index.cache.bz2 ${_source_docbook}
			DEPENDS ${_dependencies} ${_source_docbook}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}
			COMMENT "Compiling ${_language} docbook in ${_relative_dir}/${_docbook}" )

	INSTALL( FILES ${_compiled_docbook} ${_dependencies} DESTINATION share/doc/HTML/${_language})
	SET( ${_sources} ${${_sources}} ${_compiled_docbook} )
ENDMACRO( KILE_ADD_DOCBOOK_DIRECTORY _sources _docbook _language )

MACRO( KILE_ADD_TRANSLATION_DIRECTORY _sources _relative_dir _file _language )
	FILE( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir} )

	GET_FILENAME_COMPONENT(_base_name ${_file} NAME_WE)
	SET( _source_file ${CMAKE_CURRENT_SOURCE_DIR}/${_relative_dir}/${_file} )
	SET( _base_compiled_file ${_base_name}.mo)
	SET( _compiled_file ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}/${_base_compiled_file} )

	ADD_CUSTOM_COMMAND(OUTPUT ${_compiled_file}
			COMMAND ${KILE_MSGFMT} -o ${_compiled_file} ${_source_file}
			DEPENDS ${_source_file}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_relative_dir}
			COMMENT "Compiling ${_language} translation file" )

	INSTALL( FILES ${_compiled_file} DESTINATION share/locale/${_language}/LC_MESSAGES )
	SET( ${_sources} ${${_sources}} ${_compiled_file} )
ENDMACRO( KILE_ADD_TRANSLATION_DIRECTORY _sources _relative_dir _file _language )

MACRO( KILE_ADD_TRANSLATION_DIRECTORIES _documentation_sources _translation_sources _dir )
	FILE( GLOB _subdirectories ${_dir}/* )

	FOREACH( _entry ${_subdirectories} )
		IF( IS_DIRECTORY ${_entry} )
			GET_FILENAME_COMPONENT( _language ${_entry} NAME )
			IF( KILE_MSGFMT AND EXISTS ${_entry}/messages )
				KILE_ADD_TRANSLATION_DIRECTORY( ${_translation_sources} translations/${_language}/messages ${kile_PO_FILE} ${_language} )
				SET(_translations_built ${_translations_built} ${_language} )
			ENDIF( KILE_MSGFMT AND EXISTS ${_entry}/messages )
			IF( KILE_MEINPROC AND EXISTS ${_entry}/doc )
				KILE_ADD_DOCBOOK_DIRECTORY( ${_documentation_sources} translations/${_language}/doc ${kile_DOCBOOK} ${_language} )
				SET(_docbooks_built ${_docbooks_built} ${_language} )
			ENDIF( KILE_MEINPROC AND EXISTS ${_entry}/doc )
		ENDIF( IS_DIRECTORY ${_entry} )
	ENDFOREACH( _entry ${_subdirectories} )

	IF( _translations_built )
		MESSAGE( STATUS "Building translations for ${_translations_built}" )
	ELSE( _translations_built )
		MESSAGE( STATUS "Not building translations" )
	ENDIF( _translations_built )

	IF( _docbooks_built )
		MESSAGE( STATUS "Building documentation for ${_docbooks_built}" )
	ELSE( _docbooks_built )
		MESSAGE( STATUS "Not building additional documentation" )
	ENDIF( _docbooks_built )
ENDMACRO( KILE_ADD_TRANSLATION_DIRECTORIES _documentation_sources _translation_sources _dir )

MACRO( KILE_FIND_DOC_TOOLS )
	FIND_PROGRAM( KILE_MEINPROC "meinproc" DOC "path of the meinproc executable")
	IF( KILE_MEINPROC )
		MESSAGE( STATUS "Found meinproc executable: ${KILE_MEINPROC}" )
	ELSE( KILE_MEINPROC )
		MESSAGE( STATUS "meinproc executable not found" )
	ENDIF( KILE_MEINPROC )
	FIND_PROGRAM( KILE_MSGFMT "msgfmt" DOC "path of the msgfmt executable")
	IF( KILE_MEINPROC )
		MESSAGE( STATUS "Found msgfmt executable: ${KILE_MSGFMT}" )
	ELSE( KILE_MEINPROC )
		MESSAGE( STATUS "msgfmt executable not found" )
	ENDIF( KILE_MEINPROC )
	IF( KILE_MEINPROC AND KILE_MSGFMT )
		SET( KILE_DOC_TOOLS_FOUND true )
	ENDIF( KILE_MEINPROC AND KILE_MSGFMT )
ENDMACRO( KILE_FIND_DOC_TOOLS )

IF( EXISTS ${kile_SOURCE_DIR}/doc OR EXISTS ${kile_SOURCE_DIR}/translations )
	KILE_FIND_DOC_TOOLS( )
ENDIF(  EXISTS ${kile_SOURCE_DIR}/doc OR EXISTS ${kile_SOURCE_DIR}/translations )

IF( EXISTS ${kile_SOURCE_DIR}/doc )
	IF( KILE_MEINPROC )
		MESSAGE( STATUS "Building standard documentation")
		KILE_ADD_DOCBOOK_DIRECTORY( kile_DOCUMENTATION_SOURCES doc index.docbook en )
	ENDIF( KILE_MEINPROC )
ELSE( EXISTS ${kile_SOURCE_DIR}/doc )
	MESSAGE( STATUS "Not building standard documentation")
ENDIF( EXISTS ${kile_SOURCE_DIR}/doc )

IF( EXISTS ${kile_SOURCE_DIR}/translations )
	IF( KILE_MEINPROC OR KILE_MSGFMT )
		KILE_ADD_TRANSLATION_DIRECTORIES( kile_DOCUMENTATION_SOURCES kile_TRANSLATION_SOURCES ${kile_SOURCE_DIR}/translations )
	ENDIF( KILE_MEINPROC OR KILE_MSGFMT )
ELSE( EXISTS ${kile_SOURCE_DIR}/translations )
	MESSAGE( STATUS "Not building translations")
ENDIF( EXISTS ${kile_SOURCE_DIR}/translations )

IF( kile_DOCUMENTATION_SOURCES )
	ADD_CUSTOM_TARGET(docbooks ALL DEPENDS ${kile_DOCUMENTATION_SOURCES})
ENDIF( kile_DOCUMENTATION_SOURCES )

IF( kile_TRANSLATION_SOURCES )
	ADD_CUSTOM_TARGET(translations ALL DEPENDS ${kile_TRANSLATION_SOURCES})
ENDIF( kile_TRANSLATION_SOURCES)

ADD_SUBDIRECTORY( kile )

########### install files ###############

INSTALL( FILES x-kilepr.desktop DESTINATION share/mimelink/text )

INSTALL( FILES kile.upd kile1.6_upd.pl kile1.7_upd.pl kile1.9_upd.pl kile1.9-1_upd.pl kile2.0-1_upd.pl kile2.0-2_upd.pl kile2.0-3_upd.pl kile2.0-4_upd.pl kile2.0-5_upd.pl DESTINATION share/apps/kconf_update )

IF( KILE_VERSION )
	SET(_KILE_SPECIAL_DOC_DIR "share/doc/kile-${KILE_VERSION}")
ELSE( KILE_VERSION )
	SET(_KILE_SPECIAL_DOC_DIR "share/doc/kile")
ENDIF( KILE_VERSION )

INSTALL( FILES AUTHORS ChangeLog README README.cwl kile-remote-control.txt README.MacOSX TODO DESTINATION ${_KILE_SPECIAL_DOC_DIR}) 

