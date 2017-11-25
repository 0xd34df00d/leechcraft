cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_velvetbird)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (GLIB2 REQUIRED)
find_package (LibPurple REQUIRED)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	${GLIB2_INCLUDE_DIR}
	${LIBPURPLE_INCLUDE_DIRS}
	)

message (STATUS "Bindir: ${CMAKE_CURRENT_BINARY_DIR}")

add_definitions (${LIBPURPLE_DEFINITIONS})

set (VELVETBIRD_SRCS
	velvetbird.cpp
	protomanager.cpp
	protocol.cpp
	account.cpp
	accregfirstpage.cpp
	util.cpp
	buddy.cpp
	convimmessage.cpp
	accountconfigdialog.cpp
	)
set (VELVETBIRD_FORMS
	accregfirstpage.ui
	accountconfigdialog.ui
	)
set (VELVETBIRD_RESOURCES azothvelvetbirdresources.qrc)
#CreateTrs ("azoth_velvetbird" "en;ru_RU;uk_UA" VELVETBIRD_COMPILED_TRANSLATIONS)

QtWrapUi (VELVETBIRD_UIS_H ${VELVETBIRD_FORMS})
QtAddResources (VELVETBIRD_RCCS ${VELVETBIRD_RESOURCES})

add_library (leechcraft_azoth_velvetbird SHARED
	${VELVETBIRD_COMPILED_TRANSLATIONS}
	${VELVETBIRD_SRCS}
	${VELVETBIRD_UIS_H}
	${VELVETBIRD_RCCS}
	)
target_link_libraries (leechcraft_azoth_velvetbird
	${LEECHCRAFT_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBPURPLE_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_velvetbird DESTINATION ${LC_PLUGINS_DEST})
#install (FILES ${VELVETBIRD_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
#install (FILES azothvelvetbirdsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_velvetbird Widgets)
