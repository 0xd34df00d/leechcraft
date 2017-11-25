cmake_minimum_required (VERSION 3.3)
project (leechcraft_pogooglue)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	pogooglue.cpp
	)
set (RESOURCES pogooglueresources.qrc)

CreateTrs ("pogooglue" "en;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("pogooglue" "en;ru_RU" "${SRCS}" "" "")

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_pogooglue SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_pogooglue
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_pogooglue DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_pogooglue Gui)
