cmake_minimum_required (VERSION 3.3)
project (leechcraft_pintab)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	pintab.cpp
	)
set (RESOURCES
	pintabresources.qrc
	)

CreateTrs ("pintab" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("pintab" "en;ru_RU" "${SRCS}" "" "")

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_pintab SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_pintab
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_pintab DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_pintab Widgets)
