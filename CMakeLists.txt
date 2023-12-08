cmake_minimum_required(VERSION 3.16)

project(kile)

set(KILE_VERSION CACHE STRING "Kile's version; can be used to set the internal version of Kile on MacOSX")

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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message(STATUS "Found Qt version ${QT_VERSION_MAJOR}")
if(BUILD_WITH_QT6)
    message(STATUS "Using Qt version 6, honoring BUILD_WITH_QT6")
    set(QT_VERSION_MAJOR 6)
elseif(BUILD_WITH_QT5)
    message(STATUS "Using Qt version 5, honoring BUILD_WITH_QT5")
    set(QT_VERSION_MAJOR 5)
elseif(QT_MAJOR_VERSION)
    message(STATUS "Using Qt version " ${QT_MAJOR_VERSION} ", honoring QT_MAJOR_VERSION")
    set(QT_VERSION_MAJOR ${QT_MAJOR_VERSION})
endif()

find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED
	Core
	DBus
	Widgets
	Test
)

if (${QT_VERSION_MAJOR} STREQUAL 5)
    find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Script)
else()
    find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Qml)
endif()

find_package(KF${QT_VERSION_MAJOR} 5.108 REQUIRED COMPONENTS
	Config
	CoreAddons
	Crash
	DBusAddons
	DocTools
	GuiAddons
	I18n
	IconThemes
	KIO
	Parts
	TextEditor
	WindowSystem
	XmlGui
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Okular${QT_VERSION_MAJOR} REQUIRED)
set_package_properties(Okular${QT_VERSION_MAJOR} PROPERTIES
	DESCRIPTION "Okular core library"
	URL "https://okular.kde.org")

find_package(Poppler COMPONENTS Qt${QT_VERSION_MAJOR})
set_package_properties("Poppler" PROPERTIES
	TYPE RECOMMENDED
	PURPOSE "Support for PDF file operations.")

if(TARGET Poppler::Qt${QT_VERSION_MAJOR})
	set(HAVE_POPPLER TRUE)
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

if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  add_definitions(-DQT_MESSAGELOGCONTEXT)
endif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

add_subdirectory(doc)
add_subdirectory(src)

########### install files ###############

install(
	FILES kile.xml
	DESTINATION ${KDE_INSTALL_MIMEDIR})
# update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

install(
	FILES
		kile.upd
	DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kconf_update)

install(
	PROGRAMS
		kile1.6_upd.pl
		kile1.7_upd.pl
		kile1.9_upd.pl
		kile1.9-1_upd.pl
		kile2.0-1_upd.pl
		kile2.0-2_upd.pl
		kile2.0-3_upd.pl
		kile2.0-4_upd.pl
		kile2.0-5_upd.pl
	DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kconf_update)

install(FILES
		AUTHORS
		ChangeLog
		README
		README.cwl
		kile-remote-control.txt
	DESTINATION ${KDE_INSTALL_DATAROOTDIR}/doc/kile)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
