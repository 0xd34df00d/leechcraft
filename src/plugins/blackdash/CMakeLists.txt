cmake_minimum_required (VERSION 2.8)
project (leechcraft_blackdash)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	blackdash.cpp
	dashtab.cpp
	)
set (HEADERS
	blackdash.h
	dashtab.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()

add_library (leechcraft_blackdash SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_blackdash
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blackdash RUNTIME DESTINATION ${LC_PLUGINS_DEST})
