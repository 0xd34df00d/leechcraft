cmake_minimum_required (VERSION 3.3)
project (leechcraft_twifee)
include (InitLCPlugin)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	twifee.cpp
	)

add_library (leechcraft_twifee SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_twifee
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_twifee DESTINATION ${LC_PLUGINS_DEST})
