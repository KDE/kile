
add_subdirectory(relation)
add_subdirectory(arrows)
add_subdirectory(delimiters)
add_subdirectory(greek)
add_subdirectory(misc-math)
add_subdirectory(misc-text)
add_subdirectory(operators)
add_subdirectory(special)
add_subdirectory(cyrillic)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(gesymb-ng_SRCS gesymb-ng.cpp)
add_executable(gesymb-ng EXCLUDE_FROM_ALL ${gesymb-ng_SRCS})
target_link_libraries(gesymb-ng ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})

set(convertOldSymbolFormat_SRCS convertOldSymbolFormat.cpp convertOldSymbolFormat.h)
add_executable(convertOldSymbolFormat EXCLUDE_FROM_ALL ${convertOldSymbolFormat_SRCS})
target_link_libraries(convertOldSymbolFormat ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})
