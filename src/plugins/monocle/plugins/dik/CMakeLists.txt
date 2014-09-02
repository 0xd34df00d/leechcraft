cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_dik)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	dik.cpp
	mobiparser.cpp
	decompressor.cpp
	util.cpp
	document.cpp
	)

add_library (leechcraft_monocle_dik SHARED
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_dik
	${LEECHCRAFT_LIBRARIES}
	leechcraft_monocle_util
	)
install (TARGETS leechcraft_monocle_dik DESTINATION ${LC_PLUGINS_DEST})
#if (UNIX AND NOT APPLE)
#	install (FILES freedesktop/leechcraft-monocle-dik.desktop DESTINATION share/applications)
#endif ()

FindQtLibs (leechcraft_monocle_dik Core)
