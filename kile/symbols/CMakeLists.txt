# FILE( GLOB_RECURSE _allFiles * )
# FOREACH( _file ${_allFiles} )
# # MESSAGE(STATUS "checking ${_file}")
# 	IF( IS_DIRECTORY _file )
# MESSAGE(STATUS "Installing dir ${_file$}")
# 		INSTALL( DIRECTORY _file DESTINATION share/apps/kile/mathsymbols
# 		         PATTERN ".svn" EXCLUDE
# 		         PATTERN "Makefile.am" EXCLUDE)
# 	ENDIF( IS_DIRECTORY _file )
# ENDFOREACH( _file )

SET(kile_symbolSubdirs
	arrows
	cyrillic
	delimiters
	greek
	misc-math
	misc-text
	operators
	relation
	special
)

# for the future:
# INSTALL( DIRECTORY ${kile_symbolSubdirs} DESTINATION share/apps/kile/mathsymbols
# 		PATTERN ".svn" EXCLUDE
# 		PATTERN "Makefile.am" EXCLUDE)

FOREACH( _dir ${kile_symbolSubdirs} )
	ADD_SUBDIRECTORY( ${_dir} )
ENDFOREACH( _dir )

########### next target ###############

find_package(Qt4 REQUIRED)

IF(QT4_FOUND)
      INCLUDE(${QT_USE_FILE})
    ELSE(QT4_FOUND)
      MESSAGE(FATAL ERROR "Qt4 not found.")
    ENDIF(QT4_FOUND)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} )

SET(gesymb_SRCS
	gesymb.cpp
	gesymb.h
)

ADD_EXECUTABLE(gesymb EXCLUDE_FROM_ALL ${gesymb_SRCS} )
TARGET_LINK_LIBRARIES(gesymb -lQtCore -lQtGui)
# TARGET_LINK_LIBRARIES(gesymb ${QT_LIBARIES} )


########### install files ###############


