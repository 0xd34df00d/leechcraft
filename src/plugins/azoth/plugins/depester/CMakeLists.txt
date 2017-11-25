cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_depester)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (DEPESTER_SRCS
	depester.cpp
	)
set (DEPESTER_RESOURCES
	depesterresources.qrc
	)

QtAddResources (DEPESTER_RCCS ${DEPESTER_RESOURCES})
CreateTrs ("azoth_depester" "en;es;ru_RU;uk_UA" DEPESTER_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_depester" "en;ru_RU" "${DEPESTER_SRCS}" "" "")

add_library (leechcraft_azoth_depester SHARED
	${DEPESTER_COMPILED_TRANSLATIONS}
	${DEPESTER_SRCS}
	${DEPESTER_UIS_H}
	${DEPESTER_RCCS}
	)
target_link_libraries (leechcraft_azoth_depester
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_depester DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${DEPESTER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

FindQtLibs (leechcraft_azoth_depester Core Widgets)
