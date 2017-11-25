cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_otroid)
include (InitLCPlugin)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (LibOTR REQUIRED)
message (STATUS "Found OTR: ${LIBOTR_LIBRARY}")

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LIBOTR_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (OTROID_SRCS
	otroid.cpp
	otrhandler.cpp
	authenticator.cpp
	initiateauthdialog.cpp
	fpmanager.cpp
	privkeymanager.cpp
	xmlsettingsmanager.cpp
	util.cpp
	)
set (OTROID_FORMS
	initiateauthdialog.ui
	)
set (OTROID_RESOURCES azothotroidresources.qrc)
CreateTrs ("azoth_otroid" "en;ru_RU;uk_UA" OTROID_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_otroid" "en;ru_RU" "${OTROID_SRCS}" "" "azothotroidsettings.xml")
QtAddResources (OTROID_RCCS ${OTROID_RESOURCES})
QtWrapUi (OTROID_UIS_H ${OTROID_FORMS})

add_library (leechcraft_azoth_otroid SHARED
	${OTROID_COMPILED_TRANSLATIONS}
	${OTROID_SRCS}
	${OTROID_RCCS}
	${OTROID_UIS_H}
	)
target_link_libraries (leechcraft_azoth_otroid
	${LEECHCRAFT_LIBRARIES}
	${LIBOTR_LIBRARIES}
	leechcraft_azoth_common
	)
install (TARGETS leechcraft_azoth_otroid DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothotroidsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_otroid Concurrent Widgets)
