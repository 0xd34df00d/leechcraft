cmake_minimum_required (VERSION 3.3)
project (leechcraft_blackdash)
include (InitLCPlugin NO_POLICY_SCOPE)

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

add_library (leechcraft_blackdash SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_blackdash
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blackdash DESTINATION ${LC_PLUGINS_DEST})
