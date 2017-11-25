cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_rosenthal)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ROSENTHAL_SRCS
	rosenthal.cpp
	highlighter.cpp
	)
set (ROSENTHAL_RESOURCES azothrosenthalresources.qrc)
CreateTrs ("azoth_rosenthal" "en;es;ro;ru_RU;uk_UA" ROSENTHAL_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_rosenthal" "en;ru_RU" "${ROSENTHAL_SRCS}" "" "")
QtAddResources (ROSENTHAL_RCCS ${ROSENTHAL_RESOURCES})

add_library (leechcraft_azoth_rosenthal SHARED
	${ROSENTHAL_SRCS}
	${ROSENTHAL_COMPILED_TRANSLATIONS}
	${ROSENTHAL_RCCS}
	)
target_link_libraries (leechcraft_azoth_rosenthal
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_rosenthal DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_rosenthal Widgets)
