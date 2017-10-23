cmake_minimum_required (VERSION 3.3)
project (leechcraft_y7)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	y7.cpp
	)

add_library (leechcraft_y7 SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_y7
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_y7 DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_y7 Core)
