cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_modnok)
include (InitLCPlugin)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (MODNOK_SRCS
	modnok.cpp
	xmlsettingsmanager.cpp
	)
set (MODNOK_RESOURCES azothmodnokresources.qrc)
CreateTrs ("azoth_modnok" "en;es;ru_RU;uk_UA" MODNOK_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_modnok" "en;ru_RU" "${MODNOK_SRCS}" "" "azothmodnoksettings.xml")
QtAddResources (MODNOK_RCCS ${MODNOK_RESOURCES})

add_library (leechcraft_azoth_modnok SHARED
	${MODNOK_COMPILED_TRANSLATIONS}
	${MODNOK_SRCS}
	${MODNOK_RCCS}
	)
target_link_libraries (leechcraft_azoth_modnok
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_modnok DESTINATION ${LC_PLUGINS_DEST})
install (PROGRAMS lc_azoth_modnok_latexconvert.sh DESTINATION ${LC_SHARE_DEST}/azoth)
install (FILES azothmodnoksettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_modnok Core Widgets)
