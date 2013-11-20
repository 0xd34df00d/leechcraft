cmake_minimum_required (VERSION 2.8)
project (leechcraft_glance)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (HEADERS
	core.h
	glance.h
	glanceitem.h
	glanceshower.h
	)
set (SRCS
	core.cpp
	glance.cpp
	glanceitem.cpp
	glanceshower.cpp
	)
CreateTrs ("glance" "en;es;ru_RU;tt;uk_UA" COMPILED_TRANSLATIONS)

set (GLANCE_RESOURCES glanceresources.qrc)
QtAddResources (GLANCE_RCCS ${GLANCE_RESOURCES})

add_library (leechcraft_glance SHARED
	${GLANCE_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)

FindQtLibs (leechcraft_glance Core)

target_link_libraries (leechcraft_glance
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_glance DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
