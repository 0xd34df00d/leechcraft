cmake_minimum_required (VERSION 2.8)
project (leechcraft_lads)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDBUS TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	lads.cpp
	)
set (HEADERS
	lads.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)

set (RESOURCES ladsresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

if (NOT UNIX)
	message (ERROR "This plugin is intendend to be built only on *NIX systems")
endif (NOT UNIX)

add_library (leechcraft_lads SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_lads
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_lads DESTINATION ${LC_PLUGINS_DEST})
