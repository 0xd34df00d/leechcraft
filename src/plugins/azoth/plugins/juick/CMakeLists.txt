cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_juick)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (JUICK_SRCS
	juick.cpp
	)
set (JUICK_RESOURCES
	juick.qrc
	)
QtAddResources (JUICK_RCCS ${JUICK_RESOURCES})

CreateTrs ("azoth_juick" "en;ru_RU;uk_UA" JUICK_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_juick" "en;ru_RU" "${JUICK_SRCS}" "" "azothjuicksettings.xml")

add_library (leechcraft_azoth_juick SHARED
	${JUICK_COMPILED_TRANSLATIONS}
	${JUICK_SRCS}
	${JUICK_RCCS}
	)
target_link_libraries (leechcraft_azoth_juick
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_juick DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_juick Core Widgets)
