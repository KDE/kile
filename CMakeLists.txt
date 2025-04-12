cmake_minimum_required(VERSION 3.16)

project(kile VERSION 2.9.95)

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(KDE_COMPILERSETTINGS_LEVEL 6.0)

include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMSetupVersion)
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

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX KILE
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/src/cmake-kile-version.h
)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

kde_enable_exceptions()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED
	Core
	DBus
	Widgets
	Test
)

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Qml Core5Compat)

find_package(KF6 6.0.0
	REQUIRED COMPONENTS
		Codecs
		Config
		CoreAddons
		Crash
		DBusAddons
		GuiAddons
		I18n
		IconThemes
		KIO
		Parts
		TextEditor
		WindowSystem
		XmlGui
		TextWidgets
		ColorScheme
	OPTIONAL_COMPONENTS
		DocTools
)

find_package(Okular6 REQUIRED)
set_package_properties(Okular6 PROPERTIES
	DESCRIPTION "Okular core library"
	URL "https://okular.kde.org")

find_package(Poppler COMPONENTS Qt6)
set_package_properties("Poppler" PROPERTIES
	TYPE RECOMMENDED
	PURPOSE "Support for PDF file operations.")

if(TARGET Poppler::Qt6)
	set(HAVE_POPPLER TRUE)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

# find_package(SharedMimeInfo REQUIRED)

# Some of these can be dropped if KDE_COMPILERSETTINGS_LEVEL above is removed
# (or called with a higher version).
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

install(FILES
		AUTHORS
		ChangeLog
		README
		README.cwl
		kile-remote-control.txt
	DESTINATION ${KDE_INSTALL_DATAROOTDIR}/doc/kile)

ki18n_install(po)
if(KF6DocTools_FOUND)
	kdoctools_install(po)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
