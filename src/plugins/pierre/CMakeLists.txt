cmake_minimum_required (VERSION 3.3)
project (leechcraft_pierre)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	pierre.cpp
	fullscreen.mm
	reopenhandler.mm
	)

add_library (leechcraft_pierre SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_pierre
	${LEECHCRAFT_LIBRARIES}
	"-framework AppKit"
	)
install (TARGETS leechcraft_pierre DESTINATION ${LC_PLUGINS_DEST})

if (NOT USE_UNIX_LAYOUT)
	install (DIRECTORY data/ DESTINATION ${LC_BINDIR}/..)
endif ()

FindQtLibs (leechcraft_pierre Core)
