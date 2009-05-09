find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(gesymb-ng_SRCS gesymb-ng.cpp)
qt4_automoc(${gesymb-ng_SRCS})
add_executable(gesymb-ng EXCLUDE_FROM_ALL ${gesymb-ng_SRCS})
target_link_libraries(gesymb-ng ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})
