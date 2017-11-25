cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_hili)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (HILI_SRCS
	hili.cpp
	xmlsettingsmanager.cpp
	)
set (HILI_RESOURCES azothhiliresources.qrc)
CreateTrs ("azoth_hili" "en;es;ru_RU;uk_UA" HILI_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_hili" "en;ru_RU" "${HILI_SRCS}" "" "azothhilisettings.xml")
QtAddResources (HILI_RCCS ${HILI_RESOURCES})

add_library (leechcraft_azoth_hili SHARED
	${HILI_COMPILED_TRANSLATIONS}
	${HILI_SRCS}
	${HILI_RCCS}
	)
target_link_libraries (leechcraft_azoth_hili
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_hili DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothhilisettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_hili Core Widgets)
