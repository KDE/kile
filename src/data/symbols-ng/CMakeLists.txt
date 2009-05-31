
ADD_SUBDIRECTORY(relation)
ADD_SUBDIRECTORY(arrows)
ADD_SUBDIRECTORY(delimiters)
ADD_SUBDIRECTORY(greek)
ADD_SUBDIRECTORY(misc-math)
ADD_SUBDIRECTORY(misc-text)
ADD_SUBDIRECTORY(operators)
ADD_SUBDIRECTORY(special)
ADD_SUBDIRECTORY(cyrillic)

find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(gesymb-ng_SRCS gesymb-ng.cpp)
qt4_automoc(${gesymb-ng_SRCS})
add_executable(gesymb-ng EXCLUDE_FROM_ALL ${gesymb-ng_SRCS})
target_link_libraries(gesymb-ng ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})

set(convertOldSymbolFormat_SRCS convertOldSymbolFormat.cpp convertOldSymbolFormat.h)
qt4_automoc(${convertOldSymbolFormat_SRCS})
add_executable(convertOldSymbolFormat EXCLUDE_FROM_ALL ${convertOldSymbolFormat_SRCS})
target_link_libraries(convertOldSymbolFormat ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})

