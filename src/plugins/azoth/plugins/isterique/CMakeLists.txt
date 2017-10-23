cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_isterique)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ISTERIQUE_SRCS
	isterique.cpp
	xmlsettingsmanager.cpp
	)
set (ISTERIQUE_RESOURCES
	isterique.qrc
	)
CreateTrsUpTarget ("azoth_isterique" "en;ru_RU" "${ISTERIQUE_SRCS}" "" "azothisteriquesettings.xml")
CreateTrs ("azoth_isterique" "en;ru_RU;uk_UA" ISTERIQUE_COMPILED_TRANSLATIONS)
QtAddResources (ISTERIQUE_RCCS ${ISTERIQUE_RESOURCES})

add_library (leechcraft_azoth_isterique SHARED
	${ISTERIQUE_COMPILED_TRANSLATIONS}
	${ISTERIQUE_SRCS}
	${ISTERIQUE_RCCS}
	)
target_link_libraries (leechcraft_azoth_isterique
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_isterique DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothisteriquesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_isterique Core Widgets)
